import paho.mqtt.client as mqtt
import logging
import argparse
import os
import json
from logging.handlers import RotatingFileHandler

log = logging.getLogger('')

def get_install_dir():
    return os.path.dirname(os.path.realpath(__file__))

def get_post_prog():
    return get_install_dir() + "/post.py"

def get_users_file():
    return get_install_dir() + "/users.json"

def get_tools_file():
    return get_install_dir() + "/tools.json"

def get_tools():
    with open(get_tools_file()) as fh:
        all_tools = json.load(fh)
        return all_tools

def get_tool_by_id(tool_id):
    tools = get_tools()
    for tool in tools:
        if tool['id'] == tool_id:
            return tool
        

def get_user(rfid):
    with open(get_users_file()) as fh:
        all_users = json.load(fh)

    for user in all_users:
        if user['rfid'] == rfid:
            log.debug("found user %s for rfid [%s]" % (user['name'], rfid))
            return user['name'], user['tools']

    raise KeyError("no user")


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    log.info("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("tools/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):

    if msg.topic.endswith == 'tools/*/uptime':
        uptime = int(msg.payload)
        log.info("uptime = %d" % uptime)
    elif msg.topic.endswith("/rfid/unknown"):
        rfid = msg.payload
        tool_id = msg.topic.split('/')[1]
        #import ipdb; ipdb.set_trace()
        tool_info = get_tool_by_id(tool_id)
        log.info("%s got unknown rfid %s" % (tool_info['name'],rfid))
        try:
            user, users_tools = get_user(rfid)
            log.info("valid user %s" % user)
            
            if tool_id in users_tools.split(','):
                log.info("user %s has access to %s" % (user, tool_info['name']))
                r = client.publish("tools/%s/rfid/valid" % tool_id, rfid)
        except KeyError:
            log.info("invalid")
    else:
        log.warning(msg.topic+" "+str(msg.payload))
        


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="mqtt rfid logger")
    """
    parser.add_argument('--check-user', action='store_const', const=True,
        help="check an rfid")
    parser.add_argument('--list-tools', action='store_const', const=True,
        help="list tools for arduino to read")
    parser.add_argument('--rfid', action='store',
        help="rfid")
    parser.add_argument('--time', action='store',
        help="specify time to log")
    parser.add_argument('--log-tool', action='store',
        help="log a tool usage")
    """

    # setup logging
    log.setLevel(logging.DEBUG)
    log_format = logging.Formatter("%(asctime)s - %(levelname)-8s - %(message)s")

    ch = logging.StreamHandler()
    ch.setFormatter(log_format)
    log.addHandler(ch)

    fh = logging.handlers.RotatingFileHandler(get_install_dir() + '/query.log', maxBytes=(1024*1024), backupCount=7)
    fh.setFormatter(log_format)
    log.addHandler(fh)

    args = parser.parse_args()

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
        log.info("ending")
