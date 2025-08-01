# How to build your own Theremeow :cat: :robot:

![Theremeow](https://github.com/LivestockPixel/Thereminou/blob/main/Thereminou.jpg)

Theremeow is inspirated by the famous electronic instrument : **The Theremin**. You'll find here all the sources to build your own **reactive cyber-cat MIDI controler**.
Theremeow is a really lightweight device that can send MIDI messages throws a Wifi network (with RTPmidi).
This controller is based on 2 ultrasonic sensors that read the distance between your hand and the box, then send MIDI control message based on this distance value.

## Required Components:
- ESP8266 Board
- HC-SR04 Ultrasonic Sensor x2
- Breadboard
- Basic LED 5mm x2
- 220 Ohm Resistor x1
- Breadboard Jumper Wires

## Required Software:
- [Arduino IDE](https://docs.arduino.cc/software/ide/#ide-v2)
- [RTPmidi](https://www.tobias-erichsen.de/software/rtpmidi.html)

## Circuit Diagram:

![Circuit Thereminou](https://github.com/LivestockPixel/Thereminou/blob/main/Thereminou_circuit.png)

## Setup
Open the Arduino IDE code editor. Open the *ESP8266_theremin118.ino* file.
Install all the dependencies. 
Set up your your Wifi network :
```
char ssid[] = "your_wifi_network"; //  your network SSID (name)
char pass[] = "network_password";    // your network password (use for WPA, or use as key for WEP)
```

Assign the sensors to the right pins :
```
const int echoPin = 5;
const int trigPin = 16;
const int echoPin2 = 0;
const int trigPin2 = 4;
const int triggerMaxDistance = 50; // Adjust the limit distance (up to 50cm)
```

Load the program on your board.
If your board is correctly set-up and connected to your network, you'll see on the monitor the IP address. Use it to configure RTPmidi in order to receive MIDI messages on your computer.

## To go further
Theremeow could also send Midi message directly via USB (if your board supports it natively, like Arduino Leonardo or Teensy ).
Check your board here : [MIDI over USB board](https://tttapa.github.io/Control-Surface-doc/Doxygen/d8/d4a/md_pages_MIDI-over-USB.html).
Otherwise you can use Hairless to get your MIDI message via serial : [Hairless MIDI Bridge](https://projectgus.github.io/hairless-midiserial/)

Many thanks to Gurbrinder Grewal, Derek Pritchett and Bryne Carruthers for their work : [Arduino Theremin](https://www.arduinotheremin.com/)

More about the **Doc Pixel** project : [Doc Pixel](https://doc-pixel.com)
