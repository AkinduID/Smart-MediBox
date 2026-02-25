# Hardware Documentation

## Overview

The Smart MediBox is built around the **ESP32 DOIT DevKit V1** microcontroller. It uses an OLED display for local feedback, a DHT22 sensor for environmental monitoring, a servo motor for the shading panel, dual LDR sensors for light-direction detection, a buzzer and LED for alarm indication, and four pushbuttons for on-device navigation.

---

## Components

| Component | Model / Specification | Quantity |
|-----------|-----------------------|----------|
| Microcontroller | ESP32 DOIT DevKit V1 | 1 |
| OLED display | SSD1306, 128 × 64 px, I²C | 1 |
| Temperature & humidity sensor | DHT22 | 1 |
| Servo motor | Standard 5 V hobby servo | 1 |
| Light-dependent resistor | LDR (photoresistor) | 2 |
| Buzzer | Active/passive 5 V buzzer | 1 |
| LED | Any colour, 3.3 V compatible | 1 |
| Pushbutton | Momentary tactile switch | 4 |

---

## Pin Assignments

### Updated firmware (`device-firmware-updates`)

| Signal | ESP32 GPIO | Direction |
|--------|-----------|-----------|
| Buzzer | GPIO 5 | Output |
| LED | GPIO 15 | Output |
| Button – Cancel | GPIO 34 | Input |
| Button – OK | GPIO 25 | Input |
| Button – Up | GPIO 26 | Input |
| Button – Down | GPIO 35 | Input |
| DHT22 data | GPIO 12 | Input |
| Servo PWM | GPIO 27 | Output |
| LDR right | GPIO 32 (ADC) | Analog Input |
| LDR left | GPIO 33 (ADC) | Analog Input |
| OLED SDA | GPIO 21 (default I²C) | I²C |
| OLED SCL | GPIO 22 (default I²C) | I²C |

### Base firmware (`device-firmware`)

Identical to the above except:

| Signal | ESP32 GPIO |
|--------|-----------|
| Button – OK | GPIO 32 |
| Button – Up | GPIO 33 |
| Button – Down | GPIO 35 |

> **Note:** The base firmware does not include the servo or LDR connections.

---

## OLED Display (SSD1306)

- **Interface:** I²C
- **I²C address:** `0x3C`
- **Resolution:** 128 × 64 pixels
- Connected to the ESP32's default I²C pins (SDA = GPIO 21, SCL = GPIO 22).

---

## DHT22 Sensor

- **Protocol:** Single-wire
- **Data pin:** GPIO 12
- **Operating range:** Temperature −40 °C to +80 °C; Humidity 0–100 % RH
- The firmware warns on-screen when:
  - Temperature > 35 °C (`TEMP HIGH`) or < 25 °C (`TEMP LOW`)
  - Humidity > 40 % (`HUMIDITY HIGH`) or < 20 % (`HUMIDITY LOW`)

---

## Servo Motor

- Connected to GPIO 27 (PWM output via the `ESP32Servo` library).
- Controls a shading panel on the medicine compartment.
- Position is determined by the following formula:

  ```
  angle = min( minAngle × D + (180 − minAngle) × max(ValueR, ValueL) × ctrlFactor, 180 )
  ```

  Where:
  - `minAngle` – minimum servo angle (0–120°), set via dashboard slider or preset
  - `ctrlFactor` – scaling factor (0.0–1.0), set via dashboard slider or preset
  - `D` – direction factor: `1.5` when the left LDR is brighter, `0.5` otherwise
  - `ValueR`, `ValueL` – normalised LDR readings (0.0–1.0)

---

## LDR Sensors

Two LDRs are used to detect the direction of the brightest light source:

| Sensor | GPIO | Location |
|--------|------|----------|
| LDR right | GPIO 32 | Right side of shading panel |
| LDR left | GPIO 33 | Left side of shading panel |

- ADC resolution is set to 10 bits; raw values are normalised: `value = 1 − (raw / 1023)`.
- The brighter side determines the `D` factor used in the servo angle calculation.

---

## Alarm Indicators

| Component | GPIO | Behaviour |
|-----------|------|-----------|
| Buzzer | GPIO 5 | Plays a scale of 8 notes (C D E F G A B C₅) repeatedly until cancelled |
| LED | GPIO 15 | Lit high for the duration of the alarm |

Pressing the **Cancel** button (GPIO 34) stops the alarm at any point during the tone sequence.

---

## Pushbutton Navigation

The four pushbuttons are used for on-device menu navigation:

| Button | GPIO | Function in menu |
|--------|------|-----------------|
| OK | GPIO 25 | Confirm / enter selected option |
| Cancel | GPIO 34 | Go back / exit menu / stop alarm |
| Up | GPIO 26 | Scroll up / increment value |
| Down | GPIO 35 | Scroll down / decrement value |
