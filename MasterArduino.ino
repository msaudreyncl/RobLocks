#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED_G 4
#define LED_R 5
#define Alarm A0
#define door 3
#define SLAVE_ADDRESS 0x08

// Phone numbers
const char* numberLocker1 = "insert phone number 1";
const char* numberLocker2 = "insert phone number 2";

// Rental system (Locker 1)
const String ADMIN_UID_MASTER = "insert admin rfid uid";
const String USER_UID_MASTER  = "insert user rfid uid";
const unsigned long RENTAL_PERIOD = 60000UL;  // 1 minute

bool rentalActive = false;
bool thirtySecAlertSent = false;
bool rentalExpiredAlertSent = false;
bool userCardScanned = false;
unsigned long rentalStartTime = 0;

// RFID + LCD + GSM + I2C
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x26, 16, 2);
SoftwareSerial sim800(7, 8);
byte slaveMessage = 0;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LOCKER 1:");
  lcd.setCursor(0, 1);
  lcd.print("SCAN YOUR CARD");

  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(Alarm, OUTPUT);
  pinMode(door, OUTPUT);
  digitalWrite(door, HIGH);

  Wire.begin();
}

void sendSMS(const char* number, const char* message) {
  sim800.println("AT+CMGF=1"); delay(500);
  sim800.print("AT+CMGS=\""); sim800.print(number); sim800.println("\"");
  delay(500);
  sim800.print(message);
  delay(200);
  sim800.write(26);  // Ctrl+Z
  delay(3000);
}

void requestSlaveStatus() {
  Wire.requestFrom(SLAVE_ADDRESS, 1);
  if (Wire.available()) {
    slaveMessage = Wire.read();

    // Handle Locker 2 SMS Alerts
    switch (slaveMessage) {
      case 0x02:
        sendSMS(numberLocker2, "Locker 2: User Access Granted.");
        break;
      case 0x03:
        sendSMS(numberLocker2, "Locker 2: Unauthorized Card Attempt Detected!");
        break;
      case 0x04:
        sendSMS(numberLocker2, "Locker 2: Admin Access Detected. Rental Time Reset.");
        break;
      case 0x05:
        sendSMS(numberLocker2, "Locker 2: 30 seconds left before rental expires.");
        break;
      case 0x06:
        sendSMS(numberLocker2, "Locker 2: Rental time is over. Contact Admin.");
        break;
    }
  }
}

String readUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid.substring(1);
}

unsigned long getRemainingTime() {
  if (!rentalActive) return 0;
  unsigned long elapsed = millis() - rentalStartTime;
  if (elapsed >= RENTAL_PERIOD) {
    rentalActive = false;
    return 0;
  }
  return RENTAL_PERIOD - elapsed;
}

void unlockLocker() {
  digitalWrite(LED_G, HIGH);
  digitalWrite(door, LOW);
  delay(3000);
  digitalWrite(door, HIGH);
  digitalWrite(LED_G, LOW);
}

void denyAccess() {
  digitalWrite(LED_R, HIGH);
  digitalWrite(Alarm, HIGH);
  delay(1000);
  digitalWrite(LED_R, LOW);
  digitalWrite(Alarm, LOW);
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 500) {
    requestSlaveStatus();
    lastCheck = millis();
  }

  // Locker 1: Check remaining time and send alerts
  if (rentalActive && userCardScanned && !thirtySecAlertSent) {
    unsigned long rem = getRemainingTime();
    if (rem <= 30000 && rem > 0) {
      sendSMS(numberLocker1, "Locker 1: 30 seconds left before rental expires.");
      thirtySecAlertSent = true;
    }
  } 
  else if (rentalActive && userCardScanned && getRemainingTime() == 0 && !rentalExpiredAlertSent) {
    sendSMS(numberLocker1, "Locker 1: Rental time is over. Contact Admin.");
    rentalExpiredAlertSent = true;
    userCardScanned = false;  // Prevent repeat SMS
  }

  // RFID card detection
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;
  String uid = readUID();
  Serial.println("Master UID: " + uid);

  if (uid == ADMIN_UID_MASTER) {
    rentalStartTime = millis();
    rentalActive = true;
    thirtySecAlertSent = false;
    rentalExpiredAlertSent = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ADMIN ACCESS:");
    lcd.setCursor(0, 1);
    lcd.print("RENT TIME RESET");
    unlockLocker();
    sendSMS(numberLocker1, "Locker 1: Admin Access Detected. Rental Time Reset.");
  } 
  else if (uid == USER_UID_MASTER) {
    userCardScanned = true;
    if (!rentalActive) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RENT EXPIRED!");
      lcd.setCursor(0, 1);
      lcd.print("CONTACT ADMIN");
      delay(3000);
      denyAccess();
    } else {
      unsigned long rem = getRemainingTime();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CARD SCANNED:");
      lcd.setCursor(0, 1);
      lcd.print("ACCESS GRANTED!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TIME REMAINING:");
      lcd.setCursor(0, 1);
      lcd.print(String(rem / 60000) + " MIN " + String((rem / 1000) % 60) + " SECS ");
      unlockLocker();
      sendSMS(numberLocker1, "Locker 1: User Card Access Granted.");
    }
  } 
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CARD SCANNED:");
    lcd.setCursor(0, 1);
    lcd.print("ACCESS DENIED!");    
    denyAccess();
    sendSMS(numberLocker1, "Locker 1: Unauthorized Card Attempt Detected!");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOCKER 1:");
  lcd.setCursor(0, 1);
  lcd.print("SCAN YOUR CARD");
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
