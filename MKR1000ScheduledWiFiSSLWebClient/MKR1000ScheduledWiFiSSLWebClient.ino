/*
  Scheduled WiFi SSL Web Client for MKR1000

  This sketch connects to the Arduino website every minute and downloads the ASCII logo to display it on the serial monitor

  created 19 Jan 2016
  by Arturo Guadalupi <a.guadalupi@arduino.cc>

  http://arduino.cc/en/Tutorial/

  This code is in the public domain.
*/

#include <SPI.h>
#include <WiFi101.h>
#include <RTCZero.h>

char ssid[] = "officine1";      //  your network SSID (name)
char pass[] = "IoTisMagic";     // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiSSLClient client;

// server address:
char server[] = "www.arduino.cc";

bool sendRequest = true; // used to understand if the http request must be sent

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 50;
const byte minutes = 00;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;

void setup() {
  //Initialize Serial and wait for port to open:
  Serial.begin(115200);

  connectToAP();    // connect the board to the access point
  printWifiStatus();
  httpRequest();
  listenToClient();

  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(0, 0, 0);    //in this way the request is sent every minute at 0 seconds
  rtc.enableAlarm(rtc.MATCH_SS);

  rtc.attachInterrupt(alarmMatch);
}

void loop() {
  if (sendRequest) {
    sendRequest = false;
    httpRequest();
    listenToClient();
  }
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

void alarmMatch() {
  sendRequest = true;
}

void connectToAP() {
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

    // wait 1 second for connection:
    delay(1000);
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  sendRequest = false;

  // Print request time
  Serial.println();
  Serial.print("Request sent @ ");
  print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
  Serial.println();

  if (client.connect(server, 443)) {
    // Make a HTTP request:
    client.println("GET /asciilogo.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("connection failed");
  }
}

void listenToClient()
{
  unsigned long startTime = millis();
  bool received = false;

  while ((millis() - startTime < 5000) && !received) { //try to listen for 5 seconds
    while (client.available()) {
      received = true;
      char c = client.read();
      Serial.write(c);
    }
  }
  client.stop();
  Serial.println();
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

