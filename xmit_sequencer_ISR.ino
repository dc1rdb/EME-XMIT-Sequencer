/*
transmit sequencer for Arduino Uno with Arduino relay shield 
using ISR for PTT input
PTT signal directly from WSJT-X via USB (RTS), requires wiring between CH340G pin 13 and GPIO 2 on Arduino Uno clone board
optional external PTT signal (active low)
sequenced ANT pol change between RX and TX possible
DC1RDB
Ver 3.6 - Mar 26, 2026
*/

//pin assignments
const int PTT = 2;     // PTT signal directly from WSJT-X via USB (RTS), active low, assigned to INT0
const int extPTT = 3;  // external PTT signal, active low, to be used when TXing without WSJT-X, assigned to INT1
const int relay1 = 4;  // PreAmp 12V supply
const int relay2 = 7;  // PA bias
const int relay3 = 8;  // XCVR PTT
const int relay4 = 12; // ANT TX/RX polarization

// Volatile variables used in ISR
volatile bool pttState = HIGH;
volatile bool extPttState = HIGH;

// Debounce timing
const unsigned long debounceDelay = 50; // Increased to 50ms for mechanical PTT switch
volatile unsigned long lastExtPttMicros = 0; 

// System State
bool isTransmitting = false;

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

  // Attach interrupts to CHANGE to catch both press and release for debouncing
  attachInterrupt(digitalPinToInterrupt(PTT), pttISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(extPTT), extpttISR, CHANGE);
  
  delay(3000); // Safe delay for power-on transients
}

void loop() {
  // Read volatile variables safely
  noInterrupts();
  bool currentPtt = pttState;
  bool currentExtPtt = extPttState;
  interrupts();

  // Determine needed state based on either input
  bool needTX = (currentPtt == LOW || currentExtPtt == LOW);

  if (needTX && !isTransmitting) {
    // --- SEQUENCE TO TRANSMIT ---
    digitalWrite(relay1, LOW);  // 1. PreAmp off
    digitalWrite(relay4, HIGH); // 2. Switch to TX ANT
    delay(50);                  // Relay travel time
    digitalWrite(relay2, HIGH); // 3. PA bias on
    delay(50);                  // PA stabilization time
    digitalWrite(relay3, HIGH); // 4. XCVR PTT active
    isTransmitting = true;
  } 
  else if (!needTX && isTransmitting) {
    // --- SEQUENCE TO RECEIVE ---
    digitalWrite(relay3, LOW);  // 1. XCVR PTT inactive
    delay(50);                  // Let PA settle
    digitalWrite(relay2, LOW);  // 2. PA bias off
    delay(50);                  // Relay travel time
    digitalWrite(relay4, LOW);  // 3. Switch to RX ANT
    digitalWrite(relay1, HIGH); // 4. PreAmp on
    isTransmitting = false;
  }
}

// ISR for PTT (WSJT-X is usually clean/electronic, no debounce req'd)
void pttISR() {
  pttState = digitalRead(PTT);
}

// ISR for external PTT with debounce
void extpttISR() {
  // Get current time in microseconds for higher precision in ISR
  unsigned long currentTime = micros();
  // Only update state if enough time has passed since the last change
  if (currentTime - lastExtPttMicros > (debounceDelay * 1000)) {
    extPttState = digitalRead(extPTT);
    lastExtPttMicros = currentTime;
  }
}

