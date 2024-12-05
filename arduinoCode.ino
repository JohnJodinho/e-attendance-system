
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Keypad_I2C.h>  // Includes Keypad

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>


// Initialize LCD with address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize PCF8574T for Keypad
#define I2CADDR 0x20
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {7, 6, 5, 4}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {3, 2, 1, 0};  // Connect to the column pinouts of the keypad

// Initialize Keypad_I2C
Keypad_I2C customKeypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);

// Initialize Fingerprint Sensor
SoftwareSerial mySerial(D7, D8); // RX = D7, TX = D6
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Secrets
const char *GScriptId = "AKfycbxr_NuNOnrfxsmgnPfCAqUvzCwlGID0qB5Pq0MOY-am6kf0nQ-K5a76WKIb1VlXMJym";
const char* ssid = "jodu270819";
const char* password = "MyGalaxy";

// Constants
const char* host = "script.google.com";
const int httpsPort = 443;

// Utility Variables:
String url = "/macros/s/" + String(GScriptId) + "/exec";
HTTPSRedirect* client = nullptr;


int fingerprint;
String matNumber;

struct EnrollmentResult {
  int fingerprintID;
  String matricNumber;
};

String passcodes[] = {"1234", "5678", "0000", "1111"};  // Change to suitable passwords for admin 1 to admin 4


String getInputFromKeypad(bool isMasked = true) {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press * to clear");
  lcd.setCursor(0, 1);
  lcd.print("Press # to enter");
  delay(2000);

  String input = "";
  char key;

  while (true) {
    key = customKeypad.getKey();
    if (key) {
      if (key == '#') break; // Finish input with #
      if (key == '*') {      // Clear input if '*' is pressed
        input = "";
        Serial.println("\nInput cleared. Start again.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Input cleared!");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Press * to clear");
        lcd.setCursor(0, 1);
        lcd.print("Press # to enter");
      } else {
        input += key;
        if (isMasked) {
          Serial.print("*"); // Mask user input
          lcd.print("*");
        } else {
          Serial.print(key);
          lcd.print(key);
        }
      }
    }
  }
  Serial.println();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Input complete");
  delay(2000);
  lcd.clear();
  return input;
}




EnrollmentResult enrollFingerprint() {
  EnrollmentResult result = {-1, ""};  // Default result in case of failure

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter ID (0-127)");
  delay(2000);
  Serial.println("Enter ID to enroll fingerprint (0-127):");

  String input = getInputFromKeypad(false);
  // Check if the input is a valid number
  while ((input.length() == 0) || (!input.toInt() && input != "0")) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid ID!");
    Serial.println("Error: Invalid input. Please enter a valid number.");
    String input = getInputFromKeypad(false);
  } 
  int id = input.toInt();
  Serial.print("Valid input received: ");
  Serial.println(id);
  
  if (id < 0 || id > 127) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid ID!");
    Serial.println("Invalid ID.");
    delay(2000);
    return result;
  }

  if (getFingerprintEnroll(id)) {  // Assuming getFingerprintEnroll(int id) exists
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Next!");
    delay(2000);
    Serial.println("Fingerprint enrolled successfully!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter matric:");
    delay(2000);
    Serial.println("Enter matric number to associate:");
    
    String matric = getInputFromKeypad(false);

    result.fingerprintID = id;
    result.matricNumber = matric;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Data captured!");
    Serial.println("Matric number and fingerprint ID captured!");
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enroll failed!");
    Serial.println("Fingerprint enrollment failed.");
    delay(2000);
  }

  return result;
}



uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  lcd.setCursor(0, 1);
  lcd.print("Place Finger");
  Serial.println("Waiting for finger...");
  delay(2000); // Allow time for user to place finger

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Image taken");
        Serial.println("Fingerprint image taken successfully.");
        delay(1500); 
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected.");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.setCursor(0, 1);
        lcd.print("Comm Error");
        Serial.println("Communication error with fingerprint sensor.");
        delay(1500);
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.setCursor(0, 1);
        lcd.print("Imaging fail");
        Serial.println("Failed to capture fingerprint image.");
        delay(1500);
        break;
      default:
        lcd.setCursor(0, 1);
        lcd.print("Unknown err");
        Serial.println("Unknown error occurred.");
        delay(1500);
        break;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    lcd.setCursor(0, 1);
    lcd.print("Error in Temp");
    Serial.println("Error converting image to template.");
    delay(2000);
    return false;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place again");
  Serial.println("Place finger again for second capture.");
  delay(2000); // Allow time for user to place the finger again

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Image taken");
        Serial.println("Second fingerprint image taken successfully.");
        delay(1500); // Let the user see the message
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected.");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        lcd.setCursor(0, 1);
        lcd.print("Comm Error");
        Serial.println("Communication error with fingerprint sensor.");
        delay(1500);
        break;
      case FINGERPRINT_IMAGEFAIL:
        lcd.setCursor(0, 1);
        lcd.print("Imaging fail");
        Serial.println("Failed to capture fingerprint image.");
        delay(1500);
        break;
      default:
        lcd.setCursor(0, 1);
        lcd.print("Unknown err");
        Serial.println("Unknown error occurred.");
        delay(1500);
        break;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    lcd.setCursor(0, 1);
    lcd.print("Error in Temp2");
    Serial.println("Error converting second image to template.");
    delay(2000);
    return false;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0, 1);
    lcd.print("Prints matched");
    Serial.println("Fingerprints matched successfully.");
    delay(1500);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Failed");
    Serial.println("Fingerprint matching failed.");
    delay(1500);
    return false;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0, 1);
    lcd.print("Stored!");
    Serial.println("Fingerprint stored successfully.");
    delay(2000); // Allow the user to see the success message
    return true;
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Store failed");
    Serial.println("Failed to store fingerprint.");
    delay(2000);
    return false;
  }
}


int getFingerprintID() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger");
  Serial.println("Waiting for finger for verification...");
  delay(2000); // Allow time for user to place the finger
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK){
    lcd.setCursor(0, 1);
    lcd.print("Try again");
    Serial.println("Failed to detect fingerprint image.");
    delay(5000);
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    lcd.setCursor(0, 1);
    lcd.print("Error in Temp");
    Serial.println("Error converting image to template.");
    delay(2000);
    return -1;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VERIFIED!");
    lcd.setCursor(0, 1);
    lcd.print("U19EE10");
    lcd.print(finger.fingerID);
    delay(5000);
    
    delay(1000);
    Serial.print("Fingerprint found! ID: ");
    Serial.println(finger.fingerID);
    return finger.fingerID;
  }

    else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Not Found");
    Serial.println("No matching fingerprint found.");

    delay(2000);
    return -1;
  }

}

String checkPasscode() {
  String enteredPasscode;
  enteredPasscode = getInputFromKeypad();  // Missing semicolon fixed

  for (int i = 0; i < 4; i++) {  // Compare with stored passcodes
    if (enteredPasscode.equals(passcodes[i])) {  // Use `.equals()` for `String`
      return enteredPasscode;
    }
  }
  return "no match";  // No match found
}





String constructPayloadForReg(String sheetName, String matricNumber, int fingerprintID, String command) {
    String payload = "{";
    payload += "\"sheet_name\":\"" + sheetName + "\",";
    payload += "\"matricNumber\":\"" + matricNumber + "\",";
    payload += "\"fingerprintID\":\"" + String(fingerprintID) + "\",";
    payload += "\"command\":\"" + command + "\"";
    payload += "}";
    return payload;
}

String constructPayloadForAttendance(String sheetName, int fingerprintID, String command) {
    String payload = "{";
    payload += "\"sheet_name\":\"" + sheetName + "\",";
    payload += "\"fingerprintID\":\"" + String(fingerprintID) + "\",";
    payload += "\"command\":\"" + command + "\"";
    payload += "}";
    return payload;
}
/****************************************************************************************************
 * Connect to WiFi
****************************************************************************************************/
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

/****************************************************************************************************
 * Connect to Google
****************************************************************************************************/
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


void setup() {
  Serial.begin(9600);
  Serial.println("System Initializing...");

  // Initialize I2C for Keypad (PCF8574T)
  Wire.begin(D2 ,D1); // SDA = D2, SCL = D1
  customKeypad.begin(makeKeymap(hexaKeys));

  // Initialize I2C for LCD with separate pins
  lcd.init();
  lcd.backlight();



  // Test LCD
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
  delay(2000); // Allow time for the user to read the message

  
  // Initialize Fingerprint Sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint Sensor found!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint Ready");
    delay(2000);
  } else {
    Serial.println("Fingerprint Sensor not found");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor not found");
    while (true); // Stop here if the sensor is not found
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

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



void loop() {
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr && !client->connected()) {
    client->connect(host, httpsPort);
  }

  String mode = "";  // Variable to hold mode selection
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Mode:");
  Serial.println("Enter mode via keypad:");

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: Enrol Mode");
  Serial.println("1: Enrolment Mode");

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("2: Attend Mode");
  Serial.println("2: Attendance Mode");

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C: Default Mode");
  Serial.println("C: Exit to Default Mode");

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*: Clear Sensor");
  Serial.println("*: To clear fingerprints in sensor");

  delay(1500);

  mode = getInputFromKeypad(false);   

  if (mode == "1") { 
    // Enrolment Mode
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enrol activated!");
    delay(1500);
    Serial.println("Enrolment Mode Activated!");
    String payload;
    
    if (checkPasscode() != "no match") {  // Verify passcode
      Serial.println("Passcode accepted. Proceeding to enrollment.");
      Serial.println("Starting Enrollment...");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Starting...");
      delay(1500);
      EnrollmentResult enrollment = enrollFingerprint();

      if (enrollment.fingerprintID != -1) {
        fingerprint = enrollment.fingerprintID;
        matNumber = enrollment.matricNumber;
        Serial.print("Enrolled Fingerprint ID: ");
        Serial.println(fingerprint);
        Serial.print("Associated Matric Number: ");
        Serial.println(matNumber);
        payload = constructPayloadForReg("StudentDetails", matNumber, fingerprint, "insert");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Publishing Data");
        lcd.setCursor(0, 1);
        lcd.print("Please Wait...");

        Serial.println("Publishing data...");
        Serial.println(payload);

        if (client->POST(url, host, payload)) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("FingerID: " + String(fingerprint));
          lcd.setCursor(0, 1);
          lcd.print("Data Sent");
          String response = client -> getResponseBody();
          Serial.println(response);

          if(response.indexOf("Already Registered") != -1){
            lcd.print("Already");
            lcd.setCursor(0, 1);
            lcd.print("Registered"); 
            Serial.println("Already Registered Fingerprint");
          }
          else if (response.indexOf("Succesfully Registered") != -1) {
            lcd.print("Succesfully");
            lcd.setCursor(0, 1);
            lcd.print("Registered"); 
            Serial.println("Successfully Registered");
          }

          else {
            Serial.println("Error while connecting");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Failed.");
            lcd.setCursor(0, 1);
            lcd.print("Try Again");
          }
        }
      
        
      } else {
        Serial.println("Enrollment process failed.");
      }
    } else {
      Serial.println("Invalid passcode. Returning to default mode.");
    }
  } 
  else if (mode == "2") { 
    // Attendance Mode
    Serial.println("Attendance Capturing Mode Activated!");
    String payload;
    String gottenPasscode = checkPasscode();
    if (gottenPasscode != "no match") {  // Verify admin passcode
      Serial.println("Passcode accepted. Ready for attendance capturing.");
      int fingerprintSigning = getFingerprintID();
      if (fingerprintSigning){
        
        if (gottenPasscode == passcodes[0]){
          payload = constructPayloadForAttendance("Admin1 Attendance", fingerprintSigning, "insert_row");
        }
        if (gottenPasscode == passcodes[1]){
          payload = constructPayloadForAttendance("Admin2 Attendance", fingerprintSigning, "insert_row");
        }
        if (gottenPasscode == passcodes[2]){
          payload = constructPayloadForAttendance("Admin3 Attendance", fingerprintSigning, "insert_row");
        }
        if (gottenPasscode == passcodes[3]){
          payload = constructPayloadForAttendance("Admin4 Attendance", fingerprintSigning, "insert_row");
        }
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Publishing Data");
      lcd.setCursor(0, 1);
      lcd.print("Please Wait...");

      Serial.println("Publishing data...");
      Serial.println(payload);

      if (client->POST(url, host, payload)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FingerID: " + String(fingerprintSigning));
        lcd.setCursor(0, 1);
        lcd.print("Data Sent");
        String response = client -> getResponseBody();
        Serial.println(response);

        if (response.indexOf("Signed Out") != -1) {
          lcd.print("Signed Out");
          lcd.setCursor(0, 1);
          lcd.print("Goodbye");
          Serial.println("Signed Out"); 
         
        }
        else if (response.indexOf("Signed In") != -1) {
          lcd.print("Signed In");
          lcd.setCursor(0, 1);
          lcd.print("Welcome"); 
          Serial.println("Signed In");
         
        }

        else if (response.indexOf("Not Registered") != -1) {
          lcd.print("Fingerprint Not");
          lcd.setCursor(0, 1);
          lcd.print("Registered"); 
          Serial.println("Not registered");
          
        }

        else if(response.indexOf("Already Signed Out Today") != -1){
          lcd.print("Already Signed");
          lcd.setCursor(0, 1);
          lcd.print("Out Today");
          Serial.println("Already Signed Out Today");  
        }
        else {
          Serial.println("Error while connecting");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Failed.");
          lcd.setCursor(0, 1);
          lcd.print("Try Again");
        }
      }
    } else {
      Serial.println("Invalid passcode. Returning to default mode.");
    }
  } else if (mode == "C") {
    // Exit Mode
    Serial.println("Exiting to Default Mode...");
    return;  // Exit to default mode
  }
  else if (mode == "*"){
    Serial.println("clear fingerprints");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Empty Database");
    finger.emptyDatabase();
    delay(200);

    } 
    else {
    Serial.println("Invalid input. Try again.");
  }
}



