// working version w/leap motion and extra inputs-outputs > less Remote

#include <RCSwitch.h>
#include "IRremote.h"
#include "Wire.h"

#define MINLEDCOUNT 3
#define MAXLOADCNT 1

boolean game = true; // When i2c command recieve then true; or set it true manual;

int visSlaveAddr    = 21;
int dwellSlaveAddr  = 22;
int treeSlaveAddr   = 23;
int leapSlaveAddr   = 24;

struct Base1
{
  int loadCnt;
  byte endPin;
  boolean loaded;
  String baseName;
  byte B2StripCmd;
};

Base1 greenBase;
Base1 redBase;
Base1 blueBase;

Base1 Bases[3];
IRsend IRS;
RCSwitch RRX2 = RCSwitch();

int mainDoor  = A0;
int millDoor  = A1;
int brevno    = 9;
int remote    = 8;
int slowClock = 6;
int smoke     = 5;
boolean clockspeed = false;
long smokeTime = 0;
long fungiTime = 0;
boolean smokon = false;
boolean fungi  = false;
void setup()
{
  Serial.begin(9600);
  greenBase.endPin = 9;
  redBase.endPin = 10;
  blueBase.endPin = 11;
  greenBase.baseName = "Green Base";
  greenBase.B2StripCmd = 0xB3;
  redBase.baseName = "Red Base";
  redBase.B2StripCmd = 0xB2;
  blueBase.baseName = "Blue Base";
  blueBase.B2StripCmd = 0xB1;
  Bases[0] = redBase;
  Bases[1] = greenBase;
  Bases[2] = blueBase;
  for (int i = 0; i < 3; i++) {
    Bases[i].loadCnt = 0;  //Обнуление состояний баз (загружено 0 палочек)
    Bases[i].loaded = false;
  }
  RRX2.enableReceive(0);
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(visSlaveAddr);
  Wire.write(0xAA);
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(dwellSlaveAddr);
  Wire.write(0xAA);
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(treeSlaveAddr);
  Wire.write(0xAA);
  Wire.endTransmission();
  Serial.println("Base 2 started.");

  pinMode( mainDoor  , OUTPUT);
  pinMode( millDoor  , OUTPUT);
  pinMode( brevno    , OUTPUT);
  pinMode( remote    , INPUT_PULLUP);
  pinMode( slowClock , INPUT_PULLUP);
  pinMode( smoke     , OUTPUT);

  digitalWrite(mainDoor , HIGH);
  digitalWrite(millDoor , HIGH);
  digitalWrite(brevno   , LOW);
  digitalWrite(smoke    , LOW);


}

void loop() {
  if (game)
  { long now = millis();
    if (digitalRead(slowClock) == 0  && !clockspeed) {
      clockspeed = true;
      Serial.println("#S|SLOWCLOCK0|[]#");
      delay(50);
      Serial.println("#S|SLOWCLOCK1|[]#");
    }

    if ((now - smokeTime) <= 7000) {
      digitalWrite(smoke, HIGH);
    } else {
      digitalWrite(smoke, LOW);
    }

    if (now - fungiTime > 60000  && fungi == true) {
      digitalWrite(brevno, HIGH);
      fungi = false;
    }

    if (millis() % 500 == 0) {
      IRS.sendRC5(0xB21, 12);
    }
    if (RRX2.available())
    {
      int RadioData2 = (int)RRX2.getReceivedValue(); //Принимаем с палочки радиосигнал подтверждения ее присутствия
      Serial.println("FIRST RADIO SIGNAL = " + String(RadioData2));
      RRX2.resetAvailable();
      boolean Confirm = false;
      unsigned long nowTime = millis();
      while (millis() - nowTime < 200)
      {
        if (RRX2.available())
        {
          if (RadioData2 == (int)RRX2.getReceivedValue())
          {
            Confirm = true;
            //           Serial.println("Confirm");
          }
          nowTime = millis();
          RRX2.resetAvailable();
        }
      }
      if (Confirm)
      {
        //       Serial.println("CONFIRM RADIO SIGNAL = " + String(RadioData2));
        int baseCnt = 0;
        boolean greenLoading = true;
        while (RadioData2 > 100)
        {
          RadioData2 -= 100;
          baseCnt++;
        }
        baseCnt--; // Вычисляем номер палочки (и соответственно цвет)

        if (baseCnt == 1 && (!Bases[0].loaded || !Bases[2].loaded)) greenLoading = false; //не обрабатываем сигнал от зеленой палочки если синия и красная базы не заполнены

        if (RadioData2 >= MINLEDCOUNT && !Bases[baseCnt].loaded && greenLoading)
        {
          delay(200);
          Bases[baseCnt].loadCnt++;

          if (Bases[baseCnt].loadCnt >= MAXLOADCNT)
          {
            for (int k = 0; k < 3; k++) {
              IRS.sendRC5(0xB22, 12);
              delay(50);
            }
            digitalWrite(Bases[baseCnt].endPin, HIGH);
            Bases[baseCnt].loaded = true;
            goBetwino(baseCnt);
            Serial.println(String(Bases[baseCnt].baseName) + " loaded " + String(MAXLOADCNT) + " times and stop.");
            boolean allBaseLoaded = true;
            for (int i = 0; i < 3; i++) {
              allBaseLoaded &= Bases[i].loaded;
            }
            if (allBaseLoaded)
            {
              for (int i = 0; i < 3; i++) {
                Bases[i].loadCnt = 0;
                Bases[i].loaded = false;
              }
              sendToSlaves(baseCnt);
              Wire.beginTransmission(leapSlaveAddr);
              Wire.write(0xAA);
              Wire.endTransmission();
              boolean leapWait = true;
              while (leapWait)
              {
                byte unswerCnt = Wire.requestFrom(leapSlaveAddr, 1, false);
                if (unswerCnt == 1)
                {
                  if (Wire.read() == 0xFF)
                  {
                    leapWait = false;
                    goBetwino(10);
                    Wire.beginTransmission(treeSlaveAddr);
                    Wire.write(0xFF);
                    Wire.endTransmission();
                    delay(10);
                    Wire.beginTransmission(dwellSlaveAddr);
                    Wire.write(0xFF);
                    Wire.endTransmission();
                    delay(10);
                    Wire.beginTransmission(visSlaveAddr);
                    Wire.write(Bases[_baseIndex].B2StripCmd);
                    Wire.endTransmission();
                  }
                }
                delay(100);
              }
              game = false;
            }
          }
          else  Serial.println(String(Bases[baseCnt].baseName) + " loaded " + String(Bases[baseCnt].loadCnt) + " times.");
          sendToSlaves(baseCnt);
          RRX2.resetAvailable();
        }
        //RRX2.resetAvailable();
        delay(2000);
      }
    }
  }
}

void goBetwino(int _baseCount)
{
  if (_baseCount == 0) {
    Serial.println("#S|STARTRED0|[]#");
    delay(50);
    Serial.println("#S|STARTRED1|[]#");
    smokeTime = millis();
    fungiTime = millis();
    fungi = true;
    //digitalWrite(brevno,HIGH);
  }
  if (_baseCount == 1) {
    Serial.println("#S|STARTGREEN0|[]#");
    delay(50);
    Serial.println("#S|STARTGREEN1|[]#");
    smokeTime = millis();
  }
  if (_baseCount == 2) {
    Serial.println("#S|STARTBLUE0|[]#");
    delay(50);
    Serial.println("#S|STARTBLUE1|[]#");
    digitalWrite(millDoor, LOW);
    smokeTime = millis();
  }
  if (_baseCount == 10) {
    Serial.println("#S|ALLFINISH0|[]#");
    delay(50);
    Serial.println("#S|ALLFINISH1|[]#");
    digitalWrite(mainDoor, LOW);
  }
}





void sendToSlaves(int _baseIndex)
{
  Wire.beginTransmission(visSlaveAddr);
  Wire.write(Bases[_baseIndex].B2StripCmd);
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(dwellSlaveAddr);
  Wire.write(Bases[_baseIndex].B2StripCmd);
  Wire.endTransmission();
  delay(50);
  Wire.beginTransmission(treeSlaveAddr);
  Wire.write(Bases[_baseIndex].B2StripCmd);
  Wire.endTransmission();
}

