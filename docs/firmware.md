# Firmware Documentation

## Overview

The Smart MediBox firmware is written in **C++ for the Arduino framework** and built with **PlatformIO**. There are two firmware versions in this repository:

| Directory | Description |
|-----------|-------------|
| `device-firmware/` | Base version – local time display, alarms, and temperature/humidity warnings. No MQTT. |
| `device-firmware-updates/` | Production version – all base features plus MQTT telemetry, alarm remote control, and servo management. |

---

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- ESP32 board driver (usually installed automatically by PlatformIO)
- USB cable to connect the ESP32

---

## Building and Flashing

1. Open the desired firmware folder (`device-firmware/` or `device-firmware-updates/`) in VS Code.
2. PlatformIO will automatically download all required libraries defined in `platformio.ini`.
3. Click **Build** (✓ icon) to compile, or **Upload** (→ icon) to compile and flash.
4. Open the **Serial Monitor** at **115200 baud** to view debug output.

### Target Board

Both projects target the **ESP32 DOIT DevKit V1**:

```ini
[env:esp32doit-devkit-v1]
platform  = espressif32
board     = esp32doit-devkit-v1
framework = arduino
```

### Library Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit GFX Library | ^1.10.0 | OLED graphics primitives |
| Adafruit SSD1306 | ^2.4.0 | SSD1306 OLED driver |
| Adafruit BusIO | ^1.7.0 | I²C/SPI abstraction |
| DHTesp | latest | DHT22 sensor driver |
| PubSubClient | latest | MQTT client *(updated firmware only)* |
| ESP32Servo | latest | Servo PWM control *(updated firmware only)* |

---

## Wi-Fi Configuration

The firmware connects to a Wi-Fi network at startup. The default SSID is `Wokwi-GUEST` (password-free), which is the virtual Wi-Fi network provided by the [Wokwi](https://wokwi.com/) simulator.

To use a different network, update the `WiFi.begin()` call in `setup()`:

```cpp
// device-firmware/src/main.cpp
WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");

// device-firmware-updates/src/main.cpp
WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
```

---

## Time Configuration

The firmware synchronises the real-time clock via NTP:

```cpp
#define NTP_SERVER     "pool.ntp.org"
int    UTC_OFFSET     = 19800;   // seconds east of UTC (e.g. 19800 = UTC+5:30)
#define UTC_OFFSET_DST 0
```

The UTC offset can also be adjusted at runtime via the **Set Time** menu option, which increments/decrements `UTC_OFFSET` by 3600 s (hours) or 60 s (minutes) as the user scrolls.

---

## Alarm Configuration

- Up to **3 alarms** are supported (controlled from the Node-RED dashboard via MQTT in the updated firmware).
- Default alarm times are hard-coded as:
  ```cpp
  int alarm_hours[]   = {0, 1};
  int alarm_minutes[] = {1, 10};
  ```
- Each alarm triggers once per day; `alarm_triggered[]` is reset to `false` at the next day boundary.

---

## Application Architecture

### `setup()`

1. Configures all GPIO pins.
2. Initialises the OLED display, DHT22 sensor, and (updated firmware) servo.
3. Connects to Wi-Fi.
4. Synchronises time with NTP.
5. *(Updated firmware only)* Connects to the MQTT broker and subscribes to control topics.

### `loop()`

1. Calls `update_time_with_check_alarm()` – refreshes the display and fires any due alarms.
2. Checks whether the **OK** button has been pressed to enter the menu.
3. *(Updated firmware only)* Calls `mqttFunc()` – handles MQTT keep-alive, publishes sensor data.

---

## Function Reference

### Display

| Function | Signature | Description |
|----------|-----------|-------------|
| `print_line` | `(String text, int row, int column, int text_size)` | Renders a single line of text at the given cursor position. |
| `print_time_now` | `(void)` | Clears the display and renders the current day/hour/minute/second. |

### Time

| Function | Signature | Description |
|----------|-----------|-------------|
| `update_time` | `(void)` | Reads the system time (NTP-synced) into the global `hours`, `minutes`, `seconds`, `days` variables. |
| `update_time_with_check_alarm` | `(void)` | Calls `update_time()`, refreshes the display, and triggers any pending alarms. |
| `set_time` | `(void)` | Interactive menu to adjust the UTC offset (hour and minute steps). |

### Alarms

| Function | Signature | Description |
|----------|-----------|-------------|
| `ring_alarm` | `(void)` | Activates the LED and plays the note sequence on the buzzer until the Cancel button is pressed. |
| `set_alarm` | `(int alarm)` | Interactive menu to configure the hour and minute for the specified alarm index. |

### Menu

| Function | Signature | Description |
|----------|-----------|-------------|
| `goto_menu` | `(void)` | Enters the navigation menu. Exits when Cancel is pressed. |
| `run_mode` | `(int mode)` | Dispatches to `set_time()`, `set_alarm()`, or disables alarms based on the selected mode index. |
| `wait_for_button_press` | `(void) → int` | Blocking wait; returns the GPIO pin number of the first button pressed. |

### Sensors *(both firmware versions)*

| Function | Signature | Description |
|----------|-----------|-------------|
| `check_temp` | `(void)` | Reads DHT22 and shows on-screen warnings for out-of-range temperature and humidity. *(base firmware)* |
| `updateTemp` | `(void)` | Reads DHT22, publishes readings to `MEDIBOX-TEMP` and `MEDIBOX-HUMIDITY`, and shows warnings. *(updated firmware)* |
| `updateLux` | `(void)` | Reads both LDRs, determines the brighter side, publishes to `MEDIBOX-LUX`, and updates the servo. *(updated firmware)* |

### MQTT *(updated firmware only)*

| Function | Signature | Description |
|----------|-----------|-------------|
| `SetupWiFi` | `(void)` | Connects to the Wi-Fi network and waits for `WL_CONNECTED`. |
| `setupMqtt` | `(void)` | Registers the broker address and the receive callback. |
| `connectToBroker` | `(void)` | Connects to the broker, subscribes to all control topics, and publishes initial servo parameters. |
| `mqttFunc` | `(void)` | Called from `loop()`; reconnects if needed, runs the MQTT client loop, and publishes sensor data. |
| `recieveCallback` | `(char* topic, byte* payload, unsigned int length)` | Handles incoming MQTT messages for servo angle, control factor, and alarm commands. |

### Servo *(updated firmware only)*

| Function | Signature | Description |
|----------|-----------|-------------|
| `updateServoPos` | `(void)` | Recalculates and writes the servo angle using the LDR readings and the current `minAngle` / `ctrlFactor` values. |

---

## Menu Modes

The on-device menu provides four options, cycled with the Up/Down buttons and confirmed with OK:

| Index | Label | Action |
|-------|-------|--------|
| 0 | `1-Set Time` | Adjust UTC offset hour and minute |
| 1 | `2-Set Alarm 1` | Set hour and minute for Alarm 1 |
| 2 | `3-Set Alarm 2` | Set hour and minute for Alarm 2 |
| 3 | `4-Disable Alarms` | Set `alarm_enabled = false` |
