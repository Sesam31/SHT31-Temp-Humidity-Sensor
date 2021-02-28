
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

const char* ssid = "xxxxxx";
const char* password = "xxxxxx";
const char* mqtt_server = "xxxxxx";
const char* user = "xxxxxx";
const char* pass = "xxxxxx";

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), user, pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("sht31", "connected");
      // ... and resubscribe
      client.subscribe("sht31");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //  connection setup
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.begin(9600);

    while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
}

void loop() {
  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity(); 

   if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); 
    Serial.print(t); 
    Serial.print("\t\t");

    String t_mqtt = (String) t;
    char message_buff0[5];
    t_mqtt.toCharArray(message_buff0, t_mqtt.length()+1); 
    client.publish("sht31", message_buff0); 
      
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); 
    Serial.println(h);

    String h_mqtt = (String) h;
    char message_buff1[5];
    h_mqtt.toCharArray(message_buff1, h_mqtt.length()+1); 
    client.publish("sht31", message_buff1);
    
  } else { 
    Serial.println("Failed to read humidity");
  }

  delay(1000);

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (++loopCnt == 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
 
      //MQTT
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

}
