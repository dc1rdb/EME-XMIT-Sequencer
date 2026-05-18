/*
transmit sequencer for Arduino Uno with Arduino relay shield 
using polling and non-blocking state machine (no ISR, no delays)
PTT signal directly from WSJT-X via USB (RTS), requires wiring between CH340G pin 13 and GPIO 2 on Arduino Uno clone board
optional external PTT signal (active low)
sequenced ANT pol change between RX and TX possible
DC1RDB
Ver 5.0 - Fully Non-Blocking (No ISR / No delay)
*/

// pin assignments
const int PTT = 2;     // PTT signal directly from WSJT-X via USB (RTS), active low
const int extPTT = 3;  // external PTT signal, active low, to be used when TXing without WSJT-X
const int relay1 = 4;  // PreAmp 12V supply
const int relay2 = 7;  // PA bias
const int relay3 = 8;  // XCVR PTT
const int relay4 = 12; // ANT TX/RX polarization

// Debounce timing
const unsigned long debounceDelay = 50; // 50ms for mechanical PTT switch

// Variables to track debounce state
int lastPttState = HIGH;
int lastExtPttState = HIGH;
unsigned long lastPttDebounceTime = 0;
unsigned long lastExtPttDebounceTime = 0;

// Confirmed/debounced states
bool debouncedPtt = HIGH;
bool debouncedExtPtt = HIGH;

// Sequencer States
enum SequenceState {
  STATE_RX,
  STATE_TX_STEP1,
  STATE_TX_STEP2,
  STATE_TX,
  STATE_RX_STEP1,
  STATE_RX_STEP2
};

SequenceState currentState = STATE_RX;
unsigned long sequenceTimestamp = 0;
const unsigned long stepDelay = 50; // 50ms interval between steps

void setup() {
  pinMode(PTT, INPUT_PULLUP); // Ensure pullup if not using external 10k
  pinMode(extPTT, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Initial RX State: PreAmp on, PA off, PTT off, Ant RX
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  
  // Blocking delay is kept only here as it runs once during power-up stabilization
  delay(3000); 
}

void loop() {
  // 1. READ INPUTS
  int readingPtt = digitalRead(PTT);
  int readingExtPtt = digitalRead(extPTT);

  // 2. DEBOUNCE WSJT-X PTT
  if (readingPtt != lastPttState) {
    lastPttDebounceTime = millis();
  }
  if ((millis() - lastPttDebounceTime) > debounceDelay) {
    debouncedPtt = readingPtt;
  }
  lastPttState = readingPtt;

  // 3. DEBOUNCE EXTERNAL PTT
  if (readingExtPtt != lastExtPttState) {
    lastExtPttDebounceTime = millis();
  }
  if ((millis() - lastExtPttDebounceTime) > debounceDelay) {
    debouncedExtPtt = readingExtPtt;
  }
  lastExtPttState = readingExtPtt;

  // 4. DETERMINE TARGET STATE
  bool needTX = (debouncedPtt == LOW || debouncedExtPtt == LOW);

  // 5. NON-BLOCKING SEQUENCER STATE MACHINE
  unsigned long currentMillis = millis();

  switch (currentState) {
    
    case STATE_RX:
      if (needTX) {
        // Start TX sequence immediately
        digitalWrite(relay1, LOW);  // 1. PreAmp off
        digitalWrite(relay4, HIGH); // 2. Switch to TX ANT
        sequenceTimestamp = currentMillis;
        currentState = STATE_TX_STEP1;
      }
      break;

    case STATE_TX_STEP1:
      if (currentMillis - sequenceTimestamp >= stepDelay) {
        digitalWrite(relay2, HIGH); // 3. PA bias on
        sequenceTimestamp = currentMillis;
        currentState = STATE_TX_STEP2;
      }
      break;

    case STATE_TX_STEP2:
      if (currentMillis - sequenceTimestamp >= stepDelay) {
        digitalWrite(relay3, HIGH); // 4. XCVR PTT active
        currentState = STATE_TX;
      }
      break;

    case STATE_TX:
      if (!needTX) {
        // Start RX sequence immediately
        digitalWrite(relay3, LOW);  // 1. XCVR PTT inactive
        sequenceTimestamp = currentMillis;
        currentState = STATE_RX_STEP1;
      }
      break;

    case STATE_RX_STEP1:
      if (currentMillis - sequenceTimestamp >= stepDelay) {
        digitalWrite(relay2, LOW);  // 2. PA bias off
        sequenceTimestamp = currentMillis;
        currentState = STATE_RX_STEP2;
      }
      break;

    case STATE_RX_STEP2:
      if (currentMillis - sequenceTimestamp >= stepDelay) {
        digitalWrite(relay4, LOW);  // 3. Switch to RX ANT
        digitalWrite(relay1, HIGH); // 4. PreAmp on
        currentState = STATE_RX;
      }
      break;
  }
}
