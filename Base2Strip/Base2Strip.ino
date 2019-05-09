/*
   Модуль визуализации для Базы 2
   Цвета добавляются по порядку (синий или красный), потом второй из них потом зеленый
   Лента подключается к 3 пину
*/
#include <FastLED.h>
#include <Wire.h>

#define DATA_PIN    3//пин подключения ленты
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUMLEDS 1050 //количество светодоидов в ленте
#define MIN_VALUE 30  //минимальная яркость - фоновый цвет
#define MAX_VALUE 250 //максимальная яяркость (при 3 уровне яркости).
#define BLUEHUE 160
#define REDHUE 255
#define GREENHUE 96
#define PAUSE 100
/*
   HUE_RED = 0, HUE_ORANGE = 32, HUE_YELLOW = 64, HUE_GREEN = 96,
  HUE_AQUA = 128, HUE_BLUE = 160, HUE_PURPLE = 192, HUE_PINK = 224
*/

CRGB leds[NUMLEDS];

boolean redState = false;
boolean blueState = true;
boolean greenState = false;

boolean game = true;
boolean win = false;

boolean pixStates[1050];

long upSpeedDelay = 20;
long downSpeedDelay = 10;

int thisAddress = 21;//I2C адрес (не изменять, он прописан в Базе2).

CHSV redColor, blueColor, greenColor,
     currentBackColor, gameOffColor;

void setup()
{
  Serial.begin(115200);
  Wire.begin(thisAddress);
  Wire.onReceive(Receiver);

  delay(3000);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUMLEDS).setCorrection(TypicalLEDStrip);
  randomSeed(A0);
  blueColor = CHSV(BLUEHUE, 255, MIN_VALUE);
  redColor = CHSV(REDHUE, 255, MIN_VALUE);
  greenColor = CHSV(GREENHUE, 255, MIN_VALUE);
  gameOffColor = CHSV(0, 0, 0);
  eraseStates();
}
void loop()
{
  if (game)
  {
    if (greenState) Flash(greenColor);//мигаем зеленым, если он загружен
    if (redState) Flash(redColor);   //мигаем красным, если он загружен
    if (blueState) Flash(blueColor);  //мигаем синим, если он загружен
    if (win)
    {
      Flash(greenColor);
      delay(PAUSE);
      Flash(redColor);
      delay(PAUSE);
      Flash(blueColor);
    }
    delay(PAUSE);
  }
  else
  {
    setBackGrnd(gameOffColor); //глушим ленту вне игры
    delay(500);
  }
}

void setBackGrnd(CHSV _color)
{
  _color.value = MIN_VALUE;
  for (int i = 0; i < NUMLEDS; i++) leds[i] = _color;
  FastLED.show();
}

void Flash(CHSV _flashColor)
{
  setBackGrnd(currentBackColor);
  FastLED.show();
  int flashStart[3] = {800, 800, 800};
  flashStart[0] = randomIndex();
  writeStates(flashStart[0]);
  for (int r = 1; r < 3; r++)
  {
    boolean calculateRandom = true;
    while (calculateRandom)
    {
      flashStart[r] = randomIndex();
      calculateRandom = getStates(flashStart[r]);
    }
  }
  for (int bri = MIN_VALUE; bri <= MAX_VALUE; bri = bri + 10)
  {
    for (int blockIndex = 0; blockIndex < 3; blockIndex++)
    {
      fill_gradient(leds,flashStart[blockIndex]+5,currentBackColor,flashStart[blockIndex]+14,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15,_flashColor,flashStart[blockIndex]+24,currentBackColor,SHORTEST_HUES);
      
      fill_gradient(leds,flashStart[blockIndex]+150,currentBackColor,flashStart[blockIndex]+14+150,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15+150,_flashColor,flashStart[blockIndex]+29+150,currentBackColor,SHORTEST_HUES);

      fill_gradient(leds,flashStart[blockIndex]+305,currentBackColor,flashStart[blockIndex]+14+300,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15+300,_flashColor,flashStart[blockIndex]+24+300,currentBackColor,SHORTEST_HUES);
      
      /*for(int d = 0; d < 1050; d++)
      {
        CRGB color = leds[d];
        Serial.print(String(color.b > 10)+"-");
        if(d % 150 == 0) Serial.println();
      }
      */
      //Serial.println();
      //Serial.println("FlashColor Vlaue = " + String(_flashColor.value));
      delay(5);
      FastLED.show();
    }
    _flashColor.value = bri;
    delay(upSpeedDelay);
  }
  for (int bri = MAX_VALUE; bri >= MIN_VALUE; bri = bri - 10)
  {
    for (int blockIndex = 0; blockIndex < 3; blockIndex++)
    {
      fill_gradient(leds,flashStart[blockIndex]+5,currentBackColor,flashStart[blockIndex]+14,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15,_flashColor,flashStart[blockIndex]+24,currentBackColor,SHORTEST_HUES);
      
      fill_gradient(leds,flashStart[blockIndex]+150,currentBackColor,flashStart[blockIndex]+14+150,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15+150,_flashColor,flashStart[blockIndex]+29+150,currentBackColor,SHORTEST_HUES);

      fill_gradient(leds,flashStart[blockIndex]+305,currentBackColor,flashStart[blockIndex]+14+300,_flashColor,SHORTEST_HUES);
      fill_gradient(leds,flashStart[blockIndex]+15+300,_flashColor,flashStart[blockIndex]+24+300,currentBackColor,SHORTEST_HUES);
      /*
      for(int d = 0; d < 1050; d++)
      {
        CRGB color = leds[d];
        Serial.print(String(color.b > 10)+"-");
        if(d % 150 == 0) Serial.println();
      }
      */
      //Serial.println();
      //Serial.println("FlashColor Vlaue = " + String(_flashColor.value));
      delay(5);
      FastLED.show();
    }
    _flashColor.value = bri;
    delay(downSpeedDelay);
  }
  eraseStates();
  setBackGrnd(currentBackColor);
}

void writeStates(int _startIndex)
{
  for(int wr = _startIndex; wr < _startIndex + 29; wr++)
  {
    for(int line = 0; line < 3; line++)
    {
      pixStates[line*150 + wr] = true;
    }
  }
}

boolean getStates(int _startIndex)
{
  for(int wr = _startIndex; wr < _startIndex + 29; wr++)
  {
    for(int line = 0; line < 3; line++)
    {
      if(pixStates[line*150 + wr]) return true;
    }
  }
  return false;
}

void eraseStates()
{
  for(int er = 0; er < 1050; er++) pixStates[er] = 0;
}

int randomIndex()
{
  int startIndexes[4] = {0,0,0,0};
  int indexOfIndexes = random(0,3);
  for(int line = 0; line < 4; line++)
  {
    startIndexes[line] = random(line*150, line*150 + 119);
  }
  return startIndexes[indexOfIndexes];
}

void Receiver(int howMany)
{
  if (Wire.available() > 0)
  {
    byte cmd = Wire.read();

    if (cmd == 0xB1) { blueState = true; redState = false; currentBackColor = blueColor; }
    if (cmd == 0xB2) { blueState = false; redState = true; currentBackColor = redColor; }
    if (cmd == 0xB3) { greenState = true; blueState = false; redState = false; currentBackColor = greenColor; }

    if (cmd == 0xAA) game = true;
    if (cmd == 0xFF) win = true;
  }
}
