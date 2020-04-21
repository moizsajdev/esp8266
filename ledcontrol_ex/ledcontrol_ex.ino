

#include <PrayerTimes.h>
#include <EEPROM.h>
#include <ArduinoJson.h> 
#include "LedControl.h"
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(0,2,3,1);

// NTP Servers:
static const char ntpServerName[] = "pool.ntp.org";
const int timeZone = 19800;     

const int SYNC_INTERVAL = 600; //seconds

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();

/* we always wait a bit between updates of the display */
unsigned long delaytime=500;

bool blink = true;


const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets


#define WIFI_STRING_SIZE 16
const char WiFiAPPSK[] = "K1rgavalu";
const char AP_NAME[] = "RohtangLa";


typedef struct CalcTiming{
  int magrib;
  int suhr;
  int sunrise;
}CalcTiming ;


typedef struct PrayTimeSettings{
  double lat;
  double lon;
  double tz;
  double fajr_angle;
  double isha_angle;
  int hanafi;
  int suhr_mins;
  int magrib_mins;

  int led_intensity;
  int jumah_enable;
  int sync_interval;
  
}PrayTimeSettings;

typedef struct WifiSettings{
  char apName[WIFI_STRING_SIZE];
  char wifiAPSK[WIFI_STRING_SIZE];
  
}WifiSettings;

typedef struct Config{
  WifiSettings wifiSettings;
  PrayTimeSettings prayTimeSettings;
}Config;


Config configSettings;
CalcTiming calcTiming;

bool settingsFromJSON(Config c, String json);

ESP8266WebServer webServer(80);
int suhr,magrib;
  
void setupWiFi()
{
//  WiFi.mode(WIFI_AP);
//  Serial.println(configSettings.wifiSettings.apName);
//  if (sizeof(configSettings.wifiSettings.apName) > 0 && sizeof(configSettings.wifiSettings.wifiAPSK) > 0 ) {
//    WiFi.softAP(configSettings.wifiSettings.apName, configSettings.wifiSettings.wifiAPSK);
//  } else {
//    WiFi.softAP(AP_NAME, WiFiAPPSK);
//  }

  if (sizeof(configSettings.wifiSettings.apName) > 0 && sizeof(configSettings.wifiSettings.wifiAPSK) > 0 ) {
    WiFi.begin(configSettings.wifiSettings.apName, configSettings.wifiSettings.wifiAPSK);
  } else {
    WiFi.begin(AP_NAME, WiFiAPPSK);
  }
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  
}

void setupWebServer() {
  // Wait for connection
  webServer.on("/save", HTTP_POST, saveHandler);
  webServer.on("/settings", HTTP_GET, settingGetHandler);
  webServer.on("/settings", HTTP_POST, settingPostHandler);

  webServer.on("/favicon.ico", []() {
    webServer.send(404, "text/plain", "");
  });
  webServer.onNotFound(notFoundHandler);
  webServer.begin();
}


void setup() {
//Serial.begin(115200);
  EEPROM_read(0, configSettings);
   delay(10);
  setupWebServer() ;
  //set_led_intensity(configSettings.prayTimeSettings.led_intensity);

  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* and clear the display */
  lc.clearDisplay(0);

  
   scrollDigits();
    
 
  setupWiFi();

  //Serial.print("IP number assigned by DHCP is ");
  //Serial.println(WiFi.localIP());
  //Serial.println("Starting UDP");
  Udp.begin(localPort);
  //Serial.print("Local port: ");
  //Serial.println(Udp.localPort());
  //Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(SYNC_INTERVAL);
  getSaumTime(suhr, magrib);
}

void loop()
{
  if (timeStatus() != timeNotSet) {
    if (hour(now()) == 0) getSaumTime(suhr, magrib);
    if (second(now())%20 < 10) {     
      showTime(hour(now()),  minute(now()), blink);
      blink = !blink;
      delay(499);
      if(second(now())%10 == 0) showYear(year(now()));
      else if (second(now())%5 == 0) showDateMonth(day(now()), month(now()));
    } else {
      showSaumTime(suhr, magrib);
      delay(1000);
    }
  }
}
void set_led_intensity(int intensity) {

  lc.setIntensity(0,intensity);

}
