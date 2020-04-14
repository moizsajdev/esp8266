
//We always have to include the library
#include "LedControl.h"
#include <Time.h>
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
char ssid[] = "RohtangLa";  //  your network SSID (name)
char pass[] = "K1rgavalu";       // your network password

#define UPDATESECS 900UL   // update every 15 minutes
time_t update;
bool flag = true;

const long utcOffsetInSeconds = 19800;
unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

unsigned long secsSince1900;

/* we always wait a bit between updates of the display */
unsigned long delaytime=500;

bool blink = true;
void setup() {
Serial.begin(115200);
   delay(10);
   /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  notime();
   
   Serial.println();
   Serial.println();

   // We start by connecting to a WiFi network
   Serial.print("Connecting to ");
   Serial.println(ssid);
   WiFi.begin(ssid, pass);
   
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }
   Serial.println("");
   
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());

   Serial.println("Starting UDP");
   udp.begin(localPort);
   Serial.print("Local port: ");
   Serial.println(udp.localPort());
   setNTPtime();
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

void setNTPtime()
{
   time_t epoch = 0UL;
   while((epoch =  getFromNTP()) == 0) delay(5000);
   epoch -= 2208988800UL;
   setTime(epoch += dst(epoch));
   update = now() + UPDATESECS;
}

unsigned long getFromNTP()
{
   sendNTPpacket(timeServer); // send an NTP packet to a time server
   // wait to see if a reply is available
   delay(1000);
   int cb = udp.parsePacket();
   if (!cb) {
      Serial.println("no packet yet");
      return 0UL;
   }
   Serial.print("packet received, length=");
   Serial.println(cb);
   // We've received a packet, read the data from it
   udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

   //the timestamp starts at byte 40 of the received packet and is four bytes,
   // or two words, long. First, extract the two words:

   unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
   unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
   // combine the four bytes (two words) into a long integer
   // this is NTP time (seconds since Jan 1 1900):
   return (unsigned long) highWord << 16 | lowWord;
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
   Serial.println("sending NTP packet...");
   // set all bytes in the buffer to 0
   memset(packetBuffer, 0, NTP_PACKET_SIZE);
   // Initialize values needed to form NTP request
   // (see URL above for details on the packets)
   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
   packetBuffer[1] = 0;     // Stratum, or type of clock
   packetBuffer[2] = 6;     // Polling Interval
   packetBuffer[3] = 0xEC;  // Peer Clock Precision
   // 8 bytes of zero for Root Delay & Root Dispersion
   packetBuffer[12]  = 49;
   packetBuffer[13]  = 0x4E;
   packetBuffer[14]  = 49;
   packetBuffer[15]  = 52;

   // all NTP fields have been given values, now
   // you can send a packet requesting a timestamp:
   udp.beginPacket(address, 123); //NTP requests are to port 123
   udp.write(packetBuffer, NTP_PACKET_SIZE);
   udp.endPacket();
}

int dst (time_t t) // calculate if summertime
{
   tmElements_t te;
   te.Year = year(t)-1970;
   te.Month =3;
   te.Day =1;
   te.Hour = 0;
   te.Minute = 0;
   te.Second = 0;
   time_t dstStart,dstEnd, current;
   dstStart = makeTime(te);
   dstStart = lastSunday(dstStart);
   dstStart += 2*SECS_PER_HOUR;  //2AM
   te.Month=10;
   dstEnd = makeTime(te);
   dstEnd = lastSunday(dstEnd);
   dstEnd += SECS_PER_HOUR;  //1AM
   if (t>=dstStart && t<dstEnd) return (3600);  //Add back in one hours worth of seconds - DST in effect
   else return (0);  //NonDST
}

time_t lastSunday(time_t t)
{
   t = nextSunday(t);  //Once, first Sunday
   if(day(t) < 4) return t += 4 * SECS_PER_WEEK;
   else return t += 3 * SECS_PER_WEEK;
}

void printTime(const char s[],time_t t)
{
   flag =false;
   Serial.print(s);
   printLeading0(hour(t));
   Serial.print(":");
   printLeading0(minute(t));
   Serial.print(":");
   printLeading0(second(t));
   Serial.print(" ");
   Serial.print(dayStr(weekday(t)));
   Serial.print(" ");
   Serial.print(day(t));
   Serial.print(" ");
   Serial.print(monthStr(month(t)));
   Serial.print(" ");
   Serial.println(year(t));
}

void printLeading0(int n)
{
   if(n < 10) Serial.print("0");
   Serial.print(n);
}

void notime(void)
{

}
void loop() { 
   if(now() > update) setNTPtime();

   int h = hourFormat12(now());
   int m = minute(now());
   setTime(h, m, blink);
 
   if(flag && (now() % 10UL == 0UL)) printTime("Now ",now());
   else flag = true;
   delay(500);

   blink = !blink;
}
void setTime(int hour, int minutes, bool blink) {
  lc.setDigit(0, 4, minutes%10, false);
  lc.setDigit(0, 5, (minutes/10)%10, false);
  lc.setDigit(0, 6, hour%10, blink);
  lc.setDigit(0, 7, (hour/10)%10, false);
  
}
