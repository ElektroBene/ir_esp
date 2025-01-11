/*
 * SimpleSender.cpp
 *
 *  Demonstrates sending IR codes in standard format with address and command
 *  An extended example for sending can be found as SendDemo.
 *
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  MIT License
 */
#include <Arduino.h>

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

#include <IRremote.hpp> // include the library

void setup() {
    pinMode(22, OUTPUT);
    pinMode(25, INPUT);
    pinMode(32, INPUT);

    Serial.begin(9600);
    while (!Serial)
        ; // Wait for Serial to become available. Is optimized away for some cores.

    /*
     * The IR library setup. That's all!
     */
    IrSender.begin(22); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
    disableLEDFeedback(); // Disable feedback LED at default feedback LED pin

    IrSender.sendNECRaw(0x46228248, 00); // inverted on
}

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;
bool onFlag = false;
bool offFlag = false;

void loop() {

    /*
     * If you want to send a raw HEX value directly like e.g. 0xCB340102 you must use sendNECRaw()
     */
    
     Serial.println("Loop started");
    if (digitalRead(25) == HIGH && onFlag == false){
      Serial.println("Set off at 25");
      onFlag = true;
      offFlag = false;
      IrSender.sendNECRaw(0x46A88248, sRepeats); // inverted on
    }else if(digitalRead(25) == LOW){
      onFlag = false;
    }

    if (digitalRead(32) == HIGH && offFlag == false){
      Serial.println("Set on at 32");
      offFlag = true;
      onFlag = false;
      IrSender.sendNECRaw(0x46228248, sRepeats); // inverted off
    }else if(digitalRead(32) == LOW){
      offFlag = false;
    }
    
//    IrSender.sendNECRaw(0x12411562, sRepeats); // original on
//    IrSender.sendNECRaw(0x46A88248, sRepeats); // inverted on
//    IrSender.sendNECRaw(0x46228248, sRepeats); // inverted off

    /*
     * If you want to send an "old" MSB HEX value used by IRremote versions before 3.0 like e.g. 0x40802CD3 you must use sendNECMSB()
     */
//    Serial.println(F("Send old 32 bit MSB 0x12411562 with sendNECMSB()"));
//    IrSender.sendNECMSB(0x12411562, 32, sRepeats);

    /*
     * Increment send values
     */
    sCommand += 0x11;
    sRepeats++;
    // clip repeats at 4
    if (sRepeats > 4) {
        sRepeats = 4;
    }

    delay(1000);  // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal
}
