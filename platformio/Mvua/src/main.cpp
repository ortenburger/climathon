#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHTPIN D2
#define DHTTYPE DHT11
#define A0_PIN A0
#define GAS_PIN D3
#define RL_PIN D4
#define BTN_PIN D5
#define MESH_PREFIX "MvuaNzuri"
#define MESH_PASSWORD "GutenRegen"
#define MESH_PORT 5555
#define MQTT_BROKER "10.216.101.100"
#define MQTT_BROKER_PORT 1883

Scheduler userScheduler;
painlessMesh  mesh;
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void readSensors();
void readButton();

Task taskReadSensors( TASK_MILLISECOND * 3000 , TASK_FOREVER, &readSensors );
Task taskReadButton( TASK_MILLISECOND * 50 , TASK_FOREVER, &readButton );

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(RL_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskReadSensors );
  taskReadSensors.enable();
  client.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  delay(5000);
  if(client.connect("sensor1")) {
    Serial.println("Connected");
    client.publish("/mvuanzuri", "connected");
  }
  userScheduler.addTask( taskReadButton );
  taskReadButton.enable();
}

void readSensors() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int moisture = analogRead(A0_PIN);
  int gas = digitalRead(GAS_PIN);
  // Serial.print("Gas: ");
  // Serial.println(gas);
  // Serial.print("Humidity: ");
  // Serial.println(humidity);
  // Serial.print("Temperature: ");
  // Serial.println(temperature);
  // Serial.print("Moisture: ");
  // Serial.println(moisture);
  if (!client.connected()) {
    if(client.connect("sensor1")) {
      Serial.println("Connected");
      client.publish("/mvuanzuri", "connected");
    }
  }
  client.loop();
  if(client.connected()) {
    StaticJsonDocument<256> doc;
    String json="";
    doc["gas"]=gas;
    doc["humidity"]=humidity;
    doc["temperature"]=temperature;
    doc["moisture"]=moisture;
    serializeJson(doc, json);
    client.publish("/mvuanzuri", json.c_str());
  }
}

void readButton() {
  int btn=digitalRead(BTN_PIN);
  if(btn==LOW) {
    if (!client.connected()) {
      if(client.connect("sensor1")) {
        Serial.println("Connected");
        client.publish("/mvuanzuri", "connected");
      }
    }
    client.loop();
    client.publish("/mvuanzuri", "test1");
  }
}

void loop() {
  mesh.update();
}