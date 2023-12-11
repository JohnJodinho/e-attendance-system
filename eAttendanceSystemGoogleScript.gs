// Enter Spreadsheet ID here
var SS = SpreadsheetApp.openById('YOUR_SPREADSHEET_ID');
var timezone = "YOUR_TIMEZONE";  // Replace with your actual timezone
var hours = 0;
var str = "";

// The main function triggered when a POST request is received
function doPost(e) {
  var parsedData;
  try {
    parsedData = JSON.parse(e.postData.contents);
  } catch (f) {
    Logger.log("Error!");
    return ContentService.createTextOutput(str);
  }

  if (parsedData !== undefined) {
    var flag = parsedData.format;
    if (flag === undefined) {
      flag = 0;
    }

    var sheetName = parsedData.sheet_name; 
    var sheet = SS.getSheetByName(sheetName);

    var Curr_Date = new Date(new Date().setHours(new Date().getHours() + hours));
    var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');

    if (sheetName === "EmployeeDetails") {
      // Check if Employee ID already exists
      var found = false;
      var employeeId = parsedData.values; // Assuming Employee ID is the first value in the payload
      var employeeDetailsData = sheet.getDataRange().getValues();
      for(var i = 0; i < employeeDetailsData.length ; i++){  // Search first occurrence of employee id
        if(employeeDetailsData[i][0] == employeeId){ //employeeDetailsData[i][0] i.e. [0]=Column A, employee_id
          found = true;
          str = "Already Registered";
          break;
        }
          
      }
      if (!found) {
        // Assuming the other fields in the EmployeeDetails sheet are populated by someone (Admin)
        if (parsedData.command === "insert_row"){
          sheet.insertRows(2); // insert full row directly below header text
          sheet.getRange('A2').setValue(employeeId);
          str = "Successfully Registered";
          SpreadsheetApp.flush();
        }
      }
     return ContentService.createTextOutput(str);
    } 
    
    else if (sheetName === "AttendanceSheet") {
      // Check if Employee ID exists in EmployeeDetails
      var found = false;
      var employeeId = parsedData.values; // Assuming Employee ID is the first value in the payload
      var employeeDetailsData = SS.getSheetByName("EmployeeDetails").getDataRange().getValues();
    
      for(var i = 0; i < employeeDetailsData.length ; i++){  // Search first occurrence of employee id
        if(employeeDetailsData[i][0] == employeeId){ //employeeDetailsData[i][0] i.e. [0]=Column A, employee_id
          var rowWithEmployeeID = i;

          //------------------------------------------------------------------------------------------------------
          /* STEP1 - This piece of code searches for the Employee ID in the attendance sheet. If the Employee ID is found, 
          it gets the row number of that Employee ID and retrieves their time-out data. 
          */
          data = sheet.getDataRange().getValues();
          var row_number = 0;
          var time_out = "";
          //for(var i = data.length - 1; i >= 0; i--){  // Search last occurrence 
          for(var i = 0; i < data.length ; i++){  // Search first occurrence of employee id
            if(data[i][1] == employeeId){ //data[i][1] i.e. [1]=Column B, employee_id
              row_number = i+1;
              time_out = data[i][5] //time out [5]=Column F
              
              console.log("row number: "+row_number); //print row number
              console.log("time out: "+time_out); //print row number
              break; //go outside the loop
            }
          }
          /* STEP2 - Next, it checks if the time-out variable is empty. If it is empty, the current time is added to the 
          time-out field and a message is returned to NodeMcu. 
          */
          if(row_number > 0){
            if(time_out === ""){
              sheet.getRange("F"+row_number).setValue(Curr_Time);
              str = "Signed Out"; // string to return back to Arduino serial cons 
              return ContentService.createTextOutput(str);
            }
          }
          // Otherwise, the attendance is recorded as usual using the code written below
          //------------------------------------------------------------------------------------------------------  
          // Get First Name and Last Name from EmployeeDetails
          var firstName = employeeDetailsData[rowWithEmployeeID][1]; // Assuming First Name is in the second column
          var lastName = employeeDetailsData[rowWithEmployeeID][2]; // Assuming Last Name is in the third column

          // Read and execute command from the "payload_base" string specified in Arduino code
          if(parsedData.command === "insert_row"){ 
            sheet.insertRows(2); // insert full row directly below header text
            sheet.getRange('A2').setValue(Curr_Date);
            sheet.getRange('B2').setValue(employeeId);
            sheet.getRange('C2').setValue(firstName);
            sheet.getRange('D2').setValue(lastName);
            sheet.getRange('E2').setValue(Curr_Time);
            str = "Signed In"; // string to return back to Arduino serial console
            SpreadsheetApp.flush();
          }

          found = true;
          break; // go outside the loop
        }
      }

      if (!found) {
        str = "Not Registered";
      } 
    }

    Logger.log(str);
    return ContentService.createTextOutput(str);
  } else {
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}
