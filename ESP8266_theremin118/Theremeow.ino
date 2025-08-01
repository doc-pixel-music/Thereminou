//   |  _ \  ___   ___  |  _ \(_)_  _____| |
//   | | | |/ _ \ / __| | |_) | \ \/ / _ \ |
//   | |_| | (_) | (__  |  __/| |>  <  __/ |
//   |____/ \___/ \___| |_|   |_/_/\_\___|_|

// > [ https://doc-pixel.com ]

/////////////////////////////////////////////
// LIBRARY

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define SerialMon Serial
#include <AppleMIDI_Debug.h>

#include <AppleMIDI.h>

// Wifi Setup
char ssid[] = "network"; //  your network SSID (name)
char pass[] = "password";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
int8_t isConnected = 0;

char deviceName[] = "theremineow";

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, deviceName, 5004);

// PIN Ultrasonic Distance Sensor
const int echoPin = 23;
const int trigPin = 22;
const int echoPin2 = 18;
const int trigPin2 = 19;
const int triggerMaxDistance = 50; // Read info under 50cm

// PIN LED for the eyes
const int eyes = 21;

// Control MIDI
int controlMidiNumber = 31;
int controlMidiNumber2 = 32;

/////////////////////////////////////////////
// FUNCTION

// Calculate the Ultrasonic Distance Sensor (SR04) and return a value between 0 and 127

float readSensorDataControl(int trigPinSensor, int echoPinSensor){
  digitalWrite(trigPinSensor, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPinSensor, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPinSensor, LOW);  
  int distance = pulseIn(echoPinSensor, HIGH)/58;  //58 Equivalent to (340m/s*1us)/2
  if (distance <= triggerMaxDistance){
    int correctedInput = map(distance, 0, triggerMaxDistance, 127, 0); // Map 0 - 50cm to 0 - 127 Control Change
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
  pinMode(eyes, OUTPUT);

  DBG_SETUP(115200);
  DBG("Booting");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG("Establishing connection to WiFi..");
  }
  DBG("Connected to network");

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(WiFi.macAddress());
  DBG(F("Listen to incoming MIDI commands"));

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

  DBG(F("Thereminou is ready to work"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();

  int fadeValue = 0;

  // send Midi CC
  if (isConnected > 0)
  {
    // Send CC Midi Message _ CUTOFF
    unsigned int ccValue = readSensorDataControl(trigPin, echoPin);
    unsigned int prevccValue = 0;

    if ((ccValue) > 0 && (ccValue <= 127)) {
      MIDI.sendControlChange(controlMidiNumber, ccValue, 1); // control number, value, channel
      fadeValue = map(ccValue, 0, 127, 0, 255); // Map 0 - 127 Control Change to 0 - 255 LED intensity
      analogWrite(eyes, fadeValue);
      prevccValue = ccValue;
    }

    // Send CC Midi Message _ RESO
    unsigned int ccValue2 = readSensorDataControl(trigPin2, echoPin2);
    if ((ccValue2) > 0 && (ccValue2 <= 127)) {
      MIDI.sendControlChange(controlMidiNumber2, ccValue2, 1); // control number, value, channel
    }

    // If nothing happen during 5 sec : make the eyes bright at the maximum
    if ((ccValue == prevccValue) && ((millis() - t0) > 5000)) {
      t0 = millis();
      analogWrite(eyes, 255);
    }

    delay(10); //debounce

    
  }
}
