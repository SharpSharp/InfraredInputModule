//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
//#define DECODE_LG
//#define DECODE_NEC          // Includes Apple and Onkyo
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6

//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER
//#define DECODE_FAST

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

#define IR_RECEIVE_PIN      12
const byte NUMBER_OF_OUTPUTS = 16;
#define DEBOUNCE 1000 // button long press dealy in milli seconds.

#include <IRremote.hpp> // include the library

struct IrInputOutput
{
  byte pin;
  byte IrCommand;
  unsigned long repeatDelay;

  void setup()
  {
    pinMode(pin, OUTPUT);
  }

  void outputHigh()
  {
    digitalWrite(pin, HIGH);
  }

  void outputLow()
  {
    digitalWrite(pin, LOW);
  }

  void changeOutput()
  {
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

IrInputOutput IrInputOutputs[NUMBER_OF_OUTPUTS] = {
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

void setup() {
 // Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  for (int i=0; i < NUMBER_OF_OUTPUTS; i++) IrInputOutputs[i].setup();
}

void loop() {
  if (IrReceiver.decode()) 
  {
    for (int i=0; i < NUMBER_OF_OUTPUTS; i++) IrInputOutputs[i].changeOutput();

    if (IrReceiver.decodedIRData.command == commandCodeAllLow)
    {
      for (int i=0; i < NUMBER_OF_OUTPUTS; i++) IrInputOutputs[i].outputLow();
    }
    IrReceiver.resume(); // Enable receiving of the next value 
  }
}
