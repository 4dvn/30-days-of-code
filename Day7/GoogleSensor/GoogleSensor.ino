#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define TAG_NAME "SoilMoistureLevel"
#define SOIL_PIN 0

// WiFi Credentials
const char * WIFI_SSID = "MySSID";
const char *  WIFI_PASS = "myWiFiPass";

// IFTTT "Plant" Notification
String IFTTT_URL = "http://maker.ifttt.com/trigger/googleSheets/with/key/{ifttt_api_key}";

int wifiConnect() {
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    unsigned long wifiConnectStart = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        // Check to see if
        if (WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("Failed to connect to WiFi. Please verify credentials: ");
            delay(10000);
        }
        
        delay(500);
        Serial.println("...");
        // Only try for 5 seconds.
        if (millis() - wifiConnectStart > 15000) {
            Serial.println("Failed to connect to WiFi");
            return 0;
        }
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return 1;
}

int getSoilMoistureLevel(int pin) {
    int value = analogRead(pin)/10;
    return min(100, max(value,0));
}

int httpJSONPOST(String url, String json) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.print("Connecting to ");
        Serial.println(url);
        http.begin(url);
        
        http.addHeader("Content-Type", "application/json");

        Serial.print("Posting ");
        Serial.println(json);
        int httpCode = http.POST(json);
        String payload = http.getString();  
        http.end();

        Serial.println(payload); 
   
        return (httpCode == HTTP_CODE_OK) ? 0 : httpCode;
    } else {
        return -1;
    }
}
 
void setup() {
    Serial.begin(115200);
    Serial.setTimeout(2000);

    Serial.println("[-] Connecting to WiFi");
    if (wifiConnect()) {
        
        Serial.println("[-] Reading soil moisture");
        int soilMoistureLevel = getSoilMoistureLevel(SOIL_PIN);
        Serial.print("Moisture level: ");
        Serial.print(soilMoistureLevel);
        Serial.println("%");

        String json = "{\"value1\":\""+String(TAG_NAME)+"\",\"value2\":\""+String(soilMoistureLevel)+"\"}";

        Serial.print("[-] Reporting: ");
        Serial.println(json);

        int retCode = httpJSONPOST(IFTTT_URL, json);
        if (retCode) {
            Serial.print("Reporting failed: Code ");
            Serial.println(retCode);
        }
    }
    
    Serial.println("[-] Sleeping");
    ESP.deepSleep(36e8); // 3600 sec / 1 hr
}

void loop() {
    // Stub - sleep voids this
}

