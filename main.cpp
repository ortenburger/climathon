#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <BH1750.h>

#define SENSOR_PACK_ID (4711)

#define SEALEVELPRESSURE_HPA (1013.25)

#define PIN_SCL (D1)
#define PIN_SDA (D2)
#define JSON_DOC_STATIC_SIZE (200)
#define SERIAL_BAUD_RATE (9600)

/* sensors */
Adafruit_BME280 bme280;
BH1750 bh1750( 0x23 );

StaticJsonDocument<JSON_DOC_STATIC_SIZE> doc;

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
}

void loop() {
  doc.clear();
  doc["id"]          = SENSOR_PACK_ID;
  doc["temperature"] = bme280.readTemperature();
  doc["humidity"]    = bme280.readHumidity();
  doc["pressure"]    = bme280.readPressure();
  doc["altitude"]    = bme280.readAltitude(SEALEVELPRESSURE_HPA);
  doc["lightlevel"]  = bh1750.readLightLevel(true);
  serializeJsonPretty(doc, Serial);
  delay(1000);
}
