#include <Time.h>
#include <TimeLib.h>

/*
* espNTPdigitalClock.ino
*
* Created: 5/6/2015 2:17:03 PM
* Author: Nigel
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FastLED.h>

char ssid[] = "RohtangLa";  //  your network SSID (name)
char pass[] = "K1rgavalu";       // your network password

#define UPDATESECS 900UL   // update every 15 minutes
time_t update;
bool flag = true;

// How many leds in your strip?
#define NUM_LEDS 50
#define LED_PER_DIGIT 2

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2
#define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];
byte blue_print[10][7] = {
  {1,1,1,1,1,1,0},
  {1,0,0,0,0,1,0},
  {0,1,1,0,1,1,1},
  {1,1,0,0,1,1,1},
  {1,0,0,1,0,1,1},
  {1,1,0,1,1,0,1},
  {1,1,1,1,1,0,1},
  {1,0,0,0,1,1,0},
  {1,1,1,1,1,1,1},
  {1,1,0,1,1,1,1}
};

const long utcOffsetInSeconds = 19800;
unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

unsigned long secsSince1900;

void setup()
{
   Serial.begin(115200);
   delay(10);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

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
void setDigit(int start, int value, const struct CRGB & color) {
 int i = start;
 for (int segment = 0; segment < 7; segment++) {
  for (int rep = 0; rep < LED_PER_DIGIT; rep++) {
    leds[i++] = (blue_print[value][segment] == 1) ? color : CRGB::Black;
  }
 }
}
void setHighHour(bool high, const struct CRGB & color) {
  for (int i = 46; i < 50; i++)
    leds[i] = high ? color : CRGB::Black;
}
void setLowHour(int value, const struct CRGB & color) {
  setDigit(32, value, color);
}
void setHighMinute(int value, const struct CRGB & color) {
  setDigit(16, value, color);
}
void setLowMinute(int value, const struct CRGB & color) {
  setDigit(2, value, color);
}
void setMeridian(bool prime, const struct CRGB & color) {
  if (prime) {
    leds[0] = CRGB::Black;
    leds[1] = color;
  } else {
   leds[1] = CRGB::Black;
   leds[0] = color;
    
  }
  
}
void setBlinker(bool on, const struct CRGB & color){
  leds[30] = on ? color : CRGB::Black;
  leds[31] = on ? color : CRGB::Black;
  FastLED.show();
}


void setTime(int hour, int minutes, const struct CRGB & color) {
  
  setMeridian(hour >= 13, color);
  hour = (hour >= 13) ? hour - 12 : hour; 
  setLowMinute(minutes%10, color);
  setHighMinute((minutes/10)%10, color);
  setLowHour(hour%10, color);
  setHighHour( hour > 9, color);
  FastLED.show();
}
bool blink = true;
void loop()
{
   if(now() > update) setNTPtime();

   int h = hourFormat12(now());
   int m = minute(now());
   setTime(h, m, CRGB::Red);
 
   if(flag && (now() % 10UL == 0UL)) printTime("Now ",now());
   else flag = true;
   delay(500);

    setBlinker(blink, CRGB::Red);
   blink = !blink;
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
