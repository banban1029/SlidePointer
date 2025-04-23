# M5Stick Slide Pointer

M5Stick を使って作成した Bluetooth 対応のスライドポインタです。  
プレゼンテーション中に欲しい機能を盛り込みました。

![1](https://github.com/user-attachments/assets/466a9c81-3107-4c23-bc2c-ada48b90d3f9)


## 🔧 使用デバイス

- M5StickC Plus
- ESP32(M5Stickに内蔵)

## 📦 機能

- Bluetooth Device Name `SlidePointer:M5StickCPlus` 
- Status View On Screen
  - Connect Bluetooth
  - Time Function
  - Mouse Function
  - Battery
- LeftButton
  - one click: `Time Start/Stop`
  - press for 3s: `Time Reset`
  - press for 6s: `Power Off`
- FrontButton
  - one click: `Slide Next (= mouse left click)`
  - double click: `Mouse On/Off`
  - press for 3s: `Down Scroll`
- RightButton
  - one click: `Slide Reverse (= keyboad backspace)`
  - press for 3s: `Up Scroll`

## 💻 ソフトウェア構成

- Arduino IDE または PlatformIO
- 必要なライブラリ：
  - `M5StickCPlus`
  - `ESP32 BLE Combo Keyboard Mouse `

## 📝 使い方

1. Arduino IDE または PlatformIO を準備
2. このリポジトリのコードを M5Stick に書き込む
3. Connect: Bluetooth Device Name `SlidePointer:M5StickCPlus` 
4. Timer機能, Slide操作, Mouse操作(ジャイロ)

## 🛠 今後の拡張アイデア

- Mouseの操作精度を高める
- ポインター(赤色LEDやレーザー) との連携
