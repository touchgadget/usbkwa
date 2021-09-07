/*
 * ESP32 Web server with Web Socket for touchscreen keyboard
 *
 * Web app displays a 17 x 6 grid on browser screen. The location of touch or
 * mouse events are sent via web sockets back to the ESP32 web server. The
 * server translates the location to USB HID keycodes and sends them out the
 * USB HID Keyboard interface.
 *
 */

/*
 * MIT License
 *
 * Copyright (c) 2021 touchgadgetdev@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <USB.h>
#include <USBHIDKeyboard.h>
#include <WiFi.h>
#include <WiFiManager.h>      // See README.md
#include <WebSocketsServer.h> // Install WebSockets by Markus Sattler from IDE Library manager
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>  // Install from IDE Library manager

#define DEBUG_ON  0
#if DEBUG_ON
#define DBG_begin(...)    Serial.begin(__VA_ARGS__)
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#define DBG_printf(...)   Serial.printf(__VA_ARGS__)
#else
#define DBG_begin(...)
#define DBG_print(...)
#define DBG_println(...)
#define DBG_printf(...)
#endif

USBHIDKeyboard Keyboard;

MDNSResponder mdns;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#include "index_html.h"

const int MAX_ROWS = 6;
const int MAX_COLS = 17;
const uint8_t Keycodes[MAX_ROWS][MAX_COLS] = {
  // Row 0 (top row)
  {KEY_TAB, KEY_F1 , KEY_F2 , KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12},
  // Row 1
  {'`',     '1',    '2',     '3',    '4',    '5',    '6',    '7',    '8',    '9',    '0',     '-',     '+',     '\b', KEY_INSERT, KEY_HOME, KEY_PAGE_UP},
  // Row 2
  {KEY_TAB, 'q',    'w',     'e',    'r',    't',    'y',    'u',    'i',    'o',    'p',     '[',     ']',     '\\', KEY_DELETE, KEY_END,  KEY_PAGE_DOWN},
  // Row 3
  {KEY_CAPS_LOCK, 'a', 's',  'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',     '\'',     KEY_RETURN},
  // Row 4
  {KEY_LEFT_SHIFT,'z', 'x',  'c',    'v',    'b',    'n',    'm',    ',',    '.',    '/',     KEY_RIGHT_SHIFT, 0, KEY_UP_ARROW},
  // Row 5
  {KEY_LEFT_CTRL, KEY_LEFT_GUI , KEY_LEFT_ALT, ' ', KEY_RIGHT_ALT, KEY_RIGHT_GUI, 0, KEY_RIGHT_CTRL, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW},
};

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  DBG_printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      DBG_printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      {
        DBG_printf("[%u] get Text: [%d] %s \r\n", num, length, payload);

        StaticJsonDocument<96> doc;
        DeserializationError error = deserializeJson(doc, payload);;

        if (error) {
          DBG_print(F("deserializeJson() failed: "));
          DBG_println(error.f_str());
          return;
        }
        const char* event = doc["event"];
        int row = doc["row"];
        if (row < 0) {
          DBG_printf("row negative %d\n", row);
          row = 0;
        }
        if (row >= MAX_ROWS) {
          DBG_printf("row too high %d\n", row);
          row = MAX_ROWS - 1;
        }

        int col = doc["col"];
        if (col < 0) {
          DBG_printf("col negative %d\n", col);
          col = 0;
        }
        if (col >= MAX_COLS) {
          DBG_printf("col too high %d\n", col);
          col = MAX_COLS - 1;
        }
        if (strcmp(event, "touch start") == 0) {
          Keyboard.press(Keycodes[row][col]);
        }
        else if (strcmp(event, "touch end") == 0) {
          Keyboard.release(Keycodes[row][col]);
        }
      }
      break;
    case WStype_BIN:
      DBG_printf("[%u] get binary length: %u\r\n", num, length);
      //      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      DBG_printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  DBG_begin(115200);

  USB.usbClass(0);
  USB.usbSubClass(0);
  USB.usbProtocol(0);
  Keyboard.begin();
  USB.begin();

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("usbkeyboard");
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      DBG_println(F("Failed to connect"));
      delay(1000);
      ESP.restart();
  }

  if (mdns.begin("usbkeyboard")) {
    DBG_println(F("MDNS responder started"));
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    DBG_println(F("MDNS.begin failed"));
  }
  DBG_print(F("Connect to http://usbkeyboard.local or http://"));
  DBG_println(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}
