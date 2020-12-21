#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>
#include <Time.h>

#define DHTTYPE DHT11
const char* ssid = "MacBart_2.4";
const char* password =  "Siepie1962!!!";
const char* mqttServer = "www.macbart.com";
const int mqttPort = 1883;
const char* mqttUser = "macbart";
const char* mqttPassword = "Siepie1962!";
const char* sensor_topic = "macbart/sensor";
const char* reset_topic = "macbart/reset";

float tmp_min = 99;
float tmp_max = -99;
float tmp = 0;
float hum_min = 100;
float hum_max = 0;
float hum = 0;

DHT dht(DHTPIN, DHTTYPE); 
WiFiClient espClient;
PubSubClient pubSubClient(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  StaticJsonDocument<200> doc;
  pubSubClient.setServer(mqttServer, mqttPort);
  pubSubClient.setCallback(callback);
  pubSubClient.subscribe(reset_topic);
}

void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (pubSubClient.connect("macbart-sensor-001", mqttUser, mqttPassword )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(pubSubClient.state());
      delay(2000);
    }
  }
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  
}
 
void loop() {
      if (!pubSubClient.connected()) {
        reconnect();
      }

      // Reading sensor
      hum = dht.readHumidity();
      tmp = dht.readTemperature();
      
      // Check if any reads failed and exit early (to try again).
      if (isnan(hum) || isnan(tmp)) {
        Serial.println("Failed to read from DHT sensor!");
      return;
      }

      // Set max/min Temperatuur
      if (tmp > tmp_max) {
        tmp_max = tmp;
      }
      if (tmp < tmp_min) {
        tmp_min = tmp;
      }

      // Set max/min Vochtigheid
      if (hum > hum_max) {
        hum_max = hum;
      }
      if (hum < hum_min) {
        hum_min = hum;
      }

      // create json
      const int capacity = JSON_OBJECT_SIZE(512);
      StaticJsonDocument<capacity> doc;
      doc["Temperatuur"] = tmp;
      doc["Temp_min"] = tmp_min;
      doc["Temp_max"] = tmp_max;
      doc["Vochtigheid"] = hum;
      doc["Hum_min"] = hum_min;
      doc["Hum_max"] = hum_max;
      char payload[capacity + 1];
      serializeJson(doc,payload);
      
      // Publish json
      pubSubClient.publish(sensor_topic, payload, true);
      delay(1000);
}
