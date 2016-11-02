/*
  MKR1000 WiFi Robot

  This sketch use the WiFi server capability of a MKR1000 to post a web interface used to move a mini robot.
  
  Required hardware:
  - Arduino/Genuino MKR1000;
  - MKR2UNO Shield adapter;
  - Arduino Motor Shield

  created 02 Nov 2016
  by Arturo Guadalupi <a.guadalupi@arduino.cc>
*/

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiMDNSResponder.h>

#include "password.h" //where defined NET_SSID and NET_PWD

char ssid[] = NET_SSID;      // your network SSID (name)
char pass[] = NET_PWD;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

char mdnsName[] = "WiFiRobot"; // the MDNS name that the board will respond to
// Note that the actual MDNS name will have '.local' after
// the name above, so "WiFiRobot" will be accessible on
// the MDNS name "WiFiRobot.local".

int status = WL_IDLE_STATUS;

// Create a MDNS responder to listen and respond to MDNS name requests.
WiFiMDNSResponder mdnsResponder;

WiFiServer server(80);

String readString;

//Motor shield pins
const int pinDirA = 12;
const int pinDirB = 13;
const int pinPwmA = 3;
const int pinPwmB = 11;

const int motorSpeed = 255; //value used during the analogWrite() use
const int stepsDelay = 500; //delay between movements

void setup() {
  //Put motor shield pins as output
  pinMode(pinDirA, OUTPUT);
  pinMode(pinPwmA, OUTPUT);
  pinMode(pinDirB, OUTPUT);
  pinMode(pinPwmB, OUTPUT);

  stopMotors();

  //Initialize serial and wait for port to open:
  Serial.begin(9600);

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
  printWiFiStatus();

  server.begin();

  // Setup the MDNS responder to listen to the configured name.
  // NOTE: You _must_ call this _after_ connecting to the WiFi network and
  // being assigned an IP address.
  if (!mdnsResponder.begin(mdnsName)) {
    Serial.println("Failed to start MDNS responder!");
    while (1);
  }

  Serial.print("Server listening at http://");
  Serial.print(mdnsName);
  Serial.println(".local/");
  Serial.println();
}


void loop() {
  // Call the update() function on the MDNS responder every loop iteration to
  // make sure it can detect and respond to name requests.
  mdnsResponder.poll();

  // listen for incoming clients
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();// Read char by char HTTP request
        readString += c;
        Serial.write(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<head><title>WiFi Robot</title></head>");
          client.println("<center><hr/><p> Click the Buttons to move the robot <p/><hr/></center>");
          client.println("<center><input type=button value='GO UP' onmousedown=location.href='/?GO_UP'></center><br/>");
          client.println("<center><left><input type=button value='GO LEFT' onmousedown=location.href='/?GO_LEFT'><input type=button value='GO RIGHT' onmousedown=location.href='/?GO_RIGHT'></center><br/>");
          client.println("<center><input type=button value='GO DOWN' onmousedown=location.href='/?GO_DOWN'></right></center><br/><br/>");
          client.println("<hr/>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    
    // parse the received string
    if (readString.indexOf("/?GO_UP") > 0) {
      Serial.println();
      Serial.println("UP");
      Serial.println();
      goUp(); //move up
    }
    if (readString.indexOf("/?GO_DOWN") > 0) {
      Serial.println();
      Serial.println("DOWN");
      Serial.println();
      goDown(); //move down
    }
    if (readString.indexOf("/?GO_LEFT") > 0) {
      Serial.println();
      Serial.println("LEFT");
      Serial.println();
      goLeft(); //move left
    }
    if (readString.indexOf("/?GO_RIGHT") > 0) {
      Serial.println();
      Serial.println("RIGHT");
      Serial.println();
      goRight(); //move right
    }
    readString = "";// Clearing string for next read
    Serial.println("client disconnected");
  }
}


void printWiFiStatus() {
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

void stopMotors(void) {
  analogWrite(pinPwmA, 0);
  analogWrite(pinPwmB, 0);
}

void goUp(void) {
  digitalWrite(pinDirA, HIGH);
  digitalWrite(pinDirB, HIGH);
  delay(1);
  analogWrite(pinPwmA, motorSpeed);
  analogWrite(pinPwmB, motorSpeed);
  delay(stepsDelay);
  stopMotors();
}

void goDown(void) {
  digitalWrite(pinDirA, LOW);
  digitalWrite(pinDirB, LOW);
  delay(1);
  analogWrite(pinPwmA, motorSpeed);
  analogWrite(pinPwmB, motorSpeed);
  delay(stepsDelay);
  stopMotors();
}

void goLeft(void) {
  digitalWrite(pinDirA, LOW);
  digitalWrite(pinDirB, HIGH);
  delay(1);
  analogWrite(pinPwmA, motorSpeed);
  analogWrite(pinPwmB, motorSpeed);
  delay(stepsDelay*2);
  stopMotors();
}

void goRight(void) {
  digitalWrite(pinDirA, HIGH);
  digitalWrite(pinDirB, LOW);
  delay(1);
  analogWrite(pinPwmA, motorSpeed);
  analogWrite(pinPwmB, motorSpeed);
  delay(stepsDelay*2);
  stopMotors();
}

