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

## 🧩 Hardware Components & Pinout

### RFID (RC522 → Arduino)
| RC522 | Arduino | Purpose |
| :--- | :--- | :--- |
| SDA | D10 | SPI Slave Select |
| SCK | D13 | SPI Clock |
| MOSI | D11 | SPI Master Out Slave In |
| MISO | D12 | SPI Master In Slave Out |
| RST | D9 | Reset Pin |
| 3.3V | 3.3V | Power (**Do NOT use 5V**) |
| GND | GND | Ground |



### LCD (I²C → Arduino)
| LCD | Arduino | Purpose |
| :--- | :--- | :--- |
| SDA | A4 | Data Line |
| SCL | A5 | Clock Line |
| VCC | 5V | Power |
| GND | GND | Ground |

---

## 📚 Libraries Used
* `SPI.h`: SPI communication for the RFID module.
* `MFRC522.h`: RFID card detection and UID reading.
* `Wire.h`: I²C communication between Master and Slave.
* `LiquidCrystal_I2C.h`: LCD display control.
* `SoftwareSerial.h`: GSM module communication (Master only).

---

## ⚠️ Required Code Configurations

Before uploading the code to your Arduino boards, you **must** update the following placeholders in the source files to match your specific hardware:

### 1. Phone Numbers (Master Code)
In `master_code.ino`, locate the phone number variables and replace them with the actual mobile numbers that should receive the SMS alerts:
```cpp
const char* numberLocker1 = "insert phone number 1"; // Format: "+639XXXXXXXXX"
const char* numberLocker2 = "insert phone number 2";
```

### 2. RFID UIDs (Master & Slave Code)
Use the utility script GettingUIDcardCodesCommentedCode.ino to find your card's UID. Then, update these lines in both master_code.ino and slave_code.ino:

In Master Code:
```cpp
const String ADMIN_UID_MASTER = "insert admin rfid uid"; // e.g., "A1 B2 C3 D4"
const String USER_UID_MASTER  = "insert user rfid uid"; 
```

In Slave Code:
```cpp
const String ADMIN_UID_SLAVE = "insert admin rfid uid"; 
const String USER_UID_SLAVE  = "insert user rfid uid";  
```

### 3. I2C Addresses
The Master and Slave lockers may use different I2C addresses for their LCD screens. Check your specific modules and update the following if necessary:
```cpp
Master LCD: LiquidCrystal_I2C lcd(0x26, 16, 2);
Slave LCD: LiquidCrystal_I2C lcd(0x27, 16, 2);
```

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