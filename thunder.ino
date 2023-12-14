#include <Adafruit_NeoPixel.h>

#define PIN 3
#define NUMPIXELS 120

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0)); // For å forbedre tilfeldigheten
}

void loop() {
  // Slukk alle piksler for å starte med en 'ren' sky
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  
  // Tegn et antall tilfeldige lyn
  int lightningCount = random(2, 6); // Bestem antall lyn
  for (int j = 0; j < lightningCount; j++) {
    int randomPixel = random(NUMPIXELS);
    int randomLength = random(1, 6); // Lyn fra 1 til 5 piksler i lengde
    for (int i = 0; i < randomLength; ++i) {
      int pixel = (randomPixel + i) % NUMPIXELS; // Sørg for at vi ikke går utenfor striplengden
      strip.setPixelColor(pixel, strip.Color(255, 255, 255)); // Hvit farge for lyn
    }
  }

  strip.show();
  delay(random(50, 150)); // Hvor lenge lynet vises
  
  // Slukk lynet
  for(int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  
  strip.show();
  delay(random(100, 500)); // Tilfeldig forsinkelse før neste lyn
}
