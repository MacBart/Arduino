// Include libraries
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

// Define variables
#define DHTPIN 4
#define DHTTYPE DHT11
#define wifi_ssid "MacBart_2.4"
#define wifi_password "Siepie1962!!!"
#define mqtt_server "test.mosquitto.org"
#define mqtt_port 8883
#define mqtt_user "macbart"
#define mqtt_password "Siepie1962!"
#define fingerprint "28:70:6C:B8:7E:C1:94:5F:3D:EF:D6:6C:0A:F4:45:55:61:A0:C1:50"
#define humidity_topic "macbart/sensor/hum_1"
#define temperature_topic "macbart/sensor/temp_1"
DHT dht(DHTPIN, DHTTYPE);
//WiFiClient espClient;
WiFiClientSecure secureClient;
PubSubClient pubSubClient(secureClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  pubSubClient.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!secureClient.connect(mqtt_server, mqtt_port)) {
    Serial.println("connection failed");
    return;
  }

  if (secureClient.verify(fingerprint, mqtt_server)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!pubSubClient.connected()) {
      Serial.print("Attempting MQTT connection...");
    
      // Generate pubSubClient name based on MAC address and last 8 bits of microsecond counter
      String clientName;  
      clientName += "macbart-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);
      Serial.println(mqtt_user);
      Serial.println(mqtt_password);

    // Attempt to connect
    if (pubSubClient.connect((char*) clientName.c_str()), mqtt_user, mqtt_password) {
      Serial.println("connected:");
      Serial.println(pubSubClient.connected());
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
