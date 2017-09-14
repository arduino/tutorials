/*

Weather Audio Notifier

Hardware Required:
* Arduino Zero Board  
* Arduino Wifi101 Sheild
* Piezo

Software Required:
* ArduinoJson Library

 created Sept 2015
 by Helena Bisby <support@arduino.cc>

This example code is in the public domain

http://arduino.cc/en/Tutorial/WeatherAudioNotifier
 
*/


#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoJson.h>

#define JSON_BUFF_DIMENSION 2500
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

String nameOfCity = "Turin,IT";   // your city of interest here in format "city,countrycode"

String text;
int endResponse = 0;
boolean startJson = false;
int posMelody[] = {330, 415, 494, 659};  //E3,G#3,B3,E4
int negMelody[] = {392, 370, 349, 330};  //G3,F#3,F3,E3
int noteDurations[] = {4, 4, 4, 8};      //Will correspond to note lengths 8th,8th,8th,4th
int status = WL_IDLE_STATUS;

const char server[] = "api.openweathermap.org";    // name address for openweathermap (using DNS)

WiFiClient client;
unsigned long lastConnectionTime = 10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  text.reserve(JSON_BUFF_DIMENSION);

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
}

void loop() {

  // if ten minutes have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    // note the time that the connection was made:
    lastConnectionTime = millis();

    httpRequest();
  }

  char c = 0;
  if (client.available()) {
    c = client.read();
    // json contains equal number of open and close curly brackets, therefore by counting
    // the open and close occurences, we can determine when a json is completely received
    
    // endResponse == 0 means equal number of open and close curly brackets reached 
    if (endResponse == 0 && startJson == true) {
      parseJson(text.c_str());  // parse c string text in parseJson function
      text = "";                // clear text string for the next time
      startJson = false;        // set startJson to false to indicate that a new message has not yet started
    }
    if (c == '{') {
      startJson = true;         // set startJson true to indicate json message has started
      endResponse++;
    }
    if (c == '}') {
      endResponse--;
    }
    if (startJson == true) {
      text += c;
    }
  }
}
void parseJson(const char * jsonString) {
  StaticJsonBuffer<4000> jsonBuffer;

  // FIND FIELDS IN JSON TREE
  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  JsonArray& list = root["list"];
  JsonObject& now = list[0];
  JsonObject& later = list[1];

  String city = root["city"]["name"];
  float tempNow = now["main"]["temp"];
  float humidityNow = now["main"]["humidity"];
  String weatherNow = now["weather"][0]["description"];

  float tempLater = later["main"]["temp"];
  float humidityLater = later["main"]["humidity"];
  String weatherLater = later["weather"][0]["description"];

  printDiffFloat(tempNow, tempLater, "temperature", "*C");
  printDiffString(weatherNow, weatherLater, "rain");
  printDiffString(weatherNow, weatherLater, "snow");
  printDiffString(weatherNow, weatherLater, "hail");
  printDiffString(weatherNow, weatherLater, "clear");
  printDiffFloat(humidityNow, humidityLater, "humidity", "%");
  Serial.println();

}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    // Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /data/2.5/forecast?q=" + nameOfCity + "&mode=json&units=metric&cnt=2 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void printDiffString(String now, String later, String weatherType) {
  int indexNow = now.indexOf(weatherType);
  int indexLater = later.indexOf(weatherType);
  // for all types of weather except for clear skies, if the current weather does not contain the weather type and the later message does, send notification
  if (weatherType != "clear") {
    if (indexNow == -1 && indexLater != -1) {
      Serial.println("Oh no! It is going to " + weatherType + " later! Predicted " + later);
      for (int thisNote = 0; thisNote < 4; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(8, negMelody[thisNote], noteDuration);      // play negative melody through piezo
      }
    }
  }
  // for clear skies, if the current weather does not contain the word clear and the later message does, send notification that it will be sunny later
  else {
    if (indexNow == -1 && indexLater != -1) {
      Serial.println("It is going to be sunny later! Predicted " + later);
      for (int thisNote = 0; thisNote < 4; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(8, posMelody[thisNote], noteDuration);      // play positive melody through piezo
      }
    }
  }
}

void printDiffFloat(float now, float later, String parameter, String unit) {
  String change;
  if (now > later) {
    change = "drop from ";
  }
  else if (now < later) {
    change = "rise from ";
  }
  else {
    return;
  }
  Serial.print("UPDATE: The " + parameter + " will " + change);
  Serial.print(now);
  Serial.print(unit + " to ");
  Serial.print(later);
  Serial.println(unit + "!");
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

