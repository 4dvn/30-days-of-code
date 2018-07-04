#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include <Adafruit_NeoPixel.h>
    
#define USERNAME "username"
#define DEVICE_ID "reminderlight"
#define DEVICE_CREDENTIAL "AAAAAAAAAA"

#define SSID "My-WiFi-SSID"
#define SSID_PASSWORD "mywifipass"

#define LED_PIN 2 // D4, GPIO2
#define LED_FADE_STEPS 128

// Pattern types supported:
enum  pattern { NONE, FADE };
enum direction { FORWARD, REVERSE };
 
class NeoPatterns : public Adafruit_NeoPixel {
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type) {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update() {
        if((millis() - lastUpdate) > Interval) {
            lastUpdate = millis();
            switch(ActivePattern) {
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment() {
        if (Direction == FORWARD) {
           Index++;
           if (Index >= TotalSteps) {
                Index = 0;
                if (OnComplete != NULL) {
                    OnComplete();
                }
            }
        }
        else { // Direction == REVERSE
            --Index;
            if (Index < 0) {
                Index = TotalSteps-1;
                if (OnComplete != NULL) {
                    OnComplete();
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse() {
        if (Direction == FORWARD) {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate() {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        uint8_t white = ((White(Color1) * (TotalSteps - Index)) + (White(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue, white));
        show();
        Increment();
    }

    void SetOff(){
        ActivePattern = NONE;
        Index = 0;
        
        ColorSet(Color(0,0,0,0));
        show();
    }
   
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color) {
        for (int i = 0; i < numPixels(); i++) {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color) {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color) {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color) {
        return color & 0xFF;
    }

    // Returns the White component of a 32-bit color
    uint8_t White(uint32_t color) {
        return (color >> 24) & 0xFF;
    }
    
};

void Ring1Complete();
 
NeoPatterns Ring1 = NeoPatterns(24, LED_PIN, NEO_GRB + NEO_KHZ800, &Ring1Complete);
ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

int repeat = 0;
int fade_in = 0;
void Ring1Complete() {
    if (fade_in) {
        fade_in = 0;
        Ring1.Fade(Ring1.Color2, 0, LED_FADE_STEPS, Ring1.Interval);
    } else {
        Ring1.SetOff();
        if (--repeat >0) {
            fade_in = 1;
            Ring1.Fade(0, Ring1.Color1, LED_FADE_STEPS, Ring1.Interval);
        } else {
            Ring1.SetOff();
        }
    }
}

void notify(uint32_t color, int duration) {
    if (repeat > 0 && duration > 0) {
        fade_in = 1;
        int interval = (duration*500 / LED_FADE_STEPS); 
        Ring1.Fade(0, color, LED_FADE_STEPS, interval);
    }
}

void setup() {
    Ring1.begin();
    Ring1.SetOff();

    thing.add_wifi(SSID, SSID_PASSWORD);


    thing["notifyRGB"] << [](pson& in){
        int r = in["r"];
        int g = in["g"];
        int b = in["b"];
        int duration = in["duration"];
        repeat = in["repeat"];

        notify(Ring1.Color(r,g,b,0), duration);
    };

    thing["notify"] << [](pson& in){
        int color = in["color"];
        int duration = in["duration"];
        repeat = in["repeat"];

        notify(color, duration);
    };
}

void loop() {
    thing.handle();
    Ring1.Update();
}
