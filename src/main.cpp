#include <M5StickCPlus.h>
#include <BleCombo.h>
#include "battery.h"

#define TIME_LONG_PRESS 300         // ボタン長押しと判定する時間(ms)
#define DoubleClickThreshold 300    // ダブルクリックと判定
#define LCD_TOP_OFFSET 5            // LCD上部が被って見えないから、オフセット
bool isBleConnected = false;        // Bluetoothの接続判定フラグ。接続済:true / 未接続:false
bool isBtnALongPressing = false;    // BtnAの長押しフラグ
bool isBtnBLongPressing = false;    // BtnBの長押しフラグ
bool isMouseMode = false;           // MouseModeフラグ
unsigned long nowTime = 0;          // リアル時間
unsigned long lastClickTime = 0;    // 最後にクリックした時間
unsigned long nextMonitorMills = 0; // 次回のモニタを更新するミリ秒を格納する
Battery battery = Battery();
hw_timer_t *presenTimer = NULL;

void setup()
{
  /* -- Begin -- */
  M5.begin();
  Mouse.begin();
  Keyboard.begin();

  presenTimer = timerBegin(0, 80, true);
  setCpuFrequencyMhz(80);
  // presenTimer 初期停止と初期化
  timerStop(presenTimer);
  timerWrite(presenTimer, 0);
  // 加速度センサー（MPU6886）の初期化
  M5.IMU.Init();

  /* -- ボタン説明表示 -- */
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(5, 170 + LCD_TOP_OFFSET);
  M5.Lcd.printf("LF:Reset");
  M5.Lcd.setCursor(5, 190 + LCD_TOP_OFFSET);
  M5.Lcd.printf("FR:Next");
  M5.Lcd.setCursor(5, 210 + LCD_TOP_OFFSET);
  M5.Lcd.printf("RH:Reverse");

  /* -- Battery -- */
  // 表示左上の位置x, yと表示サイズ(1-7)を設定
  battery.setPosAndSize(5, 5, 2);
  // deleteBattery()時の塗りつぶし色を設定
  battery.setDeleteBgColor(TFT_BLACK);
  // 電池図形と%表示の色を設定
  battery.setTextColor(TFT_WHITE);
  // バッテリーを表示
  battery.showBattery();
}

void loop()
{
  M5.update();
  battery.batteryUpdate();

  /* -- スライド操作関連 -- */
  if (Keyboard.isConnected())
  {
    isBleConnected = true;

    // ダブルクリック判定（BtnA使用）
    if (M5.BtnA.wasPressed())
    {
      nowTime = millis();
      if (nowTime - lastClickTime < DoubleClickThreshold)
      {
        isMouseMode = !isMouseMode;
        Serial.printf("Mouse move mode: %s\n", isMouseMode ? "ON" : "OFF");
      }
      lastClickTime = nowTime;
    }

    // BtnA: 長押しスクロール or 短押し左クリック
    if (M5.BtnA.pressedFor(TIME_LONG_PRESS))
    {
      isBtnALongPressing = true;
      Serial.println("Scrolling down (long press)");
      Mouse.move(0, 0, -1.0); // ↓
    }
    else if (M5.BtnA.wasReleased())
    {
      if (!isBtnALongPressing)
      {
        Serial.println("Left click (short press)");
        Mouse.click(MOUSE_LEFT); // Left
      }
      else // 長押し後は押さない
      {
        isBtnALongPressing = false;
      }
    }
    // BtnB: 長押しスクロール or 短押し右クリック
    if (M5.BtnB.pressedFor(TIME_LONG_PRESS))
    {
      isBtnBLongPressing = true;
      Serial.println("Scrolling up (long press)");
      Mouse.move(0, 0, 1.0); // ↑
    }
    else if (M5.BtnB.wasReleased())
    {
      if (!isBtnBLongPressing)
      {
        Serial.println("Right click (short press)");
        Keyboard.write(KEY_BACKSPACE);
      }
      else // 長押し後は押さない
      {
        isBtnBLongPressing = false;
      }
    }

    // マウス移動（ジャイロで）
    // 上下左右傾き対応
    if (isMouseMode)
    {
      float gx, gy, gz;
      M5.IMU.getGyroData(&gx, &gy, &gz);
      float sensitivity = 0.05;
      int moveX = gy * sensitivity;
      int moveY = gx * sensitivity;

      if (abs(moveX) > 1 || abs(moveY) > 1)
      {
        Mouse.move(moveX, -moveY, 0);
      }
    }
  }
  else
  {
    isBleConnected = false;
  }

  /* -- Timer関連 -- */
  // 電源ボタンの押下状態を取得する。
  // HACK: GetBtnPress()は0以外の値は一度しか値を取得できないから、ここで取得する。
  //       格納しないで下のif文で呼ぶと、ボタンを長押ししても[1]ではなく、先に短い場合の[2]を返してしまうため、長押しの分岐に入らない。
  int axpBtn = M5.Axp.GetBtnPress();
  // 電源ボタンが短時間押されたときは、タイマーのStart/Stop
  if (axpBtn == 2)
  {
    if (timerStarted(presenTimer))
    {
      timerStop(presenTimer);
    }
    else
    {
      timerStart(presenTimer);
    }
  }
  // 電源ボタンが長時間(1.5s)押されたら、タイマーリセット
  if (axpBtn == 1)
  {
    timerStop(presenTimer);
    timerWrite(presenTimer, 0);
  }

  /* -- モニタ関連 -- */
  // 更新周期を1秒間隔にする
  if (nextMonitorMills < millis())
  {
    // Bluetooth接続状態の表示
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 95 + LCD_TOP_OFFSET);
    M5.Lcd.printf("BLE:");
    if (isBleConnected)
    {
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.printf("Yes");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
    else
    {
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.printf("No ");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }

    // Timer表示
    int presenTimerSec = int(timerReadSeconds(presenTimer));
    // 分は大きく
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(15, 50);
    M5.Lcd.printf("%2d", presenTimerSec / 60);
    // 秒は小さく
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(70, 50);
    M5.Lcd.printf("%02d", presenTimerSec % 60);
    // 時刻
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 115 + LCD_TOP_OFFSET);
    // Start/Stopの表示を切り替える
    M5.Lcd.printf("Time:");
    if (timerStarted(presenTimer))
    {
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.printf("Start");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
    else
    {
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.printf("Stop ");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
    // マウス
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(5, 135 + LCD_TOP_OFFSET);
    // Start/Stopの表示を切り替える
    M5.Lcd.printf("Mouse:");
    if (isMouseMode)
    {
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.printf("On ");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
    else
    {
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.printf("Off");
      M5.Lcd.setTextColor(WHITE, BLACK);
    }
    nextMonitorMills = millis() + 1000;
  }
}
