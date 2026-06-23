# SmartLight Totem
A solar-powered community infrastructure prototype for the neighbourhoods of Guinea.

## What it is
SmartLight Totem is a fully off-grid solar totem that brings five things to a neighbourhood:
- Automatic street lighting — turns on at dusk, off at dawn via ambient light sensor
- Visual flood alert system — receives LoRa wireless alerts and signals danger levels via a colour-coded status ring
- USB phone charging — timed 30-minute sessions triggered by a push button
- Real-time power monitoring — tracks voltage, current and wattage continuously
- Local data logging — stores power and event data to SD card with timestamps

## Why
- ~51% of Guinea's population has access to electricity (25.7% rural)
- 30+ deaths from flooding in Conakry in June–August 2025 alone
- No visible street-level warning system currently exists in informal settlements
- Community members need a reliable, visible, civic object that works without the grid

## Hardware
| Component | Role |
|---|---|
| Heltec WiFi LoRa 32 V3 | Main controller (ESP32-S3 + LoRa SX1262 + OLED) |
| 12V RGB LED strip | Street lighting via IRLZ44N MOSFETs (GPIO 19/20/21) |
| TEMT6000 ambient light sensor | Auto on/off with hysteresis (GPIO1) |
| NeoPixel ring 12 | Flood alert status indicator (GPIO38, 5V) |
| INA219 current sensor | Power monitoring via I2C (GPIO41/42, address 0x41) |
| MicroSD module | Data logging via SPI2 (GPIO 33/34/35/36, 5V) |
| Push button | USB charging trigger (GPIO47) |
| USB charging port | 5V 3.1A phone charging via MOSFET (GPIO48) |
| DS3231 RTC module | Real timestamps for SD logging (I2C, on order) |
| LiFePO₄ battery 12V 50Ah | Off-grid power storage |
| 150W 18V solar panel | Primary energy source |
| 20A MPPT charge controller | Solar charging regulation |

## Status
**Prototype build — hardware integration phase**

- ✅ RGB LED strip — auto on/off via light sensor with hysteresis
- ✅ Ambient light sensor — TEMT6000 on GPIO1, confirmed working
- ✅ NeoPixel ring 12 — flood alert colour states confirmed working
- ✅ Push button — GPIO47, state machine logic confirmed
- ✅ INA219 power monitor — live voltage/current/wattage readings
- ✅ MicroSD module — FAT32 formatted, write/read confirmed
- ✅ State machine — timed light control logic built and tested
- ⏳ DS3231 RTC — on order, real timestamps pending
- ⏳ USB charging MOSFET — IRLZ44N on order
- ⏳ LoRa flood alert pairing — second Heltec transmitter, next phase
- ⏳ Blue channel MOSFET replacement — on order

## What's in this repo
- `firmware/` — Arduino sketches for each subsystem and master sketch
- `docs/` — project narrative, briefs, wiring reference document
- `design/` — system connection diagram, totem architecture
- `research/` — flooding data, electricity access stats, field notes

## Connected Environments Degree Show 2026
UCL Bartlett School of Architecture
ucfndia@ucl.ac.uk
