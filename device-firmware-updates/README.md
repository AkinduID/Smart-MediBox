# device-firmware-updates

Production firmware for the Smart MediBox. This version extends the base firmware with MQTT connectivity, servo motor control driven by dual LDR sensors, and remote alarm management via a Node-RED dashboard.

## Features

- All features of the base firmware (time display, alarms, DHT22 monitoring)
- MQTT client connected to `test.mosquitto.org`
- Publishes temperature, humidity, and light-intensity readings to the dashboard
- Subscribes to servo control parameters (`MEDIBOX-ANGLE`, `MEDIBOX-CTRLFACTOR`)
- Subscribes to alarm toggle commands (`MEDIBOX-ALARM1`, `MEDIBOX-ALARM2`, `MEDIBOX-ALARM3`)
- Servo position calculated from dual-LDR light-direction detection
- UTC offset adjustable at runtime via the on-device menu

## Build & Flash

1. Open this folder (`device-firmware-updates/`) in VS Code with PlatformIO installed.
2. PlatformIO will automatically resolve all library dependencies.
3. Click **Build** (✓) to verify compilation, then **Upload** (→) to flash to the ESP32.
4. Open the **Serial Monitor** at **115200 baud** to view startup and MQTT debug output.

## Configuration

### Wi-Fi

Edit `SetupWiFi()` in `src/main.cpp`:

```cpp
WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
```

### Timezone

Adjust the initial value of `UTC_OFFSET` (in seconds) at the top of `src/main.cpp`:

```cpp
int UTC_OFFSET = 19800;  // UTC+5:30; use 0 for UTC
```

### MQTT Broker

Update `setupMqtt()` in `src/main.cpp` to point to a different broker:

```cpp
mqttClient.setServer("your.broker.host", 1883);
```

### Servo Defaults

Initial servo parameters are set at the top of `src/main.cpp`:

```cpp
float minAngle   = 30;    // minimum servo angle (degrees)
float ctrlFactor = 0.75;  // control factor (0.0–1.0)
```

These are also published to the MQTT broker on first connection so the dashboard sliders update to match.

## Pin Assignments

See [`../docs/hardware.md`](../docs/hardware.md) for the full pin table.

## MQTT Topics

See [`../docs/nodered-dashboard.md`](../docs/nodered-dashboard.md) for the complete list of MQTT topics.

## Dependencies

| Library | Purpose |
|---------|---------|
| Adafruit GFX Library | OLED graphics |
| Adafruit SSD1306 | OLED driver |
| Adafruit BusIO | I²C/SPI bus abstraction |
| DHTesp | DHT22 sensor |
| PubSubClient | MQTT client |
| ESP32Servo | Servo PWM control |
