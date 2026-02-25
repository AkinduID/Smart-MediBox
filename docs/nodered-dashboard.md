# Node-RED Dashboard Documentation

## Overview

The Smart MediBox Node-RED dashboard provides a web-based interface for:

- **Monitoring** real-time sensor readings (temperature, humidity, light intensity)
- **Controlling** the servo shading panel (minimum angle and control factor)
- **Managing** medicine alarms (enable / disable each alarm remotely)

All communication between the ESP32 and Node-RED uses **MQTT** over the public `test.mosquitto.org` broker on port **1883**.

---

## Importing the Flow

1. Open your Node-RED instance (default: `http://localhost:1880`).
2. Click the **☰ Menu** button in the top-right corner.
3. Select **Import**.
4. Click **select a file to import** and choose `nodered-dashboard/flows.json`, or paste the file contents directly into the text area.
5. Click **Import**, then **Deploy**.

> **Prerequisite:** The `node-red-dashboard` palette must be installed.  
> Go to **Menu → Manage Palette → Install** and search for `node-red-dashboard`.

---

## MQTT Topics

### Published by the ESP32 (subscribe in Node-RED)

| Topic | Payload | Description |
|-------|---------|-------------|
| `MEDIBOX-TEMP` | Float string, e.g. `"26.50"` | Current temperature in °C |
| `MEDIBOX-HUMIDITY` | Float string, e.g. `"35.20"` | Current relative humidity in % |
| `MEDIBOX-LUX` | Float string, 0.0–1.0, e.g. `"0.73"` | Normalised light intensity (brightest LDR) |

### Published by Node-RED (subscribe on ESP32)

| Topic | Payload | Description |
|-------|---------|-------------|
| `MEDIBOX-ANGLE` | Float string, 0–120, e.g. `"30"` | Minimum servo angle in degrees |
| `MEDIBOX-CTRLFACTOR` | Float string, 0.0–1.0, e.g. `"0.25"` | Servo control factor |
| `MEDIBOX-ALARM1` | `"true"` / `"false"` | Enable / disable Alarm 1 |
| `MEDIBOX-ALARM2` | `"true"` / `"false"` | Enable / disable Alarm 2 |
| `MEDIBOX-ALARM3` | `"true"` / `"false"` | Enable / disable Alarm 3 |

### Bidirectional (published by both sides)

| Topic | Direction | Purpose |
|-------|-----------|---------|
| `MEDIBOX-ANGLE` | ESP32 → Node-RED on connect; Node-RED → ESP32 on slider change | Keep slider and device in sync |
| `MEDIBOX-CTRLFACTOR` | ESP32 → Node-RED on connect; Node-RED → ESP32 on slider change | Keep slider and device in sync |

---

## Dashboard Layout

The dashboard is organised under a single **MediBox** tab with five groups.

### Temperature Group

| Widget | Type | Topic | Range |
|--------|------|-------|-------|
| Gauge | `ui_gauge` | `MEDIBOX-TEMP` | 0–50 °C |
| Line chart | `ui_chart` | `MEDIBOX-TEMP` | last 60 s |

### Humidity Group

| Widget | Type | Topic | Range |
|--------|------|-------|-------|
| Gauge | `ui_gauge` | `MEDIBOX-HUMIDITY` | 0–50 % |
| Line chart | `ui_chart` | `MEDIBOX-HUMIDITY` | last 60 s |

### Light Intensity Group

| Widget | Type | Topic | Range |
|--------|------|-------|-------|
| Gauge | `ui_gauge` | `MEDIBOX-LUX` | 0–1 |
| Line chart | `ui_chart` | `MEDIBOX-LUX` | last 60 s |

### Servo Control Group

| Widget | Type | Topic | Range | Description |
|--------|------|-------|-------|-------------|
| Tablet preset dropdown | `ui_dropdown` | — | Custom / Preset 1–3 | Applies predefined angle + factor pairs |
| Minimum angle slider | `ui_slider` | `MEDIBOX-ANGLE` | 0–120° | Sets the `minAngle` variable on the ESP32 |
| Control factor slider | `ui_slider` | `MEDIBOX-CTRLFACTOR` | 0.0–1.0 (step 0.05) | Sets the `ctrlFactor` variable on the ESP32 |

#### Tablet Presets

| Preset | Minimum Angle | Control Factor |
|--------|--------------|----------------|
| Custom | 0° | 0.00 |
| Preset 1 | 30° | 0.25 |
| Preset 2 | 60° | 0.50 |
| Preset 3 | 120° | 1.00 |

Selecting a preset automatically moves both sliders to the corresponding values and publishes them to the ESP32.

### Alarm Control Group

| Widget | Type | Topic | Description |
|--------|------|-------|-------------|
| Alarm 1 switch | `ui_switch` | `MEDIBOX-ALARM1` | Toggles Alarm 1 on the device |
| Alarm 2 switch | `ui_switch` | `MEDIBOX-ALARM2` | Toggles Alarm 2 on the device |
| Alarm 3 switch | `ui_switch` | `MEDIBOX-ALARM3` | Toggles Alarm 3 on the device |

> **Note:** The ESP32 currently implements two alarm slots. `MEDIBOX-ALARM3` is subscribed to on the device but does not map to an alarm slot in the current firmware version.

---

## MQTT Broker

The flow is pre-configured to use the public **Eclipse Mosquitto** test broker:

| Setting | Value |
|---------|-------|
| Broker host | `test.mosquitto.org` |
| Port | `1883` |
| TLS | Disabled |
| Client ID | Auto-generated |
| Protocol | MQTT v3.1 |

To use a private broker, open the broker configuration node in Node-RED and update the host and port.

---

## Accessing the Dashboard

Once Node-RED is running and the flow is deployed, open:

```
http://<node-red-host>:1880/ui
```

Replace `<node-red-host>` with `localhost` if running locally, or with the IP address / hostname of your Node-RED server.
