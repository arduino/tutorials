/*
  Google Calendar Actions Planner

  This sketch connects to Gogole Calendar, makes a HTTP request and downloads the day events.Comparing the actual time with the one of the events actions can be programmed.
  using an Arduino Wifi101 shield and Arduino Zero.

  created 08 Sept 2015
  by Arturo Guadalupi <a.guadalupi@arduino.cc>

  http://arduino.cc/en/Tutorial/GoogleCalendarWiFi101
  This code is in the public domain.

  Informations about google APIs and hnow to make HTTP requests can be found at : http://www.udel.edu/CIS/software/dist/google/calendar/java.client/gdata/doc/calendar.html#Feeds
*/

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <time.h>

// Create an rtc object
RTCZero rtc;

char ssid[] = "yourNetwork";     //  your network SSID (name)
char pass[] = "yourPassord";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// server address:
char server[] = "www.google.com";

// google magic cookie: replace your calendar private address here
char magicCookie[] = "/calendar/feeds/userID/private-magicCookie/basic";

// Used for NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP

// Data used for time
const int refreshTime = 1;
const char refreshType = 'm'; // refresh every refreshTime minutes
//const char refreshType = 'h'; // refresh every refreshTime hours
const int GMT = 2; //change this to adapt it to your time zone
int lastHours, lastMinutes, lastSeconds;
#define ITALIAN
//#define ENGLISH

// Commands that can be interpreted*/
char cmd1[] = "LED1";

// string used for the commands parsing
String clientBufferString = "";

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial);// wait for serial port to connect. Needed for Leonardo only


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWifiStatus();

  rtc.begin(H24); // start the RTC in 24 hours mode

  unsigned long epoch = readLinuxEpochUsingNetworkTimeProtocol();
  setRealTimeClock(epoch);

  // print the current time reading values from the RTC
  printTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());

  // ask for data to google calendar
  httpRequest();
}

void loop() {
  int i = 0;
  bool start = false;

  clientBufferString = "";

  checkCalendarRefresh(refreshTime, refreshType);

  if (client.available())
  {
    while (client.available())
    {
      char c = client.read();

      if (c == '<') //useful data starts from '<'
        start = true;

      if (start && isPrintable(c)) // if useful data start and significant data is received
        clientBufferString += c;   // add it to the buffer
    }
    extractEvents();
  }
}

unsigned long readLinuxEpochUsingNetworkTimeProtocol()
{
  unsigned long epoch;

  Udp.begin(localPort);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  if ( Udp.parsePacket() ) {
    Serial.println("NTP time received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
  }

  Udp.stop();
  return epoch;
}

// this function computes the current date and time to set the RTC using the time.h library
void setRealTimeClock(uint32_t epoch)
{
  time_t rawtime;
  struct tm * timeinfo;

  rawtime = (time_t) epoch;
  timeinfo = localtime (&rawtime);
  Serial.println ("Current local time and date");
  Serial.print(asctime(timeinfo));

  rtc.setSeconds(timeinfo->tm_sec);
  rtc.setMinutes(timeinfo->tm_min);
  rtc.setHours((timeinfo->tm_hour) + GMT);

  rtc.setDay(timeinfo->tm_mday);
  rtc.setMonth((timeinfo->tm_mon) + 1); //tm_mon months since January - [ 0 to 11 ]
  rtc.setYear((timeinfo->tm_year) - 100); //tm_year years since 1900 and format is yy
}

// this function checks if the calendar has to be refreshed
void checkCalendarRefresh(int howMany, char when)
{
  if (when == 'h')
  {
    if (rtc.getHours() >= (lastHours + howMany))
      refresh();
  }
  else if (when == 'm')
  {
    if (rtc.getMinutes() >= (lastMinutes + howMany) || (lastMinutes + howMany >= 60))
    {
      refresh();
    }

    delay(1000);
    // print the current time
    printTime(rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
  }
}

// this function refreshes the calendar
void refresh()
{
  lastHours = rtc.getHours();
  lastMinutes = rtc.getMinutes();
  lastSeconds = rtc.getSeconds();
  printTime(lastHours, lastMinutes, lastSeconds);
  httpRequest();
}

/* this function makes a HTTP connection to the server: */
void httpRequest()
{
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("GET ");
    client.print(magicCookie);
    client.println(" HTTP/1.0");
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

// this function is used to parse the different events
void extractEvents()
{
  // first consider all the available events and trim them
  unsigned int index1 = clientBufferString.indexOf("<title type='html'>");
  unsigned index2 = clientBufferString.lastIndexOf(";");
  clientBufferString = clientBufferString.substring(index1, index2); //remove uneseful informations

  int i = 1;

  while (clientBufferString != "")
  {

    index1 = clientBufferString.indexOf("<title type='html'>");

    if (i != 1) //if is not the first event remove other useless data
    {
      clientBufferString.remove(0, index1);
      index1 = clientBufferString.indexOf("<title type='html'>");
    }

    index2 = clientBufferString.indexOf("&");

    String event = clientBufferString.substring(index1, index2);

    if (event != "")
    {
      /*Serial.print("EVENT #");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(event);*/ // uncomment to debug
      interpretEvent(event);
      i++;
    }
    clientBufferString.remove(0, event.length());
  }

  /*Serial.println("TOTAL # EVENTS:");
    Serial.println(i);*/ // uncomment to debug
}

// this function is used to interpret the commands in the event title
void interpretEvent(String event)
{
  int index1 = event.indexOf("<title type='html'>") + strlen("<title type='html'>");
  int index2 = event.indexOf("</title>");

  String title = event.substring(index1, index2);

  index2 = event.indexOf(":");// remove unuseful data
  event.remove(0, index2 + strlen(": ddd "));// for the next step: after the when there is ": ddd " where "ddd" indicate 3 letter of the day of the weeek

  Serial.println("Title:");
  Serial.println(title);
  Serial.println();
  Serial.println("When and from to:");
  Serial.println(event);
  Serial.println();

  extractHowLong(event, title);
}

// this function executes the command from to
void extractHowLong(String whenFromTo, String command)
{
  char duratio[whenFromTo.length()];

  sprintf(duratio, whenFromTo.c_str());

  int day, month, year, fromHours, fromMinutes, toHours, toMinutes;
  char monthString[4];

  sscanf(duratio, "%d %s %d %d:%d%*s%d:%d", &day, monthString, &year, &fromHours, &fromMinutes, &toHours, &toMinutes);

  month = monthToInt(monthString);

  //Serial.println(day);
  //Serial.println(monthString);
  //Serial.println(year);
  //Serial.println(fromHours);
  //Serial.println(fromMinutes);
  //Serial.println(toHours);
  //Serial.println(toMinutes); //uncomment to debug

  if (rtc.getYear() == year - 2000) // if the year is correct
  {
    /*Serial.print("Valid year: ");
      Serial.println(year - 2000);*/

    if (rtc.getMonth() == month) // if the month is correct
    {
      /*Serial.print("Valid month: ");
        Serial.println(year - 2000);*/

      if (rtc.getDay() == day) // if the day is correct
      {
        /*Serial.print("Valid day: ");
          Serial.println(day);*/

        if (((rtc.getHours() >= fromHours) && (rtc.getMinutes() >=  fromMinutes)) || ((toHours >= rtc.getHours()) && (toMinutes > rtc.getMinutes()))) // if the execution time is valid
        {
          decodeCommand(command, true);
        }
        else
        {
          decodeCommand(command, false);
        }

      }
    }
  }
}

// this function converts the month string in the corresponding number
int monthToInt(String month)
{
  // the month string depends on your region language
#ifdef ITALIAN
  if (month == "gen")
    return 1;
  else if (month == "feb")
    return 2;
  else if (month == "mar")
    return 3;
  else if (month == "apr")
    return 4;
  else if (month == "mag")
    return 5;
  else if (month == "giu")
    return 6;
  else if (month == "lug")
    return 7;
  else if (month == "ago")
    return 8;
  else if (month == "set")
    return 9;
  else if (month == "ott")
    return 10;
  else if (month == "nov")
    return 11;
  else if (month == "dic")
    return 12;
  else
    return 0;

#elif ENGLISH
  if (month == "gen")
    return 1;
  else if (month == "feb")
    return 2;
  else if (month == "mar")
    return 3;
  else if (month == "apr")
    return 4;
  else if (month == "may")
    return 5;
  else if (month == "jun")
    return 6;
  else if (month == "jul")
    return 7;
  else if (month == "aug")
    return 8;
  else if (month == "sep")
    return 9;
  else if (month == "oct")
    return 10;
  else if (month == "nov")
    return 11;
  else if (month == "dec")
    return 12;
  else
    return 0;
#endif
}

// this function dedoded the different commands
void decodeCommand(String command, bool validTime)
{
  if (command == cmd1)
  {
    Serial.println();
    Serial.print("Known command detected: ");
    Serial.println(cmd1);

    if (validTime)
    {
      Serial.println();
      Serial.println("Valid time");
      pinMode(13, OUTPUT);
      digitalWrite(13, HIGH);
      Serial.println();
    }

    else
    {
      Serial.println();
      Serial.println("Invalid time");
      pinMode(13, INPUT);
    }
  }
}

// this function prints the current time
void printTime(int hours, int minutes, int seconds)
{

  // print the hour, minute and second:
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());
  Serial.print(" ");
  Serial.print(hours); // print the hour
  Serial.print(':');
  if (minutes < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print(minutes); // print the minute
  Serial.print(':');
  if (seconds < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.println(seconds); // print the second
}
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress & address)
{
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
