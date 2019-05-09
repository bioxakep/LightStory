import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.serial.*; 
import de.voidplus.leapmotion.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class leapSlave extends PApplet {


 // \u0443\u0441\u0442\u0430\u043d\u043e\u0432\u0438\u0442\u044c \u0431\u0438\u0431\u043b\u0438\u043e\u0442\u0435\u043a\u0443 Leap Motion for Processing (\u043d\u0435 LeapMotion, \u0438\u0445 \u0434\u0432\u0435).

LeapMotion leap;
Serial arduino;

String portName;

int circleCount = 0;
boolean game = false;
boolean firstStart = true;

public void setup() 
{
  String[] comportConfig = loadStrings("com.txt");
  portName = comportConfig[0].substring(comportConfig[0].indexOf("\"") + 1, comportConfig[0].length() - 2);
  arduino = new Serial(this, portName, 9600);
  
  background(255);
  leap = new LeapMotion(this).allowGestures("circle");
}

public void draw() 
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

public void leapOnCircleGesture(CircleGesture g, int state) {
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

public String getInput()
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
  public void settings() {  size(800, 600); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "leapSlave" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
