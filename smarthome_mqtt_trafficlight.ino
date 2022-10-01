#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#include "config.h"

//Pin for Neopixel Strip
#define PIN 12
#ifdef __AVR__
  #include <avr/power.h>
#endif

WiFiClient espClient;
PubSubClient client(espClient);

//JSON Document for MQTT Messages
StaticJsonDocument<256> doc;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

IPAddress timeServer(192, 53, 103, 108);

long lastMsg = 0;
char msg[50];
int value = 0;
int rCol = 0;
int gCol = 150;
int bCol = 0;
int brightness = 150;

const int timeZone = 1; //CET

WiFiUDP Udp;
unsigned int localPort = 8888;

//callback function for MQTT subscription
void callback(char * topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: ");
  Serial.println(topic);
  deserializeJson(doc, payload, length);
}


void wipeRing() {

  //read RGB values from MQTT document
  rCol = doc["R"];
  gCol = doc["G"];
  bCol = doc["B"];
  brightness = doc["bright"]; 

  //make sure the values are valid
  if ((rCol < 0) || (rCol > 255) || rCol == NULL) {
    rCol = 0;
  }
  if ((gCol < 0) || (gCol > 255) || gCol == NULL) {
    gCol = 140;
  }
  if ((bCol < 0) || (bCol > 255) || bCol == NULL) {
    bCol = 0;
  }
  if ((brightness < 0) || (brightness > 255) || brightness == NULL) {
    brightness = 20;
  }


  //run through ring and set the pixels to desired color
  for (int i = 0; i < 12; i++) {
    strip.setPixelColor(i, strip.Color(rCol, gCol, bCol));
    strip.setBrightness(brightness);
    strip.show();
    delay(100);
  }
}


void setup() {

  Serial.begin(115200);
  Serial.println("setup");
  
  strip.begin();
  strip.setBrightness(50);

  Serial.println("dazwischen");

  strip.show(); // Initialize all pixels to 'off'
  
  setup_wifi();

  //after WIFI start MQTT connection
  client.setServer(MQTT_BROKER, 1883);
  
  Serial.println("setup done");
}


//setup the wifi connection
void setup_wifi() {
  String newHostname = "MQTTLEDMCU";
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.hostname(newHostname.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setCallback(callback);
}


// keep MQTT-Subscription online
void reconnect() {
  while (!client.connected()) {
    Serial.print("Reconnecting...");
    if (!client.connect("TrafficLight")) {
      Serial.print("Failed, rd=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    } else {
      Serial.println("Connected");
      client.publish("Traffic", "I'm alive");
      client.subscribe("/home/test/mqttpixels");
    }
  }
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  strip.clear();
  client.loop();

  wipeRing();

  delay(1000);

}
