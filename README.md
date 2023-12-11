# RFID eAttendance System with Google Sheets Integration

## Overview

This IoT-based project implements an RFID-based attendance system using an ESP8266 microcontroller and Google Sheets integration. The system leverages Google Apps Script as a backend to handle incoming data from an Arduino sketch, updating attendance and registration records in designated Google Sheets.

## Features

### 1. RFID Card Authentication

The system employs MFRC522 RFID modules for card authentication. Users scan their RFID cards, triggering actions based on the scanned card's information.

### 2. Dual Modes: Attendance and Registration

#### Attendance Mode

In attendance mode, the system records the entry and exit times of individuals, updating a specified Google Sheet with the relevant data.

#### Registration Mode

Switching to registration mode allows the system to register new users by updating a separate Google Sheet with employee details.

### 3. Admin Authorization

Administrators can be authorized using a designated RFID card. Admins can toggle registration mode and perform additional actions, adding a layer of security to the system.

### 4. Real-time Communication with Google Sheets

The system communicates in real-time with Google Sheets via HTTPS requests, updating data dynamically.

### 5. Visual and Audio Feedback

The system provides feedback through an LCD screen and a buzzer. Messages like "Already Registered," "Successfully Registered," and "Not Registered" are displayed on the LCD, and corresponding sounds are played.

## Getting Started

### Prerequisites

- Arduino IDE
- Google Account
- RFID Cards
- ESP8266 microcontroller
- MFRC522 RFID module
- Other hardware components (LCD, buzzer, etc.)

### Installation

1. #### Arduino IDE Setup:
   - Install the ESP8266 board library:
     1. Open Arduino IDE.
     2. Go to File > Preferences.
     3. Add the following URL to the "Additional Boards Manager URLs" field:
        ```
        http://arduino.esp8266.com/stable/package_esp8266com_index.json
        ```
     4. Go to Tools > Board > Boards Manager, search for "esp8266," and install the latest version.
   - Install the HTTPSRedirect library:
     1. Download the library from [GitHub](https://github.com/electronicsguy/ESP8266).
     2. Extract the ZIP file into the Arduino libraries folder.
   - Install the MFRC522 RFID library:
     1. Go to Sketch > Include Library > Manage Libraries.
     2. Search for "MFRC522" and install the library by "miguelbalboa."

2. #### Arduino Sketch Configuration:
   - Open the Arduino sketch.
   - Replace placeholders for Google Sheets API details, WiFi credentials, and RFID card UIDs.
   - Upload the sketch to your ESP8266 microcontroller.

3. #### Admin Card ID Retrieval:
   - Use the "dumpinfo" example file from the MFRC522 library:
     1. Open the "dumpinfo" example.
     2. Upload the sketch to your ESP8266.
     3. Scan the admin RFID card.
   - Convert the UID to lowercase and ensure there are no spaces between characters before pasting it into the main program.

4. #### Google Apps Script Setup:
   - Create a new Google Apps Script project.
   - Copy and paste the provided script, replacing placeholders for Spreadsheet ID and timezone.
   - Deploy the script as a web app and obtain the provided URL.

### Project Implementation Images


## Usage

1. Power on the RFID attendance system.

2. Scan an RFID card to record attendance or register a new user.

3. Admins can toggle registration mode using the admin RFID card.

4. View real-time updates on the connected Google Sheets.

## YouTube Link
A description and demo can be found on [demo](https://youtu.be/Gx8IvVP25Pg?si=oSAFD_z7xvm3a2nR)

## Contributions

Contributions are welcome! If you'd like to enhance the project, please follow these steps:

1. Fork the project.

2. Create your feature branch:

   ```bash
   git checkout -b feature/YourFeature
   ```

3. Commit your changes:

   ```bash
   git commit -am 'Add some feature'
   ```

4. Push to the branch:

   ```bash
   git push origin feature/YourFeature
   ```

5. Submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

- [MFRC522 Arduino Library](https://github.com/miguelbalboa/rfid)
- [Google Apps Script Documentation](https://developers.google.com/apps-script)
- [Ahmad Logs](https://github.com/ahmadlogs)

---

