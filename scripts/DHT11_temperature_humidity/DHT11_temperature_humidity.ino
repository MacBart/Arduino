#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Define variables
#define DHTPIN 4
#define DHTTYPE DHT11
const char* ssid = "MacBart_2.4"; // Enter your WiFi name
const char* password =  "Siepie1962!!!"; // Enter WiFi password
const char* mqttServer = "www.macbart.com";
const int mqttPort = 1883;
const char* mqttUser = "macbart";
const char* mqttPassword = "Siepie1962!";
#define humidity_topic "macbart/sensor/hum_1"
#define temperature_topic "macbart/sensor/temp_1"

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
  pubSubClient.setServer(mqttServer, mqttPort);
  pubSubClient.setCallback(callback);
}

void reconnect() {
  while (!pubSubClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (pubSubClient.connect("ESP8266Client", mqttUser, mqttPassword )) {
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
      // Wait a few seconds between measurements.
      delay(1000);
      // Reading sensor
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
      return;
      }
      
       // Temperature
      Serial.print("Temperatuur:");
      Serial.println(String(t).c_str());
      Serial.println(temperature_topic);
      pubSubClient.publish(temperature_topic, String(t).c_str(), true);

      // Humidity
      Serial.print("Vochtigheid:");
      Serial.println(String(h).c_str());
      Serial.println(humidity_topic);
      pubSubClient.publish(humidity_topic, String(h).c_str(), true);
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}
