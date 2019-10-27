#include <Arduino.h>
#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <Wire.h>
// #include <SPI.h>

#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SENSOR_PACK_ID (4711)

#define SEALEVELPRESSURE_HPA (1013.25)

#define PIN_SCL (D1)
#define PIN_SDA (D2)
#define JSON_DOC_STATIC_SIZE (200)
#define SERIAL_BAUD_RATE (9600)

/* networking */
#define MESH_PREFIX "MvuaNzuri"
#define MESH_PASSWORD "GutenRegen"
#define MESH_PORT 5555
#define MQTT_BROKER "10.216.101.100"
#define MQTT_BROKER_PORT 1883

Scheduler userScheduler;
painlessMesh  mesh;
WiFiClient espClient;
PubSubClient client(espClient);

/* sensors */
Adafruit_BME280 bme280;
BH1750 bh1750( 0x23 );

StaticJsonDocument<JSON_DOC_STATIC_SIZE> doc;

void readSensors();

Task taskReadSensors( TASK_MILLISECOND * 3000 , TASK_FOREVER, &readSensors );

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
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);

  Wire.begin( PIN_SDA, PIN_SCL );
  if( !bme280.begin() ) {
    Serial.println("BME280 is not working properly.");
    exit(1);
  }

  if( !bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE) ) {
    Serial.println("BH1750 is not working properly.");
    exit(1);
  }

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.setRoot(false);
  mesh.setContainsRoot(true);

  userScheduler.addTask( taskReadSensors );
  taskReadSensors.enable();
  client.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  delay(500);
  if(client.connect("sensor7")) {
    Serial.println("Connected");
    client.publish("/mvuanzuri/sensor7", "connected");
  }
}


void readSensors() {
  float temperature = bme280.readTemperature();
  float humidity    = bme280.readHumidity();
  float pressure    = bme280.readPressure();
  float altitude    = bme280.readAltitude(SEALEVELPRESSURE_HPA);
  float lightlevel  = bh1750.readLightLevel(true);

  if (!client.connected()) {
    if(client.connect("sensor7")) {
      Serial.println("Connected");
      client.publish("/mvuanzuri/sensor7", "connected");
    }
  }

  client.loop();

  if(client.connected()) {
    StaticJsonDocument<256> doc;
    String json="";
    doc["temperature"] = temperature;
    doc["humidity"]    = humidity;
    doc["pressure"]    = pressure;
    doc["altitude"]    = altitude;
    doc["lightlevel"]  = lightlevel;
    serializeJson(doc, json);
    client.publish("/mvuanzuri/sensor7/values", json.c_str());
  }
}


void loop() {
  mesh.update();
}