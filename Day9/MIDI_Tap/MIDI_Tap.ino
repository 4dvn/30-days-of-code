#include <MPU6050_tockn.h>
#include <Wire.h>
#include "MIDIUSB.h"

// ms to Sample
#define SAMPLE_PERIOD 5

// 48 = D1
#define MIDI_NOTE 38

// Sensitivity Threshholds
#define ACC_MIN 1
#define ACC_MAX 10
#define VELOCITY_BIAS 5

MPU6050 mpu6050(Wire);


// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

c

void setup() {
    Serial.begin(115200);
    Wire.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
}

void loop() {
    mpu6050.update();

    // Take weighted average of samples
    float sample = mpu6050.getAccY();
    accY = ( min(sample, accY)*0.30 + max(sample, accY)*0.70 );

    // If sample period is up
    if(millis() - timer > SAMPLE_PERIOD){

        // Measure change in Y since last sample period
        float diffY = abs(accY - oldAccY)*10;
        oldAccY = accY;

        int thresh = (diffY > ACC_MIN) ? 1 : 0;
        Serial.print("Thresh : ");Serial.print(thresh);
        Serial.print("\tDiffY : ");Serial.println(diffY);


        if (thresh && !isOn) {
            // Trim to ACC_MIN <= velocity <= ACC_MAX
            velocity = max(min(ACC_MAX, diffY), ACC_MIN);
            
            // Map to Int range [VELOCITY_BIAS:127]
            velocity = int(map(velocity, ACC_MIN, ACC_MAX, VELOCITY_BIAS, 127));
           
            noteOn(1, MIDI_NOTE, velocity);
            isOn = true;
        } else {
            noteOff(1, MIDI_NOTE, 0);
            isOn = false;
        }
        
        MidiUSB.flush();
        timer = millis();
    }
}
