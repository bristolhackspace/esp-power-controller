# Proof of concept

Would use a database/spreadsheet for storing valid rfids and logging data.
Perhaps split into 2 mqtt clients:

* handles RFID requests subscribe to /arduino/+/rfid/unknown
* handles usage logging, subscribe to /arduino/+/power/on

# Requirements

use pip -r requirements

* paho-mqtt

## mosquitto 1.4.3

new version of mosquitto required for arduino pubsub client library.

first install:

* libc-ares-dev
* libc-ares2
* uuid-dev

then [download and compile mosquitto](http://mosquitto.org/download/)
