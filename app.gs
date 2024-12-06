// https://docs.google.com/spreadsheets/d/1CpAF_iey5iBh-5ckH8K3tHoDStczIsKRRyUUCmA88b4/edit?gid=0#gid=0

var SS = SpreadsheetApp.openById('1CpAF_iey5iBh-5ckH8K3tHoDStczIsKRRyUUCmA88b4');
var timezone = "Africa/Lagos"; 
var hours = 0; // Offset hours if needed
var str = "";

// Main function triggered on POST request
function doGet(e) {
 
  try {
    // Extract query parameters
    var sheetName = e.parameter.sheet_name;
    var command = e.parameter.command;

    
    var Curr_Date = new Date(new Date().setHours(new Date().getHours() + hours));
    var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');

    if (sheetName) {
      if (sheetName === "StudentDetails") {
        str  = handleStudentDetails(e.parameter); // Pass the query parameters
        
      } else if (sheetName.includes("Admin")) {
        str = handleAttendance(e.parameter, Curr_Date, Curr_Time); // Pass query parameters, date, and time
      
        
      } else {
        str = "Invalid sheet name.";
      }
    } else {
      str = "Error! Missing 'sheet_name' parameter.";
    }

  } catch (error) {
    Logger.log("Error handling GET request: " + error.message);
    str = "Error: " + error.message;
  }

  Logger.log(str);
  return ContentService.createTextOutput(str);
}

// Handle StudentDetails sheet logic
function handleStudentDetails(data) {
  var sheet = SS.getSheetByName("StudentDetails");
  var matricNumber = data.matricNumber;
  var fingerprintID = data.fingerprintID;
  var command = data.command;
  var responseMessage;

  // Validate required fields
  if (!matricNumber || !fingerprintID || !command) {
    responseMessage = "Error! Missing required fields in payload for StudentDetails.";
    Logger.log(responseMessage);
    return responseMessage;
  }

  // Get all rows from the sheet
  var allData = sheet.getDataRange().getValues();
  var foundRow = 0;

  // Search for duplicate entries
  for (var i = 0; i < allData.length; i++) {
    if (allData[i][0] == matricNumber && allData[i][1] == fingerprintID) {
      foundRow = i +1;
      break;
    }
  }

  // Handle commands
  if (command === "insert") {
    if (foundRow > 0) {
      responseMessage = "Error! Matric number and fingerprint ID already registered.";
    } else {
      sheet.appendRow([matricNumber, fingerprintID]);
      responseMessage = "Successfully registered.";
    }
  } else if (command === "update") {
    var foundRow = -1;
    for (var i = 0; i < allData.length; i++) {
      if (allData[i][0] == matricNumber) {
        foundRow = i + 1;
        break;
      }
    }

    if (foundRow > 0) {
      sheet.getRange(foundRow, 2).setValue(fingerprintID); // Update fingerprint ID
      responseMessage = "Student details updated successfully.";
    } else {
      responseMessage = "Error! Matric number not found.";
    }
  } else if (command === "delete") {
    var foundRow = -1;
    for (var i = 0; i < allData.length; i++) {
      if (allData[i][0] == matricNumber) {
        foundRow = i + 1;
        break;
      }
    }

    if (foundRow > 0) {
      sheet.deleteRow(foundRow);
      responseMessage = "Student details deleted successfully.";
    } else {
      responseMessage = "Error! Matric number not found.";
    }
  } else {
    responseMessage = "Error! Invalid command for StudentDetails.";
  }

  Logger.log(responseMessage);
  return responseMessage;
}




// Handle Attendance sheets logic
function handleAttendance(data, Curr_Date, Curr_Time) {
  var sheetName = data.sheet_name;
  var fingerprintID = data.fingerprintID;

  if (!fingerprintID || !data.command) {
    return "Error! Missing required fields in payload for Attendance.";
   
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
    return "Not Registered";
   
  }

  var sheet = SS.getSheetByName(sheetName);
  var attendanceData = sheet.getDataRange().getValues();
  var row_number = 0;
  var time_out = "";
  var dateToday = Utilities.formatDate(Curr_Date, timezone, "yyyy-MM-dd");

  for (var i = 0; i < attendanceData.length; i++) {
    if (attendanceData[i][1] == matricNumber) { // [1] is Matric Number column
      
      row_number = i + 1;
      time_out = attendanceData[i][3]; // [5] is Time Out column
      break;
      
    }
  }

  if (row_number > 0) {
    if (time_out === "") {
      sheet.getRange("D" + row_number).setValue(Curr_Time); // Sign Out
      return "Signed Out";
      
    } 
  }

  if (data.command === "insert_row") {
    sheet.insertRows(2); // Insert below the header
    sheet.getRange("A2").setValue(dateToday); // Date
    sheet.getRange("B2").setValue(matricNumber); // Matriculation Number
    sheet.getRange("C2").setValue(Curr_Time); // Time In
    SpreadsheetApp.flush();
    return "Signed In";
    
  } else {
    return "Invalid command for Attendance.";
  }
}



