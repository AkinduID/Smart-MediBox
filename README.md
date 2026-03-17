# Smart MediBox

<a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=c%2B%2B&logoColor=white"/></a>
<a href="https://nodered.org/"><img src="https://img.shields.io/badge/Node--RED-8F2F23?style=flat&logo=node-red&logoColor=white"/></a>
<a href="https://www.espressif.com/en/products/socs/esp32"><img src="https://img.shields.io/badge/ESP32-000000?style=flat&logo=espressif&logoColor=white"/></a>
<a href="https://platformio.org/"><img src="https://img.shields.io/badge/PlatformIO-FF7F32?style=flat&logo=platformio&logoColor=white"/></a>
<a href="https://code.visualstudio.com/"><img src="https://img.shields.io/badge/VS%20Code-007ACC?style=flat&logo=visual-studio-code&logoColor=white"/></a>

A smart medicine container built on the ESP32 microcontroller that reminds patients when to take their medicine. The device displays the current time, monitors temperature and humidity, and adjusts a servo-driven shading panel based on ambient light intensity. It can be fully monitored and controlled through a Node-RED dashboard over MQTT.

> Developed as part of the **EN2853 – Embedded Systems and Applications** module.

<img src="https://github.com/AkinduID/Smart-MediBox/blob/main/docs/medibox.png"/>

---

## Table of Contents

- [Features](#features)
- [Repository Structure](#repository-structure)
- [Hardware Overview](#hardware-overview)
- [Firmware Versions](#firmware-versions)
- [Node-RED Dashboard](#node-red-dashboard)
- [Quick Start](#quick-start)
- [Documentation](#documentation)

---

## Features

- **Real-time clock** – synchronised with NTP over Wi-Fi (UTC offset configurable)
- **Medicine alarms** – up to 3 configurable alarms with buzzer and LED indication
- **Temperature & humidity monitoring** – DHT22 sensor with on-screen warnings and MQTT telemetry
- **Light-intensity monitoring** – dual LDR sensors reporting to the dashboard
- **Servo-controlled shading panel** – angle calculated from light direction and a tunable control factor
- **OLED display** – 128 × 64 pixel SSD1306 display for time, warnings, and menu navigation
- **4-button menu** – Up / Down / OK / Cancel pushbuttons for local configuration
- **Node-RED dashboard** – remote monitoring of sensors and remote control of alarms and servo

---

## Repository Structure

```
Smart-MediBox/
├── device-firmware/          # Base firmware (no MQTT)
│   ├── src/
│   │   └── main.cpp          # Main application code
│   └── platformio.ini        # PlatformIO build configuration
├── device-firmware-updates/  # Updated firmware (with MQTT & servo)
│   ├── src/
│   │   └── main.cpp          # Main application code
│   └── platformio.ini        # PlatformIO build configuration
├── nodered-dashboard/
│   └── flows.json            # Node-RED flow export
└── docs/
    ├── hardware.md           # Hardware components and pin assignments
    ├── firmware.md           # Firmware architecture and function reference
    └── nodered-dashboard.md  # Dashboard setup and MQTT protocol
```

---

## Hardware Overview

| Component | Description |
|-----------|-------------|
| ESP32 DOIT DevKit V1 | Main microcontroller |
| SSD1306 (128×64 OLED) | Display via I²C |
| DHT22 | Temperature & humidity sensor |
| Servo motor | Shading panel actuator |
| LDR × 2 | Light-direction detection (left / right) |
| Buzzer | Alarm tone output |
| LED | Alarm visual indicator |
| Pushbuttons × 4 | Up / Down / OK / Cancel navigation |

For full pin-assignment tables see [`docs/hardware.md`](docs/hardware.md).

---

## Firmware Versions

### `device-firmware` (base)
The first iteration of the firmware. Handles time display, alarm management, and basic temperature/humidity warnings without any network connectivity beyond NTP synchronisation.

### `device-firmware-updates` (MQTT-enabled)
The production firmware. Extends the base version with:
- MQTT client (PubSubClient) connected to `test.mosquitto.org`
- Publishes temperature, humidity, and light-intensity readings
- Subscribes to alarm enable/disable commands and servo control parameters
- Servo position calculated from dual-LDR readings and a configurable control factor

See [`device-firmware/README.md`](device-firmware/README.md) and [`device-firmware-updates/README.md`](device-firmware-updates/README.md) for build and flash instructions.

---

## Node-RED Dashboard

The dashboard (imported from `nodered-dashboard/flows.json`) provides:

| Panel | Contents |
|-------|----------|
| Temperature | Gauge and live line chart |
| Humidity | Gauge and live line chart |
| Light Intensity | Gauge and live line chart |
| Servo Control | Minimum angle slider, control-factor slider, tablet-preset dropdown |
| Alarm Control | Toggle switches for Alarm 1, 2, and 3 |

See [`nodered-dashboard/README.md`](nodered-dashboard/README.md) for import instructions.

---

## Quick Start

1. **Flash the firmware** – open `device-firmware-updates/` in VS Code with PlatformIO, build, and upload to the ESP32.
2. **Import the Node-RED flow** – in Node-RED go to **Menu → Import**, paste the contents of `nodered-dashboard/flows.json`, and deploy.
3. **Power on the device** – the ESP32 connects to Wi-Fi (`Wokwi-GUEST` by default), syncs the time, and begins publishing sensor data.
4. **Open the dashboard** – navigate to `http://<node-red-host>:1880/ui` to monitor and control the MediBox.

---

## Documentation

| Document | Description |
|----------|-------------|
| [`docs/hardware.md`](docs/hardware.md) | Component list, pin assignments, wiring notes |
| [`docs/firmware.md`](docs/firmware.md) | Firmware architecture, function reference, configuration |
| [`docs/nodered-dashboard.md`](docs/nodered-dashboard.md) | Dashboard panels, MQTT topics, import guide |
| [`device-firmware/README.md`](device-firmware/README.md) | Base firmware build & flash guide |
| [`device-firmware-updates/README.md`](device-firmware-updates/README.md) | MQTT firmware build & flash guide |
| [`nodered-dashboard/README.md`](nodered-dashboard/README.md) | Node-RED flow import guide |
