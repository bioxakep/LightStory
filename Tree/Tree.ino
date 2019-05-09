// Slave similar to VisualSlave color linked to Dwell slave and Projector (IR) Slave
#include <Wire.h>
#define RED 0
#define BLUE 2
#define GREEN 1

#define MIN_BRI 60
#define MAX_BRI 220
#define TEST 1000

int lampPins[4][3] = {{8, 9, 10}, {2, 3, 4}, {5, 6, 7}, {44, 45, 46}};

int delayTime = 15;
long briDelay = 20;
long lampDelay = 50;
boolean game = false;
boolean win = false;
boolean blueState, redState, greenState;
boolean blueStart, redStart, greenStart, winStart;

int thisAddress = 23;

void setup() {
  Serial.begin(9600);
  Wire.begin(thisAddress);
  Wire.onReceive(Receiver);

  blueState = false;
  redState = false;
  greenState = false;
  winStart = false;
  blueStart = false;
  redStart = false;
  greenStart = false;

  for (int l = 0; l < 4; l++) {
    for (int c = 0; c < 3; c++)
    {
      pinMode(lampPins[l][c], OUTPUT);
      digitalWrite(lampPins[l][c], HIGH);
    }
  }
}

void loop() {

  if (game)
  {
    if (blueState) Run(BLUE);

    if (redStart)
    {
      Serial.println("BLUE STOP - RED START:");
      for (int lamp = 0; lamp < 4; lamp++)
      {
        for (int br = MAX_BRI; br <= 255; br++) analogWrite(lampPins[lamp][BLUE], br);
        digitalWrite(lampPins[lamp][BLUE], HIGH); Serial.println("BLUE OFF");
        for (int br = 255; br >= MAX_BRI; br--) analogWrite(lampPins[lamp][RED], br);
        Serial.println("RED ON");
      }
      blueState = false;
      redState = true;
      redStart = false;
      delay(TEST); //DEBUG!!!!
    }

    if (redState) Run(RED);

    if (greenStart)
    {
      Serial.println("RED STOP - GREEN START");
      for (int lamp = 0; lamp < 4; lamp++)
      {
        for (int br = MAX_BRI; br <= 255; br++) analogWrite(lampPins[lamp][RED], br);
        digitalWrite(lampPins[lamp][RED], HIGH); Serial.println("RED OFF");
        for (int br = 255; br >= MAX_BRI; br--) analogWrite(lampPins[lamp][GREEN], br);
        Serial.println("GREEN ON");
      }
      redState = false;
      greenState = true;
      greenStart = false;
      delay(TEST); //DEBUG!!!!
    }

    if (greenState) Run(GREEN);

    if (winStart)
    {
      Serial.println("GREEN STOP - SPLASH START");
      for (int l = 0; l < 4; l++)
      {
        for (int c = 0; c < 3; c++)
        {
          digitalWrite(lampPins[l][c], HIGH);
        }
        Serial.println("LAMP " + String(l + 1) + " OFF");
        digitalWrite(lampPins[l][RED], LOW);
        analogWrite(lampPins[l][GREEN], 0.25 * MAX_BRI);
      }
      winStart = false;
      win = true;
      delay(TEST); //DEBUG!!!!
    }
    if (win) while(true);
  }
}

void Run(int _color)
{
  for (int lamp = 0; lamp < 4; lamp++)
  {
    for (int br = MIN_BRI; br <= MAX_BRI; br = br + 20)
    {
      if (lamp == 0) analogWrite(lampPins[lamp][_color], br / 2);
      else analogWrite(lampPins[lamp][_color], br / 1.5);
      briDelay = random(22, 57);
      delay(briDelay);
    }
    lampDelay = random(22, 156);
    delay(lampDelay);
  }
}


void Receiver(int howMany)
{
  if (Wire.available() > 0)
  {
    byte cmd = Wire.read();
    if (cmd == 0xB1) blueState = true;
    if (cmd == 0xB2 && blueState) redStart = true;
    if (cmd == 0xB3 && redState) greenStart = true;
    if (cmd == 0xFF && greenState) winStart = true;
    if (cmd == 0xAA) game = true;
  }
}

