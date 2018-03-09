#include <NeoPixelBus.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <MIDI.h>
#include "midi_notes.h"

//NeoPixel Setup
const uint16_t PixelCount = 12;
const uint8_t PixelPin = 9;

#define brightness 0.5 // Between 0 and 1

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// Set rgb colours
RgbColor red(255 * brightness, 0, 0);
RgbColor green(0, 255 * brightness, 0);
RgbColor blue(0, 0, 255 * brightness);
RgbColor white(255 * brightness);
RgbColor black(0);

// Button layout for Hex panel
//     01   12
//      02 11
//  03 04   09 10
//      05 07
//     06   08

//  Order corrected for use in indexing arrays
int order[12] = {0, 11, 1, 10, 2, 3, 8, 9, 4, 6, 5, 7};

byte colors[12][3] = {{128, 0, 0}, {96, 32, 0}, {64, 64, 0}, {32, 96, 0}, {0, 128, 0}, {0, 96, 32}, {0, 64, 64}, {0, 32, 96}, {0, 0, 128}, {32, 0, 96}, {64, 0, 64}, {96, 0, 32}};
  //MPR121 Setup
  Adafruit_MPR121 cap = Adafruit_MPR121();
  // Keeps track of the last pins touched
  // so we know when buttons are 'released'
  uint16_t lasttouched = 0;
  uint16_t currtouched = 0;


  //MIDI Setup
  int* song[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5};
  int midiChannel[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // midi channel for each button
  int instruments[] = {102, 999, 999, 999, 999, 999, 999, 999, 999, 999 /*Drums*/, 999, 999, 999, 999, 999, 999};
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

  void setup() {

    MIDI.begin();
    Serial.begin(115200);  // needed for hairless midi
    //while (!Serial); // wait for serial attach
    delay(2000);

    //  Serial.println();
    //  Serial.println("Initializing...");
    //  Serial.flush();
    if (!cap.begin(0x5A)) {
      Serial.println("MPR121 not found, check wiring?");
      while (1);
    }
    strip.Begin();
    strip.Show();
    //  Serial.println();
    //  Serial.println("Running...");

    for (int led = 0; led < PixelCount; led++) {
      delay(100);
      strip.SetPixelColor(led, RgbColor(colors[led][0], colors[led][1], colors[led][2]));
      strip.Show();
      // strip.SetPixelColor(i, black);
      //  Serial.println("Set LED blue");
    }
    MIDIsoftreset();  // Midi Reset
    delay(200);
    for (uint8_t i = 0; i < 16; i++) {  // Set instruments for all 16 MIDI channels
      if (instruments[i] < 128) {
        MIDI.sendProgramChange(instruments[i], i + 1);
      }
    }
  }

  void loop() {
    currtouched = cap.touched();

    for (uint8_t i = 0; i < 12; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        MIDI.sendNoteOn(song[i], 100, midiChannel[i]);
        turnOnLED(i);
        //      Serial.print("Pressed ");
        //      Serial.println (i);
      }
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        MIDI.sendNoteOff(song[i], 100, midiChannel[i]);
        turnOffLEDs(i);
        //      Serial.print("Released ");
        //      Serial.println (i);
      }
    }
    lasttouched = currtouched;
    delay(1);
  }

  void turnOnLED( int led ) {
    strip.SetPixelColor(led, white);
    strip.Show();
    delay(10);
  }
  void turnOffLEDs( int led ) {
strip.SetPixelColor(led, RgbColor(colors[led][0], colors[led][1], colors[led][2]));
    strip.Show();
    delay(10);
  }

  void MIDIsoftreset()  // switch off ALL notes on channel 1 to 16
  {
    for (int channel = 0; channel < 16; channel++)
    {
      MIDI.sendControlChange(123, 0, channel);
    }
  }
