# Proof of concept

Would use a database/spreadsheet for storing valid rfids and logging data.
Perhaps split into 2 mqtt clients:

* handles RFID requests subscribe to /arduino/+/rfid/unknown
* handles usage logging, subscribe to /arduino/+/power/on

# Requirements

* paho.mqtt.client 

