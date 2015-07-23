import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/arduino/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):

    if msg.topic == '/arduino/uptime':
        uptime = int(msg.payload)
        print("uptime = %d" % uptime)
    elif msg.topic == '/arduino/rfid':
        rfid = msg.payload
        print("got rfid %s" % rfid)
        if rfid == "cb5b3f5b":
            print("sending power = on")
            r = client.publish("/arduino/power", True)
        else:
            print("sending power = off")
            r = client.publish("/arduino/power", False)
    else:
        print(msg.topic+" "+str(msg.payload))
        

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
try:
    client.loop_forever()
except KeyboardInterrupt as e:
    print("ending")
