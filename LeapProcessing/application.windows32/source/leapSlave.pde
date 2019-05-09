import processing.serial.*;
import de.voidplus.leapmotion.*; // установить библиотеку Leap Motion for Processing (не LeapMotion, их две).

LeapMotion leap;
Serial arduino;

String portName;

int circleCount = 0;
boolean game = false;
boolean firstStart = true;

void setup() 
{
  String[] comportConfig = loadStrings("com.txt");
  portName = comportConfig[0].substring(comportConfig[0].indexOf("\"") + 1, comportConfig[0].length() - 2);
  arduino = new Serial(this, portName, 9600);
  size(800, 600);
  background(255);
  leap = new LeapMotion(this).allowGestures("circle");
}

void draw() 
{
  if (game)
  {
    String arduinoStr = getInput();
    if (arduinoStr.substring(0, 7).equals("endGame")) game = false;
    background(255);
    textAlign(CENTER);
    textSize(50);
    fill(0);
    String text = "CIRCLE NUMBER: " + String.valueOf(circleCount);
    if (circleCount > 0) arduino.write("CIRCLE is OK\n");
    circleCount = 0;
    println("Circle Count = " + String.valueOf(circleCount));
  } else
  {
    if (portName.length() == 0) 
    {
      textAlign(CENTER);
      fill(255);
      textSize(50);
      text("CONNECT ARDUINO TO PC AND RESTART", width/2, height/2 - 20);
      text("(PRESS ESC TO EXIT)", width/2, height/2 + 70);
    } else
    {
      if (firstStart) 
      {
        arduino.stop();
        arduino = new Serial(this, portName, 9600);
        firstStart = false;
      }
      background(0);
      boolean connected = false;
      println("try Connect");
      while (!connected)
      {
        arduino.write("letsGame\n"); 
        long now = millis();
        while (millis() - now < 1000) {
          ;
        }
        String input = getInput(); 
        if (input.length() > 4)
        {
          if (input.substring(0, 9).equals("startGame")) { 
            game = true;
            connected = true;
            circleCount = 0;
            println("startGame");
          }
        }
      }
    }
  }
}
// ======================================================
// 2. Circle Gesture

void leapOnCircleGesture(CircleGesture g, int state) {
  int     id               = g.getId();
  switch(state) {
  case 1: // Start
    break;
  case 2: // Update
    break;
  case 3: // Stop
    println("CircleGesture: " + id);
    circleCount++;
    break;
  }
}

String getInput()
{
  if (arduino.available() > 0)
  {
    String inp = arduino.readStringUntil('\n');
    if (inp != null)
    {
      if (inp.length() > 1) println(inp);
      return inp;
    } else return " ";
  } else return " ";
}