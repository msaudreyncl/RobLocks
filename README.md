# 🔐 RobLocks: Automated Access-Controlled Rental Locker System

## 🎓 Academic Information
* **Course:** Fundamentals of Mixed Signals and Sensors
* **Institution:** Polytechnic University of the Philippines – Institute of Technology
* **Semester:** 2nd Semester, AY 2024–2025
* **Instructor:** Engr. Jonathan C. Manarang
* **Date Submitted:** July 08, 2025

---

## 📌 Project Overview
**RobLocks** is a microcontroller-based secure rental locker system designed to provide automated, timed access control. The system leverages RFID for security, GSM for remote notifications, and I²C for modular scalability between multiple locker units.


---

## ⚙️ Key Features
* 🔐 **Contactless RFID Authentication:** Supports Admin and User roles.
* ⏳ **Timed Rental System:** Automatic lock-out after the rental period (1-minute prototype duration).
* 📲 **Real-time SMS Alerts:** Notifications for authorized/unauthorized access and expiration warnings.
* 🔄 **I²C Communication:** Seamless data exchange between Arduino units.
* 🔍 **UID Discovery:** Includes a utility script to identify unique RFID tag IDs for system registration.

---

## 🛠 Utility Scripts

### 🆔 RFID UID Scanner (`GettingUIDcardCodesCommentedCode.ino`)
Before deploying the system, each RFID card (Admin, User, etc.) must be identified by its unique ID (UID). This utility script allows the team to:
1.  **Scan any RFID tag** using the MFRC522 module.
2.  **View the HEX code** via the Arduino Serial Monitor.
3.  **Visual Confirmation** of the code on the I2C LCD screen.

**How to use:** Flash this code to an Arduino connected to the RFID reader. Open the Serial Monitor at **9600 Baud**. When a card is tapped, the UID will be printed, which can then be hardcoded into the Master or Slave logic.

---

## 📂 Project Structure
```text
ROBLOCKS/
├── MasterArduino/
│   └── master_code.ino
├── SlaveArduino/
│   └── slave_code.ino
├── Utilities/
│   └── GettingUIDcardCodesCommentedCode.ino
└── README.md