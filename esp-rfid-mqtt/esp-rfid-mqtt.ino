/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED 4
#define SS_PIN 15
#define RST_PIN 5

const char *ssid =	"flat5";		// cannot be longer than 32 characters!
const char *pass =	"matt&inma";		//

// Update these with values suitable for your network.
IPAddress server(192,168,0,9);

void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print("=");
  Serial.println(pub.payload_string());
  if(pub.topic() == "/arduino/power")
  {
    if(pub.payload_string() == "True")
      digitalWrite(LED,HIGH);
    else
      digitalWrite(LED,LOW);
  }
}

WiFiClient wclient;
PubSubClient client(wclient, server);

void setup() {
  // Setup console
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  Serial.begin(115200);
  setup_rfid();
  delay(10);
  Serial.println();
  Serial.println();

  client.set_callback(callback);
  digitalWrite(LED,LOW);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
      //not connected - then connect & subscribe
      if(!client.connected())
      {
          if (client.connect("arduinoClient"))
          {
            Serial.println("connected");
            client.subscribe("/arduino/power");
          }
      }
      else
      {
        String rfid = read_rfid();
        if(rfid != "")
            client.publish("/arduino/rfid",rfid);
        client.publish("/arduino/uptime",String(millis()));
        client.loop();
      }
  }
  delay(1000);
}
