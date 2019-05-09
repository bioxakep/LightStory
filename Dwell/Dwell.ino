// dwell IR controlled RGB LED projector 
#include "IRremote.h"
#include "Wire.h"

IRsend IRS;  // on 3

int demora = 0;

boolean game = false;
boolean win;
boolean blueState, redState, greenState;
boolean blueStart, redStart, greenStart, winStart;
int thisAddress = 22;//I2C адрес (не изменять, он прописан в Базе2).


void setup()
{
  Serial.begin(9600);
  Wire.begin(thisAddress);
  Wire.onReceive(Receiver);

  delay(1000);
  randomSeed(A0);
  ProOn();
  ProGreen();
  ProBlue();
  ProOff();
  Serial.println("Ready");
  blueState = false;
  redState = false;
  greenState = false;
  winStart = false;
  blueStart = false;
  redStart = false;
  greenStart = false;
  win = false;
}

void loop()
{
  if (game)
  {
    if (blueState) ProBlue();
    if(redStart) 
    {
      blueState = false;
      ProOff();
      redState = true;
      redStart = false;
    }
    if (redState) ProRed();
    if(greenStart) 
    {
      greenStart = false;
      ProOff();
      redState = false;
      greenState = true;
    }
    if (greenState) ProGreen();
    if(winStart)
    {
      greenState = false;
      winStart = false;
      ProOff();
      win = true;
    }
    if(win) ProSmooth();
  }
  else
  {
    delay(100);
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
   // wingame ???
  }
}

void down() { 
 delay(100);
// Serial.println("Send DOWN");
 IRS.sendNEC(0xF7807F, 32); //down
 delay(200);

// Serial.println("Send DOWN");
 IRS.sendNEC(0xF7807F, 32); //down
 delay(300);

// Serial.println("Send DOWN");
 IRS.sendNEC(0xF7807F, 32); //down
 delay(400);

}

void ProOff() {
 Serial.println("Send OFF");
 IRS.sendNEC(0xF740BF, 32); 
 delay(100);
}

void ProOn() {
 Serial.println("Send ON");
 IRS.sendNEC(0xF7C03F, 32); //turn on 
 delay(150);
 IRS.sendNEC(0xF720DF, 32); //red
 delay(500);
 down();
}

void ProRed() {
 Serial.println("Send RED");
 IRS.sendNEC(0xF7C03F, 32); //turn on 
 delay(150);
 IRS.sendNEC(0xF720DF, 32); //red
 delay(500);
 down();
}

void ProGreen() {
 Serial.println("Send GREEN"); //green
 IRS.sendNEC(0xF7A05F, 32); 
 down();
}

void ProBlue() {
 Serial.println("Send BLUE");
 IRS.sendNEC(0xF7C03F, 32); //turn on 
 IRS.sendNEC(0xF7609F, 32); 
 down();
}

void ProSmooth() {
 Serial.println("Send SMOOTH and pause");
 IRS.sendNEC(0xF7E817, 32); 
 delay(30000);
}


