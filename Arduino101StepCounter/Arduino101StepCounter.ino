/*
 * Copyright (c) 2015 Intel Corporation.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * This sketch example demonstrates how the BMI160 accelerometer on the
 * Intel(R) Curie(TM) module can be used as a Step Counter (pedometer)
 */

#include "CurieImu.h"
int lastStepCount = 0;

void setup() {
  Serial.begin(9600);

  CurieImu.initialize(); // initialise the IMU
  CurieImu.setStepDetectionMode(BMI160_STEP_MODE_NORMAL); // set step detection mode to normal
  CurieImu.setStepCountEnabled(true); // enable step count

  Serial.println("IMU initialisation complete, waiting for events...");
}

void loop() {
  /* we can now check the step count periodically */
  updateStepCount();
  delay(1000);
}

void updateStepCount()
{
  int stepCount = CurieImu.getStepCount(); // set stepCount to read stepCount from function
  if (stepCount != lastStepCount) { // if stepCount has changed
    Serial.print("Step count: "); Serial.println(stepCount);
    lastStepCount = stepCount;
  }
}
