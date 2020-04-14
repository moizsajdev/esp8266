
//We always have to include the library
#include "LedControl.h"
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(0,2,3,1);
const char ssid[] = "RohtangLa";  //  your network SSID (name)
const char pass[] = "K1rgavalu";       // your network password

// NTP Servers:
static const char ntpServerName[] = "pool.ntp.org";


const int timeZone = 19800;     // Central European Time

const int SYNC_INTERVAL = 600; //seconds

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();

/* we always wait a bit between updates of the display */
unsigned long delaytime=500;

bool blink = true;
void setup() {
//Serial.begin(115200);
   delay(10);
   /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* and clear the display */
  lc.clearDisplay(0);

  
   writeArduinoOn7Segment();
    scrollDigits();
    
 
   // We start by connecting to a WiFi network
   //Serial.print("Connecting to ");
   //Serial.println(ssid);
   WiFi.begin(ssid, pass);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.print("IP number assigned by DHCP is ");
  //Serial.println(WiFi.localIP());
  //Serial.println("Starting UDP");
  Udp.begin(localPort);
  //Serial.print("Local port: ");
  //Serial.println(Udp.localPort());
  //Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(SYNC_INTERVAL);
}


/*
 This method will display the characters for the
 word "Arduino" one after the other on digit 0. 
 */
void writeArduinoOn7Segment() {
  lc.setChar(0,0,'a',false);
  delay(delaytime);
  lc.setRow(0,0,0x05);
  delay(delaytime);
  lc.setChar(0,0,'d',false);
  delay(delaytime);
  lc.setRow(0,0,0x1c);
  delay(delaytime);
  lc.setRow(0,0,B00010000);
  delay(delaytime);
  lc.setRow(0,0,0x15);
  delay(delaytime);
  lc.setRow(0,0,0x1D);
  delay(delaytime);
  lc.clearDisplay(0);
  delay(delaytime);
} 

/*
  This method will scroll all the hexa-decimal
 numbers and letters on the display. You will need at least
 four 7-Segment digits. otherwise it won't really look that good.
 */
void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc.setDigit(0,7,i,true);
    lc.setDigit(0,6,i+1,false);
    lc.setDigit(0,5,i+2,false);
    lc.setDigit(0,4,i+3,false);
    lc.setDigit(0,3,i+4,false);
    lc.setDigit(0,2,i+5,false);
    lc.setDigit(0,1,i+6,false);
    lc.setDigit(0,0,i+7,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
}

void loop()
{
  if (timeStatus() != timeNotSet) {

    setTime(hour(now()),  minute(now()), blink);
    blink = !blink;
    delay(499);
    if(second(now())%10 == 0) setYear(year(now()));
    else if (second(now())%5 == 0) setDateMonth(day(now()), month(now()));
  }
}
void setYear(int year) {
  lc.setDigit(0, 0, year%10, false);
  lc.setDigit(0, 1, (year/10)%10, false);
  lc.setDigit(0, 2, (year/100)%10, false);
  lc.setDigit(0, 3, (year/1000)%10, false);
  
}
void setDateMonth(int date, int month) {
  lc.setDigit(0, 0, month%10, false);
  lc.setDigit(0, 1, (month/10)%10, false);
  lc.setDigit(0, 2, date%10, true);
  lc.setDigit(0, 3, (date/10)%10, false);
  
}
void setTime(int hour, int minutes, bool blink) {
  lc.setDigit(0, 4, minutes%10, false);
  lc.setDigit(0, 5, (minutes/10)%10, false);
  lc.setDigit(0, 6, hour%10, blink);
  lc.setDigit(0, 7, (hour/10)%10, false);
  
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone ;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
