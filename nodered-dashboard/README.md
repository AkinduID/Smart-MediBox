# nodered-dashboard

Node-RED flow for the Smart MediBox dashboard. The flow connects to the public Mosquitto MQTT broker and provides a web UI for monitoring sensor readings and controlling the device.

## Prerequisites

- [Node-RED](https://nodered.org/docs/getting-started/) (v2.x or later recommended)
- `node-red-dashboard` palette installed

### Installing the dashboard palette

In Node-RED go to **Menu (☰) → Manage Palette → Install** and search for `node-red-dashboard`, then click **Install**.

## Importing the Flow

1. Open Node-RED (default: `http://localhost:1880`).
2. Go to **Menu (☰) → Import**.
3. Click **select a file to import** and choose `flows.json` from this directory, or paste the JSON directly.
4. Click **Import**, then **Deploy**.

## Accessing the Dashboard

Navigate to:

```
http://<node-red-host>:1880/ui
```

## Dashboard Panels

| Panel | Description |
|-------|-------------|
| Temperature | Gauge and live chart (°C) |
| Humidity | Gauge and live chart (%) |
| Light Intensity | Gauge and live chart (0–1) |
| Servo Control | Minimum angle slider, control factor slider, tablet preset dropdown |
| Alarm Control | Toggle switches for Alarm 1, Alarm 2, and Alarm 3 |

## MQTT Broker

The flow uses the public **Eclipse Mosquitto** test broker (`test.mosquitto.org:1883`) by default. To change this, open the **mqtt-broker** configuration node in Node-RED and update the host and port.

For full MQTT topic documentation see [`../docs/nodered-dashboard.md`](../docs/nodered-dashboard.md).
