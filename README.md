# 🔥 HeatShield AI

### Affordable IoT-Based Heat Stress Monitoring System for Worker Safety

HeatShield AI is a HealthTech solution developed by **Team Diparch** that aims to reduce heat-related health risks among outdoor workers such as construction laborers, traffic police, delivery personnel, and industrial workers.

The system continuously monitors environmental and physiological parameters and provides real-time alerts when unsafe conditions are detected.

---

## 🚀 Problem Statement

Heat waves, rising temperatures, and increasing humidity are creating serious health risks for outdoor workers across India.

Many workers continue working under extreme environmental conditions without any real-time monitoring or warning system, leading to:

- Heat Stress
- Dehydration
- Fatigue
- Heat Exhaustion
- Medical Emergencies

HeatShield AI aims to provide an affordable and accessible solution for early risk detection and worker safety.

---

## 💡 Solution

HeatShield AI is a wearable IoT prototype that continuously monitors:

- 🌡️ Temperature
- 💧 Humidity
- ❤️ Heart Rate
- 🫁 Blood Oxygen Level (SpO₂)

When unsafe conditions are detected, the system immediately generates alerts through:

- OLED Display
- Buzzer Notification

The collected data can also be monitored through a web dashboard in real time.

---

## 🛠️ Hardware Components

| Component | Purpose |
|------------|----------|
| ESP32 | Main Controller |
| DHT22 | Temperature & Humidity Monitoring |
| MAX30102 | Heart Rate & SpO₂ Monitoring |
| OLED Display | Live Data Display |
| Buzzer | Alert Generation |
| Breadboard & Jumper Wires | Prototyping |

---

## ⚙️ System Architecture

```text
DHT22 + MAX30102
          │
          ▼
       ESP32
          │
          ▼
      Wi-Fi Data
          │
          ▼
      Node.js Server
          │
          ▼
     Web Dashboard
```

---

## 📂 Repository Structure

```text
Diparch_prototype
│
├── firmware
│   └── HeatShieldAI_prototype.ino
│
└── heatshield-dashboard
    ├── public
    ├── package.json
    ├── package-lock.json
    └── server.js
```

---

## 🔧 Installation & Setup

### 1️⃣ Clone Repository

```bash
git clone https://github.com/iamshravanijadhav/Diparch_prototype.git
```

---

### 2️⃣ Upload Firmware to ESP32

Open:

```text
firmware/HeatShieldAI_prototype.ino
```

using Arduino IDE and upload it to the ESP32.

### Required Libraries

Install the following libraries from Arduino IDE Library Manager:

```text
DHT Sensor Library by Adafruit
Adafruit GFX Library
Adafruit SSD1306
SparkFun MAX3010x Pulse and Proximity Sensor Library
```

---

### 3️⃣ Run Dashboard

Navigate to dashboard folder:

```bash
cd heatshield-dashboard
```

Install dependencies:

```bash
npm install
```

Run server:

```bash
node server.js
```

Open browser and access the local dashboard.

---

## 📊 Features

✅ Real-Time Temperature Monitoring

✅ Real-Time Humidity Monitoring

✅ Real-Time Heart Rate Monitoring

✅ OLED Display Output

✅ Buzzer-Based Alert System

✅ Local Dashboard Monitoring

✅ ESP32 Wi-Fi Communication

✅ Real-Time Sensor Data Visualization

---

## 🔮 Future Scope

- TinyML-Based Heat Stress Prediction
- Edge AI Integration on ESP32
- Cloud-Based Data Storage
- Mobile Application Support
- GPS Tracking
- Emergency Notifications
- Advanced Worker Safety Analytics
- Predictive Heat Risk Monitoring

---

## 👥 Team Diparch

### Shravani Jadhav
**Founder & Team Lead**

### Shravan Kumar

### Bharat Prajapat

---

## 🎯 Vision

> "Workers should not have to choose between earning a livelihood and protecting their health."

HeatShield AI aims to make worker safety affordable, accessible, and proactive through IoT and future Edge AI technologies.

---

## 🏫 Institution

**NIAT X SGU, Kolhapur**

### Makers Conclave 2026 Project

---

### Made with by Team Diparch
