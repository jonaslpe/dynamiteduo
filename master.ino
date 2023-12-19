#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>


// Parametere for encoder
const int pinB = 9;//gul
const int pinA = 11;//lilla
Encoder myEncoder(pinA, pinB);
long oldPosition  = -999;
int encoderValue = 0;
const int maxValue = 1200;
const int minValue = 0;
const int step = 1;

// Parametere for første LED-stripe
const int numLEDs1 = 24;  
const int ledPin1 = 12;    
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(numLEDs1, ledPin1, NEO_GRB + NEO_KHZ800);

// Parametere for andre LED-stripe (puls-stripe)
const int numLEDs2 = 35;
const int ledPin2 = 13;    
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(numLEDs2, ledPin2, NEO_GRB + NEO_KHZ800);

// Parametere for tredje LED-stripe (forlengelse av strip2, men motsatt retning)
const int numLEDs3 = 256;
const int ledPin3 = 5;    
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(numLEDs3, ledPin3, NEO_GRB + NEO_KHZ800);

// Parametere for fjerde LED-stripe (forlengelse av strip3)
const int numLEDs4 = 130;  // Antall LEDer på den nye stripen
const int ledPin4 = 6;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(numLEDs4, ledPin4, NEO_GRB + NEO_KHZ800);

// Parametere for femte LED-stripe (forlengelse av strip3)
const int numLEDs5 = 130;  // Antall LEDer på den nye stripen
const int ledPin5 = 2;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(numLEDs5, ledPin5, NEO_GRB + NEO_KHZ800);

// Parametere for sjette LED-stripe (forlengelse av strip3)
const int numLEDs6 = 130;  // Antall LEDer på den nye stripen
const int ledPin6 = 3;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip6 = Adafruit_NeoPixel(numLEDs6, ledPin6, NEO_GRB + NEO_KHZ800);

// Parametere for syvende LED-stripe (forlengelse av strip3)
const int numLEDs7 = 130;  // Antall LEDer på den nye stripen
const int ledPin7 = 4;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip7 = Adafruit_NeoPixel(numLEDs7, ledPin7, NEO_GRB + NEO_KHZ800);

// Parametere for syvende LED-stripe (sky)
const int numLEDs8 = 120;  // Antall LEDer på den nye stripen
const int ledPin8 = 7;    // Velg en ledig pin på mikrokontrolleren
Adafruit_NeoPixel strip8 = Adafruit_NeoPixel(numLEDs8, ledPin8, NEO_GRB + NEO_KHZ800);



// Variabler for pulsfunksjonen
unsigned long lastPulseTime = 0;
const int pulseLength = 5;
bool pulseOnStrip2 = true; // Boolsk variabel for å spore hvilken stripe som viser pulsen
bool pulseOnStrip3 = false; // Ny variabel for å spore pulsen på strip3
bool pulseOnStrip4 = false; // Ny variabel for å spore pulsen på strip3



enum State {
  SHOW_LIGHTNING,
  WAIT_NEXT_LIGHTNING
};

volatile State currentState = WAIT_NEXT_LIGHTNING;
volatile unsigned long lastUpdateTime = 0;
volatile unsigned long waitDuration = 0;



void setup() {
  Serial.begin(9600);
  strip1.begin();  
  strip1.show();   
  strip2.begin();  
  strip2.show();
  strip3.begin();
  strip3.show();   
  strip4.begin();
  strip4.show();   
  strip5.begin();
  strip5.show(); 
  strip6.begin();
  strip6.show(); 
  strip7.begin();
  strip7.show();
  strip8.begin();
  strip8.show(); 

  randomSeed(analogRead(0)); // For å forbedre tilfeldigheten

  
}



void loop() {
  long newPosition = myEncoder.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    encoderValue = myEncoder.read();
    encoderValue = constrain(encoderValue, minValue, maxValue);
    encoderValue = 1200;
    Serial.println(encoderValue);
  }

  // Oppdater LED-stripen basert på encoderValue
  updateLEDs(encoderValue);

  // Send puls på andre eller tredje LED-stripe basert på encoderValue
  sendPulse();
  
  delay(1);
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

void updateLEDs(int value) {
  int numPairsLit = map(value, 0, maxValue, 0, numLEDs1 / 2);

  for(int i = 0; i < numPairsLit * 2; i++) {
    strip1.setPixelColor(i, strip1.Color(255, 0, 0)); // Rød farge
  }

  for(int i = numPairsLit * 2; i < numLEDs1; i++) {
    strip1.setPixelColor(i, strip1.Color(0, 0, 0));
  }

  strip1.show();
}

void sendPulse() {
  if (millis() - lastPulseTime > map(encoderValue, minValue, maxValue, 200, 5)) {
    lastPulseTime = millis();
    pulseEffect();
  }
}

void pulseEffect() {
  static int pulsePosition2 = 0;
  static int pulsePosition3 = numLEDs3 - 1;
  static int pulsePosition4 = 0;
  static int pulsePosition5 = 0;
  static int pulsePosition6 = 0;
  static int pulsePosition7 = 0;
  

  if (pulseOnStrip2) {
    // Vis puls på strip2 og slukk strip3
    for(int i = 0; i < numLEDs2; i++) {
      strip2.setPixelColor(i, strip2.Color(0, 0, 0));
    }

    for(int i = pulsePosition2; i < pulsePosition2 + pulseLength && i < numLEDs2; i++) {
      strip2.setPixelColor(i, strip2.Color(0, 255, 0)); // Grønn farge for puls
    }

    pulsePosition2++;
    if (pulsePosition2 + pulseLength > numLEDs2) {
      pulsePosition2 = 0;
      pulseOnStrip2 = false; // Bytt til strip3 for neste puls
      pulseOnStrip3 = true; // Bytt til strip3 for neste puls
    }

    strip2.show();
    strip3.clear(); // Slukk strip3
    strip3.show();
  } else if (pulseOnStrip3) {
   
    // Vis puls på strip3 og slukk strip2
    for(int i = 0; i < numLEDs3; i++) {
      strip3.setPixelColor(i, strip3.Color(0, 0, 0));
    }

    for(int i = pulsePosition3; i > pulsePosition3 - pulseLength && i >= 0; i--) {
      strip3.setPixelColor(i, strip3.Color(0, 255, 0)); // Grønn farge for puls
    }

    pulsePosition3--;
    if (pulsePosition3 < 0) {
      pulsePosition3 = numLEDs3 - 1;
      pulseOnStrip3 = false; // Bytt til strip4 for neste puls
      
      strip2.clear(); // Slukk strip2
      strip2.show();
    }
    strip3.show();
    
  }
  else{
    
    // Når pulsen har nådd enden av strip3, fortsett på strip4
    //sendpulse() ???????
    for(int i = 0; i < numLEDs4; i++) {
      strip4.setPixelColor(i, strip4.Color(0, 0, 0));
      strip5.setPixelColor(i, strip5.Color(0, 0, 0));
      strip6.setPixelColor(i, strip6.Color(0, 0, 0));
      strip7.setPixelColor(i, strip7.Color(0, 0, 0));

    }

    for(int i = pulsePosition4; i < pulsePosition4 + pulseLength && i < numLEDs4; i++) {
      strip4.setPixelColor(i, strip4.Color(0, 255, 0)); // Grønn farge for puls
      strip5.setPixelColor(i, strip5.Color(0, 255, 0)); // Grønn farge for puls
      strip6.setPixelColor(i, strip6.Color(0, 255, 0)); // Grønn farge for puls
      strip7.setPixelColor(i, strip7.Color(0, 255, 0)); // Grønn farge for puls

    }

    pulsePosition4++;
    if (pulsePosition4 + pulseLength > numLEDs4) {
      pulsePosition4 = 0;
      pulseOnStrip2 = true; // Start pulsen på strip2 igjen
      for(int i = 0; i < 3; i++) {
        setStripColor(255, 255, 255); // Skru på LED-stripen (Rød farge)
        delay(100);               // Vent i 100 millisekunder
        setStripColor(0, 0, 0);   // Skru av LED-stripen
        delay(100);               // Vent i 100 millisekunder
  }
      Timer1.initialize(50000); // Sett opp timer til å avbryte hver 50 millisekunder
      Timer1.attachInterrupt(timerIsr); // Koble til interrupt-rutinen
    }

    strip4.show();
    strip5.show();
    strip6.show();
    strip7.show();
       
  }

  
}