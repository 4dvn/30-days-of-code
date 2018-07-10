#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define TAG_NAME "SoilMoistureLevel"
#define SOIL_PIN 0

// WiFi Credentials
const char * WIFI_SSID = "MySSID";
const char *  WIFI_PASS = "MyWiFiPass";

// IFTTT "Plant" Notification
const char* GOOGLE_HOST = "script.google.com";
const char* FINGERPRINT = "29 40 55 83 01 36 FA 5D BD 5E 6B 91 58 29 EA 8F 26 7E 22 0B";
String GOOGLE_URL = "/macros/s/{script_id}/exec";

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

int httpsGET(const char* host, const char* fingerprint, String url) {
    WiFiClientSecure client;
    
    Serial.print("Connecting to: ");
    Serial.println(host);
    if (!client.connect(host, 443)) {
        Serial.println("connection failed");
        return -1;
    }
    
    if (client.verify(fingerprint, host)) {
        Serial.println("certificate matches");
    } else {
        Serial.println("certificate doesn't match");
    }

    // Send GET
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
    
    Serial.println("GET sent");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("HTTP/1.1")) {
            // Get HTTP return code
            return line.substring(9,12).toInt();
        }
    }
    
    return -1;
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

        String url = GOOGLE_URL + "?tag=" + TAG_NAME + "&value=" + soilMoistureLevel;

        Serial.print("[-] Sending GET to URL : ");
        Serial.println(url);

        int retCode = httpsGET(GOOGLE_HOST, FINGERPRINT, url);
        if (retCode==HTTP_CODE_OK) {
            Serial.println("[-] Reporting succeeded.");
        } else {
             Serial.print("[!] Reporting failed: Code ");
            Serial.println(retCode);          
        }
    }
    
    Serial.println("[-] Sleeping");
    ESP.deepSleep(36e8); // 3600 sec / 1 hr
}

void loop() {
    // Stub - sleep voids this
}

