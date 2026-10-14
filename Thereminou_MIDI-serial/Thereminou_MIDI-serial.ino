/*
- Thereminou using serial transport for MIDI, you must need Hairless and LoopMidi (or another virtual midi cable)
  http://projectgus.github.io/hairless-midiserial/
  https://www.tobias-erichsen.de/software/loopmidi.html 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  DOC PIXEL Corp. (https://doc-pixel.com/)
*/

// LIBRARY
#include <Control_Surface.h>

USBSerialMIDI_Interface MIDI = 115200;

// VARIABLE

const int ds_triggerMax = 50; // Read info under 50cm
const int ds_triggerMin = 3; // Read info after 3cm
const byte eyesPin = 15; // LED eyes Pin
unsigned int eyesIntensity = 0;
unsigned long t0 = millis(); //timer

//Distance Sensor 01
const byte ds1_echoPin = 4;
const byte ds1_trigPin = 5;
byte ds1_MidiValue = 0;
byte previous_ds1_MidiValue = 0;

const MIDIAddress ds1_Address {42, Channel_1}; // CControl Change 42 of channel 1

//Distance Sensor 02
const byte ds2_echoPin = 4;
const byte ds2_trigPin = 5;
byte ds2_MidiValue = 0;
byte previous_ds2_MidiValue = 0;

const MIDIAddress ds2_Address {43, Channel_1};

// FUNCTION

// Calculate the Ultrasonic Distance Sensor (SR04) and return a value between 0 and 127
float read_ds_value(byte trigPinSensor, byte echoPinSensor){
  digitalWrite(trigPinSensor, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPinSensor, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPinSensor, LOW);  
  unsigned int distance = pulseIn(echoPinSensor, HIGH)/58;  //58 Equivalent to (340m/s*1us)/2
  if (distance >= ds_triggerMin && distance <= ds_triggerMax){
    int correctedInput = map(distance, ds_triggerMin, ds_triggerMax, 127, 0); // Map 3 - 50cm to 0 - 127 Control Change
    return correctedInput;
  } else {
    return 0;
  }
}

void setup() {
  Serial.begin(115200);     // Init Serial Monitor
  Control_Surface.begin();  // Initialize the Control Surface

  // initialize the input:
  pinMode(ds1_echoPin, INPUT);
  pinMode(ds1_trigPin, OUTPUT);
  pinMode(ds2_echoPin, INPUT);
  pinMode(ds2_trigPin, OUTPUT);
  pinMode(eyesPin, OUTPUT);
}

void loop() {

  //Distance sensor 01
  ds1_MidiValue = read_ds_value(ds1_trigPin, ds1_echoPin);
  
  if (ds1_MidiValue > 0) {
    //Serial.println(ds1_MidiValue);
    MIDI.sendControlChange(ds1_Address, ds1_MidiValue);
    previous_ds1_MidiValue = ds1_MidiValue;
    eyesIntensity = map(ds1_MidiValue, 0, 127, 0, 255); // Map 0 - 127 Control Change to 0 - 255 LED intensity
    analogWrite(eyesPin, eyesIntensity);
  }

  //Distance sensor 02
  ds2_MidiValue = read_ds_value(ds2_trigPin, ds2_echoPin);

  if (ds2_MidiValue > 0) {
    MIDI.sendControlChange(ds2_Address, ds2_MidiValue);
    previous_ds2_MidiValue = ds2_MidiValue;
  }

  // If nothing happen during 5 sec : make the eyes bright at the maximum
  if ((ds1_MidiValue == 0) && ((millis() - t0) > 5000)) {  // Value 0 mean the sensor detect nothing within the distance
    t0 = millis();
    analogWrite(eyesPin, 255);
  }

  delay(20); //debounce
}