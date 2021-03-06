#include <Arduino.h>

const String DEFAULT_RELAY_A = "Relay A";
const String DEFAULT_RELAY_B = "Relay B";

const String KEYWORD_RELAY_A_DISPLAY = "[(KEYWORD_RELAY_A_DISPLAY)]";
const String KEYWORD_RELAY_A_STATUS = "[(KEYWORD_RELAY_A_STATUS)]";
const String KEYWORD_RELAY_A_HREF = "[(KEYWORD_RELAY_A_HREF)]";
const String KEYWORD_RELAY_B_DISPLAY = "[(KEYWORD_RELAY_B_DISPLAY)]";
const String KEYWORD_RELAY_B_STATUS = "[(KEYWORD_RELAY_B_STATUS)]";
const String KEYWORD_RELAY_B_HREF = "[(KEYWORD_RELAY_B_HREF)]";

const String RELAY_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>ESP8266's 2-Channel Relays</title>\
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
        <h1 class=\"h1\">WIFI Relays</h1>\
        <div class=\"div1\">\
            <div class=\"div2\">\
                <a class=\"button\" href=\"[(KEYWORD_RELAY_A_HREF)]\">\
                    <div>[(KEYWORD_RELAY_A_DISPLAY)]</div>\
                    <div>[(KEYWORD_RELAY_A_STATUS)]</div>\
                </a>\
            </div>\
        </div>\
        <div style=\"height: 25px;\"></div>\
        <div class=\"div1\">\
            <div class=\"div2\">\
                <a class=\"button\" href=\"[(KEYWORD_RELAY_B_HREF)]\">\
                    <div>[(KEYWORD_RELAY_B_DISPLAY)]</div>\
                    <div>[(KEYWORD_RELAY_B_STATUS)]</div>\
                </a>\
            </div>\
        </div>\
    </body>\
    </html>";

const String CONFIG_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <title>ESP8266's 2-Channel Relays Config</title>\
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
                <div>Relay A Display Name:</div>\
                <input type=\"text\" name=\"DisplayNameA\" value=\"Relay A\" style=\"width: 99%;\">\
                <br><br>\
                <div>Relay B Display Name:</div>\
                <input type=\"text\" name=\"DisplayNameB\" value=\"Relay B\" style=\"width: 99%;\">\
                <br><br>\
                <div style=\"text-align: center;\">\
                    <input type=\"submit\" value=\"Apply\"\
                        style=\"font-size: 24px; width: 25%; min-width: 96px; height: 48px;\">\
                </div>\
            </div>\
        </form>\
    </body>\
    </html>";

const String REDIRECT_PAGE = "<!DOCTYPE html>\
    <html>\
    <meta http-equiv=\"refresh\" content=\"0; url=../\">\
    <head>\
        <title>ESP8266's 2-Channel Relays</title>\
    </head>\
    <body>\
    </body>\
    </html>";
