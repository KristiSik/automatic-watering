/*
  This software, the ideas and concepts is Copyright (c) David Bird 2021
  All rights to this software are reserved.
  It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
  5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies or substantial portions of the software
     and where the software use is visible to an end-user.
  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
  FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR
  A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OR
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  See more at http://dsbird.org.uk
*/
//################# LIBRARIES ################
#include <WiFi.h>                      // Built-in
#include <ESPmDNS.h>                   // Built-in
#include <SPIFFS.h>                    // Built-in
#include <ESPAsyncWebSrv.h>         // Built-in
#include "AsyncTCP.h"                  // https://github.com/me-no-dev/AsyncTCP
#include "Secrets.h"

//################  VERSION  ###########################################
String version = "1.0";      // Programme version, see change log at end
//################ VARIABLES ###########################################

const char* ServerName = "Controller"; // Connect to the server with http://controller.local/ e.g. if name = "myserver" use http://myserver.local/

#define Channels        12              // n-Channels
#define NumOfEvents     4              // Number of events per-day, 4 is a practical limit
#define noRefresh       false          // Set auto refresh OFF
#define Refresh         true           // Set auto refresh ON
#define ON              true           // Set the Relay ON
#define OFF             false          // Set the Relay OFF
#define Channel1        0              // Define Channel-1
#define Channel2        1              // Define Channel-2
#define Channel3        2              // Define Channel-3
#define Channel4        3              // Define Channel-4
#define Channel5        4              // Define Channel-5
#define Channel6        5              // Define Channel-6
#define Channel7        6              // Define Channel-7
#define Channel8        7              // Define Channel-8
#define Channel9        8              // Define Channel-9
#define Channel10       9              // Define Channel-10
#define Channel11       10             // Define Channel-11
#define Channel12       11             // Define Channel-12
// Now define the GPIO pins to be used for relay control
#define Channel1_Pin    0              // Define the Relay Control pin
#define Channel2_Pin    2              // Define the Relay Control pin
#define Channel3_Pin    4              // Define the Relay Control pin
#define Channel4_Pin    5              // Define the Relay Control pin
#define Channel5_Pin    18             // Define the Relay Control pin
#define Channel6_Pin    19             // Define the Relay Control pin
#define Channel7_Pin    21             // Define the Relay Control pin
#define Channel8_Pin    22             // Define the Relay Control pin
#define Channel9_Pin    23             // Define the Relay Control pin
#define Channel10_Pin   15             // Define the Relay Control pin
#define Channel11_Pin   13             // Define the Relay Control pin
#define Channel12_Pin   12             // Define the Relay Control pin

#define LEDPIN          5              // Define the LED Control pin
#define ChannelReverse  true          // Set to true for Relay that requires a signal HIGH for ON, usually relays need a LOW to actuate

struct settings {
  String DoW;                          // Day of Week for the programmed event
  String Start[NumOfEvents];           // Start time
  String Stop[NumOfEvents];            // End time
};

String       DataFile = "params.txt";  // Storage file name on flash
String       Time_str, DoW_str;        // For Date and Time
settings     Timer[Channels][7];       // Timer settings, n-Channels each 7-days of the week

//################ VARIABLES ################
const char* Timezone   = "GMT0BST,M3.5.0,M10.5.0/3";
// Example time zones
//const char* Timezone = "MET-1METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
//const char* Timezone = "EST-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* Timezone = "EST5EDT,M3.2.0,M11.1.0";           // EST USA
//const char* Timezone = "CST6CDT,M3.2.0,M11.1.0";           // CST USA
//const char* Timezone = "MST7MDT,M4.1.0,M10.5.0";           // MST USA
//const char* Timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";      // Auckland
//const char* Timezone = "EET-2EEST,M3.5.5/0,M10.5.5/0";     // Asia
//const char* Timezone = "ACST-9:30ACDT,M10.1.0,M4.1.0/3":   // Australia

// System values
String sitetitle            = "Контролер поливу";
String Year                 = "березень 2023";     // For the footer line

bool   ManualOverride       = true;      // Manual override
String Units                = "M";        // or Units = "I" for °F and 12:12pm time format

String webpage              = "";         // General purpose variables to hold HTML code for display
int    TimerCheckDuration   = 1000;       // Check for timer event every 1-second
int    wifi_signal          = 0;          // WiFi signal strength
long   LastTimerSwitchCheck = 0;          // Counter for last timer check
int    UnixTime             = 0;          // Time now (when updated) of the current time
String Channel1_State       = "OFF";      // Status of the channel
String Channel2_State       = "OFF";      // Status of the channel
String Channel3_State       = "OFF";      // Status of the channel
String Channel4_State       = "OFF";      // Status of the channel
String Channel5_State       = "OFF";      // Status of the channel
String Channel6_State       = "OFF";      // Status of the channel
String Channel7_State       = "OFF";      // Status of the channel
String Channel8_State       = "OFF";      // Status of the channel
String Channel9_State       = "OFF";      // Status of the channel
String Channel10_State      = "OFF";      // Status of the channel
String Channel11_State      = "OFF";      // Status of the channel
String Channel12_State      = "OFF";      // Status of the channel
bool   Channel1_Override    = false;
bool   Channel2_Override    = false;
bool   Channel3_Override    = false;
bool   Channel4_Override    = false;
bool   Channel5_Override    = false;
bool   Channel6_Override    = false;
bool   Channel7_Override    = false;
bool   Channel8_Override    = false;
bool   Channel9_Override    = false;
bool   Channel10_Override   = false;
bool   Channel11_Override   = false;
bool   Channel12_Override   = false;

AsyncWebServer server(80); // Server on IP address port 80 (web-browser default, change to your requirements, e.g. 8080

// To access server from outside of a WiFi (LAN) network e.g. on port 8080 add a rule on your Router that forwards a connection request
// to http://your_WAN_address:8080/ to http://your_LAN_address:8080 and then you can view your ESP server from anywhere.
// Example http://yourhome.ip:8080 and your ESP Server is at 192.168.0.40, then the request will be directed to http://192.168.0.40:8080

//#########################################################################################
void setup() {
  SetupSystem();                          // General system setup
  StartWiFi();                            // Start WiFi services
  SetupTime();                            // Start NTP clock services
  StartSPIFFS();                          // Start SPIFFS filing system
  Initialise_Array();                     // Initialise the array for storage and set some values
  RecoverSettings();                      // Recover settings from LittleFS
  SetupDeviceName(ServerName);            // Set logical device name

  // Set handler for '/'
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->redirect("/homepage");       // Go to home page
  });
  // Set handler for '/homepage'
  server.on("/homepage", HTTP_GET, [](AsyncWebServerRequest * request) {
    Homepage();
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer1'
  server.on("/timer1", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel1);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer2'
  server.on("/timer2", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel2);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer3'
  server.on("/timer3", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel3);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer4'
  server.on("/timer4", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel4);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer5'
  server.on("/timer5", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel5);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer6'
  server.on("/timer6", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel6);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer7'
  server.on("/timer7", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel7);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer8'
  server.on("/timer8", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel8);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer9'
  server.on("/timer9", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel9);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer10'
  server.on("/timer10", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel10);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer11'
  server.on("/timer11", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel11);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer12'
  server.on("/timer12", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel12);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/help'
  server.on("/help", HTTP_GET, [](AsyncWebServerRequest * request) {
    Help();
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/handletimer0' inputs
  server.on("/handletimer0", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[0][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[0][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer1' inputs
  server.on("/handletimer1", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[1][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[1][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer2' inputs
  server.on("/handletimer2", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[2][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[2][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer3' inputs
  server.on("/handletimer3", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[3][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[3][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer4' inputs
  server.on("/handletimer4", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[4][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[4][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer5' inputs
  server.on("/handletimer5", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[5][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[5][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer6' inputs
  server.on("/handletimer6", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[6][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[6][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer7' inputs
  server.on("/handletimer7", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[7][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[7][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer8' inputs
  server.on("/handletimer8", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[8][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[8][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer9' inputs
  server.on("/handletimer9", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[9][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[9][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer10' inputs
  server.on("/handletimer10", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[10][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[10][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handletimer11' inputs
  server.on("/handletimer11", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (byte dow = 0; dow < 7; dow++) {
      for (byte p = 0; p < 4; p++) {
        Timer[11][dow].Start[p] = request->arg(String(dow) + "." + String(p) + ".Start");
        Timer[11][dow].Stop[p]  = request->arg(String(dow) + "." + String(p) + ".Stop");
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handlesetup' inputs
  server.on("/handlesetup", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("manualoverride1")) {
      String stringArg = request->arg("manualoverride1");
      if (stringArg == "ON") Channel1_Override = true; else Channel1_Override = false;
    }
    if (request->hasArg("manualoverride2")) {
      String stringArg = request->arg("manualoverride2");
      if (stringArg == "ON") Channel2_Override = true; else Channel2_Override = false;
    }
    if (request->hasArg("manualoverride3")) {
      String stringArg = request->arg("manualoverride3");
      if (stringArg == "ON") Channel3_Override = true; else Channel3_Override = false;
    }
    if (request->hasArg("manualoverride4")) {
      String stringArg = request->arg("manualoverride4");
      if (stringArg == "ON") Channel4_Override = true; else Channel4_Override = false;
    }
    if (request->hasArg("manualoverride5")) {
      String stringArg = request->arg("manualoverride5");
      if (stringArg == "ON") Channel5_Override = true; else Channel5_Override = false;
    }
    if (request->hasArg("manualoverride6")) {
      String stringArg = request->arg("manualoverride6");
      if (stringArg == "ON") Channel6_Override = true; else Channel6_Override = false;
    }
    if (request->hasArg("manualoverride7")) {
      String stringArg = request->arg("manualoverride7");
      if (stringArg == "ON") Channel7_Override = true; else Channel7_Override = false;
    }
    if (request->hasArg("manualoverride8")) {
      String stringArg = request->arg("manualoverride8");
      if (stringArg == "ON") Channel8_Override = true; else Channel8_Override = false;
    }
    if (request->hasArg("manualoverride9")) {
      String stringArg = request->arg("manualoverride9");
      if (stringArg == "ON") Channel9_Override = true; else Channel9_Override = false;
    }
    if (request->hasArg("manualoverride10")) {
      String stringArg = request->arg("manualoverride10");
      if (stringArg == "ON") Channel10_Override = true; else Channel10_Override = false;
    }
    if (request->hasArg("manualoverride11")) {
      String stringArg = request->arg("manualoverride11");
      if (stringArg == "ON") Channel11_Override = true; else Channel11_Override = false;
    }
    if (request->hasArg("manualoverride12")) {
      String stringArg = request->arg("manualoverride12");
      if (stringArg == "ON") Channel12_Override = true; else Channel12_Override = false;
    }
    SaveSettings();
    delay(1000);                                          // Wait for channel to be updated in main thread, so the home page will display correct channel state
    request->redirect("/homepage");                       // Go back to home page
  });
  server.begin();
  LastTimerSwitchCheck = millis() + TimerCheckDuration;   // Preload timer value with update duration
}
//#########################################################################################
void loop() {
  if (millis() - LastTimerSwitchCheck > TimerCheckDuration) {
    LastTimerSwitchCheck = millis();                      // Reset time for next event
    UpdateLocalTime();                                    // Updates Time UnixTime to 'now'
    CheckTimerEvent();                                    // Check for schedule actuated
    Serial.println("Checking timer");
  }
}
//#########################################################################################
void Homepage() {
  bool TimerSummary[4][7][48]; // 7 days each with 48 timer periods
  append_HTML_header(Refresh);
  webpage += "<h2>Статус каналу @ " + Time_str + "</h2><br>";
  webpage += "<table class='centre channels-table'>";
  webpage += "<tr>";
  webpage += " <td>Канал-1<br>Клумба-1</td>";
  webpage += " <td>Канал-2<br>Клумба-2</td>";
  webpage += " <td>Канал-3<br>Клумба-3</td>";
  webpage += " <td>Канал-4<br>Клумба-4</td>";
  webpage += " <td>Канал-5<br>Клумба-5</td>";
  webpage += " <td>Канал-6<br>Клумба-6</td>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += " <td><div class='circle'><a class='" + String((Channel1_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride1=" + String((Channel1_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel1_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel2_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride2=" + String((Channel2_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel2_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel3_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride3=" + String((Channel3_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel3_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel4_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride4=" + String((Channel4_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel4_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel5_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride5=" + String((Channel5_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel5_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel6_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride6=" + String((Channel6_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel6_State) + "</a></div></td>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += " <td>Канал-7<br>Клумба-7</td>";
  webpage += " <td>Канал-8<br>Клумба-8</td>";
  webpage += " <td>Канал-9<br>Клумба-9</td>";
  webpage += " <td>Канал-10<br>Клумба-10</td>";
  webpage += " <td>Канал-11<br>Клумба-11</td>";
  webpage += " <td>Канал-12<br>Клумба-12</td>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += " <td><div class='circle'><a class='" + String((Channel7_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride7=" + String((Channel7_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel7_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel8_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride8=" + String((Channel8_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel8_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel9_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride9=" + String((Channel9_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel9_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel10_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride10=" + String((Channel10_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel10_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel11_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride11=" + String((Channel11_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel11_State) + "</a></div></td>";
  webpage += " <td><div class='circle'><a class='" + String((Channel12_State == "ON" ? "on'" : "off'")) + " href='/handlesetup?manualoverride12=" + String((Channel12_State == "ON" ? "OFF'" : "ON'")) + "'>" + String(Channel12_State) + "</a></div></td>";
  webpage += "</tr>";
  webpage += "</table>";
  webpage += "<br>";
  append_HTML_footer();
}
//#########################################################################################
bool CheckTime(byte channel, String DoW_Str, String TimeNow_Str) {
  for (byte dow = 0; dow < 7; dow++) {                 // Look for any valid timer events, if found turn the heating on
    for (byte p = 0; p < NumOfEvents; p++) {
      // Now check for a scheduled ON time, if so Switch the Timer ON and check the temperature against target temperature
      if (String(dow) == DoW_Str && (TimeNow_Str >= Timer[channel][dow].Start[p] && TimeNow_Str < Timer[channel][dow].Stop[p] && Timer[channel][dow].Start[p] != ""))
        return true;
    }
  }
  return false;
}
//#########################################################################################
void TimerSet(int channel) {
  append_HTML_header(noRefresh);
  webpage += "<h2>Налаштування розкладу Канал-" + String(channel + 1) + "</h2><br>";
  webpage += "<h3>Клумба-" + String(channel + 1) + "</h3><br>";
  webpage += "<FORM action='/handletimer" + String(channel) + "'>";
  webpage += "<table class='centre timer'>";
  webpage += "<col><col><col><col><col><col><col><col>";
  webpage += "<tr><td>Контроль</td>";
  webpage += "<td>" + Timer[channel][0].DoW + "</td>";
  for (byte dow = 1; dow < 6; dow++) { // Heading line showing DoW
    webpage += "<td>" + Timer[channel][dow].DoW + "</td>";
  }
  webpage += "<td>" + Timer[channel][6].DoW + "</td>";
  webpage += "</tr>";
  for (byte p = 0; p < NumOfEvents; p++) {
    webpage += "<tr>";
    webpage += "<td>Старт</td>";
    webpage += "<td><input type='time' name='" + String(0) + "." + String(p) + ".Start' value='"    + Timer[channel][0].Start[p] + "'></td>";
    for (int dow = 1; dow < 6; dow++) {
      webpage += "<td><input type='time' name='" + String(dow) + "." + String(p) + ".Start' value='" + Timer[channel][dow].Start[p] + "'></td>";
    }
    webpage += "<td><input type='time' name='" + String(6) + "." + String(p) + ".Start' value='" + Timer[channel][6].Start[p] + "'></td>";
    webpage += "</tr>";
    webpage += "<tr><td>Стоп</td>";
    webpage += "<td><input type='time' name='" + String(0) + "." + String(p) + ".Stop' value='" + Timer[channel][0].Stop[p] + "'></td>";
    for (int dow = 1; dow < 6; dow++) {
      webpage += "<td><input type='time' name='" + String(dow) + "." + String(p) + ".Stop' value='" + Timer[channel][dow].Stop[p] + "'></td>";
    }
    webpage += "<td><input type='time' name='" + String(6) + "." + String(p) + ".Stop' value='" + Timer[channel][6].Stop[p] + "'></td>";
    webpage += "</tr>";
    if (p < (NumOfEvents - 1)) {
      webpage += "<tr><td></td><td></td>";
      for (int dow = 2; dow < 7; dow++) {
        webpage += "<td>-</td>";
      }
      webpage += "<td></td>";
      webpage += "</tr>";
    }
  }
  webpage += "</table>";
  webpage += "<div class='centre'>";
  webpage += "<br><input type='submit' value='Зберегти'><br><br>";
  webpage += "</div></form>";
  append_HTML_footer();
}
//#########################################################################################
void Help() {
  append_HTML_header(noRefresh);
  webpage += "<h2>Довідка</h2><br>";
  webpage += "<div style='text-align: left;font-size:1.1em;'>";
  webpage += "<br><u><b>Сторінка статусу</b></u>";
  webpage += "<p>Надає огляд каналів, що показує увімкнено або вимкнено для кожного каналу, ЗЕЛЕНИЙ - ON та ЧЕРВОНИЙ - OFF.</p>";
  webpage += "<p>Кожна позиція має посилання на налаштування таймера каналу, тому, натиснувши Channel-2 'ON' або 'OFF' ви перейдете на сторінку налаштувань таймера каналу.</p>";
  webpage += "<p>Щотижневий підсумок показує 4 канали на день, з 00:00 до 23:30, використовуйте колір каналу, щоб вказати, чи заплановано його ввімкнення.</p>";
  webpage += "<p>ПРИМІТКА: мінімальний період часу, який можна відобразити, становить 30 хвилин. наприклад, з 12:05 до 12:20 не відображатиметься, але з 12:00 до 12:20 буде.</p>";
  webpage += "<p>Це тому, що мінімальна роздільна здатність налаштувань становить 30 хвилин, що не впливає на точність синхронізації.</p>";
  webpage += "<br><u><b>Налаштування каналів від 1 до 4</b></u>";
  webpage += "<p>Кожен канал можна налаштувати на 7 днів на тиждень із максимум 4 періодами на день.</p>";
  webpage += "<p>Ви можете використовувати один слот, щоб увімкнути канал, наприклад, між 09:00 і 13:00</p>";
  webpage += "<p>або ви можете встановити період-1 з 09:00 до 10:00; період-2 з 10:00 до 11:00; період-3 з 11:00 до 12:00 та період-4 з 12:00 до 13:00</p>";
  webpage += "<u><b>Сторінка налаштування</b></u>";
  webpage += "<p>Дозволяє ручне перемикання каналів на ON, але лише доки не набуде чинності попередній період розкладу.</p>";
  webpage += "<p>Перемикання вручну скасовується, коли починається запрограмований попередній період таймера.</p>";
  webpage += "</div>";
  append_HTML_footer();
}
//#########################################################################################
void CheckTimerEvent() {
  String TimeNow;
  TimeNow        = ConvertUnixTime(UnixTime);           // Get the current time e.g. 15:35
  Channel1_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel2_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel3_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel4_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel5_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel6_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel7_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel8_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel9_State = "OFF";                               // Switch Channel OFF until the schedule decides otherwise
  Channel10_State = "OFF";                              // Switch Channel OFF until the schedule decides otherwise
  Channel11_State = "OFF";                              // Switch Channel OFF until the schedule decides otherwise
  Channel12_State = "OFF";                              // Switch Channel OFF until the schedule decides otherwise
  if (Channel1_Override) {                              // If manual override is requested then turn the Channel on
    Channel1_State = "ON";
    ActuateChannel(ON, Channel1, Channel1_Pin);         // Switch Channel ON if requested
  }
  if (Channel2_Override) {
    Channel2_State = "ON";
    ActuateChannel(ON, Channel2, Channel2_Pin);         // Switch Channel ON if requested
  }
  if (Channel3_Override) {
    Channel3_State = "ON";
    ActuateChannel(ON, Channel3, Channel3_Pin);         // Switch Channel ON if requested
  }
  if (Channel4_Override) {
    Channel4_State = "ON";
    ActuateChannel(ON, Channel4, Channel4_Pin);         // Switch Channel ON if requested
  }
  if (Channel5_Override) {
    Channel5_State = "ON";
    ActuateChannel(ON, Channel5, Channel5_Pin);         // Switch Channel ON if requested
  }
  if (Channel6_Override) {
    Channel6_State = "ON";
    ActuateChannel(ON, Channel6, Channel6_Pin);         // Switch Channel ON if requested
  }
  if (Channel7_Override) {
    Channel7_State = "ON";
    ActuateChannel(ON, Channel7, Channel7_Pin);         // Switch Channel ON if requested
  }
  if (Channel8_Override) {
    Channel8_State = "ON";
    ActuateChannel(ON, Channel8, Channel8_Pin);         // Switch Channel ON if requested
  }
  if (Channel9_Override) {
    Channel9_State = "ON";
    ActuateChannel(ON, Channel9, Channel9_Pin);         // Switch Channel ON if requested
  }
  if (Channel10_Override) {
    Channel10_State = "ON";
    ActuateChannel(ON, Channel10, Channel10_Pin);       // Switch Channel ON if requested
  }
  if (Channel11_Override) {
    Channel11_State = "ON";
    ActuateChannel(ON, Channel11, Channel11_Pin);       // Switch Channel ON if requested
  }
  if (Channel12_Override) {
    Channel12_State = "ON";
    ActuateChannel(ON, Channel12, Channel12_Pin);       // Switch Channel ON if requested
  }
  for (byte channel = 0; channel < Channels; channel++) {
    for (byte dow = 0; dow < 7; dow++) {                // Look for any valid timer events, if found turn the heating on
      for (byte p = 0; p < NumOfEvents; p++) {
        // Now check for a scheduled ON time, if so Switch the Timer ON and check the temperature against target temperature
        if (String(dow) == DoW_str && (TimeNow >= Timer[channel][dow].Start[p] && TimeNow < Timer[channel][dow].Stop[p] && Timer[channel][dow].Start[p] != ""))
        {
          if (channel == 0) Channel1_State = "ON";
          if (channel == 1) Channel2_State = "ON";
          if (channel == 2) Channel3_State = "ON";
          if (channel == 3) Channel4_State = "ON";
          if (channel == 4) Channel5_State = "ON";
          if (channel == 5) Channel6_State = "ON";
          if (channel == 6) Channel7_State = "ON";
          if (channel == 7) Channel8_State = "ON";
          if (channel == 8) Channel9_State = "ON";
          if (channel == 9) Channel10_State = "ON";
          if (channel == 10) Channel11_State = "ON";
          if (channel == 11) Channel12_State = "ON";
        }
      }
    }
  }
  if (Channel1_State == "ON") ActuateChannel(ON, Channel1, Channel1_Pin); else ActuateChannel(OFF, Channel1, Channel1_Pin);
  if (Channel2_State == "ON") ActuateChannel(ON, Channel2, Channel2_Pin); else ActuateChannel(OFF, Channel2, Channel2_Pin);
  if (Channel3_State == "ON") ActuateChannel(ON, Channel3, Channel3_Pin); else ActuateChannel(OFF, Channel3, Channel3_Pin);
  if (Channel4_State == "ON") ActuateChannel(ON, Channel4, Channel4_Pin); else ActuateChannel(OFF, Channel4, Channel4_Pin);
  if (Channel5_State == "ON") ActuateChannel(ON, Channel5, Channel5_Pin); else ActuateChannel(OFF, Channel5, Channel5_Pin);
  if (Channel6_State == "ON") ActuateChannel(ON, Channel6, Channel6_Pin); else ActuateChannel(OFF, Channel6, Channel6_Pin);
  if (Channel7_State == "ON") ActuateChannel(ON, Channel7, Channel7_Pin); else ActuateChannel(OFF, Channel7, Channel7_Pin);
  if (Channel8_State == "ON") ActuateChannel(ON, Channel8, Channel8_Pin); else ActuateChannel(OFF, Channel8, Channel8_Pin);
  if (Channel9_State == "ON") ActuateChannel(ON, Channel9, Channel9_Pin); else ActuateChannel(OFF, Channel9, Channel9_Pin);
  if (Channel10_State == "ON") ActuateChannel(ON, Channel10, Channel10_Pin); else ActuateChannel(OFF, Channel10, Channel10_Pin);
  if (Channel11_State == "ON") ActuateChannel(ON, Channel11, Channel11_Pin); else ActuateChannel(OFF, Channel11, Channel11_Pin);
  if (Channel12_State == "ON") ActuateChannel(ON, Channel12, Channel12_Pin); else ActuateChannel(OFF, Channel12, Channel12_Pin);
}
//#########################################################################################
void ActuateChannel(bool demand, byte channel, byte channel_pin) {
  if (demand) {
    if (ChannelReverse) digitalWrite(channel_pin, HIGH); else digitalWrite(channel_pin, LOW);
    Serial.println("Putting Channel-" + String(channel) + " ON");
  }
  else
  {
    if (ChannelReverse) digitalWrite(channel_pin, LOW); else digitalWrite(channel_pin, HIGH);
    Serial.println("Putting Channel-" + String(channel) + " OFF");
  }
  if (Channel1_State == "ON" ||        // Switch ON LED if any channel is ON otherwise switch ot OFF
      Channel2_State == "ON" ||
      Channel3_State == "ON" ||
      Channel4_State == "ON" ||
      Channel5_State == "ON" ||
      Channel6_State == "ON" ||
      Channel7_State == "ON" ||
      Channel8_State == "ON" ||
      Channel9_State == "ON" ||
      Channel10_State == "ON" ||
      Channel11_State == "ON" ||
      Channel12_State == "ON") digitalWrite(LEDPIN, LOW); else digitalWrite(LEDPIN, HIGH);
}
//#########################################################################################
void append_HTML_header(bool refreshMode) {
  webpage  = "<!DOCTYPE html><html lang='en'>";
  webpage += "<head>";
  webpage += "<title>" + sitetitle + "</title>";
  webpage += "<meta charset='UTF-8'>";
  if (refreshMode) webpage += "<meta http-equiv='refresh' content='5'>"; // 5-secs refresh time, test needed to prevent auto updates repeating some commands
  webpage += "<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>";
  webpage += "<style>";
  webpage += "body {width:68em;margin-left:auto;margin-right:auto;font-family:Arial,Helvetica,sans-serif;font-size:14px;color:blue;background-color:#e1e1ff;text-align:center;}";
  webpage += ".centre {margin-left:auto;margin-right:auto;}";
  webpage += "h2 {margin-top:0.3em;margin-bottom:0.3em;font-size:1.4em;}";
  webpage += "h3 {margin-top:0.3em;margin-bottom:0.3em;font-size:1.2em;}";
  webpage += ".on {color:limegreen;text-decoration:none;}";
  webpage += ".off {color:red;text-decoration:none;}";
  webpage += ".topnav {overflow: hidden;background-color:lightcyan;}";
  webpage += ".topnav a {float:left;color:blue;text-align:center;padding:1em 1.14em;text-decoration:none;font-size:1.3em;}";
  webpage += ".topnav a:hover {background-color:deepskyblue;color:white;}";
  webpage += ".topnav a.active {background-color:lightblue;color:blue;}";
  webpage += "table.timer tr {padding:0.2em 0.5em 0.2em 0.5em;font-size:1.1em;}";
  webpage += "table.timer td {padding:0.2em 0.5em 0.2em 0.5em;font-size:1.1em;}";
  webpage += "table.sum tr {padding:0.2em 0.5em 0.2em 0.5em;font-size:1.1em;border:1px solid blue;}";
  webpage += "table.sum td {padding:0.2em 0.6em 0.2em 0.6em;font-size:1.1em;border:1px solid blue;}";
  webpage += "col:first-child {background:lightcyan}col:nth-child(2){background:#CCC}col:nth-child(8){background:#CCC}";
  webpage += "tr:first-child, .channels-table tr:nth-child(odd) {background:lightcyan}";
  webpage += ".medium {font-size:1.4em;padding:0;margin:0}";
  webpage += ".ps {font-size:0.7em;padding:0;margin:0}";
  webpage += "footer {padding:0.08em;background-color:cyan;font-size:1.1em;}";
  webpage += ".circle {border:0.15em solid orange;background-color:lightgray;border-radius:50%;width:2.7em;height:2.7em;padding:0.2em;text-align:center;font-size:3em;display:inline-flex;justify-content:center;align-items:center;}";
  webpage += ".coff {background-color:gainsboro;}";
  webpage += ".c1on {background-color:orange;}";
  webpage += ".c2on {background-color:orange;}";
  webpage += ".c3on {background-color:orange;}";
  webpage += ".c4on {background-color:orange;}";
  webpage += ".wifi {padding:3px;position:relative;top:1em;left:0.36em;}";
  webpage += ".wifi-info {float: right; margin-right: 20px;margin-top: 5px;}";
  webpage += ".wifi, .wifi:before {display:inline-block;border:9px double transparent;border-top-color:currentColor;border-radius:50%;}";
  webpage += ".wifi:before {content:'';width:0;height:0;}";
  webpage += "</style></head>";
  webpage += "<body>";
  webpage += "<div class='topnav'>";
  webpage += "<a href='/'>Статус</a>";
  webpage += "<a href='help'>Довідка</a>";
  webpage += "<div class='wifi-info'><div class='wifi'></div><span>" + WiFiSignal() + "</span></div>";
  webpage += "</div><br>";
}
//#########################################################################################
void append_HTML_footer() {
  webpage += "<footer>";
  webpage += "<p class='medium'>Контролер поливу</p>";
  webpage += "<p class='ps'><i>Copyright &copy;&nbsp;D L Bird " + String(Year) + " V" + version + "</i></p>";
  webpage += "</footer>";
  webpage += "</body></html>";
}
//#########################################################################################
void SetupDeviceName(const char *DeviceName) {
  if (MDNS.begin(DeviceName)) { // The name that will identify your device on the network
    Serial.println("mDNS responder started");
    Serial.print("Device name: ");
    Serial.println(DeviceName);
    MDNS.addService("n8i-mlp", "tcp", 23); // Add service
  }
  else
    Serial.println("Error setting up MDNS responder");
}
//#########################################################################################
void StartWiFi() {
  Serial.print("\r\nConnecting to: "); Serial.println(String(ssid));
  IPAddress dns(8, 8, 8, 8); // Use Google as DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);       // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(50);
  }
  Serial.println("\nWiFi connected at: " + WiFi.localIP().toString());
}
//#########################################################################################
void SetupSystem() {
  Serial.begin(115200);                                           // Initialise serial communications
  while (!Serial);
  Serial.println(__FILE__);
  Serial.println("Starting...");
  pinMode(Channel1_Pin, OUTPUT);
  pinMode(Channel2_Pin, OUTPUT);
  pinMode(Channel3_Pin, OUTPUT);
  pinMode(Channel4_Pin, OUTPUT);
  pinMode(Channel5_Pin, OUTPUT);
  pinMode(Channel6_Pin, OUTPUT);
  pinMode(Channel7_Pin, OUTPUT);
  pinMode(Channel8_Pin, OUTPUT);
  pinMode(Channel9_Pin, OUTPUT);
  pinMode(Channel10_Pin, OUTPUT);
  pinMode(Channel11_Pin, OUTPUT);
  pinMode(Channel12_Pin, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
}
//#########################################################################################
boolean SetupTime() {
  configTime(0, 0, "time.nist.gov");                               // (gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);                                       // setenv()adds "TZ" variable to the environment, only used if set to 1, 0 means no change
  tzset();
  delay(200);
  bool TimeStatus = UpdateLocalTime();
  return TimeStatus;
}
//#########################################################################################
boolean UpdateLocalTime() {
  struct tm timeinfo;
  time_t now;
  char  time_output[30];
  while (!getLocalTime(&timeinfo, 15000)) {                        // Wait for up to 15-sec for time to synchronise
    return false;
  }
  time(&now);
  UnixTime = now;
  //See http://www.cplusplus.com/reference/ctime/strftime/
  strftime(time_output, sizeof(time_output), "%H:%M", &timeinfo);  // Creates: '14:05'
  Time_str = time_output;
  strftime(time_output, sizeof(time_output), "%w", &timeinfo);     // Creates: '0' for Sun
  DoW_str  = time_output;
  return true;
}
//#########################################################################################
String ConvertUnixTime(int unix_time) {
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  strftime(output, sizeof(output), "%H:%M", now_tm);               // Returns 21:12
  return output;
}
//#########################################################################################
void StartSPIFFS() {
  Serial.println("Starting SPIFFS");
  boolean SPIFFS_Status;
  SPIFFS_Status = SPIFFS.begin();
  if (SPIFFS_Status == false)
  { // Most likely SPIFFS has not yet been formated, so do so
    Serial.println("Formatting SPIFFS (it may take some time)...");
    SPIFFS.begin(true); // Now format SPIFFS
    File datafile = SPIFFS.open("/" + DataFile, "r");
    if (!datafile || !datafile.isDirectory()) {
      Serial.println("SPIFFS failed to start..."); // Nothing more can be done, so delete and then create another file
      SPIFFS.remove("/" + DataFile); // The file is corrupted!!
      datafile.close();
    }
  }
  else Serial.println("SPIFFS Started successfully...");
}
//#########################################################################################
void Initialise_Array() {
  for (int channel = 0; channel < Channels; channel++) {
    Timer[channel][0].DoW = "Sun";
    Timer[channel][1].DoW = "Mon";
    Timer[channel][2].DoW = "Tue";
    Timer[channel][3].DoW = "Wed";
    Timer[channel][4].DoW = "Thu";
    Timer[channel][5].DoW = "Fri";
    Timer[channel][6].DoW = "Sat";
  }
}
//#########################################################################################
void SaveSettings() {
  Serial.println("Getting ready to Save settings...");
  File dataFile = SPIFFS.open("/" + DataFile, "w");
  if (dataFile) { // Save settings
    Serial.println("Saving settings...");
    for (byte channel = 0; channel < Channels; channel++) {
      for (byte dow = 0; dow < 7; dow++) {
        //Serial.println("Day of week = " + String(dow));
        for (byte p = 0; p < NumOfEvents; p++) {
          dataFile.println(Timer[channel][dow].Start[p]);
          dataFile.println(Timer[channel][dow].Stop[p]);
          //Serial.println("Period: " + String(p) + " from: " + Timer[channel][dow].Start[p] + " to: " + Timer[channel][dow].Stop[p]);
        }
      }
    }
    dataFile.close();
    Serial.println("Settings saved...");
  }
}
//#########################################################################################
void RecoverSettings() {
  String Entry;
  Serial.println("Reading settings...");
  File dataFile = SPIFFS.open("/" + DataFile, "r");
  if (dataFile) { // if the file is available, read it
    Serial.println("Recovering settings...");
    while (dataFile.available()) {
      for (byte channel = 0; channel < Channels; channel++) {
        //Serial.println("Channel-" + String(channel + 1));
        for (byte dow = 0; dow < 7; dow++) {
          //Serial.println("Day of week = " + String(dow));
          for (byte p = 0; p < NumOfEvents; p++) {
            Timer[channel][dow].Start[p] = dataFile.readStringUntil('\n'); Timer[channel][dow].Start[p].trim();
            Timer[channel][dow].Stop[p]  = dataFile.readStringUntil('\n'); Timer[channel][dow].Stop[p].trim();
            //Serial.println("Period: " + String(p) + " from: " + Timer[channel][dow].Start[p] + " to: " + Timer[channel][dow].Stop[p]);
          }
        }
      }
      dataFile.close();
      Serial.println("Settings recovered...");
    }
  }
}
//#########################################################################################
String WiFiSignal() {
  float Signal = WiFi.RSSI();
  Signal = 90 / 40.0 * Signal + 212.5; // From Signal = 100% @ -50dBm and Signal = 10% @ -90dBm and y = mx + c
  if (Signal > 100) Signal = 100;
  return " " + String(Signal, 0) + "%";
}

/*
   Version 1.0 667 lines of code
   All HTML fully validated by W3C
*/
