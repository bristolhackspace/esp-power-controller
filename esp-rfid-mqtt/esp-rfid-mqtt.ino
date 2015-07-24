/*
uses mqtt pubsub library from https://github.com/Imroy/pubsubclient
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED 4
#define SS_PIN 15
#define RST_PIN 5

//FSM states
#define S_START 0
#define S_START_WIFI 1
#define S_START_MQTT 2
#define S_READ_RFID 3
#define S_VALID_RFID 4
#define S_UNKNOWN_RFID 5
#define S_VALID_REMOVED 6
#define S_CHECK_CONNECTED 7

int fsm_state_user = S_START;

const char *ssid =	"flat5";		// cannot be longer than 32 characters!
const char *pass =	"matt&inma";		//

// Update these with values suitable for your network.
IPAddress server(192,168,0,9);
String current_rfid = "";

#define rfid_cache_size 10
int cache_index = 0;
String valid_rfids[rfid_cache_size];

WiFiClient wclient;
PubSubClient client(wclient, server);

void callback(const MQTT::Publish& pub) 
{
    Serial.print(pub.topic());
    Serial.print("=");
    Serial.println(pub.payload_string());
    if(pub.topic() == "arduino/rfid/valid")
    {
        valid_rfids[cache_index++] = pub.payload_string();
        if(cache_index > rfid_cache_size)
            cache_index = 0;
    }
}

void setup() 
{
    // Setup console
    pinMode(LED,OUTPUT);
    digitalWrite(LED,HIGH);
    Serial.begin(115200);
    Serial.println("");
    Serial.println("");
    setup_rfid();
    for(int i = 0; i < rfid_cache_size; i ++)
        valid_rfids[i] = "";

    client.set_callback(callback);
    digitalWrite(LED,LOW);
}

void loop()
{
    switch(fsm_state_user)
    {
        case S_START:
            fsm_state_user = S_START_WIFI;
            break;
        
        case S_START_WIFI:
            Serial.print("Connecting to ");
            Serial.print(ssid);
            Serial.println("...");
            WiFi.begin(ssid, pass);
            if (WiFi.status() != WL_CONNECTED) 
            {
                if (WiFi.waitForConnectResult() != WL_CONNECTED)
                    break;
            }
            Serial.println("WiFi connected");
            fsm_state_user = S_START_MQTT;
            break;

        case S_START_MQTT:
            //connect to mqtt broker with id = arduinoClient
            if(client.connect("arduinoClient"))
            {
                Serial.println("mqtt connected");
                client.subscribe("arduino/rfid/valid");
                fsm_state_user = S_READ_RFID;
            }
            else
            {
                Serial.println("mqtt not connected");
                fsm_state_user = S_START_WIFI;
            }
            break;

        case S_READ_RFID:
            current_rfid = read_rfid();
            //rfid is present
            if(current_rfid != "")
            {
                if(valid_rfid(current_rfid))
                    fsm_state_user = S_VALID_RFID;
                else
                    fsm_state_user = S_UNKNOWN_RFID;
            }
            //no rfid present
            else
                fsm_state_user = S_CHECK_CONNECTED;
            break;

        case S_CHECK_CONNECTED:
            if(client.connected())
                fsm_state_user = S_READ_RFID;
            else
                fsm_state_user = S_START_MQTT;
            break;

        case S_VALID_RFID:
            Serial.println("valid rfid present");
            digitalWrite(LED, HIGH);
            client.publish("arduino/power","on");
            if(read_rfid() != current_rfid)
                fsm_state_user = S_VALID_REMOVED;
            break;

        case S_UNKNOWN_RFID:
            Serial.println("invalid rfid present");
            client.publish("arduino/rfid/unknown", current_rfid);
            fsm_state_user = S_READ_RFID;
            break;

        case S_VALID_REMOVED:
            Serial.println("valid rfid removed");
            fsm_state_user = S_READ_RFID;
            digitalWrite(LED, LOW);
            break;
            
    }
    client.publish("arduino/uptime",String(millis()));
    Serial.print("fsm state=");
    Serial.println(fsm_state_user);
    client.loop();
    delay(1000);
}

//look through local cache of valid rfids
//if not present, publish unknown rfid message
boolean valid_rfid(String rfid)
{
    for(int i = 0; i < rfid_cache_size; i ++)
    {
        if(valid_rfids[i] == rfid)
            return true;
    }
    return false;
}
