import processing.serial.*;
Serial myPort;

int newLine = 13; // new line character in ASCII
float yaw;
float pitch;
float roll;
String message;
String [] ypr = new String [3];

void setup()
{
  size(600, 500, P3D);
  
  /*Set my serial port to same as Arduino, baud rate 9600*/
  myPort = new Serial(this, Serial.list()[2], 9600); 
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
  rotateY(-yaw); // yaw
  rotateZ(-roll); // roll
  
  drawArduino(); // function to draw rough Arduino shape
  
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
  message = myPort.readStringUntil(newLine); // read from port until new line (ASCII code 13)
  if (message != null) {
    ypr = split(message, ","); // split message by commas and store in String array 
    yaw = float(ypr[0]); // convert to float yaw
    pitch = float(ypr[1]); // convert to float pitch
    roll = float(ypr[2]); // convert to float roll
  }
}
void drawArduino() {
  /* function contains shape(s) that are rotated with the IMU */
  stroke(0, 90, 90); // set outline colour to darker teal
  fill(0, 130, 130); // set fill colour to lighter teal
  box(300, 10, 200); // draw Arduino board base shape

  stroke(0); // set outline colour to black
  fill(80); // set fill colour to dark grey

  translate(60, -10, 90); // set position to edge of Arduino box
  box(170, 20, 10); // draw pin header as box

  translate(-20, 0, -180); // set position to other edge of Arduino box
  box(210, 20, 10); // draw other pin header as box
}