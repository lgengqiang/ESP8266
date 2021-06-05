#include <Arduino.h>
#include "ESP8266WebServer.h"
#include "ESP8266WiFi.h"
#include "FS.h"
#include "LittleFS.h"
#include "resource.h"
#include "ArduinoJson.h"
#include "WiFiUdp.h"
#include "NTPClient.h"

#define RELAY_STATE_OFF 0
#define RELAY_STATE_ON 1
#define RELAY_STATE_DEFAULT RELAY_STATE_OFF

#define BUTTON_PIN D3
#define LED_STATE_PIN LED_BUILTIN
#define RELAY_PIN D5

unsigned long perviousMillis = 0;

int relayState = RELAY_STATE_DEFAULT;
String ssidName;
String ssidPassword;
String relayDisplayName(RELAY_DEFAULT_NAME);
bool enableTurnOnThreshold = false;
float turnOnThreshold = -1.0f;
bool enableShutdownThreshold = false;
float shutdownThreshold = -1.0f;

// bool enableTurnOnTime = false;
// int turnOnHour = -1;
// int turnOnMinute = -1;
// bool enableShutdownTime = false;
// int shutdownHour = -1;
// int shutdownMinute = -1;

/* -------------------------------------------------- */

bool enableTurnOnTimeRange = false;

int turnOnBeginHour = -1;
int turnOnBeginMinute = -1;
time_t turnOnBeginTime = 0;

int turnOnEndHour = -1;
int turnOnEndMinute = -1;
time_t turnOnEndTime = 0;

bool enableShutdownTimeRange = false;

int shutdownBeginHour = -1;
int shutdownBeginMinute = -1;
time_t shutdownBeginTime = 0;

int shutdownEndHour = -1;
int shutdownEndMinute = -1;
time_t shutdownEndTime = 0;

/* -------------------------------------------------- */

ESP8266WebServer webserver;

IPAddress ipAddress(192, 168, 10, 1);

WiFiEventHandler onSoftAPModeStationConnectedEvent;
WiFiEventHandler onSoftAPModeStationDisconnectedEvent;
WiFiEventHandler onStationModeConnectedEvent;
WiFiEventHandler onStationModeGotIPEvent;
WiFiEventHandler onStationModeDisconnectedEvent;
WiFiEventHandler onStationModeAuthModeChangedEvent;
WiFiEventHandler onStationModeDHCPTimeoutEvent;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "cn.ntp.org.cn");

void serial_init(void);
void button_init(void);
void relay_init(void);
void led_init(void);
void misc_init(void);

void IRAM_ATTR buttonHandler(void);

float getLDRValue(void);

void ledStatusOn(void);
void ledStatusOff(void);

int readRelay(void);
void writeRelay(int state);

bool loadWifiConfig(void);
bool saveWifiConfig(void);

void onPageNotFound(void);
void onStatusPage(void);
void onConfigHomePage(void);
void onConfigApplyPage(void);
void onRelayHomePage(void);
void onRelayOn(void);
void onRelayOff(void);

String buildStatusPageHtml(void);
String buildConfigPageHtml(void);
String buildHomePageHtml(void);
String buildRedirectHtml(void);

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected &event);
void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected &event);

void onStationModeConnected(const WiFiEventStationModeConnected &event);
void onStationModeGotIP(const WiFiEventStationModeGotIP &event);
void onStationModeDisconnected(const WiFiEventStationModeDisconnected &event);
void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged &event);
void onStationModeDHCPTimeout(void);

String getStatusString(void);

void makeTurnOnTime(void);
void makeShutdownTime(void);
bool isInTimeRange(time_t* from, time_t* to, time_t* cur);

void setup()
{
    // put your setup code here, to run once:

    /* Peripherals */
    serial_init();
    led_init();
    relay_init();
    misc_init();
    Serial.println("[Setup] Peripherals have been initialized.");

    /* Load WIFI config */
    if (loadWifiConfig() == true)
    {
        /* WIFI already configured */
        Serial.println("[Setup] Load configuration successfully.");
    }
    else
    {
        /* WIFI does not been configured */
        Serial.println("[Setup] Load configuration failed.");
    }

    /* Web Server */
    webserver.begin(80);
    webserver.on("/", onRelayHomePage);
    webserver.on("/relay_on", onRelayOn);
    webserver.on("/relay_off", onRelayOff);
    webserver.on("/config", onConfigHomePage);
    webserver.on("/postconfig", onConfigApplyPage);
    webserver.on("/status", onStatusPage);
    webserver.onNotFound(onPageNotFound);

    /* WIFI */
    WiFi.mode(WIFI_AP_STA);

    onSoftAPModeStationConnectedEvent = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
    onSoftAPModeStationDisconnectedEvent = WiFi.onSoftAPModeStationDisconnected(&onSoftAPModeStationDisconnected);

    onStationModeConnectedEvent = WiFi.onStationModeConnected(&onStationModeConnected);
    onStationModeGotIPEvent = WiFi.onStationModeGotIP(&onStationModeGotIP);
    onStationModeDisconnectedEvent = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
    onStationModeAuthModeChangedEvent = WiFi.onStationModeAuthModeChanged(&onStationModeAuthModeChanged);
    onStationModeDHCPTimeoutEvent = WiFi.onStationModeDHCPTimeout(&onStationModeDHCPTimeout);

    /* WIFI SoftAP */
    WiFi.softAPConfig(ipAddress, ipAddress, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SOFTAP_SSID_NAME);

    /* WIFI Station */
    if (!ssidName.isEmpty() && !ssidPassword.isEmpty())
    {
        WiFi.begin(ssidName, ssidPassword);
    }

    /* Finished */
    Serial.println("[Setup] Finished.");
    Serial.println();

    perviousMillis = millis();
}

void loop()
{
    // put your main code here, to run repeatedly:
    webserver.handleClient();

    unsigned long currentMillis = millis();
    if ((currentMillis - perviousMillis) > 5000L)
    {
        perviousMillis = currentMillis;

        // LDR value
        float ldr = getLDRValue();
        Serial.printf("[LDR] LDR Value: %.2f\r\n", ldr);

        // NTP Time
        bool ntpUpdated = timeClient.update();
        Serial.printf("[NTP] NTP %s. Time Now: %02d:%02d:%02d\r\n", ntpUpdated ? "True" : "False", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
        tm now = {0};
        now.tm_hour = timeClient.getHours();
        now.tm_min = timeClient.getMinutes();
        time_t timeNow = mktime(&now);

        // Turn On
        if (enableTurnOnThreshold && turnOnThreshold >= 0.0f)
        {
            bool shouldTurnOn = (ldr <= turnOnThreshold);
            if (enableTurnOnTimeRange)
            {
                if (!ntpUpdated)
                {
                    shouldTurnOn = false;
                }

                if (!isInTimeRange(&turnOnBeginTime, &turnOnEndTime, &timeNow))
                {
                    shouldTurnOn = false;
                }
            }

            if (shouldTurnOn && (readRelay() == RELAY_STATE_OFF))
            {
                Serial.printf("Turn On Automatic.\r\n");
                writeRelay(RELAY_STATE_ON);
            }
        }

        // Shutdown
        if (enableShutdownThreshold && shutdownThreshold >= 0.0f)
        {
            bool shouldShutdown = (ldr >= shutdownThreshold);

            if (enableShutdownTimeRange)
            {
                if (!ntpUpdated)
                {
                    shouldShutdown = false;
                }

                if (!isInTimeRange(&shutdownBeginTime, &shutdownEndTime, &timeNow))
                {
                    shouldShutdown = false;
                }
            }

            if (shouldShutdown && (readRelay() == RELAY_STATE_ON))
            {
                Serial.printf("Shutdown Automatic.\r\n");
                writeRelay(RELAY_STATE_OFF);
            }
        }
/*
        // Turn On
        if (enableTurnOnThreshold && turnOnThreshold > 0.0f)
        {
            bool shouldTurnOn = (ldr <= turnOnThreshold);
            if (enableTurnOnTime)
            {
                tm tm = {0};
                tm.tm_hour = turnOnHour;
                tm.tm_min = turnOnMinute;
                time_t time = mktime(&tm);

                if (timeNow < time)
                {
                    shouldTurnOn = false;
                }
            }

            if (shouldTurnOn && (readRelay() == RELAY_STATE_OFF))
            {
                Serial.printf("Automatic Turn On\r\n");
                writeRelay(RELAY_STATE_ON);
            }
        }

        // Shutdown
        if (enableShutdownThreshold && shutdownThreshold > 0.0f)
        {
            bool shouldShutdown = (ldr >= shutdownThreshold);

            if (enableShutdownTime)
            {
                tm tm = {0};
                tm.tm_hour = shutdownHour;
                tm.tm_min = shutdownMinute;
                time_t time = mktime(&tm);

                if (timeNow < time)
                {
                    shouldShutdown = false;
                }

                if (shouldShutdown && (readRelay() == RELAY_STATE_ON))
                {
                    Serial.printf("Automatic Shutdown\r\n");
                    writeRelay(RELAY_STATE_OFF);
                }
            }
        }
*/
/*
        if (enableTurnOnThreshold)
        {
            if (turnOnThreshold > 0.0f)
            {
                if (ldr <= turnOnThreshold)
                {
                    Serial.printf("Turn On by LDR.\r\n    LDR Value: %.2f;  Threshold: %.2f.\r\n", ldr, turnOnThreshold);
                    writeRelay(RELAY_STATE_ON);
                }
            }
        }

        if (enableShutdownThreshold)
        {
            if (shutdownThreshold > 0.0f)
            {
                if (ldr >= shutdownThreshold)
                {
                    Serial.printf("Shutdown by LDR.\r\n    LDR Value: %.2f;  Threshold: %.2f.\r\n", ldr, shutdownThreshold);
                    writeRelay(RELAY_STATE_OFF);
                }
            }
        }

        if (enableTurnOnTime || enableShutdownTime)
        {
            bool updateSuccessfully = timeClient.update();

            if (updateSuccessfully)
            {
                Serial.printf("[Debug] NTP Time: %02d:%02d:%02d\r\n", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());

                tm now = {0};
                now.tm_hour = timeClient.getHours();
                now.tm_min = timeClient.getMinutes();
                time_t timeNow = mktime(&now);

                if (enableTurnOnTime)
                {
                    tm tm = {0};
                    tm.tm_hour = turnOnHour;
                    tm.tm_min = turnOnMinute;
                    time_t time = mktime(&tm);

                    if (timeNow >= time)
                    {
                        Serial.println("Turn On by Time");
                        writeRelay(RELAY_STATE_ON);
                    }
                }

                if (enableShutdownTime)
                {
                    tm tm = {0};
                    tm.tm_hour = shutdownHour;
                    tm.tm_min = shutdownMinute;
                    time_t time = mktime(&tm);

                    if (timeNow >= time)
                    {
                        Serial.println("Shutdown by Time");
                        writeRelay(RELAY_STATE_OFF);
                    }
                }
            }
        }
*/
    }
}

void serial_init(void)
{
    Serial.begin(9600);
}

void button_init(void)
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, buttonHandler, FALLING);
}

void relay_init(void)
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}

void led_init(void)
{
    pinMode(LED_STATE_PIN, OUTPUT);
    digitalWrite(LED_STATE_PIN, LOW);
}

void misc_init(void)
{
    // LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS.");
    }

    // NTP
    timeClient.begin();
    timeClient.setTimeOffset(28800); // 28800: UTC+8
}

void IRAM_ATTR buttonHandler(void)
{
    Serial.println("Button pressed. Reset configuration.");
    if (LittleFS.exists("/config.json"))
    {
        LittleFS.remove("/config.json");
        ESP.restart();
    }
}

float getLDRValue(void)
{
    int adcValue = analogRead(A0);
    return 10.0f * (1024.0f - adcValue) / adcValue;
}

void ledStatusOn(void)
{
    digitalWrite(LED_STATE_PIN, LOW);
}

void ledStatusOff(void)
{
    digitalWrite(LED_STATE_PIN, HIGH);
}

int readRelay(void)
{
    return relayState;
}

void writeRelay(int state)
{
    relayState = state;
    digitalWrite(RELAY_PIN, (relayState == RELAY_STATE_OFF) ? LOW : HIGH);
}

bool loadWifiConfig(void)
{
    File file = LittleFS.open("/config.json", "r");
    if (!file)
    {
        Serial.println("Failed to open config.json.");
        return false;
    }

    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, file);

    if (error)
    {
        Serial.printf("Failed to deserialize Json, error code: %s\r\n", String(error.f_str()).c_str());
        file.close();
        return false;
    }

    ssidName = String(doc["SSID"].as<const char *>());
    ssidPassword = String(doc["Password"].as<const char *>());
    relayDisplayName = String(doc["RelayDisplayName"].as<const char *>());
    enableTurnOnThreshold = doc["EnableTurnOnThreshold"].as<bool>();
    turnOnThreshold = doc["TurnOnThreshold"].as<float>();
    enableShutdownThreshold = doc["EnableShutdownThreshold"].as<bool>();
    shutdownThreshold = doc["ShutdownThreshold"].as<float>();
    // enableTurnOnTime = doc["EnableTurnOnTime"].as<bool>();
    // turnOnHour = doc["TurnOnHour"].as<int>();
    // turnOnMinute = doc["TurnOnMinute"].as<int>();
    // enableShutdownTime = doc["EnableShutdownTime"].as<bool>();
    // shutdownHour = doc["ShutdownHour"].as<int>();
    // shutdownMinute = doc["ShutdownMinute"].as<int>();

    enableTurnOnTimeRange = doc["EnableTurnOnTimeRange"].as<bool>();
    turnOnBeginHour = doc["TurnOnBeginHour"].as<int>();
    turnOnBeginMinute = doc["TurnOnBeginMinute"].as<int>();
    turnOnEndHour = doc["TurnOnEndHour"].as<int>();
    turnOnEndMinute = doc["TurnOnEndMinute"].as<int>();
    makeTurnOnTime();

    enableShutdownTimeRange = doc["EnableShutdownTimeRange"].as<bool>();
    shutdownBeginHour = doc["ShutdownBeginHour"].as<int>();
    shutdownBeginMinute = doc["ShutdownBeginMinute"].as<int>();
    shutdownEndHour = doc["ShutdownEndHour"].as<int>();
    shutdownEndMinute = doc["ShutdownEndMinute"].as<int>();
    makeShutdownTime();

    Serial.println("Load Configuration:");
    Serial.printf("    SSID: %s\r\n", ssidName.c_str());
    Serial.printf("    Password: %s\r\n", ssidPassword.c_str());
    Serial.printf("    RelayDisplayName: %s\r\n", relayDisplayName.c_str());
    Serial.printf("    EnableTurnOnThreshold: %s\r\n", enableTurnOnThreshold ? "True" : "False");
    Serial.printf("    TurnOnThreshold: %.2f\r\n", turnOnThreshold);
    Serial.printf("    EnableShutdownThreshold: %s\r\n", enableShutdownThreshold ? "True" : "False");
    Serial.printf("    ShutdownThreshold: %.2f\r\n", shutdownThreshold);
    // Serial.printf("    EnableTurnOnTime: %s\r\n", enableTurnOnTime ? "True" : "False");
    // Serial.printf("    TurnOnTime: %02d:%02d\r\n", turnOnHour, turnOnMinute);
    // Serial.printf("    EnableShutdownTime: %s\r\n", enableShutdownTime ? "True" : "False");
    // Serial.printf("    ShutdownTime: %02d:%02d\r\n", shutdownHour, shutdownMinute);

    Serial.printf("    EnableTurnOnTimeRange: %s\r\n", enableTurnOnTimeRange ? "True" : "False");
    Serial.printf("    Turn On Begin at %02d:%02d\r\n", turnOnBeginHour, turnOnBeginMinute);
    Serial.printf("    Turn On End at %02d:%02d\r\n", turnOnEndHour, turnOnEndMinute);

    Serial.printf("    EnableShutdownTimeRange: %s\r\n", enableShutdownTimeRange ? "True" : "False");
    Serial.printf("    Shutdown Begin at %02d:%02d\r\n", shutdownBeginHour, shutdownBeginMinute);
    Serial.printf("    Shutdown End at %02d:%02d\r\n", shutdownEndHour, shutdownEndMinute);

    file.close();
    return true;
}

bool saveWifiConfig(void)
{
    File file = LittleFS.open("/config.json", "w");
    if (!file)
    {
        Serial.println("[SaveConfig] Failed to write configuration.");
        return false;
    }

    StaticJsonDocument<768> doc;
    doc["SSID"] = ssidName;
    doc["Password"] = ssidPassword;
    doc["RelayDisplayName"] = relayDisplayName;
    doc["EnableTurnOnThreshold"] = enableTurnOnThreshold;
    doc["TurnOnThreshold"] = turnOnThreshold;
    doc["EnableShutdownThreshold"] = enableShutdownThreshold;
    doc["ShutdownThreshold"] = shutdownThreshold;
    // doc["EnableTurnOnTime"] = enableTurnOnTime;
    // doc["TurnOnHour"] = turnOnHour;
    // doc["TurnOnMinute"] = turnOnMinute;
    // doc["EnableShutdownTime"] = enableShutdownTime;
    // doc["ShutdownHour"] = shutdownHour;
    // doc["ShutdownMinute"] = shutdownMinute;

    doc["EnableTurnOnTimeRange"] = enableTurnOnTimeRange;
    doc["TurnOnBeginHour"] = turnOnBeginHour;
    doc["TurnOnBeginMinute"] = turnOnBeginMinute;
    doc["TurnOnEndHour"] = turnOnEndHour;
    doc["TurnOnEndMinute"] = turnOnEndMinute;

    doc["EnableShutdownTimeRange"] = enableShutdownTimeRange;
    doc["ShutdownBeginHour"] = shutdownBeginHour;
    doc["ShutdownBeginMinute"] = shutdownBeginMinute;
    doc["ShutdownEndHour"] = shutdownEndHour;
    doc["ShutdownEndMinute"] = shutdownEndMinute;

    size_t result = serializeJson(doc, file);
    file.close();
    return result == 0 ? true : false;
}

void onPageNotFound(void)
{
    Serial.print("[WebServer] Page Not Found: ");
    Serial.println(webserver.uri());
}

void onStatusPage(void)
{
    Serial.println("[WebServer] Opening 'Status' page.");
    webserver.send(200, "text/html", buildStatusPageHtml());
}

void onConfigHomePage(void)
{
    Serial.println("[WebServer] Opening 'Config' page.");
    webserver.send(200, "text/html", buildConfigPageHtml());
}

void onConfigApplyPage(void)
{
    Serial.println("[WebServer] Received New Configuration.");
    if (webserver.method() != HTTP_POST)
    {
        Serial.println("[WebServer] Only allow POST method.");
        webserver.send(404, "text/plain", "Method not allow");
        return;
    }

    ssidName = webserver.arg("SSID");
    ssidPassword = webserver.arg("Password");
    relayDisplayName = webserver.arg("RelayDisplayName");

    enableTurnOnThreshold = webserver.arg("EnableTurnOnThreshold") == "on" ? true : false;
    turnOnThreshold = webserver.arg("TurnOnThreshold").toFloat();
    enableShutdownThreshold = webserver.arg("EnableShutdownThreshold") == "on" ? true : false;
    shutdownThreshold = webserver.arg("ShutdownThreshold").toFloat();

    // if (webserver.arg("TurnOnTime").length() == 5)
    // {
    //     enableTurnOnTime = webserver.arg("EnableTurnOnTime") == "on" ? true : false;
    //     turnOnHour = webserver.arg("TurnOnTime").substring(0, 2).toInt();
    //     turnOnMinute = webserver.arg("TurnOnTime").substring(3, 5).toInt();
    // }
    // else
    // {
    //     enableTurnOnTime = false;
    //     turnOnHour = -1;
    //     turnOnMinute = -1;
    // }

    // if (webserver.arg("ShutdownTime").length() == 5)
    // {
    //     enableShutdownTime = webserver.arg("EnableShutdownTime") == "on" ? true : false;
    //     shutdownHour = webserver.arg("ShutdownTime").substring(0, 2).toInt();
    //     shutdownMinute = webserver.arg("ShutdownTime").substring(3, 5).toInt();
    // }
    // else
    // {
    //     enableShutdownTime = false;
    //     shutdownHour = -1;
    //     shutdownMinute = -1;
    // }

    /*
     * EnableTurnOnTimeRange
     * TurnOnBeginTime
     * TurnOnEndTime
     */
    if (webserver.arg("TurnOnBeginTime").length() == 5 && webserver.arg("TurnOnEndTime").length() == 5 )
    {
        enableTurnOnTimeRange = webserver.arg("EnableTurnOnTimeRange") == "on" ? true : false;
        turnOnBeginHour = webserver.arg("TurnOnBeginTime").substring(0, 2).toInt();
        turnOnBeginMinute = webserver.arg("TurnOnBeginTime").substring(3, 5).toInt();
        turnOnEndHour = webserver.arg("TurnOnEndTime").substring(0, 2).toInt();
        turnOnEndMinute = webserver.arg("TurnOnEndTime").substring(3, 5).toInt();
        makeTurnOnTime();
    }
    else
    {
        enableTurnOnTimeRange = false;
        turnOnBeginHour = -1;
        turnOnBeginMinute = -1;
        turnOnEndHour = -1;
        turnOnEndMinute = -1;
        turnOnBeginTime = 0;
        turnOnEndTime = 0;
    }

    /*
     * EnableShutdownTimeRange
     * ShutdownBeginTime
     * ShutdownEndTime
     */
    if (webserver.arg("ShutdownBeginTime").length() == 5 && webserver.arg("ShutdownEndTime").length() == 5 )
    {
        enableShutdownTimeRange = webserver.arg("EnableShutdownTimeRange") == "on" ? true : false;
        shutdownBeginHour = webserver.arg("ShutdownBeginTime").substring(0, 2).toInt();
        shutdownBeginMinute = webserver.arg("ShutdownBeginTime").substring(3, 5).toInt();
        shutdownEndHour = webserver.arg("ShutdownEndTime").substring(0, 2).toInt();
        shutdownEndMinute = webserver.arg("ShutdownEndTime").substring(3, 5).toInt();
        makeShutdownTime();
    }
    else
    {
        enableShutdownTimeRange = false;
        shutdownBeginHour = -1;
        shutdownBeginMinute = -1;
        shutdownEndHour = -1;
        shutdownEndMinute = -1;
        shutdownBeginTime = 0;
        shutdownEndTime = 0;
    }

    Serial.printf("[WebServer] SSID: %s\r\n", ssidName.c_str());
    Serial.printf("[WebServer] Password: %s\r\n", ssidPassword.c_str());

    Serial.printf("[WebServer] Turn On Threshold: %s, %.2f\r\n", enableTurnOnThreshold ? "True" : "False", turnOnThreshold);
    Serial.printf("[WebServer] Shutdown Threshold: %s, %.2f\r\n", enableShutdownThreshold ? "True" : "False", shutdownThreshold);

    // Serial.printf("[WebServer] Turn On Time: %s, %02d:%02d\r\n", enableTurnOnTime ? "True" : "False", turnOnHour, turnOnMinute);
    // Serial.printf("[WebServer] Shutdown Time: %s, %02d:%02d\r\n", enableShutdownTime ? "True" : "False", shutdownHour, shutdownMinute);

    Serial.printf("[WebServer] Turn On by Time: %s; From %02d:%02d to %02d:%02d\r\n", enableTurnOnTimeRange ? "True" : "False", turnOnBeginHour, turnOnBeginMinute, turnOnEndHour, turnOnEndMinute);
    Serial.printf("[WebServer] Shutdown by Time: %s; From %02d:%02d to %02d:%02d\r\n", enableShutdownTimeRange ? "True" : "False", shutdownBeginHour, shutdownBeginMinute, shutdownEndHour, shutdownEndMinute);

    webserver.send(200, "text/html", buildRedirectHtml());

    saveWifiConfig();

    WiFi.disconnect(false);
    if (!ssidName.isEmpty() && !ssidPassword.isEmpty())
    {
        WiFi.begin(ssidName, ssidPassword);
    }
}

void onRelayHomePage(void)
{
    Serial.println("[WebServer] Opening 'Home' page.");
    webserver.send(200, "text/html", buildHomePageHtml());
}

void onRelayOn(void)
{
    Serial.println("[WebServer] Relay ON");
    writeRelay(RELAY_STATE_ON);
    webserver.send(200, "text/html", buildRedirectHtml());
}

void onRelayOff(void)
{
    Serial.println("[WebServer] Relay OFF");
    writeRelay(RELAY_STATE_OFF);
    webserver.send(200, "text/html", buildRedirectHtml());
}

String buildHomePageHtml(void)
{
    String str = String(RELAY_PAGE);
    str.replace(RELAY_NAME, relayDisplayName);
    str.replace(RELAY_STATE, (relayState == RELAY_STATE_OFF) ? "关" : "开");
    str.replace(RELAY_SHOW_ON, (relayState == RELAY_STATE_OFF) ? "" : RELAY_DISPLAY_NONE);
    str.replace(RELAY_SHOW_OFF, (relayState == RELAY_STATE_OFF) ? RELAY_DISPLAY_NONE : "");
    return str;
}

String buildStatusPageHtml(void)
{
    String str = String(STATUS_PAGE);
    str.replace(STATUS_STA_STATUS, getStatusString());
    str.replace(STATUS_SSID_NAME, (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : NOT_AVAILABLE);
    str.replace(STATUS_STA_IP_ADDRESS, (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : NOT_AVAILABLE);
    str.replace(STATUS_AP_SSID, SOFTAP_SSID_NAME);
    str.replace(STATUS_AP_IP_ADDRESS, WiFi.softAPIP().toString());
    str.replace(STATUS_LDR_VALUE, String(getLDRValue()));
    return str;
}

String buildConfigPageHtml(void)
{
    return CONFIG_PAGE;
}

String buildRedirectHtml(void)
{
    return REDIRECT_PAGE;
}

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected &event)
{
    Serial.printf("[SoftAP] Device connected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected &event)
{
    Serial.printf("[SoftAP] Device disconnected, MAC: %02X-%02X-%02X-%02X-%02X-%02X\r\n", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5]);
}

void onStationModeConnected(const WiFiEventStationModeConnected &event)
{
    Serial.printf("[WIFI] Connected. SSID: %s\r\n", event.ssid.c_str());
    ledStatusOn();
}

void onStationModeGotIP(const WiFiEventStationModeGotIP &event)
{
    Serial.printf("[WIFI] Got IP: %s\r\n", event.ip.toString().c_str());
    ledStatusOff();
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected &event)
{
    Serial.println("[WIFI] Disconnected.");
    ledStatusOn();
}

void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged &event)
{
    Serial.println("[WIFI] Auth Mode Changed.");
    ledStatusOn();
}

void onStationModeDHCPTimeout(void)
{
    Serial.println("[WIFI] DHCP Timeout.");
    ledStatusOn();
}

String getStatusString(void)
{
    switch (WiFi.status())
    {
    case WL_CONNECTED:
        return "已连接";
    case WL_NO_SSID_AVAIL:
        return "无法找到网络";
    case WL_CONNECT_FAILED:
        return "连接失败";
    case WL_DISCONNECTED:
        return "断开连接";
    case WL_IDLE_STATUS:
        return "空闲";
    default:
        return "未知";
    }
}

void makeTurnOnTime(void)
{
    tm begin = {0};
    begin.tm_hour = turnOnBeginHour;
    begin.tm_min = turnOnBeginMinute;
    turnOnBeginTime = mktime(&begin);

    tm end = {0};
    end.tm_hour = turnOnEndHour;
    end.tm_min = turnOnEndMinute;
    turnOnEndTime = mktime(&end);
}

void makeShutdownTime(void)
{
    tm begin = {0};
    begin.tm_hour = shutdownBeginHour;
    begin.tm_min = shutdownBeginMinute;
    shutdownBeginTime = mktime(&begin);

    tm end = {0};
    end.tm_hour = shutdownEndHour;
    end.tm_min = shutdownEndMinute;
    shutdownEndTime = mktime(&end);
}

bool isInTimeRange(time_t* from, time_t* to, time_t* cur)
{
    if (*from > *to)
    {
        return (*cur >= *from) || (*cur <= *to);
    }
    else if (*from < *to)
    {
        return (*cur >= *from) && (*cur <= *to);
    }
    else
    {
        return *from == *cur;
    }
}