/*
  ===============================================
  Example sketch for CurieImu library for Intel(R) Curie(TM) devices.
  Copyright (c) 2015 Intel Corporation.  All rights reserved.

  Based on I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050
  class by Jeff Rowberg: https://github.com/jrowberg/i2cdevlib

  ===============================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2011 Jeff Rowberg

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
  
  Genuino 101 CurieIMU Orientation Visualiser
  Hardware Required:
  * Arduino Zero or Uno Board
  
  Modified Nov 2015
  by Helena Bisby <support@arduino.cc>
  This example code is in the public domain
  http://arduino.cc/en/Tutorial/Genuino101CurieIMUOrientationVisualiser
*/

#include "CurieImu.h"
#include "MadgwickAHRS.h"

int16_t ax, ay, az;
int16_t gx, gy, gz;
float yaw;
float pitch;
float roll;
int factor = 4000; // variable by which to divide gyroscope values, used to control sensitivity

#define CALIBRATE_ACCELGRYO_OFFSETS

void setup() {
  // initialize Serial communication
  Serial.begin(115200);

  // initialize device
  Serial.println("Initializing IMU device...");
  CurieImu.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(CurieImu.testConnection() ? "CurieImu connection successful" : "CurieImu connection failed");

#ifdef CALIBRATE_ACCELGRYO_OFFSETS
  // use the code below to calibrate accel/gyro offset values
  Serial.println("Internal sensor offsets BEFORE calibration...");
  Serial.print(CurieImu.getXAccelOffset()); Serial.print("\t"); // -76
  Serial.print(CurieImu.getYAccelOffset()); Serial.print("\t"); // -235
  Serial.print(CurieImu.getZAccelOffset()); Serial.print("\t"); // 168
  Serial.print(CurieImu.getXGyroOffset()); Serial.print("\t"); // 0
  Serial.print(CurieImu.getYGyroOffset()); Serial.print("\t"); // 0
  Serial.print(CurieImu.getZGyroOffset()); Serial.print("\t"); // 0
  Serial.println("");

  // To manually configure offset compensation values, use the following methods instead of the autoCalibrate...() methods below
  //    CurieImu.setXGyroOffset(220);
  //    CurieImu.setYGyroOffset(76);
  //    CurieImu.setZGyroOffset(-85);
  //    CurieImu.setXAccelOffset(-76);
  //    CurieImu.setYAccelOffset(--235);
  //    CurieImu.setZAccelOffset(168);

  // IMU device must be resting in a horizontal position for the following calibration procedure to work correctly!

  // Serial.print("Starting Gyroscope calibration...");
  CurieImu.autoCalibrateGyroOffset();
  // Serial.println(" Done");
  // Serial.print("Starting Acceleration calibration...");
  CurieImu.autoCalibrateXAccelOffset(0);
  CurieImu.autoCalibrateYAccelOffset(0);
  CurieImu.autoCalibrateZAccelOffset(1);
  // Serial.println(" Done");
  /*
    Serial.println("Internal sensor offsets AFTER calibration...");
    Serial.print(CurieImu.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(CurieImu.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(CurieImu.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(CurieImu.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(CurieImu.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(CurieImu.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.println("");
  */
  // Serial.println("Enabling Gyroscope/Acceleration offset compensation");
  CurieImu.setGyroOffsetEnabled(true);
  CurieImu.setAccelOffsetEnabled(true);
#endif
}

void loop() {
  // read raw accel/gyro measurements from device
  CurieImu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  // use function from MagdwickAHRS.h to return quaternions
  MadgwickAHRSupdateIMU(gx / factor, gy / factor, gz / factor, ax, ay, az);

  // equations to find yaw roll and pitch from quaternions
  yaw = atan2(2 * q1 * q2 - 2 * q0 * q3, 2 * q0 * q0 + 2 * q1 * q1 - 1);
  roll = -1 * asin(2 * q1 * q3 + 2 * q0 * q2);
  pitch = atan2(2 * q2 * q3 - 2 * q0 * q1, 2 * q0 * q0 + 2 * q3 * q3 - 1);

  /*
    Serial.print(ax); Serial.print("\t");
    Serial.print(ay); Serial.print("\t");
    Serial.print(az); Serial.print("\t");
    Serial.print(gx); Serial.print("\t");
    Serial.print(gy); Serial.print("\t");
    Serial.print(gz); Serial.print("\t");
    Serial.println("");
  */

  if (Serial.available() > 0) {
    int val = Serial.read();
    if (val == 's') { // if incoming serial is "s"
      Serial.print(yaw);
      Serial.print(","); // print comma so values can be parsed
      Serial.print(pitch);
      Serial.print(","); // print comma so values can be parsed
      Serial.println(roll);
    }
  }
}
