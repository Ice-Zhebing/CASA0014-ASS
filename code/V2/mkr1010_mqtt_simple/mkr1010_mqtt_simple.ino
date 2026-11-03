#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Adafruit_VEML7700.h>
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>

// WiFi and MQTT credentials
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* ssid1         = SECRET_SSID1;
const char* password1     = SECRET_PASS1;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

// Keep the original course MQTT settings
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ID 22.
String lightId = "22";
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;
String clientId = "";

// LED payload: 72 pixels, 3 bytes per pixel
const int num_leds = 72;
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];

const int trigPin = 2;
const int echoPin = 3;

Adafruit_VEML7700 lightSensor;

const float BRIGHT_LUX = 100.0f;
const float DARK_LUX = 50.0f;

const float NEAR_CM = 20.0f;
const float FAR_CM = 60.0f;

const byte COLOUR_LEVEL = 120;

int litPixels = num_leds;

void setup() {
  Serial.begin(115200);

  WiFiDrv::pinMode(25, OUTPUT);
  WiFiDrv::pinMode(26, OUTPUT);
  WiFiDrv::pinMode(27, OUTPUT);
  LedRed();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  Wire.begin();

  if (!lightSensor.begin()) {
    Serial.println(
      F("VEML7700 not found. Check power, GND, SDA and SCL.")
    );

    while (true) {
      delay(1000);
    }
  }

  lightSensor.setGain(VEML7700_GAIN_1_8);
  lightSensor.setIntegrationTime(VEML7700_IT_100MS);
  delay(200);

  // Unique MQTT client ID
  byte mac[6];
  WiFi.macAddress(mac);

  char macStr[18];
  snprintf(
    macStr,
    sizeof(macStr),
    "%02X%02X%02X%02X%02X%02X",
    mac[5], mac[4], mac[3],
    mac[2], mac[1], mac[0]
  );

  clientId = "MKR1010_Sender_" + String(macStr);

  Serial.println(F("Starting Vespera sensor controller..."));
  Serial.print(F("Client ID: "));
  Serial.println(clientId);
  Serial.print(F("MQTT topic: "));
  Serial.println(mqtt_topic);

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(30);

  // Defined in connections.ino
  startWifi();

  Serial.println(F("Setup complete."));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();

    if (WiFi.status() != WL_CONNECTED) {
      return;
    }
  }


  if (!mqttClient.connected()) {
    reconnectMQTT();

    if (!mqttClient.connected()) {
      return;
    }
  }

  mqttClient.loop();

  float lux = lightSensor.readLux();

  if (lux >= BRIGHT_LUX) {
    litPixels = num_leds/ 2;       // 36  LEDs
  } else if (lux <= DARK_LUX) {
    litPixels = num_leds ;   // 72 LEDs
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);

  float distance = duration * 0.0343f / 2.0f;

  const char* colour;

  if (duration == 0 || distance >= FAR_CM) {
    send_colour_to_all(0, COLOUR_LEVEL, 0);
    colour = "GREEN";
  } else if (distance >= NEAR_CM) {
    send_colour_to_all(0, 0, COLOUR_LEVEL);
    colour = "BLUE";
  } else {
    send_colour_to_all(COLOUR_LEVEL, 0, 0);
    colour = "RED";
  }

  // Diagnostic output
  Serial.print(F("Lux: "));
  Serial.print(lux, 1);

  Serial.print(F(" | Distance: "));
  if (duration == 0) {
    Serial.print(F("No echo"));
  } else {
    Serial.print(distance, 1);
    Serial.print(F(" cm"));
  }

  Serial.print(F(" | Colour: "));
  Serial.print(colour);

  Serial.print(F(" | LEDs on: "));
  Serial.println(litPixels);

  mqttClient.loop();
  delay(200);
}

// Half mode 
void send_colour_to_all(int r, int g, int b) {
  if (!mqttClient.connected()) {
    Serial.println(F("MQTT disconnected; cannot publish."));
    return;
  }

  byte red   = (byte)constrain(r, 0, 255);
  byte green = (byte)constrain(g, 0, 255);
  byte blue  = (byte)constrain(b, 0, 255);

  for (int pixel = 0; pixel < num_leds; pixel++) {
    bool on = (litPixels == num_leds) || (pixel % 6 < 3);

    RGBpayload[pixel * 3 + 0] = on ? red : 0;
    RGBpayload[pixel * 3 + 1] = on ? green : 0;
    RGBpayload[pixel * 3 + 2] = on ? blue : 0;
  }

  bool sent = mqttClient.publish(
    mqtt_topic.c_str(),
    RGBpayload,
    payload_size
  );

  if (!sent) {
    Serial.println(F("MQTT publish failed."));
  }
}

// Switch every LED off
void send_all_off() {
  send_colour_to_all(0, 0, 0);
}