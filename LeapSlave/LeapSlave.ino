#include <Wire.h>

boolean game = true;
boolean leapOnline = false;

byte unswer = 0x00;

int thisAddress = 24;

void setup()
{
  Serial.begin(9600);
  Wire.begin(thisAddress);
  Wire.onReceive(Receiver);
  Wire.onRequest(requestEvent);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
}

void loop()
{
  if (game)
  {
    if (leapOnline)
    {
      if (Serial.available() > 0)
      {
        String inStr = Serial.readStringUntil('\n');
        if (inStr.indexOf("OK") > 0)
        {
          unswer = 0xFF;
          Serial.println("endGame");
          digitalWrite(13,HIGH);
          game = false;
          leapOnline = false;
        }
      }
    }
    else connectToMonitor();
  }
}

void Receiver(int howMany)
{
  if (Wire.available() > 0)
  {
    if (Wire.read() == 0xAA) game = true;
  }
}

void requestEvent()
{
  Wire.write(unswer);
  unswer = 0x00;
}

void connectToMonitor()
{
  if (Serial.available() > 0)
  {
    String input = "";
    input = Serial.readStringUntil('\n');
    if (input.startsWith("letsGame")) {
      leapOnline = true;
      Serial.println("startGame");
      delay(3000);
    }
  }
}
