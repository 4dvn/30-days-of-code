function doGet(e){
    try {
        // this helps during debuggin
        if (e == null){e={}; e.parameters = {tag:"tag",value:"-1"};}
        
        var tag = e.parameters.tag;
        var value = e.parameters.value;
        
        // save the data to spreadsheet
        save_data(tag, value);
        
    } catch(error) { 
        Logger.log(error);    
    }  
}
 
// Method to save given data to a sheet
function save_data(tag, value){
    try {
        var dateTime = new Date();
        
        var ss = SpreadsheetApp.openByUrl("https://docs.google.com/spreadsheets/d/{document_id}/edit");
        var dataLoggerSheet = ss.getSheetByName("MySheetName");
        
        
        // Get last edited row from DataLogger sheet
        var row = dataLoggerSheet.getLastRow() + 1;
        
        // ID, DateTime, Tag, Value
        dataLoggerSheet.getRange("A" + row).setValue(row -1); // ID
        dataLoggerSheet.getRange("B" + row).setValue(dateTime); // dateTime
        dataLoggerSheet.getRange("C" + row).setValue(tag); // tag
        dataLoggerSheet.getRange("D" + row).setValue(value); // value
    }
 
    catch(error) {
        Logger.log(JSON.stringify(error));
    }
}
