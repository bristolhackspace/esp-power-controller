# Requirements

* ESP8266 with enough pins for SPI - I used ESP8266-12
* ESP8266 [board environment](https://github.com/esp8266/Arduino) for Arduino IDE
* [ESP8266 fork of MQTT client library](https://github.com/Imroy/pubsubclient)

# Wiring

## SPI RFID reader

[spi details](http://d.av.id.au/blog/esp8266-hardware-spi-hspi-general-info-and-pinout/)

* SPI SS_PIN 15
* SPI MOSI 13
* SPI MISO 12
* SPI CLK 14
* RST_PIN 5
* LED 4

