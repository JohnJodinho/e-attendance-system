// https://docs.google.com/spreadsheets/d/1CpAF_iey5iBh-5ckH8K3tHoDStczIsKRRyUUCmA88b4/edit?gid=0#gid=0

var SS = SpreadsheetApp.openById('1CpAF_iey5iBh-5ckH8K3tHoDStczIsKRRyUUCmA88b4');
var timezone = "Africa/Lagos"; 
var hours = 0; // Offset hours if needed
var str = "";

// Main function triggered on POST request
function doGet(e) {
  var parsedData;
  try {
    parsedData = JSON.parse(e.postData.contents);
  } catch (error) {
    Logger.log("Error parsing payload!");
    return ContentService.createTextOutput("Error: Invalid JSON payload.");
  }

  if (parsedData) {
    var sheetName = parsedData.sheet_name;
    var command = parsedData.command;
    var Curr_Date = new Date(new Date().setHours(new Date().getHours() + hours));
    var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');

    if (sheetName === "StudentDetails") {
      handleStudentDetails(parsedData);
    } else if (sheetName.includes("Attendance")) {
      handleAttendance(parsedData, Curr_Date, Curr_Time);
    } else {
      str = "Invalid sheet name.";
    }

    Logger.log(str);
    return ContentService.createTextOutput(str);
  } else {
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}

// Handle StudentDetails sheet logic
function handleStudentDetails(data) {
  var sheet = SS.getSheetByName("StudentDetails");
  var matricNumber = data.matricNumber;
  var fingerprintID = data.fingerprintID;

  if (!matricNumber || !fingerprintID || !data.command) {
    str = "Error! Missing required fields in payload for StudentDetails.";
    return;
  }

  var allData = sheet.getDataRange().getValues();
  var foundRow = -1;

  // Search for the matricNumber in StudentDetails
  for (var i = 0; i < allData.length; i++) {
    if (allData[i][0] === matricNumber) { // [0] is Matriculation Number column
      foundRow = i + 1;
      break;
    }
  }

  if (data.command === "insert") {
    if (foundRow > 0) {
      str = "Already Registered";
    } else {
      sheet.appendRow([matricNumber, fingerprintID]);
      str = "Succesfully Registered";
    }
  } else if (data.command === "update") {
    if (foundRow > 0) {
      sheet.getRange(foundRow, 2).setValue(fingerprintID); // Update Fingerprint ID
      str = "Student details updated successfully.";
    } else {
      str = "Matric number not found.";
    }
  } else if (data.command === "delete") {
    if (foundRow > 0) {
      sheet.deleteRow(foundRow);
      str = "Student details deleted successfully.";
    } else {
      str = "Matric number not found.";
    }
  } else {
    str = "Invalid command for StudentDetails.";
  }
}

// Handle Attendance sheets logic
function handleAttendance(data, Curr_Date, Curr_Time) {
  var sheetName = data.sheet_name;
  var fingerprintID = data.fingerprintID;

  if (!fingerprintID || !data.command) {
    str = "Error! Missing required fields in payload for Attendance.";
    return;
  }

  var studentDetailsSheet = SS.getSheetByName("StudentDetails");
  var studentDetailsData = studentDetailsSheet.getDataRange().getValues();
  var matricNumber = "";
  var found = false;

  // Search for Fingerprint ID in StudentDetails
  for (var i = 0; i < studentDetailsData.length; i++) {
    if (studentDetailsData[i][1] == fingerprintID) { // [1] is Fingerprint ID column
      matricNumber = studentDetailsData[i][0]; // [0] is Matriculation Number column
      found = true;
      break;
    }
  }

  if (!found) {
    str = "Not Registered";
    return;
  }

  var sheet = SS.getSheetByName(sheetName);
  var attendanceData = sheet.getDataRange().getValues();
  var row_number = 0;
  var time_out = "";
  var dateToday = Utilities.formatDate(Curr_Date, timezone, "yyyy-MM-dd");

  for (var i = 0; i < attendanceData.length; i++) {
    if (attendanceData[i][1] == matricNumber) { // [1] is Matric Number column
      if (attendanceData[i][0] === dateToday) { // [0] is Date column
        row_number = i + 1;
        time_out = attendanceData[i][5]; // [5] is Time Out column
        break;
      }
    }
  }

  if (row_number > 0) {
    if (time_out === "") {
      sheet.getRange("F" + row_number).setValue(Curr_Time); // Sign Out
      str = "Signed Out";
      return;
    } else {
      str = "Already Signed Out Today"; // User already signed out for the day
      return;
    }
  }

  if (data.command === "insert_row") {
    sheet.insertRows(2); // Insert below the header
    sheet.getRange("A2").setValue(dateToday); // Date
    sheet.getRange("B2").setValue(matricNumber); // Matriculation Number
    sheet.getRange("C2").setValue(Curr_Time); // Time In
    str = "Signed In";
    SpreadsheetApp.flush();
  } else {
    str = "Invalid command for Attendance.";
  }
}

