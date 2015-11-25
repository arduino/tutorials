/*
  Genuino 101 CurieIMU Orientation Visualiser
  Hardware Required:
  * Arduino/Genuino 101
  
  ---------------------------------------------------------------------
  Created Nov 2015
  by Helena Bisby <support@arduino.cc>
  This example code is in the public domain
  http://arduino.cc/en/Tutorial/Genuino101CurieIMUOrientationVisualiser
*/

import processing.serial.*;
Serial myPort;
float yaw;
float pitch;
float roll;
String message;
String [] ypr = new String [3];

void setup()
{
  size(600, 500, P3D);
  //Set my serial port to same as Arduino, baud rate 115200*
  myPort = new Serial(this, "/dev/cu.usbmodemfa131", 115200); 
  textSize(16); // set text size
  textMode(SHAPE); // set text mode to shape
}

void draw()
{
  serialEvent();  // read and parse incoming serial message
  background(255); // set background to white

  translate(width/2, height/2); // set position to centre
  pushMatrix(); // begin object
  rotateX(pitch); // RotateX pitch value
  rotateY(-yaw);//yaw
  rotateZ(-roll);//roll

  stroke(0, 90, 90); // set outline colour to darker teal
  fill(0, 128, 128); // set fill colour to lighter teal
  box(300, 10, 200); // draw Arduino board base shape

  stroke(0); // set outline colour to black
  fill(80); // set fill colour to dark grey

  translate(60, -10, 90); // set position to edge of Arduino box
  box(170, 20, 10); // draw pin header as box

  translate(-20, 0, -180); // set position to other edge of Arduino box
  box(210, 20, 10); // draw other pin header as box

  fill(255); // set fill colour to white
  text("GENUINO 101", -10, -20, 40); // write GENUINO 101
  popMatrix(); // end of object

  // Print values to console
  print(pitch);
  print("\t");
  print(roll); 
  print("\t");
  print(-yaw);   
  println("\t");

  myPort.write("s"); // write an "s" to receive more data from Arduino
} 

void serialEvent()
{
  message = myPort.readStringUntil(13); // read from port until new line
  if (message != null) {
    ypr = split(message, ","); // split message by commas and store in String array 
    yaw = float(ypr[0]); // convert to float yaw
    pitch = float(ypr[1]); // convert to float pitch
    roll = float(ypr[2]);// convert to float roll
  }
}