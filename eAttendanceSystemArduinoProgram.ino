#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define RST_PIN  D3  // Reset pin for MFRC522
#define SS_PIN   D4  // Slave Select pin for MFRC522
#define BUZZER   D8  // Buzzer pin
#define LED_PIN D0    // LED pin
#define BUTTON_PIN D9  // Push button pin

// Initialize MFRC522 object
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Google Sheets API information
const char *GScriptId = "YOUR GSCRIPT ID";
const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI PASS";

// Google Sheets connection details
const char* host = "script.google.com";
const int httpsPort = 443;
String url = "/macros/s/" + String(GScriptId) + "/exec";
HTTPSRedirect* client = nullptr;

// Other global variables
String student_id;  // Employee ID
String sheetName1 = "AttendanceSheet";
String sheetName2 = "EmployeeDetails";

// Admin mode variables
bool registrationMode = false;
String adminCardUID = "ADMIN CARD UID";  // Replace with the actual UID of the admin card
bool adminMode = false;
bool adminAuthorizationChecked = false;
String msg = "";

// Setup function
void setup() {
  Serial.begin(9600);

  // Set pin modes
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Display initialization message on LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  // Connect to WiFi and Google Sheets
  connectToWiFi();
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("GoogleSheets...");
  delay(5000);

  connectToGoogle();
  delete client;
  client = nullptr;
}

// Loop function
void loop() {
  static bool flag = false;

  // Initialize HTTPSRedirect object on the first loop
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setContentTypeHeader("application/json");
  }

  // Reconnect if not connected
  if (client != nullptr && !client->connected()) {
    client->connect(host, httpsPort);
  }

  // Toggle between attendance and registration mode with button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    registrationMode = !registrationMode;
    delay(500);  // Debounce delay
  }

  // Display appropriate message on LCD based on mode
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(registrationMode? "Scan Card": "Scan your tag");
  lcd.setCursor(0, 1);
  lcd.print(registrationMode? "to Register": "Please wait...");

  // Check admin authorization during registration mode switch
  if (registrationMode && !adminAuthorizationChecked) {
    String adminCardScan = waitForCardScan();
    if (adminCardScan == adminCardUID) {
      adminMode = true;
      adminAuthorizationChecked = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Admin Auth");
      lcd.setCursor(0, 1);
      lcd.print("Success");
      delay(2000);
    } else {
      registrationMode = false;  // Not authorized, go back to attendance mode
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Not Authorized");
      lcd.setCursor(0, 1);
      lcd.print("Back to Attend");
      delay(2000);
    }
  } else if (!registrationMode) {
    // Reset admin authorization check when toggling back to attendance mode
    adminAuthorizationChecked = false;
  }

  // Read card UID and perform actions based on mode
  mfrc522.PCD_Init();
  String card_UID = getCardUID();

  if (card_UID != "") {
    String payload = getPayload(card_UID, registrationMode ? sheetName2 : sheetName1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Publishing Data");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait...");

    Serial.println("Publishing data...");
    Serial.println(payload);

    // Send data to Google Sheets
    if (client->POST(url, host, payload)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("EmpID: " + student_id);
      lcd.setCursor(0, 1);
      lcd.print("Data Sent");
      String response = client -> getResponseBody();
      Serial.println(response);

      // Display appropriate message on LCD based on Google Sheets response
      lcd.clear();
      lcd.setCursor(0, 0);
      if (response.indexOf("Already Registered") != -1){
        lcd.print("Already");
        lcd.setCursor(0, 1);
        lcd.print("Registered"); 
        notRecognised();
      }
      else if (response.indexOf("Succesfully Registered") != -1) {
        lcd.print("Successfully");
        lcd.setCursor(0, 1);
        lcd.print("Registered"); 
        recognized();
      }
      // Add other response cases...

    } else {
      // Display error message on LCD if connection fails
      Serial.println("Error while connecting");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Failed.");
      lcd.setCursor(0, 1);
      lcd.print("Try Again");
    }

    delay(5000);
  }
}

// Function to wait for card scan during admin authorization
String waitForCardScan() {
  String card_UID = "";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Auth Card");
  lcd.setCursor(0, 1);
  lcd.print("to Register");

  // Wait for a card to be scanned
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(100);
  }

  // Read card UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    card_UID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    card_UID += String(mfrc522.uid.uidByte[i], HEX);
  }

  return card_UID;
}

// Function to play buzzer when card is not recognized
void notRecognised() {
  tone(BUZZER, 1000);
  delay(300);
  delay(300);
  delay(300);
  noTone(BUZZER);
}

// Function to indicate successful recognition
void recognized() {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER, 30);
  delay(100);
  noTone(BUZZER);
  delay(100);
  tone(BUZZER, 30);
  delay(100);
  noTone(BUZZER);
  delay(300);
  digitalWrite(LED_PIN, LOW);
}

// Function to get card UID
String getCardUID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }

  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  return cardUID;
}

// Function to generate Employee ID based on card UID
String generateEmployeeID(const String& cardUID) {
  // FNV-1a hash function
  const uint32_t prime = 0x01000193; // FNV prime value
  uint32_t hash = 0x811C9DC5;        // FNV offset basis

  for (size_t i = 0; i < cardUID.length(); i++) {
    hash ^= static_cast<uint32_t>(cardUID[i]);
    hash *= prime;
  }

  // Take the last 5 digits of the hash value
  String employeeID = String(hash % 100000); // Ensures a 5-digit number
  student_id = employeeID;
  return employeeID;
}

// Function to generate payload for Google Sheets
String getPayload(const String& cardUID, const String& sheetName) {
  String ID = generateEmployeeID(cardUID);
  String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"" + sheetName + "\", \"values\": ";
  String payload = payload_base + "\"" + String(ID) + "\"}";
  return payload;
}

// Function to connect to WiFi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

// Function to connect to Google Sheets
void connectToGoogle() {
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);

    if (retval == 1) {
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(msg);
      delay(2000);
      break;
    } else {
      Serial.println("Connection failed. Retrying...");
    }
  }

  // Display connection failure message if unsuccessful
  if (!flag) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connection fail");
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
  }
}
