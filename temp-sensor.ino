/**
 * Secure Write Example code for InfluxDBClient library for Arduino
 * Enter WiFi and InfluxDB parameters below
 *
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - unsecured http://...
 *  - secure https://... (appropriate certificate is required)
 *  - InfluxDB 2 Cloud at https://cloud2.influxdata.com/ (certificate is preconfigured)
 * Measures signal level of the actually connected WiFi network
 * This example demonstrates time handling, secure connection and measurement writing into InfluxDB
 * Data can be immediately seen in a InfluxDB 2 Cloud UI - measurement wifi_status
 **/

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>


#include "wifi.h"
#include "influxdb.h"

// WiFi AP SSID
#define WIFI_SSID "PTL-N"
// WiFi password
#define WIFI_PASSWORD "P0stT3n3br4sL4b"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "http://epic-potato.lan.posttenebraslab.ch:8086"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "A4O2gx1lMVL5qgw8YzlLsWc3ZFDoQAjzy8O6pTkGyCoQ1yEPzH0KmkRWf4ebkIl2DvUmv85uXD38JAGcDnZoIw=="
// InfluxDB v2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "ptl"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_BUCKET "sensor1"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// InfluxDB client instance without preconfigured InfluxCloud certificate for insecure connection 
// InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Data point
Point sensor("sensor");

// Sensor config
#include <dhtnew.h>
DHTNEW dht(D7);

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Alternatively, set insecure connection to skip server certificate validation 
  //client.setInsecure();

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  while(true) {
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
      break;
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
      delay(2000);
    }
  }
}

void loop() {
  // Store measured value into point
  sensor.clearFields();
  // Read data from the sensor
  int ret = dht.read();
  if (ret != DHTLIB_OK) {
    delay(10000);
    return;
  }
  // Report temperature and humidity
  sensor.addField("temperature", dht.getTemperature());
  sensor.addField("humidity", dht.getHumidity());
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));
  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }
  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Wait 10s
  Serial.println("Wait 10s");

  delay(10000);
}