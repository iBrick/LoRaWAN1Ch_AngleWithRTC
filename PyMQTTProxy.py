from time import sleep

import paho.mqtt.client as mqtt


SUB_TOPICS = ("gateway/#")
RECONNECT_DELAY_SECS = 2


def on_connect(client, userdata, flags, rc):
    print "Connected with result code %s" % rc
    for topic in SUB_TOPICS:
        client.subscribe(topic, qos=2)

def on_connectPublic(client, userdata, flags, rc):
    print "Connected with result code %s" % rc
    
def on_disconnect(client, userdata, rc):
    print("disconnected with rtn code [%d]"% (rc) )

def on_publish(client, userdata, msgID):
    print("Published with MsgID [%d]"% (msgID) )

# EDIT: I've removed this function because the library handles
#       reconnection on its own anyway.
# def on_disconnect(client, userdata, rc):
#     print "Disconnected from MQTT server with code: %s" % rc
#     while rc != 0:
#         sleep(RECONNECT_DELAY_SECS)
#         print "Reconnecting..."
#         rc = client.reconnect()


def on_msg(client, userdata, msg):
    print "%s %s" % (msg.topic, msg.payload)
    rc=clientPublic.publish(msg.topic, msg.payload, qos=2, retain=True)
    print "Return="+str(rc)


if __name__ == "__main__":
  
    client = mqtt.Client(client_id="mqttClientPy", clean_session=False)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_publish = on_publish
    client.on_message = on_msg
    
    client.connect("127.0.0.1", 1883, 60)
    client.loop_start()
    
    clientPublic = mqtt.Client(client_id="mqttClientProxy", clean_session=False)
    clientPublic.on_connect = on_connectPublic
    clientPublic.on_disconnect = on_disconnect
    clientPublic.on_publish = on_publish
    clientPublic.connect("91.77.164.13", 1883, 60)
    clientPublic.loop_start()

    try:
        while True:
            sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        client.loop_stop()
