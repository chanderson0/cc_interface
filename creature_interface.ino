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

// MIDI note states
#define CREATURE_NOTE_START    60
#define CREATURE_MAX_NOTES     12
bool notesOn[CREATURE_MAX_NOTES];

void setup() {
  // Start MIDI
  midiSetup();
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
  for (int i = 0; i < 10; ++i) {
    digitalWrite(STAT1, HIGH);  
    digitalWrite(STAT2, HIGH);

    delay(100);

    digitalWrite(STAT1, LOW);  
    digitalWrite(STAT2, LOW);

    delay(100);
  }
  
  // All notes are off to start
  for(int i = 0; i < CREATURE_MAX_NOTES; ++i) {
    notesOn[i] = false;
  }

  digitalWrite(STAT1, HIGH);
  digitalWrite(STAT2, HIGH);

  // start serial with midi baudrate
  Serial.begin(31250);
}

void loop() {
  bool button1On, button2On;

  midiReceive();
  
  // Handle incoming MIDI signals here
  if (midiReady) {
    if (action == ACTION_ON) {
      digitalWrite(STAT1, HIGH);
    } else if (action == ACTION_OFF) {
      digitalWrite(STAT1, LOW);
    }
    midiReset();
  }
  
  // Example of how to send midi
  button1On = digitalRead(BUTTON1);
  button2On = digitalRead(BUTTON2);
  if (button1On)
    noteOn(0);
  else
    noteOff(0);
  
  if (button2On)
    noteOn(1);
  else
    noteOff(1);
}

void noteOn(short note) {
  // Check if outside of creature range.
  if (note < 0 || note >= CREATURE_MAX_NOTES)
    return;
    
  // Check if already on.
  if (notesOn[note])
    return;

  midiSend(MIDI_NOTE_ON, note + CREATURE_NOTE_START, MIDI_DEFAULT_VELOCITY);
  notesOn[note] = true;
}

void noteOff(short note) {
  // Check if outside of creature range.
  if (note < 0 || note >= CREATURE_MAX_NOTES)
    return;
    
  // Check if already off.
  if (!notesOn[note])
    return;

  midiSend(MIDI_NOTE_OFF, note + CREATURE_NOTE_START, MIDI_DEFAULT_VELOCITY);
  notesOn[note] = false;
}

void midiSend(byte cmd, byte data1, byte data2) {
  Serial.write(cmd);
  Serial.write(data1);
  Serial.write(data2);
}

void midiReceive() {
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

