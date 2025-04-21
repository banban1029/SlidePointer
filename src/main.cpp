#include <M5StickCPlus.h>
#include <BleMouse.h>
#include "battery.h"

#define TIME_BTNB_LONG_PRESS 1000  // ボタンBの長押しと判定する時間(ms)
#define LCD_TOP_OFFSET 5  // LCD上部が被って見えないから、オフセット
#define ALARM_BAT_LEVEL 25     // 内蔵バッテリーレベル[%]の警告表示のしきい値
bool FlagBleConnected = false;  // Bluetoothの接続判定フラグ。接続済:true / 未接続:false
unsigned long nextMonitorMills = 0;  // 次回のモニタを更新するミリ秒を格納する

Battery battery = Battery();
BleMouse bleMouse("BanbanPointer");
hw_timer_t * presenTimer = NULL;

void setup() {

  /* -- Begin -- */
  M5.begin(); 
  bleMouse.begin();
  presenTimer = timerBegin(0, 80, true);
  setCpuFrequencyMhz(80);
  // presenTimer 初期停止と初期化
  timerStop(presenTimer);
  timerWrite(presenTimer, 0);

  /* -- ボタン説明表示 -- */
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5,170 + LCD_TOP_OFFSET);
  M5.Lcd.printf("LF:Reset");
  M5.Lcd.setCursor(5,190 + LCD_TOP_OFFSET);
  M5.Lcd.printf("FR:Next");
  M5.Lcd.setCursor(5,210 + LCD_TOP_OFFSET);
  M5.Lcd.printf("RH:Reverse");

  // Battery
  // 表示左上の位置x, yと表示サイズ(1-7)を設定
  battery.setPosAndSize(5, 5, 2);

  // deleteBattery()時の塗りつぶし色を設定
  battery.setDeleteBgColor(TFT_BLACK); 

  // 電池図形と%表示の色を設定
  battery.setTextColor(TFT_WHITE);

  // バッテリーを表示
  battery.showBattery();
}

void loop() {
  M5.update();
  battery.batteryUpdate();

  /* -- スライド操作関連 -- */
  if (bleMouse.isConnected()) {
    FlagBleConnected = true;

    if (M5.BtnA.wasReleased()) {
      Serial.println("Send Scroll down");
      bleMouse.move(0, 0, -1);
    }
  
    if (M5.BtnB.wasReleased()) {
      Serial.println("Send Scroll up");
      bleMouse.move(0, 0, 1);
    }
  }
  else{
    FlagBleConnected = false;
  }

  /* -- Timer関連 -- */
  // 電源ボタンの押下状態を取得する。
  // HACK: GetBtnPress()は0以外の値は一度しか値を取得できないから、ここで取得する。
  //       格納しないで下のif文で呼ぶと、ボタンを長押ししても[1]ではなく、先に短い場合の[2]を返してしまうため、長押しの分岐に入らない。
  int axpBtn = M5.Axp.GetBtnPress();
  // 電源ボタンが短時間押されたときは、タイマーのStart/Stop
  if( axpBtn == 2 ){
      if ( timerStarted(presenTimer) ){
          timerStop(presenTimer);
      } else {
          timerStart(presenTimer);
      }
  }
  // 電源ボタンが長時間(1.5s)押されたら、タイマーリセット
  if ( axpBtn == 1 ){
      timerStop(presenTimer);
      timerWrite(presenTimer, 0);
  }

  /* -- モニタ関連 -- */
  // 更新周期を1秒間隔にする
  if ( nextMonitorMills < millis() ){
    // Bluetooth接続状態の表示
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 105 + LCD_TOP_OFFSET);
    M5.Lcd.printf("BLE:");
    if ( FlagBleConnected ){
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("Yes");
        M5.Lcd.setTextColor(WHITE, BLACK);
    } else {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.printf("No ");
        M5.Lcd.setTextColor(WHITE, BLACK);
    }

    // Timer表示
    int presenTimerSec = int(timerReadSeconds(presenTimer));
    // 分は大きく
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(15,50);
    M5.Lcd.printf("%2d", presenTimerSec / 60);
    // 秒は小さく
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(70,50);
    M5.Lcd.printf("%02d", presenTimerSec % 60);
    // 時刻
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5,125 + LCD_TOP_OFFSET);
    // Start/Stopの表示を切り替える
    M5.Lcd.printf("Time:");
    if ( timerStarted(presenTimer) ){
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.printf("Start");
        M5.Lcd.setTextColor(WHITE, BLACK);
    } else {
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.printf("Stop ");
        M5.Lcd.setTextColor(WHITE, BLACK);
    }
    nextMonitorMills = millis() + 1000;
  }
}

