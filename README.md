# Temperature Sensor
Arduino code running on an ESP8266 that send temperature and humidity to an InfluxDB server.

## Configuration

Copy the `wifi.h.template` into `wifi.h` and modify the values inside to connect to the WiFi access point.

Copy the `influxdb.h.template` into `influxdb.h` and modify the values to connect to the influxdb server. It is currently using the server from @maximeborges, and a generic user. Send him a message to get the login/token.

## Build steps

### With `Arduino IDE`

Install the following libraries:

    ESP8266 Influxdb
    DHTNEW

Click on the `Upload` button and you're done!

### With `arduino-cli`
Install libraries, compile and upload:

    arduino-cli lib install "ESP8266 Influxdb"
    arduino-cli lib install "DHTNEW"
    arduino-cli compile --fqbn esp8266:esp8266:d1_mini
    arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:d1_mini

