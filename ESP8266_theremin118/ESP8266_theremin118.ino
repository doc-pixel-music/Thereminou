/////////////////////////////////////////////
// LIBRARY

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define SerialMon Serial
#include <AppleMIDI_Debug.h>
#include <AppleMIDI.h>

/////////////////////////////////////////////
// VARIABLES

// Wifi Setup
char ssid[] = "your_wifi_network"; //  your network SSID (name)
char pass[] = "network_password";    // your network password (use for WPA, or use as key for WEP)
unsigned long t0 = millis();
int8_t isConnected = 0; //Used to check if the board is connected
char deviceName[] = "thereminou";

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, deviceName, 5004);


// Ultrasonic Distance Sensor
const int echoPin = 5;
const int trigPin = 16;
const int echoPin2 = 0;
const int trigPin2 = 4;
const int triggerMaxDistance = 50; // Read info under 50cm

// Control MIDI
int controlMidiNumber = 31; // Number of the controler you want to use
int controlMidiNumber2 = 32;


/////////////////////////////////////////////
// FUNCTION

// Calculate the Ultrasonic Distance Sensor (SR04) and return a value between 0 and 127
float readSensorDataControl(){
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);  
  int distance = pulseIn(echoPin, HIGH)/58;  //58 Equivalent to (340m/s*1us)/2
  if (distance <= triggerMaxDistance){
    int correctedInput = map(distance, 0, triggerMaxDistance, 127, 0); // Map 0 - 50cm to 0 - 127 Control Change
    return correctedInput;
  } else {
    return 0;
  }
}

// Calculate the Ultrasonic Distance Sensor (SR04) and return a value between 0 and 127
float readSensorDataControl2(){
  digitalWrite(trigPin2, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);  
  int distance = pulseIn(echoPin2, HIGH)/58;  //58 Equivalent to (340m/s*1us)/2
  if (distance <= triggerMaxDistance){
    int correctedInput = map(distance, 0, triggerMaxDistance, 127, 0);
    return correctedInput;
  } else {
    return 0;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Pin Config
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Serial Monitor Debug
  DBG_SETUP(115200);

  // Start Wifi
  DBG("Booting");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG("Establishing connection to WiFi..");
  }
  DBG("Connected to network");
  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(WiFi.macAddress()); //Important if you want to configure a fixed IP adress
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));
  DBG(F("Listen to incoming MIDI commands"));

  // MIDI
  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });
  
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), note);
  });

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();
  
  
  if (isConnected > 0)
  {

    // LED on to tell  that the connection is OK !
    digitalWrite(LED_BUILTIN, HIGH);

    // Send CC Midi Message _ CUTOFF
    unsigned int ccValue = readSensorDataControl();
    if ((ccValue) > 0 && (ccValue <= 127)) {
      MIDI.sendControlChange(controlMidiNumber, ccValue, 1); // control number, value, channel
    }

    // Send CC Midi Message _ RESO
    unsigned int ccValue2 = readSensorDataControl2();
    if ((ccValue2) > 0 && (ccValue2 <= 127)) {
      MIDI.sendControlChange(controlMidiNumber2, ccValue2, 1); // control number, value, channel
    }

  delay(50); //debounce
  }

}
