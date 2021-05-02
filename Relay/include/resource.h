#include <Arduino.h>

const String RELAY_DEFAULT_NAME = "Relay";

const String RELAY_NAME = "[RELAY_DISPLAY_NAME]";
const String RELAY_STATE = "[RELAY_STATUS]";
const String RELAY_HREF = "[RELAY_HREF]";

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
