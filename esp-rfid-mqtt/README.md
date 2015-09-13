# Requirements

* ESP8266 with enough pins for SPI - I used ESP8266-12
* ESP8266 [board environment](https://github.com/esp8266/Arduino) for Arduino IDE
* [ESP8266 fork of MQTT client library](https://github.com/Imroy/pubsubclient)
* [ESP spi pins](http://d.av.id.au/blog/esp8266-hardware-spi-hspi-general-info-and-pinout/)
* [MFRC522 RFID library & pin connections](https://github.com/miguelbalboa/rfid)

# Wiring

## SPI RFID reader


* SPI SS_PIN 15 -> SDA
* SPI MOSI 13   -> MOSI
* SPI MISO 12   -> MISO
* SPI CLK 14    -> SCK
* RST_PIN 5     -> RST
* LED 4

