#include <math.h> // Math functions

// 74HC4067 control pins
#define S0 PA9
#define S1 PA8
#define S2 PB15
#define S3 PB14
#define MUX_OUT PB13  // Mux common output

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

int ROW[11] = {ROW0, ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8, ROW9, ROW10}; // Row pin array

// Column-to-channel truth table (S3 S2 S1 S0)
const byte COL_CHANNELS[12] = {0b0000, 0b0001, 0b0010, 0b0011,
                               0b0100, 0b0101, 0b0110, 0b0111,
                               0b1000, 0b1001, 0b1010, 0b1011};


void setMuxChannel(byte channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, channel & 0x02);
  digitalWrite(S2, channel & 0x04);
  digitalWrite(S3, channel & 0x08);
}

void setup() {
  Serial3.begin(115200);

  // Init mux control pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  // Configure mux output pin
  pinMode(MUX_OUT, OUTPUT);
  

  // Init row pins
  for (int x = 0; x < 11; x++) {
    pinMode(ROW[x], INPUT_PULLDOWN);
  }
}

void loop() {
  // Column scan loop (12 columns)
  for (int col = 0; col < 12; col++) {
    setMuxChannel(COL_CHANNELS[col]);  // Select channel from map
    digitalWrite(MUX_OUT, HIGH);
    delayMicroseconds(100);            // Signal settle time
    // Row scan loop
    for (int row = 0; row < 11; row++) {
      if (digitalRead(ROW[row]) == HIGH) {  // Key detect
        Serial3.print("Pressed: Row ");
        Serial3.print(row);
        Serial3.print(", Col ");
        Serial3.println(col);

      }
    }
   digitalWrite(MUX_OUT, LOW);
  }
  
  delay(1000);
}
