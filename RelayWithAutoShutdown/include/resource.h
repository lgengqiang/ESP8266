#include <Arduino.h>

const String SOFTAP_SSID_NAME = "RelayCFG";

const String RELAY_DEFAULT_NAME = "Relay";
const String NOT_AVAILABLE = "N/A";

const String STATUS_STA_STATUS = "{{sta_status}}";
const String STATUS_SSID_NAME = "{{ssid_name}}";
const String STATUS_STA_IP_ADDRESS = "{{sta_ip_address}}";
const String STATUS_AP_SSID = "{{ap_ssid}}";
const String STATUS_AP_IP_ADDRESS = "{{ap_ip_address}}";

const String RELAY_NAME = "{{relay_display_name}}";
const String RELAY_STATE = "{{relay_state}}";
const String RELAY_SHOW_ON = "{{show_on_button}}";
const String RELAY_SHOW_OFF = "{{show_off_button}}";
const String RELAY_DISPLAY_NONE = "display: none;";

const String RELAY_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <meta charset=\"utf-8\" />\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\
        <title>ESP8266 Relay</title>\
        <style>\
            .body {\
                text-align: center;\
            }\
            .nav {\
                text-align: left;\
            }\
            .button {\
                border: none;\
                color: white;\
                padding: 16px 40px;\
                text-decoration: none;\
                font-size: 30px;\
                margin: 2px;\
                cursor: pointer;\
            }\
            .button_on {\
                background-color: #198754;\
            }\
            .button_off {\
                background-color: #dc3545;\
            }\
        </style>\
    </head>\
    <body class=\"body\">\
        <nav class=\"nav\">\
            <a href=\"../\">Home</a>\
            &nbsp;|&nbsp;\
            <a href=\"/status\">Status</a>\
            &nbsp;|&nbsp;\
            <a href=\"/config\">Config</a>\
        </nav>\
        <h1>ESP8266无线开关</h1>\
        <p style=\"padding: 10px;\">{{relay_display_name}} : {{relay_state}}</p>\
        <div>\
            <a href=\"/relay_on\" class=\"button button_on\" style=\"{{show_on_button}}\">开启</a>\
            <a href=\"/relay_off\" class=\"button button_off\" style=\"{{show_off_button}}\">关闭</a>\
        </div>\
    </body>\
    </html>";

const String STATUS_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <meta charset=\"utf-8\" />\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\
        <title>Relay Status</title>\
        <style>\
            .nav {\
                text-align: left;\
                color: black;\
            }\
        </style>\
    </head>\
    <body>\
        <nav class=\"nav\">\
            <a href=\"../\">Home</a>\
            &nbsp;|&nbsp;\
            <a href=\"/status\">Status</a>\
            &nbsp;|&nbsp;\
            <a href=\"/config\">Config</a>\
        </nav>\
        <div style=\"max-width: 600px; text-align: left\">\
            <h2>WIFI连接状态</h2>\
            <p>{{sta_status}}</p>\
            <table>\
                <tr>\
                    <td style=\"min-width: 100px;\">\
                        <b>SSID:</b>\
                    </td>\
                    <td>\
                        {{ssid_name}}\
                    </td>\
                </tr>\
                <tr>\
                    <td>\
                        <b>IP Address:</b>\
                    </td>\
                    <td>\
                        {{sta_ip_address}}\
                    </td>\
                </tr>\
            </table>\
            <p>&nbsp;</p>\
            <h2>后台网络状态</h2>\
            <table>\
                <tr>\
                    <td style=\"min-width: 100px;\">\
                        <b>SSID:</b>\
                    </td>\
                    <td>\
                        {{ap_ssid}}\
                    </td>\
                </tr>\
                <tr>\
                    <td>\
                        <b>IP Address:</b>\
                    </td>\
                    <td>\
                        {{ap_ip_address}}\
                    </td>\
                </tr>\
            </table>\
        </div>\
    </body>\
    </html>";

const String CONFIG_PAGE = "<!DOCTYPE html>\
    <html>\
    <head>\
        <meta charset=\"utf-8\" />\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\
        <title>Relay Config</title>\
        <style>\
            .body {\
                text-align: center;\
            }\
            .nav {\
                text-align: left;\
                color: black;\
            }\
            .input_text {\
                width: 100%;\
                margin-top: 5px;\
                margin-bottom: 5px;\
            }\
            .button_submit {\
                font-size: 24px;\
                width: 25%;\
                min-width: 125px;\
                height: 64px;\
                border: none;\
                margin: 20px;\
                padding: 10px;\
                background-color: #0d6efd;\
                color: white;\
            }\
        </style>\
    </head>\
    <body class=\"body\">\
        <nav class=\"nav\">\
            <a href=\"../\">Home</a>\
            &nbsp;|&nbsp;\
            <a href=\"/status\">Status</a>\
            &nbsp;|&nbsp;\
            <a href=\"/config\">Config</a>\
        </nav>\
        <h1>设置</h1>\
        <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"postconfig\">\
            <div style=\"display: inline-block;\">\
                <table style=\"text-align: left;\">\
                    <tr>\
                        <td style=\"min-width: 50px;\">\
                            无线网络:\
                        </td>\
                        <td>\
                            <input type=\"text\" name=\"SSID\" class=\"input_text\" />\
                        </td>\
                    </tr>\
                    <tr>\
                        <td>\
                            网络密码:\
                        </td>\
                        <td>\
                            <input type=\"text\" name=\"Password\" class=\"input_text\" />\
                        </td>\
                    </tr>\
                    <tr>\
                        <td>\
                            开关名称:\
                        </td>\
                        <td>\
                            <input type=\"text\" name=\"RelayDisplayName\" value=\"开关\" class=\"input_text\" />\
                        </td>\
                    </tr>\
                </table>\
                <input type=\"submit\" value=\"Apply\" class=\"button_submit\" />\
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