#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

/* 
 *  2019/07/07
all settings made in config.h 
const char* SSID = "";
const char* PSK = "";
const char* MQTT_BROKER = "";
*/

const int pinStatusGarageDoor = D4;


WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  pinMode(pinStatusGarageDoor, INPUT);
  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

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
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Reconnecting...");
    if (!client.connect("Garage", "cheffe", "wikiwins")) {
      Serial.print("Failed, rd=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    } else {
      Serial.println("Connected");
      client.publish("Garage", "I'm alive");
      client.subscribe("/home/garage/door");
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //snprintf(msg, 50, "Alive since %ld milliseconds", millis());
  //Serial.print("Publish message: ");
  //Serial.println(msg);
  //client.publish("/home/garage", msg);

  if (digitalRead(pinStatusGarageDoor) == LOW) {
    Serial.println("Closed");
    snprintf(msg, 50, "{\"status\":\"closed\"}");
    Serial.println(msg);
    client.publish("/home/garage/door", msg);
  }
  else {
    Serial.println("open");
    snprintf(msg, 50, "{\"status\":\"open\"}");
    client.publish("/home/garage/door", msg);
    Serial.println(msg);
  }
  
  delay(1000);
}