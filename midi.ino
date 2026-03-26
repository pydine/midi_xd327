#include <math.h>  // Math functions
#include <USBComposite.h>
USBMIDI midi;
// 74HC4067 control pins
#define S0 PA9
#define S1 PA8
#define S2 PB15
#define S3 PB14
#define MUX_OUT PB13  // Mux common I/O

#define ROW0 PB1
#define ROW1 PB0
#define ROW2 PA7
#define ROW3 PA6
#define ROW4 PA5
#define ROW5 PA4
#define ROW6 PA3
#define ROW7 PA2
#define ROW8 PA1
#define ROW9 PA0
#define ROW10 PA10

int values[11][6] = {
  {0, 0, 0, 0, 0, 36},
  {37, 38, 39, 40, 41, 42},
  {43, 44, 45, 46, 47, 48},
  {49, 50, 51, 52, 53, 54},
  {55, 56, 57, 58, 59, 60},
  {61, 62, 63, 64, 65, 66},
  {67, 68, 69, 70, 71, 72},
  {73, 74, 75, 76, 77, 78},
  {79, 80, 81, 82, 83, 84},
  {85, 86, 87, 88, 89, 90},
  {91, 92, 93, 94, 95, 96}
};

#define LedPin PC13  // MIDI activity LED

#define thresholdTime_max 2400  // Max time threshold
#define constrain_min 1         // Min clamp value
#define constrain_max 200       // Max clamp value
#define log_multiplier 25       // Log multiplier for velocity

byte channel = 0;              // MIDI channel 0
byte velocityOn;               // Velocity on press
byte velocityOff;              // Velocity on release
int constrainTimeOn;           // Clamped time on press
int constrainTimeOff;          // Clamped time on release
double logTime1;               // Log time on press
double logTime2;               // Log time on release
int keyState[66];              // Key state array
int x, col, row, blk;          // Loop variables

unsigned long startTime1[66];  // Press start time array
unsigned long startTime2[66];  // Release start time array
unsigned long startTimeOn;     // Press duration
unsigned long startTimeOff;    // Release duration

int MK[6];
int BR[6];
int ROW[11] = { ROW0, ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8, ROW9, ROW10};  // Row pin array


// Column-to-channel truth table (S3 S2 S1 S0)
const byte COL_CHANNELS[12] = {0b0000, 0b0001, 0b0010, 0b0011,
                               0b0100, 0b0101, 0b0110, 0b0111,
                               0b1000, 0b1001, 0b1010, 0b1011
                              };



void setMuxChannel(byte channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, channel & 0x02);
  digitalWrite(S2, channel & 0x04);
  digitalWrite(S3, channel & 0x08);
}


void setup() {
  // Init mux control pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Configure mux output pin
  pinMode(MUX_OUT, OUTPUT);
  digitalWrite(MUX_OUT, HIGH);

  // Init row pins
  for (int x = 0; x < 11; x++) {
    pinMode(ROW[x], INPUT_PULLDOWN);
  }
  // Map COL_CHANNELS: first 6 to MK, last 6 to BR
  for (int i = 0; i < 6; i++) {
    MK[i] = COL_CHANNELS[i + 6];
  }
  for (int i = 0; i < 6; i++) {
    BR[i] = COL_CHANNELS[i];
  }
  pinMode(LedPin, OUTPUT);  // LED output
  digitalWrite(LedPin, LOW); // LED off (PC13 to 3.3V)
  // Init all states and times to 0
  for (int x = 0; x < 66; x++) {
    startTime1[x] = 0;
    startTime2[x] = 0;
    keyState[x] = 0;
  }

  USBComposite.setProductId(0x0031); // Init USB MIDI
  midi.begin();
  while (!USBComposite);
}

void loop() {
  blk = 0;  // Initial block offset
  for (col = 0; col < 6; col++) {  // Scan columns
    for (row = 0; row < 11; row++) {  // Scan rows
      scanKeys(keyState[row + col + blk]);  // Scan key state
    }
    blk += 10;  // Block offset for scan index
  }
}
  void scanKeys(int var) {
    // Handle the current key state
    switch (var) {
      case 0: // Idle state
      digitalWrite(MUX_OUT, HIGH); 
        setMuxChannel(BR[col]);  // Select Break contact
        if (digitalRead(ROW[row]) == HIGH) {  // Key press detected
          startTime1[row + col + blk] = millis();  // Record press start time
          keyState[row + col + blk] = 1;  // Next state
        }
        digitalWrite(MUX_OUT, LOW);
        break;

      case 1:
            digitalWrite(MUX_OUT, HIGH); 
        setMuxChannel(BR[col]);  // Select Break contact
        if (digitalRead(ROW[row]) == LOW) {  // Key released from Break
          keyState[row + col + blk] = 2;  // Next state
        }
        digitalWrite(MUX_OUT, LOW);
        break;

      case 2:
      digitalWrite(MUX_OUT, HIGH); 
        setMuxChannel(MK[col]);  // Select Make contact
        if (digitalRead(ROW[row]) == HIGH) {  // Make contact pressed
        startTimeOn = millis() - startTime1[row + col + blk];  // Press duration
          logTime1 = log_multiplier * log(startTimeOn);  // Log time for velocity
          if (startTimeOn > thresholdTime_max) {  // Over threshold
            keyState[row + col + blk] = 0;  // Reset state
            digitalWrite(MUX_OUT, LOW);
            break;
          }
          constrainTimeOn = constrain(logTime1, constrain_min, constrain_max);  // Clamp time
          velocityOn = map(constrainTimeOn, constrain_min, constrain_max, 127, 1);  // Map to velocity
          midi.sendNoteOn(channel, values[row][col], velocityOn);  // Send MIDI note-on
          digitalWrite(LedPin, HIGH);  // LED on for MIDI activity
          keyState[row + col + blk] = 3;  // Next state
        }
        digitalWrite(MUX_OUT, LOW);
        break;

      case 3:
      digitalWrite(MUX_OUT, HIGH);
          setMuxChannel(MK[col]);  // Select Make contact
          if (digitalRead(ROW[row]) == LOW) {  // Key released from Make
          startTime2[row + col + blk] = millis();  // Record release start time
            keyState[row + col + blk] = 4;  // Next state
          }
          digitalWrite(MUX_OUT, LOW);
        break;

      case 4:
      digitalWrite(MUX_OUT, HIGH);
          setMuxChannel(BR[col]);  // Select Break contact
          if (digitalRead(ROW[row]) == LOW) {  // Break contact returned
          startTimeOff = millis() - startTime2[row + col + blk];  // Release duration
            logTime2 = log_multiplier * log(startTimeOff);  // Log time for velocity
            constrainTimeOff = constrain(logTime2, constrain_min, constrain_max);  // Clamp time
            velocityOff = map(constrainTimeOff, constrain_max, constrain_min, 1, 127);  // Map to velocity
            midi.sendNoteOff(channel, values[row][col], velocityOff); // Send MIDI note-off
            digitalWrite(LedPin, LOW);  // LED off after note-off
            keyState[row + col + blk] = 0;  // Reset state
          }
          digitalWrite(MUX_OUT, LOW);
        break;

      default:
          break;
        }
  }
