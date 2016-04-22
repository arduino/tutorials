/*
  Arduino Zero voice processing
  Hardware required:
  Arduino Zero
  2 x 10 kOhm potentiometer
  1 Ohm resistor
  1 uF electrolytic capacitor
  1 female audio jack
  1 microphone module (i.e https://www.sparkfun.com/products/9964)
  micro USB cable.
  Breadboard
  Breadboard jumpers
  PC speakers

  This example uses the ring modulation technique. For more informations about this type of modulation refer to:
  https://en.wikipedia.org/wiki/Ring_modulation

  created Sept 2015
  by Angelo Scialabba <support@arduino.cc>
  modified Dec 2015
  by Arturo Guadalupi <a.guadalupi@arduino.cc>

  This example code is in the public domain
  http://arduino.cc/en/Tutorial/ArduinoZeroVoiceProcessing

*/
/**WARNING: do not directly connect the audio output to a non-amplified speaker or headphones**/

#include "ADCtuningHelpers.h"

const int numberOfSamples = 1000;
double sample, sineIndex, inputAverageValue;
unsigned long counter47k6Hz, counter2kHz;
const unsigned long timeout47k6Hz = 21;
const unsigned long timeout2kHz = 500;
int ringFrequency;

void setup() {
  // ADC and DAC configuration
  increaseADCfrequency();
  analogWriteResolution(10);
  analogReadResolution(12);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  /*Initialize variables*/
  counter47k6Hz = 0;
  counter2kHz = 0;
  sineIndex = 0;
  inputAverageValue = 0;
  ringFrequency = 0;

  // compute  microphone input average value used to remove bias from the input signal
  for (int i = 0; i < numberOfSamples; i++) {
    inputAverageValue += analogRead(A1) / 6;                                            // the /6 has been found empirically. You can experiment changing it, however if removed the voice will sound too much distorted since it defines the output volume
  }
  inputAverageValue = inputAverageValue / numberOfSamples;
}

void loop() {

  if (check47k6Hz()) {
    sample = analogRead(A1) / 6 - inputAverageValue;                                    // sample microphone output and removing DC component. The /6 has been found empirically. You can experiment changing it, however if removed the voice will sound too 
                                                                                        // much distorted since it defines the output volume
    sample = sample * sin(2 * PI * sineIndex / (1 + ringFrequency)) + inputAverageValue;// processing sample with ring modulation.
    sineIndex++;                                                                        // sine time moves forward
    analogWrite(A0, sample);                                                            // write sample to output

  }

  if (check2kHz()) {
    ringFrequency = int(analogRead(A2) / 350) * 40 - 30;                                // the range 0-4096 of ADC is divided into 4096/350 intervals and each interval adds 40 to the ringFrequency variable
  }
}

bool check47k6Hz()
{
  if (micros() - counter47k6Hz > timeout47k6Hz) {
    counter47k6Hz = micros();                                                           // refresh counter
    return true;
  }
  else
    return false;
}

bool check2kHz()
{
  if (micros() - counter2kHz > timeout2kHz) {
    counter2kHz = micros();                                                             // update the ring frequency parameter with the value set by potentiometer
    return true;
  }
  else
    return false;
}
