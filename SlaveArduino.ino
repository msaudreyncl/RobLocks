#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SS_PIN 10
#define RST_PIN 9
#define LED_G 4
#define LED_R 5
#define Alarm A0
#define door 3
#define SLAVE_ADDRESS 0x08

// RFID UIDs
const String ADMIN_UID_SLAVE = "insert admin rfid uid";
const String USER_UID_SLAVE  = "insert user rfid uid";

// Rental timing
const unsigned long RENTAL_PERIOD = 60000UL; // 1 minute
unsigned long rentalStartTime = 0;
bool rentalActive = false;
bool thirtySecAlertSent = false;
bool rentalExpiredAlertSent = false;
bool userCardScanned = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte responseCode = 0x00;

String readUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid.substring(1);
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

unsigned long getRemainingTime() {
  if (!rentalActive) return 0;
  unsigned long elapsed = millis() - rentalStartTime;
  if (elapsed >= RENTAL_PERIOD) {
    rentalActive = false;
    return 0;
  }
  return RENTAL_PERIOD - elapsed;
}

void sendStatusToMaster() {
  Wire.write(responseCode);
  responseCode = 0x00; // Reset after sending
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init(); lcd.backlight();

  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(Alarm, OUTPUT);
  pinMode(door, OUTPUT);
  digitalWrite(door, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("LOCKER 2:");
  lcd.setCursor(0, 1);
  lcd.print("SCAN YOUR CARD");

  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(sendStatusToMaster);
}

void loop() {
  // Rental alert triggers
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) {
    if (rentalActive && userCardScanned && !thirtySecAlertSent) {
      unsigned long rem = getRemainingTime();
      if (rem <= 30000 && rem > 0) {
        responseCode = 0x05; // 30 seconds alert
        thirtySecAlertSent = true;
      }
    } else if (rentalActive && userCardScanned && getRemainingTime() == 0 && !rentalExpiredAlertSent) {
      responseCode = 0x06; // Rental expired
      rentalExpiredAlertSent = true;
      userCardScanned = false; // Reset to avoid spam
    }
    lastCheck = millis();
  }

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;
  String uid = readUID();
  Serial.println("Scanned UID: " + uid);

  if (uid == ADMIN_UID_SLAVE) {
    rentalStartTime = millis();
    rentalActive = true;
    thirtySecAlertSent = false;
    rentalExpiredAlertSent = false;
    userCardScanned = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ADMIN ACCESS:");
    lcd.setCursor(0, 1);
    lcd.print("RENT TIME RESET");
    unlockLocker();
    responseCode = 0x04;
  }
  else if (uid == USER_UID_SLAVE) {
    userCardScanned = true;

    if (!rentalActive) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RENT EXPIRED!");
      lcd.setCursor(0, 1);
      lcd.print("CONTACT ADMIN");
      delay(3000);
      denyAccess();
      responseCode = 0x03;
    } else {
      unsigned long rem = getRemainingTime();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("USER ACCESS:");
      lcd.setCursor(0, 1);
      lcd.print("ACCESS GRANTED");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TIME REMAINING:");
      lcd.setCursor(0, 1);
      lcd.print(String(rem / 60000) + " MIN " + String((rem / 1000) % 60) + " SECS ");
      unlockLocker();
      responseCode = 0x02;
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CARD SCANNED:");
    lcd.setCursor(0, 1);
    lcd.print("ACCESS DENIED!");
    denyAccess();
    responseCode = 0x03;
  }

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOCKER 2:");
  lcd.setCursor(0, 1);
  lcd.print("SCAN YOUR CARD");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
