/*
    fauna - Media control buttons for cypress

    Date: July 27th, 2020 @ 10:52PM
    Version: 3.0
    Programmer: by ohitsdylan & TheEngineer
    Target Board: Arduino Beetle @ 5v/16MHz
    Program Software: Arduino IDE 1.8.8
    Program Device: USBasp
    Programed Chip: Atmel ATMEGA32U4

    Additional Libraies: HID-Project

    Program Scope: Monitor 3 Media keys for change in status and volume knob for changing volume.

*/

// HID-Projects includes media key (also called consumer keys) functions
#include <HID-Project.h>

#include <HID-Settings.h>

// Set readable variables for specific input pins (unsure which pin numbers are which board holes)
static int Up = 0; // RX Pin on Beetle - Hardware interrupt pin is physical pin 20 (PD2)
static int Down = 1; // TX Pin on Beetle - Hardware interrupt pin is physical pin 21 (PD3)

const int inputPrev = 9;
const int inputNext = 10;
const int inputPlayPause = 11;

volatile byte aFlag = 0; // Flag for rising edge on VolUp to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // Flag for rising edge on VolDn to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte reading = 0; //somewhere to store the direct values read from the interrupt pins before checking to see if we have moved a whole detent

void setup() {

    attachInterrupt(2, VolUp, RISING); // set an interrupt on VolUp, looking for a rising edge signal and executing the "VolUp" ISR (below)
    attachInterrupt(3, VolDn, RISING); // set an interrupt on VolDn, looking for a rising edge signal and executing the "VolDn" ISR (below)

    //Rotary encoder section of setup
    pinMode(VolUp, INPUT_PULLUP); // set VolUp as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
    pinMode(VolDn, INPUT_PULLUP); // set VolDn as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)

    // Set pins as pullup resistors
    pinMode(inputPrev, INPUT_PULLUP);
    pinMode(inputNext, INPUT_PULLUP);
    pinMode(inputPlayPause, INPUT_PULLUP);

    // Necessary to establish media key comm function
    Consumer.begin();
}

void loop() {

    if (digitalRead(inputPrev) == LOW) {
        Keyboard.write(0xD8); //Hexidecimal for left arrow key
        delay(2);
        Keyboard.releaseAll();
        while (digitalRead(inputPrev) == LOW) {
            delay(100);
        }
    }

    if (digitalRead(inputNext) == LOW) {
        Keyboard.write(0xD7); //Hexidecimal for right arrow key
        delay(2);
        Keyboard.releaseAll();
        while (digitalRead(inputNext) == LOW) {
            delay(100);
        }
    }

    if (digitalRead(inputPlayPause) == LOW) {
        Keyboard.write(0xB0); //Hexidecimal for enter/return key
        delay(2);
        Keyboard.releaseAll();
        while (digitalRead(inputPlayPause) == LOW) {
            delay(100);
        }
    }
}

void VolUp() { // ISR for Rotary Up
    detachInterrupt(2);
    detachInterrupt(3);
    reading = PIND & 0xC; // read all eight pin values then strip away all but Rotary Up and Rotary Down's values
    if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
        Consumer.write(MEDIA_VOLUME_UP);
        delay(2);
        Keyboard.releaseAll();

        bFlag = 0; //reset flags for the next turn
        aFlag = 0; //reset flags for the next turn
    } else if (reading == B00000100) bFlag = 1; //signal that we're expecting Rotary Down to signal the transition to detent from free rotation
    attachInterrupt(2, VolUp, RISING); // set an interrupt on VolUp, looking for a rising edge signal and executing the "Rotary Up" ISR (below)
    attachInterrupt(3, VolDn, RISING); // set an interrupt on VolDn, looking for a rising edge signal and executing the "Rotary Down" ISR (below)
}

void VolDn() { // ISR for Rotary Down
    detachInterrupt(2);
    detachInterrupt(3);
    reading = PIND & 0xC; //read all eight pin values then strip away all but  Rotary Up and Rotary Down's values
    if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
        Consumer.write(MEDIA_VOLUME_DOWN);
        delay(2);
        Keyboard.releaseAll();

        bFlag = 0; //reset flags for the next turn
        aFlag = 0; //reset flags for the next turn
    } else if (reading == B00001000) aFlag = 1; //signal that we're expecting VolUp to signal the transition to detent from free rotation
    attachInterrupt(2, VolUp, RISING); // set an interrupt on VolUp, looking for a rising edge signal and executing the "Rotary Up" ISR (below)
    attachInterrupt(3, VolDn, RISING); // set an interrupt on VolDn, looking for a rising edge signal and executing the "Rotary Down" ISR (below)
}
