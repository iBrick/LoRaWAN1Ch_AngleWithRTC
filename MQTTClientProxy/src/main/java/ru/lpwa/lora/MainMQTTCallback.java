package ru.lpwa.lora;

import org.eclipse.paho.client.mqttv3.*;
import java.time.ZonedDateTime;
import java.util.Iterator;

import static ru.lpwa.lora.MQTTClientProxy.publicBrokerClient;
import static ru.lpwa.lora.MQTTClientProxy.mqttClient;

class MainMQTTCallback implements MqttCallbackExtended {
    private final static int mqttQoS = 1;

    MainMQTTCallback() {
        super();
    }
    /* Methods to implement the MqttCallback interface              */
    /****************************************************************/

    @Override
    public void connectComplete(boolean reconnect, java.lang.String serverURI){
        MQTTClientProxy.log.warn("MainMqttCallback Connection reconnect = " + reconnect);
        if(reconnect) {
            try {
                mqttClient.subscribe(Settings.MQTT_RX_TOPIC, 1);
            } catch (MqttException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * @see MqttCallback#connectionLost(Throwable)
     */
    public void connectionLost(Throwable cause) {
        // code to reconnect to the broker would go here if desired

        MQTTClientProxy.log.warn("MainMqttCallback Connection lost!!" );
        mqttClient = MQTTClientProxy.connectMQTTClient(Settings.LOCALRUN ? Settings.BROKER_URL_LOCAL : Settings.BROKER_URL,
                "mqttProxyLocalClient", "", "", new MainMQTTCallback());
        connectComplete(true, "");
    }

    /**
     * @see MqttCallback#deliveryComplete(IMqttDeliveryToken)
     */
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Called when a message has been delivered to the
        // server. The token passed in here is the same one
        // that was passed to or returned from the original call to publish.
        // This allows applications to perform asynchronous
        // delivery without blocking until delivery completes.
    }

    /**
     * @see MqttCallback#messageArrived(String, MqttMessage)
     */
    public void messageArrived(String topic, MqttMessage message) {
        // Called when a message arrives from the server that matches any
        // subscription made by the client
        String time = ZonedDateTime.now().toString();

        if (topic.startsWith("gateway") //&& topic.endsWith("up")
        ) {
            if (!topic.endsWith("down")) {
                MQTTClientProxy.log.debug("Topic:\t {}, time: {}, payload: {}", topic, time, message.toString());
                publish2PublicBroker(topic, message);
            }
        }
    }

    private static void publish2PublicBroker(String topic, MqttMessage message) {
        if (Settings.PUBLISHPUBLIC) {
            message.setQos(mqttQoS);

            try {
                if (MQTTClientProxy.publicBrokerClient == null || !MQTTClientProxy.publicBrokerClient.isConnected()) {
                    MQTTClientProxy.publicBrokerClient = MQTTClientProxy.connectMQTTClient(Settings.PUBLICBROKER,
                            "mqttProxyPublicClient", "", "", new SimpleMqttCallback());
                }
                if (publicBrokerClient.isConnected()) {
                    Iterator it = MQTTClientProxy.arrayList.iterator();
                    while (it.hasNext())
                    {
                        MqttMessage message1 = (MqttMessage) it.next();
                        MQTTClientProxy.log.debug("Sending messages from queue");
                        publicBrokerClient.publish(MQTTClientProxy.topic, message1.getPayload(),
                                mqttQoS, false); // Blocking publish
                        it.remove();
                    }

                    publicBrokerClient.publish(topic, message.getPayload(), mqttQoS, false); // Blocking publish
                } else {
                    if (topic.endsWith("up"))
                        MQTTClientProxy.topic = topic;
                        MQTTClientProxy.arrayList.add(message);
                }

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}