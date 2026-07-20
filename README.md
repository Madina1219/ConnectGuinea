# Guinea Smart Community Totem
A community-focused smart infrastructure prototype designed for future solar-powered deployment in Guinea.

## About the Project
The **Guinea Smart Community Totem** explores how connected technologies can support communities experiencing limited access to electricity, public lighting and local emergency-warning infrastructure.

The prototype combines three main services:
- Automatic community lighting
- A wireless visual flood-warning system
- Shared USB device charging
- 
The system is designed to operate locally without depending on Wi-Fi, mobile data or internet access.

## Main Functions

### Automatic Lighting

The totem uses a **TEMT6000 ambient-light sensor** to measure the surrounding light level.

When the environment becomes dark, the main light switches on automatically. When the environment becomes bright again, the light returns to its normal daytime state.

This demonstrates how the totem could provide automatic public lighting from dusk until dawn.

### Visual Flood Alert
A separate portable flood-alert transmitter sends a wireless warning to the totem.
When a flood alert is received:
1. The totem light flashes blue to attract attention.
2. The light then remains continuously blue while the flood risk is active.
3. When the warning is cleared, the totem returns to its normal automatic-lighting mode.

The flood-warning system is entirely visual.
The transmitter and totem communicate locally without requiring internet access.

### USB Device Charging
The totem provides USB ports for charging mobile phones and other small devices.
Charging does not require a push button and is not controlled through the totem software. Users connect their device directly to an available USB charging port.
When other people are waiting, each user should limit their charging session to a maximum of **20 minutes** before giving someone else a turn.

## Why the Project Matters
Some communities in Guinea experience challenges including:
- Limited or unreliable access to electricity
- Insufficient public lighting
- Increased exposure to flooding
- Limited access to visible local warning systems
- Difficulty charging mobile phones during power interruptions

Mobile phones are important for communication, emergency information and maintaining contact with family members.

The project investigates how one shared community object could provide lighting, flood-risk communication and device charging.

## System Overview
The prototype consists of two connected devices:

### Smart Community Totem
The main totem contains:
- A Heltec WiFi LoRa 32 V3 development board
- A TEMT6000 ambient-light sensor
- An LED lighting system
- A battery-powered electrical system
- USB charging connections
- A visual flood-alert receiver

### Flood-Alert Transmitter
The separate portable transmitter contains:
- A second Heltec development board
- A small flood-alert button
- A wireless communication system
- A portable power supply

Pressing the alert button sends a flood-warning message to the main totem.

## Hardware
| Component | Role |
|---|---|
| Heltec WiFi LoRa 32 V3.2 | Main controller (ESP32-S3) |
| 12V RGB LED strip | Street lighting via IRLZ44N MOSFETs (GPIO 19/20/4) |
| TEMT6000 ambient light sensor | Auto on/off with hysteresis (GPIO1) |
| NeoPixel ring x2 (24 total) | Flood alert status indicator, chained (GPIO38, 5V) |
| INA219 current sensor | Power monitoring via I2C (GPIO41/42, address 0x41) |
| MicroSD module | Data logging via SPI (GPIO 33/34/35/36, 5V) |
| USB charging port | 5V phone charging, 20-min timed session via mechanical spring-wound SPDT timer |
| DS3231 RTC module | Real timestamps for SD logging (I2C, confirmed working) |
| Second Heltec WiFi LoRa 32 V3.2 | Flood alert transmitter, communicates via ESP-NOW |
| LiFePO₄ battery 12V 50Ah | Off-grid power storage |
| 150W 18V solar panel | Primary energy source |
| 20A MPPT charge controller | Solar charging regulation |

## Status
**Prototype build — exhibited at Degree Show 2026**
- ✅ RGB LED strip — auto on/off via light sensor with hysteresis
- ✅ Ambient light sensor — TEMT6000 on GPIO1, confirmed working
- ✅ NeoPixel rings — flood alert colour states confirmed working
- ✅ USB charging — mechanical 20-minute timer, no button trigger needed
- ✅ INA219 power monitor — live voltage/current/wattage readings
- ✅ MicroSD module — FAT32 formatted, write/read confirmed
- ✅ State machine — non-blocking millis()-based light and alert logic built and tested
- ✅ DS3231 RTC — confirmed working, coin cell backup survives reboot
- ✅ ESP-NOW flood alert pairing — second Heltec transmitter, confirmed sending reliably
  
## Prototype Status

**Functional proof-of-concept completed**
- [x] Automatic light activation using the ambient-light sensor
- [x] Normal lighting behaviour tested
- [x] Wireless communication between the transmitter and totem
- [x] Flood-alert message transmission tested
- [x] Blue flashing flood-alert pattern implemented
- [x] Continuous blue warning state implemented
- [x] Return to normal lighting after the warning is cleared
- [x] USB device-charging demonstration
- [x] Prototype assembled for public exhibition
- [x] Demonstrated at the Connected Environments Degree Show 2026

## Repository Structure
Smartstreet-Guinea/
│
├── Arduino Codes/
│   ├── Totem Receiver
│   ├── Flood Transmitter
│   └── Test Sketches
│
├── docs/
│   ├── Wiring Diagrams
│   ├── System Architecture
│   ├── User Guide
│   └── Project Documentation
│
├── building stages Images/
│   └── Prototype Development Photos
│
├── components Images/
│   └── Hardware Components
│
├── README.md
├── LICENSE
└── .gitignore
