/*
transmit sequencer for Arduino Uno with Arduino relay shield 
using ISR for PTT input
PTT signal directly from WSJT-X via USB (RTS)
requires wiring between CH340G pin 13 and GPIO 2 on Arduino Uno clone board
sequenced ANT pol change between RX and TX possible
DC1RDB
Ver 3.1 - Mar 19, 2025
*/

//pin assignments
int PTT     =  2;  // PTT signal directly from WSJT-X via USB (RTS), active low, assigned to INT0
int relay1  =  4;  // fixed on relay shield
int relay2  =  7;  // fixed on relay shield
int relay3  =  8;  // fixed on relay shield
int relay4  =  12; // fixed on relay shield

volatile int state = HIGH; // LOW == TX, HIGH == RX
  
void setup()  {

pinMode(PTT, INPUT);          //PTT input
pinMode(relay1, OUTPUT);      //PreAmp 12V supply
pinMode(relay2, OUTPUT);      //PA PTT
pinMode(relay3, OUTPUT);      //XCVR PTT
pinMode(relay4, OUTPUT);      //ANT TX/RX polarization

digitalWrite(relay1, LOW);    //init all relays off
digitalWrite(relay2, LOW);
digitalWrite(relay3, LOW);
digitalWrite(relay4, LOW);

attachInterrupt(0, pttChange, CHANGE);

delay(3000);                      // suppress power-on transients
}


// MAIN CODE
void loop() {

    if (state == LOW) {           // sequence to transmit
      digitalWrite(relay1, LOW);  // PreAmp off
      digitalWrite(relay4, HIGH); // switch to TX ANT pol
      delay(120);
      digitalWrite(relay2, HIGH); // PA on
      delay(40);
      digitalWrite(relay3, HIGH); // XCVR PTT active   
    }
  
    if (state == HIGH) {          // sequence to receive
      digitalWrite(relay3, LOW);  // XCVR PTT inactive
      delay(40);
      digitalWrite(relay2, LOW);  // PA off
      delay(120);
      digitalWrite(relay4, LOW);  // switch to RX ANT pol
      digitalWrite(relay1, HIGH); // PreAmp on
    }
}

 // PTT ISR
void pttChange()
{
  int newstate = digitalRead(PTT);
 
  if (newstate == LOW) {
    delay(10); // debounce
    newstate = digitalRead(PTT);
    if (newstate == LOW) {
      state = LOW;
    }
  }

  if (newstate == HIGH) {
    delay(10); // debounce
    newstate = digitalRead(PTT);
    if (newstate == HIGH) {
      state = HIGH;
    }
  }

}