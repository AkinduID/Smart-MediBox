# device-firmware

Base firmware for the Smart MediBox. This version handles local time display, configurable medicine alarms, and on-screen temperature/humidity warnings. It does **not** include MQTT or servo control.

## Features

- NTP time synchronisation over Wi-Fi
- Real-time clock displayed on the SSD1306 OLED
- Two configurable medicine alarms (buzzer + LED)
- DHT22 temperature and humidity monitoring with on-screen warnings
- 4-button menu for setting the time and alarms

## Build & Flash

1. Open this folder (`device-firmware/`) in VS Code with PlatformIO installed.
2. PlatformIO will automatically resolve all library dependencies.
3. Click **Build** (✓) to verify compilation, then **Upload** (→) to flash to the ESP32.
4. Open the **Serial Monitor** at **115200 baud** to view startup messages.

## Configuration

### Wi-Fi

Edit the `WiFi.begin()` call in `src/main.cpp`:

```cpp
WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
```

### Timezone

Adjust `UTC_OFFSET` (in seconds) in `src/main.cpp`:

```cpp
#define UTC_OFFSET 0   // UTC; use 19800 for UTC+5:30, etc.
```

### Alarm Defaults

Pre-set alarm times are defined near the top of `src/main.cpp`:

```cpp
int alarm_hours[]   = {0, 1};   // hours for alarm 1 and alarm 2
int alarm_minutes[] = {1, 10};  // minutes for alarm 1 and alarm 2
```

## Pin Assignments

See [`../docs/hardware.md`](../docs/hardware.md) for the full pin table.

## Dependencies

| Library | Purpose |
|---------|---------|
| Adafruit GFX Library | OLED graphics |
| Adafruit SSD1306 | OLED driver |
| Adafruit BusIO | I²C/SPI bus abstraction |
| DHTesp | DHT22 sensor |
