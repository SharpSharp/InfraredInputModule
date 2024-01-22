#define commandCode0  0x19
#define commandCode1  0x45
#define commandCode2  0x46
#define commandCode3  0x47
#define commandCode4  0x44
#define commandCode5  0x40
#define commandCode6  0x43
#define commandCode7  0x7
#define commandCode8  0x15
#define commandCode9  0x9
#define commandCode10 0x16
#define commandCode11 0xD
#define commandCode12 0x18
#define commandCode13 0x5A
#define commandCode14 0x52
#define commandCode15 0x8

#define commandCodeAllLow 0x1C

#include <EEPROM.h>
const byte NUMBER_OF_OUTPUTS = 16;
#define DEBOUNCE 500 // button long press dealy in milli seconds.

#define IR_RECEIVE_PIN  12
#define LEARNING_PIN     11
#define USE_EEPROM_ADDRESS 128
#define USE_EEPROM_VALUE   128
byte learningMode = 0;

#include <IRremote.hpp>

struct IrInputOutput
{
  byte pin; 
  byte IrCommand;
  unsigned long repeatDelay;

  void setup() {
    if (learningMode == 0) 
    {
      pinMode(pin, OUTPUT);
      if (EEPROM.read(USE_EEPROM_ADDRESS) == USE_EEPROM_VALUE)
        readDataFromEEPROM();
    }
    else
    {
      pinMode(pin, INPUT_PULLUP);
    }
  }
  void outputHigh() {
    digitalWrite(pin, HIGH);
  }
  void outputLow() {
    digitalWrite(pin, LOW);
  }
  void writeDataToEEPROM() {
    EEPROM.write(pin, IrCommand);
  }
  void readDataFromEEPROM() {
    IrCommand = EEPROM.read(pin);
  }
  void learnCommandCode() {
    // if this pin is low, update the recived command code to the pin
    if (digitalRead(pin) == LOW)
    {
      IrCommand = IrReceiver.decodedIRData.command;
      digitalWrite(LED_BUILTIN, HIGH);   
      delay(1000);                       
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  void changeOutput() {
    if (IrReceiver.decodedIRData.command == IrCommand)
    {
      // a non blocking delay to prevent fast toggling of the pin
      if (millis() > repeatDelay) 
      {
        repeatDelay = millis() + DEBOUNCE;
        // toggle the pin value
        if (digitalRead(pin) == LOW)  outputHigh();
        else                          outputLow();
      }
    }
  }
};

IrInputOutput IrOutputs[NUMBER_OF_OUTPUTS] = {
  {0,  commandCode0 },
  {1,  commandCode1 },
  {2,  commandCode2 },
  {3,  commandCode3 },
  {4,  commandCode4 },
  {5,  commandCode5 },
  {6,  commandCode6 },
  {7,  commandCode7 },
  {8,  commandCode8 },
  {9,  commandCode9 },
  {14, commandCode10},
  {15, commandCode11},
  {16, commandCode12},
  {17, commandCode13},
  {18, commandCode14},
  {19, commandCode15}
};

// the onboard LED blinks during learning mode.
// this function controls the  number of blinks and frequency.
// this code is blocking, use it carefully.
void blinkLED(int blinks, int blinkRate) {
  for (int i=0; i<blinks; i++) {
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(blinkRate);                       
    digitalWrite(LED_BUILTIN, LOW); 
    delay(blinkRate);
  }
}
void setup() {
  //Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // when the LEARNING_PIN is LOW on startup the arduino is setup in learning mode
  // to do this connect the LEARNING_PIN to GND with a jumper wire
  pinMode(LEARNING_PIN, INPUT_PULLUP);
  if (digitalRead(LEARNING_PIN) == LOW) 
  {
    learningMode = 1;
    IrReceiver.begin(IR_RECEIVE_PIN);
  }else{
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  }
 
  for (int i=0; i < NUMBER_OF_OUTPUTS; i++) 
    IrOutputs[i].setup();
}

void loop() {
  // programmMode has 5 states
  // 0 not in learningMode
  // 1 learningMode initiated at startup
  // 2 learning the command codes from remote control
  // 3 writing command codes to EEPROM
  // 4 learning compete. Now restart Arduino without jumper wire.
  switch (learningMode) {
    case 0:
    // opperation mode. IR remote controls the outputs 0–9 and A0–A5
      if (IrReceiver.decode()) {
        for (int i=0; i < NUMBER_OF_OUTPUTS; i++) 
          IrOutputs[i].changeOutput();
        
        // addtional command code, this turns all output pins LOW
        if (IrReceiver.decodedIRData.command == commandCodeAllLow)
          for (int i=0; i < NUMBER_OF_OUTPUTS; i++) 
            IrOutputs[i].outputLow();

         // further commandCode responses can be added here.
         // e.g. the red button sets pins 0 to 3 HIGH
         
        IrReceiver.resume(); // Enable receiving of the next value 
      }
    break;
    
    case 1:
    // wait for jumper to be removed from LEARNING_PIN to start learnig command codes
      blinkLED(1, 500);
      if (digitalRead(LEARNING_PIN) == HIGH) 
      {
        learningMode = 2;
        blinkLED(5, 100);
      }
    break;

    case 2:
      if (IrReceiver.decode()) {
        for (int i=0; i < NUMBER_OF_OUTPUTS; i++) 
          IrOutputs[i].learnCommandCode();
        IrReceiver.resume(); // Enable receiving of the next value 
      }
      // when jumper from GND is returned to LEARNING_PIN learning is complete.
      if (digitalRead(LEARNING_PIN) == LOW) 
      {
        learningMode = 3;        
        blinkLED(5, 100);
      }
    break;

    case 3:
    // write command code data to EEPROM 
      for (int i=0; i < NUMBER_OF_OUTPUTS; i++) 
        IrOutputs[i].writeDataToEEPROM();
      // write to an address to flag that learnt codes are stored.
      // this will be checked in start up to see if there are codes stored in EEPROM
      EEPROM.write(USE_EEPROM_ADDRESS, USE_EEPROM_VALUE); 

      // data has been written to EEPROM programming is complete.
      // remove the jumper wire and restart the arduino
      learningMode = 4;
    break;
   
  }
}
