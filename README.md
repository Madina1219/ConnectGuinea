# Guinea Smart Community Totem

A community-focused smart infrastructure prototype exploring how connected technologies could support **automatic public lighting, flood-risk communication and shared device charging** in communities in Guinea.

---

## Final Prototype

<p align="center">
  <img src=<img width="406" height="647" alt="Final Prototype" src="https://github.com/user-attachments/assets/ba2e89c9-d6c2-4280-92ed-880876d1831d" />
"
       alt="Final Guinea Smart Community Totem prototype exhibited at the Connected Environments Degree Show 2026"
       width="500">
</p>

<p align="center">
  <em>Final Guinea Smart Community Totem proof-of-concept exhibited at the Connected Environments Degree Show 2026.</em>
</p>

---

## About the Project

The **Guinea Smart Community Totem** is a functional proof-of-concept developed as part of the **MSc Connected Environments** programme at University College London.

The project investigates how a single community infrastructure object could combine three services:

- 💡 **Automatic community lighting**
- 🌊 **Visual flood-warning communication**
- 🔋 **Shared USB device charging**

The prototype was developed around the context of communities where access to reliable electricity, public lighting and locally visible emergency information may be limited.

The final system was designed, built, integrated, tested and publicly demonstrated at the **Connected Environments Degree Show 2026**.

---

## Main Functions

### 💡 Automatic Lighting

The totem uses a **TEMT6000 ambient-light sensor** to monitor surrounding light levels.

When the environment becomes dark, the main lighting system switches on automatically. When sufficient ambient light returns, the system returns to its daytime state.

Hysteresis is implemented in the control logic to prevent rapid switching when the measured light level is close to the activation threshold.

This demonstrates how a future outdoor version of the totem could provide automatic dusk-to-dawn community lighting.

---

### 🌊 Visual Flood Warning

The prototype includes a visual flood-warning system designed to make an incoming warning immediately noticeable within the surrounding community.

For the proof-of-concept and exhibition, a second portable Heltec device acts as a **flood-alert demonstrator**.

When an alert is transmitted:

1. The Smart Community Totem receives the wireless warning.
2. The lighting system immediately **flashes blue** to attract attention.
3. The light then remains **continuously blue** while the warning is active.
4. Once the warning is cleared, the system returns to its normal automatic-lighting behaviour.

The warning system is intentionally **visual rather than audible**.

### 🎥 Flood-Warning Demonstration

The video below demonstrates the final prototype responding to a simulated flood warning.

https://github.com/user-attachments/assets/4ddb0561-cca5-4e6a-a8d4-dea9fc01bdc1

```

**Demonstration sequence:**

```text
Flood Alert Received
        │
        ▼
Blue Flashing Warning
        │
        ▼
Continuous Blue Warning
        │
        ▼
Flood Warning Cleared
        │
        ▼
Return to Normal Operation
```

---

### 🔋 USB Device Charging

The totem provides USB charging for mobile phones and other small devices.

Charging does **not** require an electronic push button or software trigger. A device is connected directly to the available USB charging port.

A **mechanical spring-wound timer** supports fair access to the shared charging facility.

When other people are waiting, each user should limit their charging session to a maximum of:

> **20 minutes per turn**

The time limit is intended to allow more members of the community to access the shared charging facility.

---

## Why the Project Matters

The project responds to interconnected challenges that can affect communities in Guinea, including:

- Limited or unreliable access to electricity
- Insufficient public lighting
- Exposure to seasonal and flash flooding
- Limited access to visible street-level emergency warnings
- Difficulty maintaining access to charged mobile devices during electricity interruptions

Mobile phones can provide an important connection to family, information and emergency communication.

Rather than treating lighting, warning communication and charging as completely separate pieces of infrastructure, this project investigates how these functions could be brought together within a single recognisable community object.

---

# System Overview

The proof-of-concept consists of **two connected devices**.

## 1. Smart Community Totem — Receiver

The Smart Community Totem is the main infrastructure unit.

It integrates:

- Automatic ambient-light sensing
- Main LED illumination
- Visual flood-warning indication
- USB device charging
- Power monitoring
- Local data logging
- Real-time timestamping
- Wireless reception of flood-alert messages

The system continuously monitors its normal operating conditions while remaining capable of responding to an incoming flood-warning message.

---

## 2. Flood-Alert Demonstrator — Transmitter

For prototype testing and the Degree Show exhibition, a second portable device was built to **simulate an external flood-warning source**.

The demonstrator contains:

- A second **Heltec WiFi LoRa 32 V3.2**
- A manual alert button
- Wireless communication with the main totem
- A portable power supply

Pressing the alert button simulates the arrival of an external flood warning.

The device sends a wireless message to the Smart Community Totem, allowing the complete warning sequence to be demonstrated without requiring access to a live meteorological warning service.

> **Note:** The handheld transmitter is a proof-of-concept demonstration device. It is not intended to be the source of flood warnings in a real deployment.

---

# Intended Real-World Deployment

In a future deployment, flood warnings would originate from **authorised meteorological, flood-monitoring or emergency-management services**, rather than from the handheld demonstration device.

Potential alert sources could include:

- National meteorological services
- Regional or local weather-monitoring stations
- Government emergency-management agencies
- Official flood forecasting and early-warning systems

A future deployed Smart Community Totem could receive these official alerts through a **4G/cellular data connection**.

The role of the totem would therefore not necessarily be to predict the flood itself. Instead, it would act as a **last-mile community warning interface**, translating an incoming digital warning into a highly visible physical signal at street level.

---

## Intended Alert Flow

Once an official flood warning is issued, the alert could be transmitted automatically to the Smart Community Totem.

The totem would then convert the digital alert into a locally visible blue warning signal without requiring manual intervention.

```text
National / Regional Meteorological Service
                    │
                    │
                    │ Official Flood Warning
                    ▼
             4G / Cellular Network
                    │
                    ▼
          Smart Community Totem
                    │
                    ▼
           Blue Flashing Warning
                    │
                    ▼
            Constant Blue Light
                    │
                    ▼
             Community Alert
```

---

# Hardware

| Component | Role |
|---|---|
| Heltec WiFi LoRa 32 V3.2 | Main ESP32-S3 controller |
| Second Heltec WiFi LoRa 32 V3.2 | Prototype flood-alert transmitter |
| 12V RGB LED strip | Main lighting and visual warning output |
| IRLZ44N MOSFETs | Control of RGB LED channels |
| TEMT6000 ambient-light sensor | Automatic light-level detection |
| 2 × NeoPixel rings | Additional visual flood-alert indication |
| INA219 current sensor | Voltage, current and power monitoring |
| MicroSD module | Local system and event data logging |
| DS3231 RTC module | Real-time timestamps for logged data |
| USB charging port | 5V charging for mobile devices |
| Mechanical spring-wound timer | 20-minute shared charging control |
| LiFePO₄ battery | Energy storage for intended off-grid system |
| Solar panel | Renewable energy source for intended deployment |
| MPPT charge controller | Solar charging regulation |
| Pole and base structure | Physical support for the prototype |
| Diffusing canopy | Houses and diffuses the main lighting output |

---

# Prototype Behaviour

| Condition | System Response |
|---|---|
| Daylight / bright environment | Main automatic light remains off |
| Darkness detected | Main community light activates |
| Flood warning received | Blue flashing sequence begins |
| Flood warning remains active | Continuous blue warning light |
| Flood warning cleared | System returns to normal automatic-lighting behaviour |
| Device connected for charging | USB charging available |
| Other users waiting | Charging limited to 20 minutes per turn |

---

# Prototype Status

**Functional proof-of-concept completed and exhibited at the Connected Environments Degree Show 2026.**

### Implemented and Tested

- ✅ Automatic RGB lighting
- ✅ TEMT6000 ambient-light sensing
- ✅ Light-level hysteresis
- ✅ Visual blue flood-alert sequence
- ✅ Continuous blue warning state
- ✅ Return to normal operation after alert clearance
- ✅ Wireless communication between demonstrator and totem
- ✅ USB device charging
- ✅ Mechanical 20-minute charging timer
- ✅ INA219 voltage, current and power monitoring
- ✅ MicroSD local data logging
- ✅ DS3231 real-time clock
- ✅ Timestamped system data
- ✅ Non-blocking `millis()`-based control logic
- ✅ Integrated physical prototype
- ✅ Public exhibition demonstration

---

# Prototype Development

The project progressed through iterative stages of electronic prototyping, subsystem testing, physical construction and final system integration.

Development documentation includes:

- Individual component testing
- Breadboard prototyping
- Sensor calibration
- LED control testing
- Wireless communication testing
- Power-system development
- Soldering and hardware integration
- Structural development
- Final assembly
- Exhibition testing

Photographs documenting these stages are available in the **`Building Phases`** directory.

---

# Repository Structure

```text
ConnectGuinea/
│
├── Arduino codes/
│   └── Firmware and subsystem test sketches
│
├── Building Phases/
│   └── Photographs documenting prototype development,
│       testing, construction and assembly
│
├── Data/
│   └── Experimental and system data collected during testing
│
├── Dissertation Support and Additional Information/
│   └── Supporting project and dissertation material
│
├── Docs/
│   └── Wiring diagrams, technical documentation
│       and system architecture
│
├── Final Prototype and Exhibition/
│   └── Final prototype photographs, exhibition images
│       and demonstration media
│
├── Totem Components/
│   └── Reference images and documentation for components
│       used in the final prototype
│
├── README.md
├── LICENSE
└── .gitignore
```

---

# Current Limitations

The Guinea Smart Community Totem is a **proof-of-concept**, not a deployment-ready public infrastructure system.

Current limitations include:

- The exhibition flood warning is initiated using a manual demonstration transmitter.
- A live connection to an official meteorological warning service has not yet been implemented.
- 4G/cellular warning reception remains part of the intended deployment architecture.
- The prototype has not undergone long-term outdoor environmental testing.
- The final structure would require further weatherproofing for permanent outdoor installation.
- Long-term solar-generation and battery-performance testing would be required.
- Communication reliability would require evaluation under real deployment conditions.
- Community deployment would require engagement with local residents, authorities and relevant meteorological services.

---

# Future Development

Future development could investigate:

- Integration with official meteorological and flood-warning services
- 4G/cellular connectivity
- Secure remote alert delivery
- Deployment of multiple interconnected community totems
- Full solar-energy integration and optimisation
- Long-term battery and power-performance monitoring
- Weatherproof outdoor construction
- Remote system-health monitoring
- Field trials in Guinea
- Participatory co-design with local communities
- Collaboration with meteorological and emergency-management organisations

---

# Connected Environments Degree Show 2026

**MSc Connected Environments**  
**The Bartlett, University College London**

The final Guinea Smart Community Totem proof-of-concept was publicly exhibited as part of the **Connected Environments Degree Show 2026**.

---

## Academic Project Notice

This repository documents an academic proof-of-concept developed as part of an MSc dissertation.

The project does not claim to provide an operational flood-forecasting or emergency-warning service.

Any future real-world implementation would require appropriate technical validation, collaboration with relevant authorities, integration with authorised warning sources and further field testing.
