#include <Wire.h>
#include <Adafruit_MPR121.h>

// defines for midi signals
#define MIDI_NOTE_ON             0x90
#define MIDI_NOTE_OFF            0x80
#define MIDI_DEFAULT_VELOCITY    127

// defines for MIDI Shield components only
#define KNOB1     0
#define KNOB2     1
#define BUTTON1   2
#define BUTTON2   3
#define BUTTON3   4
#define STAT1     7
#define STAT2     6

// Serial data
byte incomingByte;
byte note = 0;
byte velocity = 0;
bool midiReady = false;

// Enum for state of serial
#define ACTION_OFF   1
#define ACTION_ON    2
#define ACTION_WAIT  3
int action = ACTION_WAIT;

// I2C cap board
#define NUM_PADS     12
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t capPrevTouched = 0;
uint16_t capTouched = 0;

void setup() {
  // Start MIDI
  midiSetup();
  
  // Start Cap
  if (!cap.begin(0x5A)) {
    // If we can't find the cap, blink the lights on and off
    while(true) {
      digitalWrite(STAT1, HIGH);  
      digitalWrite(STAT2, LOW);
  
      delay(30);
  
      digitalWrite(STAT1, LOW);  
      digitalWrite(STAT2, HIGH);
  
      delay(30);
    }
  }
}

void midiSetup() {
  pinMode(STAT1, OUTPUT);   
  pinMode(STAT2, OUTPUT);

  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  digitalWrite(BUTTON1, HIGH);
  digitalWrite(BUTTON2, HIGH);
  digitalWrite(BUTTON3, HIGH);
  
  // flash MIDI Sheild LEDs on startup
  for (int i = 0; i < 10; i++) {
    digitalWrite(STAT1, HIGH);  
    digitalWrite(STAT2, HIGH);

    delay(30);

    digitalWrite(STAT1, LOW);  
    digitalWrite(STAT2, LOW);

    delay(30);
  }

  digitalWrite(STAT1, HIGH);
  digitalWrite(STAT2, HIGH);

  // start serial with midi baudrate
  Serial.begin(31250);
}

void loop() {
  midiRecieve();
  
  // Handle incoming MIDI signals here
  if (midiReady) {
    if (action == ACTION_ON) {
      digitalWrite(STAT1, HIGH);
    } else if (action == ACTION_OFF) {
      digitalWrite(STAT1, LOW);
    }
    midiReset();
  }
  
  // Send MIDI notes if touch state changed
  capTouched = cap.touched();
  for (int i = 0; i < NUM_PADS; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((capTouched & _BV(i)) && !(capPrevTouched & _BV(i))) {
      midiSend(MIDI_NOTE_ON, i, MIDI_DEFAULT_VELOCITY);
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(capTouched & _BV(i)) && (capPrevTouched & _BV(i))) {
      midiSend(MIDI_NOTE_OFF, i, MIDI_DEFAULT_VELOCITY);
    }
  }
}

void midiSend(byte cmd, byte data1, byte data2) {
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void midiRecieve() {
  // Ignore if there's nothing to process
  if (Serial.available() == 0) 
    return;
    
  // read the incoming byte:
  incomingByte = Serial.read();

  // wait for as status-byte, channel 1, note on or off
  if (incomingByte == MIDI_NOTE_ON) { 
    // Note on
    action = ACTION_ON;
  } else if (incomingByte == MIDI_NOTE_OFF) { 
    // Note off
    action = ACTION_OFF;
  } else if (note == 0 && action != ACTION_WAIT) {
    // note on, wait for note value
    note = incomingByte;
  } else if (note != 0 && action != ACTION_WAIT) {
    // velocity
    velocity = incomingByte;
    midiReady = true;
  }
}

void midiReset() {
  midiReady = false;
  action = ACTION_WAIT;
  note = 0;
  velocity = 0;
}

