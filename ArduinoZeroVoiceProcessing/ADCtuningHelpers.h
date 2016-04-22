/*
Arduino Zero voice processing
Hardware required:
* Arduino Zero
* 2 x 10 kOhm potentiometer
* 1 Ohm resistor
* 1 uF electrolytic capacitor
* 1 female audio jack
* 1 microphone module (i.e https://www.sparkfun.com/products/9964)
* micro USB cable.
* Breadboard
* Breadboard jumpers
* PC speakers


created Sept 2015
by Angelo Scialabba <support@arduino.cc>
modified Dec 2015
by Arturo Guadalupi <a.guadalupi@arduino.cc>

This example code is in the public domain
http://arduino.cc/en/Tutorial/ArduinoZeroVoiceProcessing
 
*/
/**WARNING: do not directly connect the audio output to a non-amplified speaker or headphones**/

void increaseADCfrequency()                        // this routine change the default configuration of ADC and DAC. It is needed to increase the sampling frequency.
{
  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

  ADC->CTRLB.reg |= ADC_CTRLB_PRESCALER_DIV8;      // Divide Clock by 8.

  ADC->SAMPCTRL.reg = 0x3F;                        // Set max Sampling Time Length

  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains
}
