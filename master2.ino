#include <FastLED.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>




#define FIRST_STRIP_NUM_LEDS 62   // Antall LED-er i den første stripen
#define SECOND_STRIP_NUM_LEDS 264 // Antall LED-er i den andre stripen
#define OTHER_STRIPS_NUM_LEDS 130 // Antall LED-er i de andre stripene (3, 4, 5, 6)
#define TOTAL_NUM_LEDS (FIRST_STRIP_NUM_LEDS + SECOND_STRIP_NUM_LEDS) // Totalt antall LED-er i første og andre stripe
#define FIRST_STRIP_DATA_PIN 13   // Pin som den første LED-stripen er koblet til
#define SECOND_STRIP_DATA_PIN 5   // Pin som den andre LED-stripen er koblet til
#define THIRD_STRIP_DATA_PIN 6    // Pin for den tredje LED-stripen
#define FOURTH_STRIP_DATA_PIN 2   // Pin for den fjerde LED-stripen
#define FIFTH_STRIP_DATA_PIN 3    // Pin for den femte LED-stripen
#define SIXTH_STRIP_DATA_PIN 4    // Pin for den sjette LED-stripen
#define MAX_PULSES 20             // Maks antall pulser som kan være aktive samtidig

float PULSE_SPEED = 10;


CRGB firstStripLeds[FIRST_STRIP_NUM_LEDS];
CRGB secondStripLeds[SECOND_STRIP_NUM_LEDS];
CRGB thirdStripLeds[OTHER_STRIPS_NUM_LEDS];
CRGB fourthStripLeds[OTHER_STRIPS_NUM_LEDS];
CRGB fifthStripLeds[OTHER_STRIPS_NUM_LEDS];
CRGB sixthStripLeds[OTHER_STRIPS_NUM_LEDS];
int pulsePositions[MAX_PULSES];
int numPulses = 0;

#define THUNDER_STRIP_NUM_LEDS 120 // Antall LEDer for torden-effekt-stripa
#define THUNDER_STRIP_DATA_PIN 7   // Datapin for torden-effekt-stripa
CRGB thunderStripLeds[THUNDER_STRIP_NUM_LEDS];

unsigned long lastUpdate = 0;
const long updateInterval = 40; // Oppdater hver 10ms, juster etter behov



// Parametere for syvende LED-stripe (sky)
const int numLEDs8 = 120;  // Antall LEDer på den nye stripen
const int ledPin8 = 7;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip8 = Adafruit_NeoPixel(numLEDs8, ledPin8, NEO_GRB + NEO_KHZ800);

// Parametere for kontroller-ledstripe
const int numLEDs1 = 25;  
const int ledPin1 = 12;    
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(numLEDs1, ledPin1, NEO_GRB + NEO_KHZ800);


const int pinB = 9;//gul
const int pinA = 11;//lilla
Encoder myEncoder(pinA, pinB);
long oldPosition  = -999;
int encoderValue = 500;
const int maxValue = 1200;
const int minValue = 0;
const int step = 1;
const int debounceDelay = 10; // Millisekunder
unsigned long lastEncoderChange = 0;
int lastEncoderValue = -1;


unsigned long forrigeMillis = 0;  // Lagrer tiden da LED sist endret status
const long interval = 100;         // Intervallet mellom endringer (100 millisekunder)
bool isLedOn = false;

enum State {
  SHOW_LIGHTNING,
  WAIT_NEXT_LIGHTNING
};

State currentState = WAIT_NEXT_LIGHTNING;
unsigned long lastUpdateTime = 0;
unsigned long waitDuration = 400;

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, FIRST_STRIP_DATA_PIN>(firstStripLeds, FIRST_STRIP_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, SECOND_STRIP_DATA_PIN>(secondStripLeds, SECOND_STRIP_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, THIRD_STRIP_DATA_PIN>(thirdStripLeds, OTHER_STRIPS_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, FOURTH_STRIP_DATA_PIN>(fourthStripLeds, OTHER_STRIPS_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, FIFTH_STRIP_DATA_PIN>(fifthStripLeds, OTHER_STRIPS_NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, SIXTH_STRIP_DATA_PIN>(sixthStripLeds, OTHER_STRIPS_NUM_LEDS);
  

  
  FastLED.addLeds<NEOPIXEL, THUNDER_STRIP_DATA_PIN>(thunderStripLeds, THUNDER_STRIP_NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  randomSeed(analogRead(0)); // For å forbedre tilfeldigheten

  
  memset(pulsePositions, -1, sizeof(pulsePositions));
  pulsePositions[0] = 0;
  numPulses = 1;

  strip8.begin();
  strip8.show(); 
  strip1.begin();
  strip1.show();
  randomSeed(analogRead(0)); // For å forbedre tilfeldigheten

}


void knappLEDs(int value) {
  int numPairsLit = map(value, 0, maxValue, 0, numLEDs1 / 2);

  for (int i = 0; i < numLEDs1; i++) {
    int currentLED = (8 - i + numLEDs1) % numLEDs1; // Starter på indeks 8 og går bakover

    // Hopper over LED 1 (indeks 0)
    if (currentLED == 0) {
      continue;
    }

    // Setter farge på LEDene basert på numPairsLit
    if (i < numPairsLit * 2) {
      // Spesiell håndtering for LED 25 (indeks 24)
      if (currentLED == 24) {
        strip1.setPixelColor(24, strip1.Color(255, 0, 0)); // Tennes LED 25
        strip1.setPixelColor(23, strip1.Color(255, 0, 0)); // Tennes LED 24
        i++; // Øker i for å hoppe over behandling av LED 24 (da den allerede er tent)
      }
      // Håndtering for par av LEDer (utenom LED 25 og 24)
      else if (currentLED % 2 == 0) {
        strip1.setPixelColor(currentLED, strip1.Color(255, 0, 0)); // Tennes nåværende LED
        strip1.setPixelColor(currentLED - 1, strip1.Color(255, 0, 0)); // Tennes forrige LED
        i++; // Øker i for å hoppe over behandling av forrige LED (da den allerede er tent)
      } 
      else {
        strip1.setPixelColor(currentLED, strip1.Color(255, 0, 0)); // Rød farge for andre LEDer
      }
    } else {
      strip1.setPixelColor(currentLED, strip1.Color(0, 0, 0)); // Slukker LEDene som ikke er en del av numPairsLit
    }
  }

  strip1.show();
}










void updateLeds() {
 // Nullstill alle LED-ene
  fill_solid(firstStripLeds, FIRST_STRIP_NUM_LEDS, CRGB::Black);
  fill_solid(secondStripLeds, SECOND_STRIP_NUM_LEDS, CRGB::Black);
  fill_solid(thirdStripLeds, OTHER_STRIPS_NUM_LEDS, CRGB::Black);
  fill_solid(fourthStripLeds, OTHER_STRIPS_NUM_LEDS, CRGB::Black);
  fill_solid(fifthStripLeds, OTHER_STRIPS_NUM_LEDS, CRGB::Black);
  fill_solid(sixthStripLeds, OTHER_STRIPS_NUM_LEDS, CRGB::Black);

  for (int i = 0; i < numPulses; i++) {
    int pos = pulsePositions[i];
    if (pos < FIRST_STRIP_NUM_LEDS) {
      firstStripLeds[pos] = CRGB(173, 216, 230);
    } else if (pos < TOTAL_NUM_LEDS) {
      // Baklengs bevegelse for den andre stripen
      int secondPos = TOTAL_NUM_LEDS - 1 - pos;
      secondStripLeds[secondPos] = CRGB(173, 216, 230);
    } else {
      int otherStripPos = pos - TOTAL_NUM_LEDS;
      if (otherStripPos < OTHER_STRIPS_NUM_LEDS) {
        thirdStripLeds[otherStripPos] = CRGB(173, 216, 230);
        fourthStripLeds[otherStripPos] = CRGB(173, 216, 230);
        fifthStripLeds[otherStripPos] = CRGB(173, 216, 230);
        sixthStripLeds[otherStripPos] = CRGB(173, 216, 230);
      }
    }
  }

  FastLED.show();

}

void addNewPulse() {
  if (numPulses < MAX_PULSES) {
    pulsePositions[numPulses++] = 0; // Legger til en ny puls ved begynnelsen av den første stripen
  }
}




void timerIsr() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case SHOW_LIGHTNING:
      if (currentMillis - lastUpdateTime > waitDuration) {
        // Slukk lynet
        for(int i = 0; i < numLEDs8; i++) {
          strip8.setPixelColor(i, strip8.Color(0, 0, 0));
        }
        strip8.show();
        waitDuration = random(100, 500); // Tilfeldig forsinkelse før neste lyn
        lastUpdateTime = currentMillis;
        currentState = WAIT_NEXT_LIGHTNING;
      }
      break;

    case WAIT_NEXT_LIGHTNING:
      if (currentMillis - lastUpdateTime > waitDuration) {
        // Slukk alle piksler for å starte med en 'ren' sky
        for(int i = 0; i < numLEDs8; i++) {
          strip8.setPixelColor(i, strip8.Color(0, 0, 0));
        }
        
        // Tegn et antall tilfeldige lyn
        int lightningCount = random(2, 6); // Bestem antall lyn
        for (int j = 0; j < lightningCount; j++) {
          int randomPixel = random(numLEDs8);
          int randomLength = random(1, 6); // Lyn fra 1 til 5 piksler i lengde
          for (int i = 0; i < randomLength; ++i) {
            int pixel = (randomPixel + i) % numLEDs8; // Sørg for at vi ikke går utenfor striplengden
            strip8.setPixelColor(pixel, strip8.Color(255, 255, 255)); // Hvit farge for lyn
          }
        }
        strip8.show();
        waitDuration = random(50, 150); // Hvor lenge lynet vises
        lastUpdateTime = currentMillis;
        currentState = SHOW_LIGHTNING;
      }
      break;
  }
}


void setStripColor(byte red, byte green, byte blue) {
  for(int i = 0; i < strip8.numPixels(); i++) {
    strip8.setPixelColor(i, strip8.Color(red, green, blue));
  }
  strip8.show();
}

void loop() {
   //rotary encoder
  int currentEncoderValue = myEncoder.read(); // Leser den nåværende verdien
  
  unsigned long currentMillis = millis();
  if (currentEncoderValue != lastEncoderValue && currentMillis - lastEncoderChange > debounceDelay) {
        // Behandle endringen
        lastEncoderChange = currentMillis;
        lastEncoderValue = currentEncoderValue;

    long newPosition = myEncoder.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      encoderValue = myEncoder.read();
      encoderValue = constrain(encoderValue, minValue, maxValue);
      
      knappLEDs(encoderValue);
      Serial.println(encoderValue);
  }
}
  if (encoderValue > 50){
    if (encoderValue > 50 && encoderValue <= 100){
      PULSE_SPEED = 10;
    }else if (encoderValue > 100 && encoderValue <= 200){
      PULSE_SPEED = 5;
    }else if (encoderValue > 200 && encoderValue <= 300){
      PULSE_SPEED = 3;
    }else if (encoderValue > 300 && encoderValue <= 400){
      PULSE_SPEED = 2;
    }else if (encoderValue > 400 && encoderValue <= 500){
      PULSE_SPEED = 1;
    }else if (encoderValue > 500 && encoderValue <= 600){
      PULSE_SPEED = 0.8;
    }else if (encoderValue > 600 && encoderValue <= 700){
      PULSE_SPEED = 0.6;
    }else if (encoderValue > 700 && encoderValue <= 800){
      PULSE_SPEED = 0.5;
    }else if (encoderValue > 800 && encoderValue <= 900){
      PULSE_SPEED = 0.4;
    }else if (encoderValue > 900 && encoderValue <= 1000){
      PULSE_SPEED = 0.3;
    }else if (encoderValue > 1000 && encoderValue <= 1100){
      PULSE_SPEED = 0.2;
    }else if (encoderValue > 1100 && encoderValue <= 1200){
      PULSE_SPEED = 0.1;
    }
    //Serial.println(PULSE_SPEED);

    unsigned long currentMillis = millis();
    static unsigned long lastPulseUpdate = 0;
    unsigned long naaMillis = millis();



    if (currentMillis - lastPulseUpdate > PULSE_SPEED) {
      lastPulseUpdate = currentMillis;

      for (int i = 0; i < numPulses; i++) {
        pulsePositions[i]++; // Flytt hver puls ett steg
        if (pulsePositions[i] >= TOTAL_NUM_LEDS + OTHER_STRIPS_NUM_LEDS) {
          pulsePositions[i] = pulsePositions[--numPulses]; // Fjern fullførte pulser
          i--; // Juster løkkeindeksen etter fjerning

          for(int i = 0; i < 3; i++) {
              if(naaMillis - forrigeMillis >= interval) {
                // Oppdater tidspunktet for siste endring
                forrigeMillis = naaMillis;

                // Sjekk om LED-stripens farge skal endres
                if (isLedOn) {
                  setStripColor(0, 0, 0);  // Skru av LED-stripen
                  isLedOn = false;
                } else {
                  setStripColor(255, 255, 255); // Skru på LED-stripen (hvit farge)
                  isLedOn = true;
                }
              }
            }

            Timer1.initialize(50000);          // Sett opp timer til å avbryte hver 50 millisekunder
            //Timer1.attachInterrupt(timerIsr);  // Koble til interrupt-rutinen
      }
      
        }
      }


      // Legger alltid til en ny puls når en eksisterende puls forlater den første stripen
      if (numPulses < MAX_PULSES && pulsePositions[numPulses - 1] > FIRST_STRIP_NUM_LEDS) {
        addNewPulse();
      }

    unsigned long oppdateringsMillis = millis();
      if (oppdateringsMillis - lastUpdate > updateInterval) {
          lastUpdate = oppdateringsMillis;
          updateLeds();
      }
    
  }
  
}



