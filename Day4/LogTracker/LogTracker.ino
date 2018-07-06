#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>

float lat = 0.0,lon = 0.0, alt = 0.0;
SoftwareSerial gpsSerial(18,19);//rx,tx
TinyGPS gps;

long counter = 0;
const int chipSelect = 4;

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    
    gpsSerial.begin(9600); // connect gps sensor
    
    Serial.print("Initializing SD card...");
    
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        while (1);
    }
    Serial.println("card initialized.");
}

void loop() {
    while(gpsSerial.available()) { // check for gps data
        if(gps.encode(gpsSerial.read())) { 
            gps.f_get_position(&lat,&lon); // get latitude and longitude
            alt = gps.f_altitude();
        }
    }
    
    // make a string for assembling the data to log:
    String count = String(counter++);
    String latitude = String(lat,6);
    String longitude = String(lon,6);
    String altitude = String(alt);
    String dataString = count+","+latitude+","+longitude+","+altitude;
    Serial.println(dataString);

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("location.txt", FILE_WRITE);
    
    // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
    }
    
    // if the file isn't open, pop up an error:
    else {
        Serial.println("error opening location.txt");
    }

    delay(30000);
}









