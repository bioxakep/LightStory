//NEW

#include <IRremote.h>
#include <RCSwitch.h>
#include <Adafruit_NeoPixel.h>

#define TPIXEL 10
#define MINLEDCOUNT 1

#define RTXPIN 10
#define IRRXPIN 11
#define STRIPPIN 5

#define FAST_UNCHARGE_DELAY 100
#define NORMAL_UNCHARGE_DELAY 500

unsigned long Delay = 0;
unsigned long nowTime = 0;

int loadDelay = 400;
int chargeDelay = 500;
int unChargeDelay = FAST_UNCHARGE_DELAY;

int irValue = 0;
int irB1Cmd = 0xB12;
int irB2Cmd1 = 0xB21;
int irB2Cmd2 = 0xB22;
int radioCode = 200;
int noIR = 0;
int isIR = 0;
int isIRB2 = 0;

int currLedIndex = 9;

uint32_t light, dark, loaded;

boolean game = true;
boolean slowMode = false;

RCSwitch RTX = RCSwitch();
IRrecv irrecv(IRRXPIN);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TPIXEL, STRIPPIN, NEO_GRB + NEO_KHZ800);
decode_results results;

void setup()
{//=========Portable Settings============//
  Serial.begin(9600);
  RTX.enableTransmit(RTXPIN);
  irrecv.enableIRIn();

  Delay = unChargeDelay;
  light = strip.Color(0, 50, 0);
  dark = strip.Color(0, 0, 0);
  loaded = strip.Color(50, 50, 50);
  //=========Portable Settings============//

  strip.begin(); //Start LEDStrip

  signalBlinking(light);//Debug mode
}
void loop() {
  //===========================================================================
  while ((millis() - nowTime < Delay) && game)
  {
    if (irrecv.decode(&results) && results.value > 0) {
      irValue = results.value;
      //Serial.println("VALUE = = " + String(irValue, HEX));
      irrecv.resume();
    }
  }
  //==========================================================================
  if (((irValue>>4) & 0xFFF) == (irB1Cmd & 0xFFF)) 
  {
    if (isIR == 0)
    {
      Serial.println("Start charge");
      Delay = chargeDelay;
      noIR = 0;
      isIRB2 = 0;
      if (currLedIndex == 0)
      {
        strip.setPixelColor(currLedIndex++, light);
        strip.show();
        Serial.println(String(currLedIndex) + "::ON");
        if (currLedIndex > TPIXEL - 1) currLedIndex = TPIXEL - 1;
      }
    }
    slowMode = (irValue & 0x000F) == 1 ? true : false;
    if(slowMode) unChargeDelay = NORMAL_UNCHARGE_DELAY;
    isIR++;
    if (isIR == 3)
    {
      if (currLedIndex < TPIXEL)
      {
        Serial.println(String(currLedIndex + 1) + "::ON");
        strip.setPixelColor(currLedIndex++, light);
        strip.show();
      }
      else Serial.println("CHARGED");
      isIR = 1;
    }
  }
  //================================================================
  if (irValue == (irB2Cmd1 & 0xFFF)) //Сигнал от Базы2
  {
    noIR = 0;
    isIR = 0;
    if(currLedIndex + 1 >= MINLEDCOUNT)
    {
      for (int r = 0; r < 3; r++)
      {
        RTX.send((currLedIndex + 1 + radioCode) & 0xFFF, 12);
        delay(40);
      }
      for (int k = 0; k <= currLedIndex; k++)
      {
        Serial.println("Load to Base2 Led" + String(currLedIndex - k + 1));
        strip.setPixelColor(currLedIndex - k, dark);
        strip.show();
        delay(200);
      }
      signalBlinking(loaded);
      currLedIndex = 0;
      Serial.println("LOADED");
    }
    isIRB2 = 0;
    Delay = unChargeDelay;
  }
  //===================================================================
  if (irValue == 0xB22)//Сигнал от Базы2 (Разрядился 3 раза)
  {
    game = false;
  }
  //=====================================================================
  if ((irValue & 0xFFF) != irB2Cmd2 && (irValue & 0xFFF) != irB2Cmd1 && ((irValue >> 4) & 0xFFF) != irB1Cmd)
  {
    if (noIR == 1 && (isIR > 0 || isIRB2 > 0))
    {
      Delay = unChargeDelay;
      isIR = 0;
      isIRB2 = 0;
      Serial.print("Stop Charge ");
    }
    if (noIR == 3)
    {
      if (currLedIndex > 0)
      {
        Serial.println(String(currLedIndex) + "::OFF");
        strip.setPixelColor(--currLedIndex, dark);//FOR_BASE2_IMITATION
        strip.show();
      }
      else Serial.println("UNCHARGED");
      
      noIR = 1;
    }
    noIR++;
  }
  nowTime = millis();
  irValue = 0;
}
void signalBlinking(uint32_t color)
{
  for (int i = 0; i < TPIXEL; i++) strip.setPixelColor(i, color);
  strip.show();
  delay(300);
  for (int i = 0; i < TPIXEL; i++) strip.setPixelColor(i, dark);
  strip.show();
}
