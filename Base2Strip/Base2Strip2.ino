/// final release december 27

#include <FastLED.h>
#include <Wire.h>

FASTLED_USING_NAMESPACE

#define DATA_PIN           12

#define LED_TYPE       WS2811
#define COLOR_ORDER       GRB
#define NUM_LEDS         1050
CRGB leds[NUM_LEDS];

#define BRIGHTNESS        200
#define FRAMES_PER_SECOND  60

#define BLUEHUE 155   //  160 
#define REDHUE   0   //   255
#define GREENHUE 90   //   96
#define PAUSE 100
/*  HUE_RED = 0, HUE_ORANGE = 32, HUE_YELLOW = 64, HUE_GREEN = 96,
    HUE_AQUA = 128, HUE_BLUE = 160, HUE_PURPLE = 192, HUE_PINK = 224
*/

boolean redState = false;
boolean blueState = false; // true
boolean greenState = false;

boolean game = false;
boolean win = false;

//boolean pixStates[1050];

int thisAddress = 21;//I2C адрес (не изменять, он прописан в Базе2).

CHSV redColor, blueColor, greenColor,
     currentBackColor, gameOffColor;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void setup()
{
  Serial.begin(115200);
  Wire.begin(thisAddress);
  Wire.onReceive(Receiver);

  delay(3000);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  randomSeed(A0);
  blueColor = CHSV(BLUEHUE, 255, 150);
  redColor = CHSV(REDHUE, 255, 150);
  greenColor = CHSV(GREENHUE, 255, 150);
  gameOffColor = CHSV(0, 0, 0);
}
void loop()
{
  if (game)
  {
   FastLED.show();  
   FastLED.delay(1000/FRAMES_PER_SECOND);
   EVERY_N_MILLISECONDS( 40 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    
    if (win) {  rainbow();  
    } else {
      if (blueState) confetti(BLUEHUE);  //мигаем синим, если он загружен
      if (redState) confetti(REDHUE);   //мигаем красным, если он загружен
      if (greenState) confetti(GREENHUE);//мигаем зеленым, если он загружен
      if (!greenState && !redState && !blueState)  juggle(); // фон начало
     }
    delay(PAUSE);
  }
  else
  {
//   setBackGrnd(gameOffColor); //глушим ленту вне игры
//  delay(500);
  }

FastLED.show();  
} // end of loop

void setBackGrnd(CHSV _color)
{
  _color.value = 0;
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = _color;
  FastLED.show();
}


void Receiver(int howMany)
{
  if (Wire.available() > 0)
  {
    byte cmd = Wire.read();

    if (cmd == 0xB1) { blueState = true; redState = false; currentBackColor = blueColor; }
    if (cmd == 0xB2) { blueState = false; redState = true; currentBackColor = redColor; }
    if (cmd == 0xB3) { greenState = true; blueState = false; redState = false; currentBackColor = greenColor; }
    if (cmd == 0xFF) { win = true;  }
    if (cmd == 0xAA) { game = true; }
  }
}

void confetti(int Colors) 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 5);
  int pos = random16(NUM_LEDS);
  if( Colors != 0) Colors = Colors + random8(10);
  leds[pos] += CHSV( Colors , 255, 255); //+ random8(64)
// leds[pos] += CHSV( gHue , 200, 255); //+ random8(64)
// if (gHue > 110) gHue=80; // green
// if (gHue > 20) gHue=0; // red
// if (gHue > 170) gHue=150; // blue
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
   fill_rainbow( leds, NUM_LEDS, gHue, 7);
}


void juggle() {  //start
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 10);
  byte dothue = 0;
  for( int i = 0; i < 10; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 100, 100);
    dothue += 62;
  }
  FastLED.delay(100); 
}
