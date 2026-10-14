/*
  Thereminou using ESP-NOW transport for MIDI, you must need the ESP-NOW to USB-MIDI bridge 
  You'll find it on my Github page (https://github.com/doc-pixel-music)
  Many thanks to Rui Santos (https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/)
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  DOC PIXEL Corp. (https://doc-pixel.com/)
*/

#include <WiFi.h>
#include <esp_now.h>

// VARIABLE
const int ds_triggerMax = 50; // Read info under 50cm
const int ds_triggerMin = 3; // Read info after 3cm
const byte eyesPin = 21; // LED eyes Pin
unsigned int eyesIntensity = 0;
unsigned long t0 = millis(); //timer

//Distance Sensor 01
const byte ds1_echoPin = 23;
const byte ds1_trigPin = 22;
unsigned int ds1_Value = 0;
byte ds1_MidiValue = 0;
byte previous_ds1_MidiValue = 0;

//Distance Sensor 02
const byte ds2_echoPin = 18;
const byte ds2_trigPin = 19;
unsigned int ds2_Value = 0;
byte ds2_MidiValue = 0;
byte previous_ds2_MidiValue = 0;

// ESP-NOW SETTINGS
uint8_t broadcastAddress[] = {0x44, 0xB1, 0x76, 0xCD, 0x67, 0x2C}; // Receiver MAC Address

// Structure of data ; Must match the receiver structure
typedef struct struct_message {
  byte channel; //midi channel to use
  byte type;  // 1 : Note ; 2 : Controle Change; 3 : Program
  byte address; // Address of note, cc or program change (0-127)
  byte value; // Velocity on type note or Controle Change value
  bool keypress; // Note ON or OFF (only for Type 1)
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

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
    return distance;
  } else {
    return 0;
  }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // initialize the input:
  pinMode(ds1_echoPin, INPUT);
  pinMode(ds1_trigPin, OUTPUT);
  pinMode(ds2_echoPin, INPUT);
  pinMode(ds2_trigPin, OUTPUT);
  pinMode(eyesPin, OUTPUT);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {

  myData.channel = 1;
  myData.type = 2;

  //Distance sensor 01
  ds1_Value = read_ds_value(ds1_trigPin, ds1_echoPin);
  
  if (ds1_Value > 0) {
    myData.address = 31;
    ds1_MidiValue = map(ds1_Value, ds_triggerMin, ds_triggerMax, 127, 0); // Map 3 - 50cm to 0 - 127 Control Change
    myData.value = ds1_MidiValue;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData)); // Send message via ESP-NOW
    previous_ds1_MidiValue = ds1_MidiValue;
    eyesIntensity = map(ds1_Value, 0, 50, 255, 0); // Change to 0 - 255 LED intensity
    analogWrite(eyesPin, eyesIntensity);
  }

  //Distance sensor 02
  ds2_Value = read_ds_value(ds2_trigPin, ds2_echoPin);

  if (ds2_Value > 0) {
    myData.address = 32;
    ds2_MidiValue = map(ds2_Value, ds_triggerMin, ds_triggerMax, 127, 0); // Map 3 - 50cm to 0 - 127 Control Change
    myData.value = ds2_MidiValue;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData)); // Send message via ESP-NOW
    previous_ds2_MidiValue = ds2_MidiValue;
  }

  // If nothing happen during 5 sec : make the eyes bright at the maximum
  if ((ds1_Value == 0) && ((millis() - t0) > 5000)) {  // Value 0 mean the sensor detect nothing within the distance
    t0 = millis();
    analogWrite(eyesPin, 255);
  }
   
  delay(20); //Debounce
}