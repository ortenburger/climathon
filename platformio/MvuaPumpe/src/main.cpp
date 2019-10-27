#include <Arduino.h>
#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RL_PIN D1
#define MESH_PREFIX "MvuaNzuri"
#define MESH_PASSWORD "GutenRegen"
#define MESH_PORT 5555
#define MQTT_BROKER "10.216.101.100"
#define MQTT_BROKER_PORT 1883

painlessMesh  mesh;
WiFiClient espClient;
PubSubClient client(espClient);
Scheduler userScheduler;

void mqttCallback(char* topic, byte* payload, unsigned int length);

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
  pinMode(RL_PIN, OUTPUT);

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  client.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  client.setCallback(mqttCallback);
  delay(5000);
  if(client.connect("pump1")) {
    Serial.println("Connected");
    client.publish("/mvuanzuri/pump1", "connected");
    client.subscribe("/mvuanzuri/pump1/action");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String action="";
  for(uint i=0;i<length;i++) {
    action+=(char)payload[i];
  }
  Serial.println(action);
  if(action=="on") {
    digitalWrite(RL_PIN, HIGH);
  }else {
    digitalWrite(RL_PIN, LOW);  
  }
}

void loop() {
  mesh.update();
  if (!client.connected()) {
    if(client.connect("pump1")) {
      Serial.println("Connected");
      client.publish("/mvuanzuri/pump1", "connected");
      client.subscribe("/mvuanzuri/pump1/action");
    }
  }
  client.loop();
}