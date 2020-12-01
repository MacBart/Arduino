#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "ESPDateTime.h"
#include <ArduinoJson.h>

// Define variables
#define DHTPIN 4
#define DHTTYPE DHT11
const char* ssid = "MacBart_2.4";
const char* password =  "Siepie1962!!!";
const char* mqttServer = "www.macbart.com";
const int mqttPort = 1883;
const char* mqttUser = "macbart";
const char* mqttPassword = "Siepie1962!";
const char* sensor_topic = "macbart/sensor";
char buffer[256];
float tmp_i = 99;
float tmp_a = -99;
float hum_i = 100;
float hum_a = 0;

DHT dht(DHTPIN, DHTTYPE); 
WiFiClient espClient;
PubSubClient pubSubClient(espClient);
StaticJsonDocument<256> jdoc;

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  pubSubClient.setServer(mqttServer, mqttPort);
  pubSubClient.setCallback(callback);


  // DateTime
  DateTime.setServer("nl.pool.ntp.org");
  DateTime.begin(15 * 1000);
  DateTime.setTimeZone(8);
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
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
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}
 
void loop() {
      if (!pubSubClient.connected()) {
        reconnect();
      }
      pubSubClient.loop();
      delay(1000);
      
      // Reading sensor
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
      return;
      }

      // Set DateTime
      time_t d = DateTime.now();
      String dt = DateTime.toString();
      
      // Reset max / min
//      if (%H == 0 && reset = false) {
//        reset = true;  
//      } elif (%H == 23 && reset = true {
//        reset = false;
//      }

      // Set max/min Temperatuur
      if (t > tmp_a) {
        tmp_a = t;
      }
      if (t < tmp_i) {
        tmp_i = t;
      }
      String tmp = String(t).c_str();
      String tmp_min = String(tmp_i).c_str();
      String tmp_max = String(tmp_a).c_str();

      // Set max/min Vochtigheid
      if (h > hum_a) {
        hum_a = h;
      }
      if (h < hum_i) {
        hum_i = h;
      }
      String hum = String(h).c_str();
      String hum_min = String(hum_i).c_str();
      String hum_max = String(hum_a).c_str();
      
      // Publish json
      String json = "{\"DatumTijd\": " + dt + ", \"Temperatuur\": " + tmp + ", \"Temp_min\": " + tmp_min + ", \"Temp_max\": " + tmp_max + ", \"Vochtigheid\": " + hum + ", \"Hum_min\": " + hum_min + ", \"Hum_max\": " + hum_max + "}";
      json.toCharArray(buffer, (json.length() + 1));
      pubSubClient.publish(sensor_topic, buffer, true);
}
