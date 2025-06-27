# 📦 IoT-Based Container Tracking System

This repository contains the source files for a thesis project focused on developing an IoT-powered smart container tracking system. The system is designed for a logistics company named **Tanto** and aims to **monitor container conditions** (e.g., location, temperature, and vibrations) in real time while in transit.

---

## 🧠 Project Overview

The system enables continuous tracking of shipping containers by leveraging:

- 🌍 **GPS** for geolocation tracking
- 🌡️ **Temperature & humidity sensors** to monitor container environment
- 📈 **IMU sensors (MPU6050)** to detect vibration and shock
- 🌐 **Node-RED backend** to visualize and manage data
- 🗄️ **SQLite database** to store telemetry locally
- 🔋 **Solar-powered lithium battery** system for off-grid operation

This prototype emphasizes modularity and future scalability for real-world logistics use.

---

## 🔧 Hardware Components

| Component                  | Description                                           |
|---------------------------|-------------------------------------------------------|
| **TTGO T-CALL ESP32**     | Microcontroller with SIM module for IoT connectivity |
| **GPS NEO-8M**            | GPS module for tracking real-time location           |
| **DHT22**                 | Sensor for temperature and humidity                  |
| **MPU6050**               | Accelerometer and gyroscope for vibration detection  |
| **Solar Panel + SCC**     | Mini solar panel with Solar Charge Controller        |
| **3.7V Lithium Battery**  | Powers the system off-grid                           |
| **Orange Pi 5 Pro**       | Server for managing backend + local database         |

---

## 📂 Repository Structure

```bash
IoT-Container-Tracking/
├── Analysis MPU6050/         # Raw and processed data from vibration sensor
├── Casing Design/            # Physical enclosure designs (CAD/STL)
├── Database/                 # SQLite schema and storage logic
├── Documentation/            # Research paper, specs, diagrams
├── Hardware Code/            # Microcontroller code (ESP32, sensors)
├── Node-RED Backend/         # Flow files and setup for Node-RED dashboard
├── .gitignore
└── README.md
```

## ⚙️ System Architecture
1. ESP32 collects data from GPS, DHT22, and MPU6050
2. Data is sent over a cellular or Wi-Fi connection
3. Orange Pi 5 Pro stores and forwards data to the dashboard
4. Node-RED handles visualization, alerts, and control logic
5. SQLite stores historical data for offline recovery

## 🚀 Setup Instructions
### 1. Hardware Assembly
- Follow connection diagrams in `Hardware Code/`
- Power the system using the solar setup or USB for testing

### 2. Flashing Microcontroller
- Use Arduino IDE or PlatformIO
- Install libraries for `TinyGSM`, `DHT`, `Wire`, `Adafruit MPU6050`, etc.
- Upload code from `Hardware Code/`

3. Running Node-RED Backend
- Import flow files from `Node-RED Backend/` into your local Node-RED
- Set up a SQLite connection on Orange Pi or your PC

## 📈 Limitations & Future Work
- Sensor precision may vary and needs calibration
- Cellular connectivity should be optimized for power saving
- Further development is needed for long-distance deployments
- Optional integration with cloud dashboards (e.g., ThingsBoard, Firebase)

## 📚 Thesis Context
This work was conducted as part of a final undergraduate thesis focused on smart logistics solutions using IoT and embedded systems. It is a functional prototype that demonstrates the feasibility of low-power, field-deployable container monitoring.

## 🧾 License
This project is for educational and research purposes. If reused, please attribute the original author.

## 📬 Contact
- GitHub: [@Lunardy2509](https://github.com/Lunardy2509)
- Email: ferdilunardy@gmail.com

If you find this project insightful or want to contribute, feel free to ⭐️ star or fork the repo!

---

Let me know if you'd like to:
- Translate this to **Bahasa Indonesia**,
- Add **system diagrams** or architecture visuals,
- Generate a **PDF report** from `Documentation/`.

Happy to help enhance your academic work!
