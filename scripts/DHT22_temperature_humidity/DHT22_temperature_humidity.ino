#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>
#include <Time.h>

#define DHTPIN 4
#define DHTTYPE DHT22
const char* ssid = "MacBart_2.4";
const char* password =  "Siepie1962!!!";
const char* mqttServer = "www.macbart.com";
const int mqttPort = 1883;
const char* mqttUser = "macbart";
const char* mqttPassword = "Siepie1962!";
const char* sensor_topic = "macbart/sensor";
const char* reset_topic = "macbart/reset/sensor/dht22_1";
const char* client_name = "macbart-sensor-dht22_1";

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
  pubSubClient.setBufferSize(512);
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  StaticJsonDocument<200> doc;
  pubSubClient.setServer(mqttServer, mqttPort);
  pubSubClient.setCallback(callback);
  connectmqtt();
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(500);  
  }
  Serial.println("Connected to the WiFi network");
}

void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (pubSubClient.connect(client_name, mqttUser, mqttPassword )) {
      Serial.println("connected");
      pubSubClient.subscribe(reset_topic); 
    } else {
      Serial.print("failed with state ");
      Serial.print(pubSubClient.state());
      delay(1000);
    }
  }
}
 
void callback(char* topic, byte* payload, unsigned int length) {
//  if (topic == "macbart/reset/sensor/dht22_1") {
    ESP.restart();
//  } 
}
 
void loop() {
  if (!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();
  
  // Reading sensor
  float hum = dht.readHumidity();
  float tmp = dht.readTemperature();
  float vcc = ESP.getVcc();
  Serial.println(vcc);
  
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

void connectmqtt() {
  pubSubClient.connect(client_name);
  pubSubClient.subscribe("macbart/reset/sensor/dht22_1");
  if (!pubSubClient.connected()) {
    reconnect();
  }
}
