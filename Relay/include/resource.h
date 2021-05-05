#include <Arduino.h>


const String SOFTAP_SSID_NAME = "RelayCFG";

const String DEFAULT_HOST_NAME = "ESP8266 Relay";

const String RELAY_DEFAULT_NAME = "Relay";
const String NOT_AVAILABLE = "N/A";

const String STATUS_CONNECTED = "Connected";
const String STATUS_NOT_CONNECT = "No connect";
const String STATUS_STA_STATUS = "[STA_CONNECTED]";
const String STATUS_SSID_NAME = "[STA_SSID]";
const String STATUS_STA_IP_ADDRESS = "[STA_IP_ADDRESS]";
const String STATUS_AP_SSID = "[AP_SSID]";
const String STATUS_AP_IP_ADDRESS = "[AP_IP_ADDRESS]";

const String RELAY_NAME = "[RELAY_DISPLAY_NAME]";
const String RELAY_STATE = "[RELAY_STATUS]";
const String RELAY_HREF = "[RELAY_HREF]";

const String STATUS_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>ESP8266 Relay Status</title>\
    </head>\
    <body>\
        <h1>Status for ESP8266 Relay</h1>\
        <div>\
            <h2>WIFI(STA) Status</h2>\
            <p>[STA_CONNECTED]</P>\
            <p>SSID: [STA_SSID]</p>\
            <p>IP Address: [STA_IP_ADDRESS]</p>\
        </div>\
        <div>\
            <h2>SoftAP Status</h2>\
            <p>SoftAP SSID: [AP_SSID]</p>\
            <p>SoftAP IP Address: [AP_IP_ADDRESS]</p>\
        </div>\
    </body>\
    </html>";

const String RELAY_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>ESP8266 Relay</title>\
        <style>\
            .h1 {\
                font-size: 32px;\
                text-align: center;\
            }\
            .div1 {\
                background-color: #04AA6D;\
                margin: auto;\
                text-align: center;\
                width: 25%;\
            }\
            .div2 {\
                padding: 15px 0;\
            }\
            .button {\
                color: white;\
                display: inline-block;\
                text-decoration: none;\
            }\
        </style>\
    </head>\
    <body style=\"background-color: GhostWhite;\">\
        <h1 class=\"h1\">WIFI Relay</h1>\
        <div class=\"div1\">\
            <div class=\"div2\">\
                <a class=\"button\" href=\"/[RELAY_HREF]\">\
                    <div>[RELAY_DISPLAY_NAME]</div>\
                    <div>[RELAY_STATUS]</div>\
                </a>\
            </div>\
        </div>\
    </body>\
    </html>";

const String CONFIG_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>ESP8266 Relay Config</title>\
    </head>\
    <body style=\"background-color: LightSkyBlue;\">\
        <h1 style=\"font-size: 32px; text-align: center;\">Configure WIFI</h1>\
        <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"postconfig\">\
            <div style=\"width: 75%; margin: auto;\">\
                <div>SSID:</div>\
                <input type=\"text\" name=\"SSID\" style=\"width: 99%;\">\
                <br><br>\
                <div>Password:</div>\
                <input type=\"text\" name=\"Password\" style=\"width: 99%;\">\
                <br><br>\
                <div>Relay Display Name:</div>\
                <input type=\"text\" name=\"RelayDisplayName\" value=\"Relay\" style=\"width: 99%;\">\
                <br><br>\
                <div style=\"text-align: center;\">\
                    <input type=\"submit\" value=\"Apply\" style=\"font-size: 24px; width: 25%; min-width: 96px; height: 48px;\">\
                </div>\
            </div>\
        </form>\
    </body>\
    </html>";

const String REDIRECT_PAGE = "<!DOCTYPE html>\
    <html>\
    <meta http-equiv=\"refresh\" content=\"0; url=../\">\
    <head>\
        <title>ESP8266 Relay</title>\
    </head>\
    <body>\
    </body>\
    </html>";
