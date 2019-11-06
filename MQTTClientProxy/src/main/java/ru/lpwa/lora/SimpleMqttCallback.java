package ru.lpwa.lora;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import static ru.lpwa.lora.MQTTClientProxy.mqttClient;

public class SimpleMqttCallback implements MqttCallback {
    @Override
    public void messageArrived(String topic, MqttMessage mqttMessage) {
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {

    }

    /**
     * @see MqttCallback#connectionLost(Throwable)
     */
    public void connectionLost(Throwable cause) {
        // code to reconnect to the broker would go here if desired
        MQTTClientProxy.log.warn("Public MqttCallback Connection lost!!" );
        mqttClient = MQTTClientProxy.connectMQTTClient(Settings.LOCALRUN ? Settings.BROKER_URL_LOCAL : Settings.BROKER_URL,
                "mqttProxyPublicClient", Settings.MQTT_USERNAME, Settings.MQTT_PASSWORD_MD5, new MainMQTTCallback());
    }
}
