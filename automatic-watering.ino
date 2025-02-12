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
#include "Dusk2Dawn.h"

//################  VERSION  ###########################################
String version = "1.0";      // Programme version, see change log at end
//################ VARIABLES ###########################################

const char* ServerName = "Controller"; // Connect to the server with http://controller.local/ e.g. if name = "myserver" use http://myserver.local/

#define Channels        13             // n-Channels
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
#define Channel13       12             // Define Channel-13
// Now define the GPIO pins to be used for relay control
#define Channel1_Pin    23              // Define the Relay Control pin
#define Channel2_Pin    19              // Define the Relay Control pin
#define Channel3_Pin    18              // Define the Relay Control pin
#define Channel4_Pin    17              // Define the Relay Control pin
#define Channel5_Pin    16             // Define the Relay Control pin
#define Channel6_Pin    4             // Define the Relay Control pin
#define Channel7_Pin    2             // Define the Relay Control pin
#define Channel8_Pin    13             // Define the Relay Control pin
#define Channel9_Pin    27             // Define the Relay Control pin
#define Channel10_Pin   26             // Define the Relay Control pin
#define Channel11_Pin   25             // Define the Relay Control pin
#define Channel12_Pin   33             // Define the Relay Control pin
#define Channel13_Pin   32             // Define the Relay Control pin

#define LEDPIN          5              // Define the LED Control pin
#define ChannelReverse  false          // Set to true for Relay that requires a signal HIGH for ON, usually relays need a LOW to actuate
#define SunsetOffset    26             // Difference between sunset and nautical twilight in minutes

struct Settings {
  bool IsSunriseSunsetMode;              // Indicates wether the sunrise&sunset time should be used instead start&stop
  String DoW;                          // Day of Week for the programmed event
  String Start[NumOfEvents];           // Start time
  String Stop[NumOfEvents];            // End time
};

struct Schedule {
  String Date;                      // yyyy.mm.dd format
  String SunriseTime;               // HH:MM format
  String SunsetTime;                // HH:MM format
};

struct ChannelOverride {
  bool Overriden = false;
  String State = "";
  void setState(String newState) {
    State = newState;
    Overriden = !Overriden;
  }
};

String       DataFile = "params.txt";  // Storage file name on flash
String       Time_str, DoW_str;        // For Date and Time
Settings     Timer[Channels][7];       // Timer settings, n-Channels each 7-days of the week
Schedule     SunriseSunsetSchedule;    // Sunrise and sunset time for today

//################ VARIABLES ################
const char* Timezone   = "UTC";

Dusk2Dawn location(locationLatitude, locationLongitude, locationTimeZone);

// System values    
String sitetitle                = "Контролер поливу";
String Year                     = "березень 2023";     // For the footer line

bool   ManualOverride           = true;      // Manual override
String Units                    = "M";        // or Units = "I" for °F and 12:12pm time format

String webpage                  = "";         // General purpose variables to hold HTML code for display
int    TimerCheckDuration       = 1000;       // Check for timer event every 1-second
int    wifi_signal              = 0;          // WiFi signal strength
long   LastTimerSwitchCheck     = 0;          // Counter for last timer check
int    UnixTime                 = 0;          // Time now (when updated) of the current time
String Channel1_State           = "OFF";      // Status of the channel
String Channel2_State           = "OFF";      // Status of the channel
String Channel3_State           = "OFF";      // Status of the channel
String Channel4_State           = "OFF";      // Status of the channel
String Channel5_State           = "OFF";      // Status of the channel
String Channel6_State           = "OFF";      // Status of the channel
String Channel7_State           = "OFF";      // Status of the channel
String Channel8_State           = "OFF";      // Status of the channel
String Channel9_State           = "OFF";      // Status of the channel
String Channel10_State          = "OFF";      // Status of the channel
String Channel11_State          = "OFF";      // Status of the channel
String Channel12_State          = "OFF";      // Status of the channel
String Channel13_State          = "OFF";      // Status of the channel

ChannelOverride Channel1Override;             // Override state of the channel
ChannelOverride Channel2Override;             // Override state of the channel
ChannelOverride Channel3Override;             // Override state of the channel
ChannelOverride Channel4Override;             // Override state of the channel
ChannelOverride Channel5Override;             // Override state of the channel
ChannelOverride Channel6Override;             // Override state of the channel
ChannelOverride Channel7Override;             // Override state of the channel
ChannelOverride Channel8Override;             // Override state of the channel
ChannelOverride Channel9Override;             // Override state of the channel
ChannelOverride Channel10Override;            // Override state of the channel
ChannelOverride Channel11Override;            // Override state of the channel
ChannelOverride Channel12Override;            // Override state of the channel

int CurrentManualOverridings  = 0;        // Determines how many manual overridings are in progress at the moment
bool CheckingTimerEvent       = false;    // Indicates wether the checking timer event is in progress

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
  bool recoveredSettings = RecoverSettings();                      // Recover settings from LittleFS
  if (!recoveredSettings) {
    InitializeSunriseSunsetChannels();
  }

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
    TimerSet(Channel1, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer2'
  server.on("/timer2", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel2, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer3'
  server.on("/timer3", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel3, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer4'
  server.on("/timer4", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel4, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer5'
  server.on("/timer5", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel5, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer6'
  server.on("/timer6", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel6, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer7'
  server.on("/timer7", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel7, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer8'
  server.on("/timer8", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel8, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer9'
  server.on("/timer9", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel9, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer10'
  server.on("/timer10", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel10, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer11'
  server.on("/timer11", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel11, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer12'
  server.on("/timer12", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel12, false);
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/timer13'
  server.on("/timer13", HTTP_GET, [](AsyncWebServerRequest * request) {
    TimerSet(Channel13, true);
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
  // Set handler for '/handletimer12' inputs
  server.on("/handletimer12", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool useSunriseSunsetMode = request->arg("UseSunriseSunset").equalsIgnoreCase("on");
    Serial.println(request->arg("StartTime"));
    Serial.println(request->arg("StopTime"));
    Timer[12][0].IsSunriseSunsetMode = useSunriseSunsetMode;
    if (!useSunriseSunsetMode) {
      for (byte dow = 0; dow < 7; dow++) {
        for (byte p = 0; p < 4; p++) {
          Timer[12][dow].Start[p] = request->arg("StartTime");
          Timer[12][dow].Stop[p]  = request->arg("StopTime");
        }
      }
    }
    SaveSettings();
    request->redirect("/homepage");                       // Go back to home page
  });
  // Set handler for '/handlesetup' inputs
  server.on("/handlesetup", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!ManualOverride) {
      request->redirect("/homepage");
      return;
    }

    while (CheckingTimerEvent) {
      delay(10);
    }
    ++CurrentManualOverridings;
    
    if (request->hasArg("manualoverride1")) {
      String stringArg = request->arg("manualoverride1");
      Channel1Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride2")) {
      String stringArg = request->arg("manualoverride2");
      Channel2Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride3")) {
      String stringArg = request->arg("manualoverride3");
      Channel3Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride4")) {
      String stringArg = request->arg("manualoverride4");
      Channel4Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride5")) {
      String stringArg = request->arg("manualoverride5");
      Channel5Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride6")) {
      String stringArg = request->arg("manualoverride6");
      Channel6Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride7")) {
      String stringArg = request->arg("manualoverride7");
      Channel7Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride8")) {
      String stringArg = request->arg("manualoverride8");
      Channel8Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride9")) {
      String stringArg = request->arg("manualoverride9");
      Channel9Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride10")) {
      String stringArg = request->arg("manualoverride10");
      Channel10Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride11")) {
      String stringArg = request->arg("manualoverride11");
      Channel11Override.setState(stringArg);
    }
    if (request->hasArg("manualoverride12")) {
      String stringArg = request->arg("manualoverride12");
      Channel12Override.setState(stringArg);
    }
    
    --CurrentManualOverridings;
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
    UpdateSunriseSunsetSchedule();
    CheckTimerEvent();                                    // Check for schedule actuated
  }
}
//#########################################################################################
void Homepage() {
  append_HTML_header(Refresh);
  webpage += "<h2>Статус каналів @ " + Time_str + "</h2><br>";
  webpage += "<table class='centre channels-table'>";
  webpage += "<tr>";
  webpage += " <td><a class='channel-name' href='/timer1'>" + ChannelTitles[0] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer2'>" + ChannelTitles[1] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer3'>" + ChannelTitles[2] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer4'>" + ChannelTitles[3] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer5'>" + ChannelTitles[4] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer6'>" + ChannelTitles[5] + "</a></td>";
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
  webpage += " <td><a class='channel-name' href='/timer7'>" + ChannelTitles[6] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer8'>" + ChannelTitles[7] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer9'>" + ChannelTitles[8] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer10'>" + ChannelTitles[9] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer11'>" + ChannelTitles[10] + "</a></td>";
  webpage += " <td><a class='channel-name' href='/timer12'>" + ChannelTitles[11] + "</a></td>";
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
bool CheckTime(byte channel, String TimeNow_Str) {
  if (Timer[channel][0].IsSunriseSunsetMode) {
    return !(TimeNow_Str >= SunriseSunsetSchedule.SunriseTime && TimeNow_Str < SunriseSunsetSchedule.SunsetTime);
  }

  for (byte dow = 0; dow < 7; dow++) {                 // Look for any valid timer events, if found turn the heating on
    for (byte p = 0; p < NumOfEvents; p++) {
      // Now check for a scheduled ON time, if so Switch the Timer ON and check the temperature against target temperature
      if (String(dow) == DoW_str && (TimeNow_Str >= Timer[channel][dow].Start[p] && TimeNow_Str < Timer[channel][dow].Stop[p] && Timer[channel][dow].Start[p] != ""))
        return true;
    }
  }
  return false;
}
//#########################################################################################
void TimerSet(int channel, bool sunriseSunsetMode) {
  append_HTML_header(noRefresh);
  webpage += "<h2>Налаштування розкладу Канал-" + String(channel + 1) + "</h2><br>";
  webpage += "<h3>" + ChannelTitles[channel] + "</h3><br>";
  webpage += "<FORM action='/handletimer" + String(channel) + "'>";
  webpage += "<table class='centre timer'>";
  if (sunriseSunsetMode) {
    SunriseSunsetScheduleHtml(channel);
  } else {
    WeekScheduleHtml(channel);
  }
  webpage += "</table>";
  webpage += "<div class='centre'>";
  webpage += "<br><input type='submit' value='Зберегти'><br><br>";
  webpage += "</div></form>";
  append_HTML_footer();
}
//#########################################################################################
void SunriseSunsetScheduleHtml(int channel) {
  String startTime = Timer[channel][0].IsSunriseSunsetMode
    ? SunriseSunsetSchedule.SunriseTime
    : Timer[channel][0].Start[0];
  String stopTime = Timer[channel][0].IsSunriseSunsetMode
    ? SunriseSunsetSchedule.SunsetTime
    : Timer[channel][0].Stop[0];
  String sunriseSunsetCheckedAttribute = Timer[channel][0].IsSunriseSunsetMode
    ? "checked"
    : "";

  webpage += "<tbody>";
  webpage += "<tr><td>";
  webpage += "<input id='sunrise-sunset' name='UseSunriseSunset' type='checkbox' " + sunriseSunsetCheckedAttribute + " /><label for='sunrise-sunset'>Схід/Захід</label>";
  webpage += "</td></tr>";
  webpage += "<tr>";
  webpage += "<td>Старт</td>";
  webpage += "<td><input name='StartTime' type='time' value='" + startTime + "'></td>";
  webpage += "</tr>";
  webpage += "<tr>";
  webpage += "<td>Стоп</td>";
  webpage += "<td><input name='StopTime' type='time' value='" + stopTime + "'></td>";
  webpage += "</tr>";
  webpage += "</tbody>";
  webpage += "<script> const checkbox = document.getElementById('sunrise-sunset'); const startTimeInput = document.getElementsByName('StartTime')[0]; const stopTimeInput = document.getElementsByName('StopTime')[0]; startTimeInput.disabled = stopTimeInput.disabled = checkbox.checked; checkbox.addEventListener('change', (event) => { startTimeInput.disabled = stopTimeInput.disabled = event.currentTarget.checked; }) </script>";
}
//#########################################################################################
void WeekScheduleHtml(int channel) {
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
    for (int dow = 0; dow < 7; dow++) {
      webpage += "<td><input type='time' name='" + String(dow) + "." + String(p) + ".Start' value='" + Timer[channel][dow].Start[p] + "'></td>";
    }
    webpage += "</tr>";
    webpage += "<tr><td>Стоп</td>";
    for (int dow = 0; dow < 7; dow++) {
      webpage += "<td><input type='time' name='" + String(dow) + "." + String(p) + ".Stop' value='" + Timer[channel][dow].Stop[p] + "'></td>";
    }
    webpage += "</tr>";
    webpage += "<tr>";
    webpage += "<td></td>";
    for (int dow = 0; dow < 7; dow++) {
      if (p < (NumOfEvents - 1)) {
        webpage += "<td>-</td>";
      } else {
        webpage += "<td><button type='button' onClick='copy(" + String(dow) + ")'>Копіювати</button></td>";
      }
    }
      webpage += "</tr>";
  }

  webpage += "<script> function copy(dayOfWeekToCopyFrom) { if (!confirm('Ви впевнені, що хочете скопіювати розклад на всі інші дні тижня?')) { return; } var startTimes = []; var stopTimes = []; for (var i = 0; i < 4; i++) { var start = document.getElementsByName(dayOfWeekToCopyFrom + '.' + i + '.Start')[0].value; var stop = document.getElementsByName(dayOfWeekToCopyFrom + '.' + i + '.Stop')[0].value; startTimes[i] = start; stopTimes[i] = stop; } for (var day = 0; day < 7; day++) { if (day == dayOfWeekToCopyFrom) { continue; } for (var i = 0; i < 4; i++) { document.getElementsByName(day + '.' + i + '.Start')[0].value = startTimes[i]; document.getElementsByName(day + '.' + i + '.Stop')[0].value = stopTimes[i]; } } } </script>";
}
//#########################################################################################
void Help() {
  append_HTML_header(noRefresh);
  webpage += "<h2>Довідка</h2><br>";
  webpage += "<div style='text-align: left;font-size:1.1em;'>";
  webpage += "<br><u><b>Сторінка статусу</b></u>";
  webpage += "<p>Надає огляд каналів, що показує увімкнено або вимкнено для кожного каналу, ЗЕЛЕНИЙ - ON та ЧЕРВОНИЙ - OFF.</p>";
  webpage += "<p>Кожна позиція має посилання на налаштування таймера каналу, тому, натиснувши на назву каналу, ви перейдете на сторінку налаштувань таймера каналу.</p>";
  webpage += "<p>Щотижневий підсумок показує 4 канали на день, з 00:00 до 23:30, використовуйте колір каналу, щоб вказати, чи заплановано його ввімкнення.</p>";
  webpage += "<p>ПРИМІТКА: мінімальний період часу, який можна відобразити, становить 30 хвилин. наприклад, з 12:05 до 12:20 не відображатиметься, але з 12:00 до 12:20 буде.</p>";
  webpage += "<p>Це тому, що мінімальна роздільна здатність налаштувань становить 30 хвилин, що не впливає на точність синхронізації.</p>";
  webpage += "<br><u><b>Налаштування каналів від 1 до 12</b></u>";
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
  CheckingTimerEvent = true;

  // Wait until all current manual overridings are finished
  while (CurrentManualOverridings > 0) {
    delay(10);
  }

  String TimeNow;
  TimeNow        = ConvertUnixTime(UnixTime, "%H:%M");           // Get the current time e.g. 15:35
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
  Channel13_State = "OFF";                              // Switch Channel OFF until the schedule decides otherwise

  for (byte channel = 0; channel < Channels; channel++) {
    if (CheckTime(channel, TimeNow))
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
      if (channel == 12) Channel13_State = "ON";
    }
  }

  if (Channel1Override.Overriden) {                              // If manual override is requested then turn the Channel on
    if (Channel1_State == Channel1Override.State) {
      Channel1Override.Overriden = false;
    }
    Channel1_State = Channel1Override.State;
  }
  if (Channel2Override.Overriden) {
    if (Channel2_State == Channel2Override.State) {
      Channel2Override.Overriden = false;
    }
    Channel2_State = Channel2Override.State;
  }
  if (Channel3Override.Overriden) {
    if (Channel3_State == Channel3Override.State) {
      Channel3Override.Overriden = false;
    }
    Channel3_State = Channel3Override.State;
  }
  if (Channel4Override.Overriden) {
    if (Channel4_State == Channel4Override.State) {
      Channel4Override.Overriden = false;
    }
    Channel4_State = Channel4Override.State;
  }
  if (Channel5Override.Overriden) {
    if (Channel5_State == Channel5Override.State) {
      Channel5Override.Overriden = false;
    }
    Channel5_State = Channel5Override.State;
  }
  if (Channel6Override.Overriden) {
    if (Channel6_State == Channel6Override.State) {
      Channel6Override.Overriden = false;
    }
    Channel6_State = Channel6Override.State;
  }
  if (Channel7Override.Overriden) {
    if (Channel7_State == Channel7Override.State) {
      Channel7Override.Overriden = false;
    }
    Channel7_State = Channel7Override.State;
  }
  if (Channel8Override.Overriden) {
    if (Channel8_State == Channel8Override.State) {
      Channel8Override.Overriden = false;
    }
    Channel8_State = Channel8Override.State;
  }
  if (Channel9Override.Overriden) {
    if (Channel9_State == Channel9Override.State) {
      Channel9Override.Overriden = false;
    }
    Channel9_State = Channel9Override.State;
  }
  if (Channel10Override.Overriden) {
    if (Channel10_State == Channel10Override.State) {
      Channel10Override.Overriden = false;
    }
    Channel10_State = Channel10Override.State;
  }
  if (Channel11Override.Overriden) {
    if (Channel11_State == Channel11Override.State) {
      Channel11Override.Overriden = false;
    }
    Channel11_State = Channel11Override.State;
  }
  if (Channel12Override.Overriden) {
    if (Channel12_State == Channel12Override.State) {
      Channel12Override.Overriden = false;
    }
    Channel12_State = Channel12Override.State;
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
  if (Channel13_State == "ON") ActuateChannel(ON, Channel13, Channel13_Pin); else ActuateChannel(OFF, Channel13, Channel13_Pin);
  CheckingTimerEvent = false;
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
  String channel13Icon = Timer[Channel13][0].IsSunriseSunsetMode
    ? "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='36' height='36' viewBox='0, 0, 400, 400'><g id='svgg'><path id='path0' d='M193.750 87.500 C 193.750 111.111,196.042 118.750,203.125 118.750 C 210.208 118.750,212.500 111.111,212.500 87.500 C 212.500 63.889,210.208 56.250,203.125 56.250 C 196.042 56.250,193.750 63.889,193.750 87.500 M100.000 90.119 C 100.000 103.943,120.991 137.416,129.688 137.460 C 140.917 137.517,139.292 124.891,125.069 101.563 C 112.168 80.404,100.000 74.849,100.000 90.119 M276.563 105.974 C 259.920 135.120,259.573 137.500,271.967 137.500 C 281.729 137.500,306.185 102.922,306.227 89.063 C 306.275 72.993,290.144 82.189,276.563 105.974 M178.125 149.802 C 138.487 158.845,102.526 192.059,95.763 225.873 C 92.217 243.603,91.809 243.750,46.094 243.750 C 9.288 243.750,0.000 245.639,0.000 253.125 C 0.000 261.263,26.389 262.500,200.000 262.500 C 373.611 262.500,400.000 261.263,400.000 253.125 C 400.000 245.639,390.712 243.750,353.906 243.750 C 308.191 243.750,307.783 243.603,304.237 225.873 C 294.364 176.506,230.343 137.889,178.125 149.802 M31.250 162.592 C 31.250 169.959,68.172 193.692,79.688 193.727 C 94.260 193.770,87.513 178.177,68.204 167.188 C 46.707 154.952,31.250 153.030,31.250 162.592 M332.813 169.469 C 310.766 183.588,306.534 193.750,322.700 193.750 C 335.453 193.750,375.000 169.734,375.000 161.990 C 375.000 152.161,354.036 155.877,332.813 169.469 M245.254 179.132 C 263.289 188.217,277.143 206.226,284.145 229.688 L 288.342 243.750 200.000 243.750 L 111.658 243.750 116.024 229.688 C 119.675 217.928,142.000 181.465,145.738 181.156 C 146.363 181.104,158.125 177.076,171.875 172.204 C 198.029 162.937,216.580 164.689,245.254 179.132 M62.500 296.875 C 62.500 304.924,81.944 306.250,200.000 306.250 C 318.056 306.250,337.500 304.924,337.500 296.875 C 337.500 288.826,318.056 287.500,200.000 287.500 C 81.944 287.500,62.500 288.826,62.500 296.875 M112.500 334.375 C 112.500 342.262,126.389 343.750,200.000 343.750 C 273.611 343.750,287.500 342.262,287.500 334.375 C 287.500 326.488,273.611 325.000,200.000 325.000 C 126.389 325.000,112.500 326.488,112.500 334.375 ' stroke='none' fill='blue' fill-rule='evenodd'></path></g></svg>"
    : "<svg id='schedule-svg' style='margin-top: 7px;' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='28' height='28' viewBox='0, 0, 400,400'><g id='svgg'><path id='path0' d='M70.814 1.483 C 66.486 4.336,65.636 6.623,65.630 15.430 L 65.625 23.438 50.195 23.458 C 26.265 23.489,15.159 28.672,5.873 44.141 C -0.526 54.801,-0.052 43.093,0.182 184.766 L 0.391 310.547 3.063 316.016 C 6.559 323.171,12.766 329.379,19.922 332.875 L 25.391 335.547 117.794 335.938 L 210.198 336.328 212.768 342.239 C 242.656 410.995,338.509 420.564,382.322 359.165 C 413.687 315.211,401.623 248.833,357.229 221.094 L 351.602 217.578 351.562 137.891 L 351.522 58.203 349.776 53.080 C 342.569 31.941,328.585 23.438,301.030 23.438 L 285.938 23.438 285.932 15.430 C 285.925 3.979,282.910 0.010,274.219 0.010 C 265.528 0.010,262.513 3.979,262.505 15.430 L 262.500 23.438 241.406 23.438 L 220.313 23.438 220.307 15.430 C 220.300 3.979,217.285 0.010,208.594 0.010 C 199.903 0.010,196.888 3.979,196.880 15.430 L 196.875 23.438 175.781 23.438 L 154.688 23.438 154.682 15.430 C 154.675 3.979,151.660 0.010,142.969 0.010 C 134.278 0.010,131.263 3.979,131.255 15.430 L 131.250 23.438 110.156 23.438 L 89.063 23.438 89.057 15.430 C 89.052 6.623,88.202 4.336,83.874 1.483 C 80.835 -0.521,73.852 -0.521,70.814 1.483 M65.630 54.883 C 65.638 66.333,68.653 70.302,77.344 70.302 C 86.035 70.302,89.050 66.333,89.057 54.883 L 89.063 46.875 110.156 46.875 L 131.250 46.875 131.255 54.883 C 131.263 66.333,134.278 70.302,142.969 70.302 C 151.660 70.302,154.675 66.333,154.682 54.883 L 154.688 46.875 175.781 46.875 L 196.875 46.875 196.880 54.883 C 196.888 66.333,199.903 70.302,208.594 70.302 C 217.285 70.302,220.300 66.333,220.307 54.883 L 220.313 46.875 241.406 46.875 L 262.500 46.875 262.505 54.883 C 262.513 66.333,265.528 70.302,274.219 70.302 C 282.936 70.302,285.925 66.342,285.932 54.784 L 285.938 46.678 299.414 47.030 C 324.735 47.690,327.516 51.079,327.982 81.836 L 328.281 101.563 175.781 101.563 L 23.282 101.563 23.579 81.836 C 24.049 50.626,27.075 47.214,54.492 46.973 L 65.625 46.875 65.630 54.883 M328.125 166.280 L 328.125 207.560 320.898 206.148 C 260.592 194.358,204.688 240.503,204.688 302.072 L 204.688 312.533 118.555 312.321 C 21.636 312.083,29.486 312.595,25.532 306.250 L 23.828 303.516 23.617 214.258 L 23.405 125.000 175.765 125.000 L 328.125 125.000 328.125 166.280 M125.797 155.275 C 119.705 158.335,117.475 167.003,121.305 172.736 C 124.455 177.452,126.908 178.123,141.016 178.123 C 158.144 178.123,162.500 175.668,162.500 166.016 C 162.500 156.363,158.144 153.909,141.016 153.909 C 130.673 153.909,128.046 154.144,125.797 155.275 M195.328 155.275 C 189.236 158.335,187.006 167.003,190.836 172.736 C 193.987 177.452,196.439 178.123,210.547 178.123 C 227.675 178.123,232.031 175.668,232.031 166.016 C 232.031 156.363,227.675 153.909,210.547 153.909 C 200.204 153.909,197.578 154.144,195.328 155.275 M264.859 155.275 C 260.816 157.306,258.594 161.116,258.594 166.016 C 258.594 175.668,262.950 178.123,280.078 178.123 C 297.207 178.123,301.563 175.668,301.563 166.016 C 301.563 156.363,297.207 153.909,280.078 153.909 C 269.735 153.909,267.109 154.144,264.859 155.275 M55.189 208.514 C 48.049 213.221,48.049 224.279,55.189 228.986 C 57.168 230.291,59.019 230.458,71.484 230.458 C 89.610 230.458,92.958 228.633,92.958 218.750 C 92.958 208.867,89.610 207.042,71.484 207.042 C 59.019 207.042,57.168 207.209,55.189 208.514 M124.720 208.514 C 121.071 210.919,119.542 213.944,119.542 218.750 C 119.542 228.633,122.890 230.458,141.016 230.458 C 159.141 230.458,162.490 228.633,162.490 218.750 C 162.490 208.867,159.141 207.042,141.016 207.042 C 128.551 207.042,126.699 207.209,124.720 208.514 M194.251 208.514 C 190.603 210.919,189.073 213.944,189.073 218.750 C 189.073 228.633,192.421 230.458,210.547 230.458 C 228.673 230.458,232.021 228.633,232.021 218.750 C 232.021 208.867,228.673 207.042,210.547 207.042 C 198.082 207.042,196.231 207.209,194.251 208.514 M318.638 230.535 C 378.457 244.337,396.206 321.829,348.241 359.785 C 299.799 398.118,228.906 363.994,228.906 302.344 C 228.906 255.042,272.757 219.950,318.638 230.535 M295.814 253.045 C 290.645 256.452,290.635 256.511,290.635 282.813 C 290.635 309.114,290.645 309.173,295.814 312.580 C 299.239 314.838,344.511 314.838,347.936 312.580 C 355.076 307.873,355.076 296.814,347.936 292.108 C 345.934 290.788,344.064 290.635,329.883 290.630 L 314.063 290.625 314.057 274.805 C 314.051 254.716,312.466 251.573,302.344 251.573 C 299.445 251.573,297.320 252.052,295.814 253.045 M56.266 260.743 C 47.811 264.991,47.811 277.978,56.266 282.225 C 58.515 283.356,61.141 283.591,71.484 283.591 C 88.613 283.591,92.969 281.137,92.969 271.484 C 92.969 261.832,88.613 259.377,71.484 259.377 C 61.141 259.377,58.515 259.613,56.266 260.743 M125.797 260.743 C 121.754 262.775,119.531 266.585,119.531 271.484 C 119.531 281.137,123.887 283.591,141.016 283.591 C 158.144 283.591,162.500 281.137,162.500 271.484 C 162.500 261.832,158.144 259.377,141.016 259.377 C 130.673 259.377,128.046 259.613,125.797 260.743 ' stroke='none' fill='blue' fill-rule='evenodd'></path></g></svg>";
  String channel13StartTime = Timer[Channel13][0].IsSunriseSunsetMode
    ? SunriseSunsetSchedule.SunriseTime
    : Timer[Channel13][0].Start[0];
  String channel13StopTime = Timer[Channel13][0].IsSunriseSunsetMode
    ? SunriseSunsetSchedule.SunsetTime
    : Timer[Channel13][0].Stop[0];
  
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
  webpage += ".channels-table tr:nth-child(odd) td a {padding: 10px 5px}";
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
  webpage += ".right-panel {float:right;margin-right:20px;margin-top:5px;}";
  webpage += ".wifi-info {float:right;}";
  webpage += ".sunrise-sunset {float:left;margin-right:20px;margin-top:5px;cursor:pointer;}";
  webpage += ".wifi, .wifi:before {display:inline-block;border:9px double transparent;border-top-color:currentColor;border-radius:50%;}";
  webpage += ".wifi:before {content:'';width:0;height:0;}";
  webpage += ".channel-name {display:block;text-decoration:none;color:blue;}";
  webpage += "input[type='submit'] {cursor: pointer;background:blue;border:2px black;color:white;font-size:15px;padding:5px 15px;}";
  webpage += "</style></head>";
  webpage += "<body>";
  webpage += "<div class='topnav'>";
  webpage += "<a href='/'>Статус</a>";
  webpage += "<a href='help'>Довідка</a>";
  webpage += "<div class='right-panel'>";
  webpage += "<div class='sunrise-sunset' onclick='location.href=\"/timer13\"'>";
  webpage += channel13Icon;
  webpage += "<div style='float: right; margin-top: 14px; margin-left: 12px;'>" + channel13StartTime + " | " + channel13StopTime + " <b>(" + Channel13_State + ")</b></div>";
  webpage += "</div>";
  webpage += "<div class='wifi-info'><div class='wifi'></div><span>" + WiFiSignal() + "</span></div>";
  webpage += "</div>";
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
  if (!ChannelReverse) {
    digitalWrite(Channel1_Pin, HIGH);
    digitalWrite(Channel2_Pin, HIGH);
    digitalWrite(Channel3_Pin, HIGH);
    digitalWrite(Channel4_Pin, HIGH);
    digitalWrite(Channel5_Pin, HIGH);
    digitalWrite(Channel6_Pin, HIGH);
    digitalWrite(Channel7_Pin, HIGH);
    digitalWrite(Channel8_Pin, HIGH);
    digitalWrite(Channel9_Pin, HIGH);
    digitalWrite(Channel10_Pin, HIGH);
    digitalWrite(Channel11_Pin, HIGH);
    digitalWrite(Channel12_Pin, HIGH);
    digitalWrite(Channel13_Pin, HIGH);
  }
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
  pinMode(Channel13_Pin, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
}
//#########################################################################################
boolean SetupTime() {
  configTime(60 * 60 * 2, 60 * 60 * 1, "time.nist.gov");                               // (gmtOffset_sec, daylightOffset_sec, ntpServer)
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
String ConvertUnixTime(int unix_time, const char* format) {
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  strftime(output, sizeof(output), format, now_tm);               // Returns 21:12
  return output;
}
//#########################################################################################
void UpdateSunriseSunsetSchedule() {
  time_t tm = UnixTime;
  struct tm *now_tm = localtime(&tm);
  String DateNow;
  DateNow = ConvertUnixTime(UnixTime, "%Y.%m.%d");           // Get the current time e.g. 2023.04.23
  if (SunriseSunsetSchedule.Date >= DateNow)
  {
    return;
  }

  int sunriseMins = location.sunrise(1900 + now_tm->tm_year, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_isdst);
  int sunsetMins = location.sunset(1900 + now_tm->tm_year, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_isdst) + SunsetOffset;
  char sunrise[] = "00:00";
  char sunset[] = "00:00";
  location.min2str(sunrise, sunriseMins);
  location.min2str(sunset, sunsetMins);
  SunriseSunsetSchedule.Date = DateNow;
  SunriseSunsetSchedule.SunriseTime = String(sunrise);
  SunriseSunsetSchedule.SunsetTime = String(sunset);
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
void InitializeSunriseSunsetChannels() {
  Timer[Channel13][0].IsSunriseSunsetMode = true;
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
          dataFile.println(String(Timer[channel][dow].IsSunriseSunsetMode));
          dataFile.println(Timer[channel][dow].Start[p]);
          dataFile.println(Timer[channel][dow].Stop[p]);
          //Serial.println("Period: " + String(p) + " from: " + Timer[channel][dow].Start[p] + " to: " + Timer[channel][dow].Stop[p]);
        }
      }
    }
    dataFile.close();
    File dataFile1 = SPIFFS.open("/" + DataFile, "r");
    // Serial.println(dataFile1.readString());
    dataFile1.close();
    Serial.println("Settings saved...");
  }
}
//#########################################################################################
// Recivers settings from the file. Returns boolean value that indicates wether the settings have been recovered.
bool RecoverSettings() {
  String Entry;
  Serial.println("Reading settings...");
  File dataFile = SPIFFS.open("/" + DataFile, "r");
  if (!dataFile) {
    return false;
  }

  // if the file is available, read it
  Serial.println("Recovering settings...");
  while (dataFile.available()) {
    for (byte channel = 0; channel < Channels; channel++) {
      //Serial.println("Channel-" + String(channel + 1));
      for (byte dow = 0; dow < 7; dow++) {
        //Serial.println("Day of week = " + String(dow));
        for (byte p = 0; p < NumOfEvents; p++) {
          String isSunriseSunsetModeStr = dataFile.readStringUntil('\n'); isSunriseSunsetModeStr.trim();
          Timer[channel][dow].IsSunriseSunsetMode = isSunriseSunsetModeStr == "1";
          Timer[channel][dow].Start[p] = dataFile.readStringUntil('\n'); Timer[channel][dow].Start[p].trim();
          Timer[channel][dow].Stop[p]  = dataFile.readStringUntil('\n'); Timer[channel][dow].Stop[p].trim();
          //Serial.println("Period: " + String(p) + " from: " + Timer[channel][dow].Start[p] + " to: " + Timer[channel][dow].Stop[p]);
        }
      }
    }
    dataFile.close();
    Serial.println("Settings recovered...");
  }

  return true;
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
