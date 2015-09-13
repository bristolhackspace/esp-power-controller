/*
uses mqtt pubsub library from https://github.com/Imroy/pubsubclient
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED 4
#define SS_PIN 15
#define RST_PIN 5

#define TOOL_ID 2

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

const char *ssid =	"hackspace";		// cannot be longer than 32 characters!
const char *pass =	"Altman37";		//

// Update these with values suitable for your network.
//IPAddress server(10,255,157,76);
IPAddress server(192,168,0,116);
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
    if(pub.topic() == "tools/2/rfid/valid") //TODO, fix hardcoded tool id
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
            WiFi.begin(ssid, pass);
            while(WiFi.status() != WL_CONNECTED) 
            {
                Serial.print(".");
                delay(500);
            }
            Serial.println("WiFi connected");
            Serial.println(WiFi.localIP());
            fsm_state_user = S_START_MQTT;
            break;

        case S_START_MQTT:
            //connect to mqtt broker with id = 2
            if(client.connect("2"))
            {
                Serial.println("mqtt connected");
                client.subscribe("tools/2/rfid/valid");
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
            client.publish("tools/2/power","on");
            if(read_rfid() != current_rfid)
                fsm_state_user = S_VALID_REMOVED;
            break;

        case S_UNKNOWN_RFID:
            Serial.println("invalid rfid present");
            client.publish("tools/2/rfid/unknown", current_rfid);
            fsm_state_user = S_READ_RFID;
            break;

        case S_VALID_REMOVED:
            Serial.println("valid rfid removed");
            client.publish("tools/2/power","off");
            fsm_state_user = S_READ_RFID;
            digitalWrite(LED, LOW);
            break;
            
    }
    client.publish("tools/2/uptime",String(millis()));
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
