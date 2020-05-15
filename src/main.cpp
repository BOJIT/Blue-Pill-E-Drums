#include <Arduino.h>
#include <USBComposite.h>

// Macros
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define THRESHOLD 1550
#define HOLDOFF 75
#define DEBUG

// Global Configuration:
int PAD_NOTES[10] = {36 , 44 , 40 , 37 , 45 , 41 , 42 , 38 , 46 , 39 };   // MIDI notes
int PAD_PINS[10] =  {PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0, PB1};   // Pad pins
bool PAD_ACTIVE[10];  // Pad status
int PAD_SAMPLE[10];   // Previous sample buffer
unsigned long PAD_HOLDOFF[10];  // Holdoff time for each pad

USBCompositeSerial CompositeSerial;
USBMIDI MIDI;

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);   // Disable JTAG Pins
  USBComposite.clear();   // Setup USB Device Properties
  USBComposite.setProductId(0x0031);
  USBComposite.setManufacturerString("BOJIT");
  USBComposite.setProductString("E-Drum Pads");

  MIDI.registerComponent();   // Register USB Endpoints
  #ifdef DEBUG
    //CompositeSerial.registerComponent();  // Adds addition serial endpoint for debugging
  #endif
  USBComposite.begin();
  while (!USBComposite); // Wait for USB Connection to be Established
}

void loop() {
  unsigned long time = millis();
  for(size_t i = 0; i < 10; i++) {
    int sample = analogRead(PAD_PINS[i]);
    //CompositeSerial.println(sample);
    if((sample > PAD_SAMPLE[i]) && (sample > THRESHOLD) && (time - PAD_HOLDOFF[i] > HOLDOFF)) { // Check for trigger based on threshold, holdoff, and slope direction
      PAD_ACTIVE[i] = true;
      PAD_HOLDOFF[i] = millis();
    }
    if((PAD_ACTIVE[i] == true) & (sample < PAD_SAMPLE[i])) {  // Wait for local peak to send note (for velocity)
      int velocity = (sample - HOLDOFF)*127/(4095 - HOLDOFF);
      MIDI.sendNoteOn(1, PAD_NOTES[i], velocity);
      PAD_ACTIVE[i] = false;
      MIDI.sendNoteOff(1, PAD_NOTES[i], 0);
    }
    PAD_SAMPLE[i] = sample; // Cache sample for next measurement
  }
}