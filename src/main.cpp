#include <M5StickCPlus.h>
#include <BleMouse.h>

BleMouse bleMouse("BanbanPointer");

void setup() {
  M5.begin(); 
  bleMouse.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Controller");
  M5.Lcd.println(" BtnA :Next");
  M5.Lcd.println(" BtnP :Reset");
}

void loop() {
  M5.update();
  if (bleMouse.isConnected()) {
    if (M5.BtnA.wasReleased()) {
      Serial.println("Send Scroll down");
      bleMouse.move(0, 0, -1);
    }
  
    if (M5.BtnB.wasReleased()) {
      Serial.println("Send Scroll up");
      bleMouse.move(0, 0, 1);
    }
  }
}

