/*
transmit sequencer for Arduino Uno with Arduino relay shield 
using ISR for PTT input
PTT signal directly from WSJT-X via USB (RTS), requires wiring between CH340G pin 13 and GPIO 2 on Arduino Uno clone board
optional external PTT signal (active low)
sequenced ANT pol change between RX and TX possible
DC1RDB
Ver 3.3 - Mar 25, 2026
*/

//pin assignments
int PTT     =  2;  // PTT signal directly from WSJT-X via USB (RTS), active low, assigned to INT0
int extPTT  =  3;  // external PTT signal, active low, to be used when TXing without WSJT-X, assigned to INT1
int relay1  =  4;  // fixed on relay shield
int relay2  =  7;  // fixed on relay shield
int relay3  =  8;  // fixed on relay shield
int relay4  =  12; // fixed on relay shield

volatile int state1 = HIGH; // LOW == TX, HIGH == RX
volatile int state2 = HIGH; // LOW == TX, HIGH == RX
  
void setup()  {

pinMode(PTT, INPUT);           //PTT input
pinMode(extPTT, INPUT_PULLUP); //ext PTT input
pinMode(relay1, OUTPUT);       //PreAmp 12V supply
pinMode(relay2, OUTPUT);       //PA bias
pinMode(relay3, OUTPUT);       //XCVR PTT
pinMode(relay4, OUTPUT);       //ANT TX/RX polarization

digitalWrite(relay1, LOW);     //init all relays off
digitalWrite(relay2, LOW);
digitalWrite(relay3, LOW);
digitalWrite(relay4, LOW);

attachInterrupt(0, pttChange, CHANGE);
attachInterrupt(1, extpttChange, CHANGE);

delay(3000);                      // suppress power-on transients
}


// MAIN CODE
void loop() {

    // sequence to transmit
    if ((state1 == LOW) || (state2 == LOW)){
      digitalWrite(relay1, LOW);  // PreAmp off, RX/TX relay to TX
      digitalWrite(relay4, HIGH); // switch to TX ANT pol
      delay(50);
      digitalWrite(relay2, HIGH); // PA bias on
      delay(50);
      digitalWrite(relay3, HIGH); // XCVR PTT active   
    }
  
    // sequence to receive
    if ((state1 == HIGH) && (state2 == HIGH)){
      digitalWrite(relay3, LOW);  // XCVR PTT inactive
      delay(50);
      digitalWrite(relay2, LOW);  // PA bias off
      delay(50);
      digitalWrite(relay4, LOW);  // switch to RX ANT pol
      digitalWrite(relay1, HIGH); // PreAmp on, RX/TX relay to RX
    }
}

 // PTT ISRs
void pttChange()
{
  int newstate1 = digitalRead(PTT);
 
  if (newstate1 == LOW) {
    delay(10); // debounce
    newstate1 = digitalRead(PTT);
    if (newstate1 == LOW) {
      state1 = LOW;
    }
  }

  if (newstate1 == HIGH) {
    delay(10); // debounce
    newstate1 = digitalRead(PTT);
    if (newstate1 == HIGH) {
      state1 = HIGH;
    }
  }
}

void extpttChange()
{
  int newstate2 = digitalRead(extPTT);
 
  if (newstate2 == LOW) {
    delay(10); // debounce
    newstate2 = digitalRead(extPTT);
    if (newstate2 == LOW) {
      state2 = LOW;
    }
  }

  if (newstate2 == HIGH) {
    delay(10); // debounce
    newstate2 = digitalRead(PTT);
    if (newstate2 == HIGH) {
      state2 = HIGH;
    }
  }
}  
