package ru.lpwa.lora;

import org.apache.logging.log4j.LogManager;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import org.apache.logging.log4j.Logger;

import java.util.ArrayList;

public class MQTTClientProxy {
    static Logger log = LogManager.getLogger(MQTTClientProxy.class.getName());

    static ArrayList<MqttMessage> arrayList = new ArrayList<>();
    static String topic ="";

    static MqttClient mqttClient;
    static MqttClient publicBrokerClient;
    private static String propertyFile = "src/main/resources/settings.properties";

    public static void main(String[] args){

        if (args.length!=1) {
            CliArgs cliArgs = new CliArgs(args);
            boolean config = cliArgs.switchPresent("-config");
            if (config) {
                propertyFile = cliArgs.switchValue("-config");
            } else {
                log.warn("Option '-config <fileName>' not used.");
            }
        }

        Settings.populateProperties(propertyFile);

        log.info(Settings.MQTT_RX_TOPIC);

        try {
            mqttClient = connectMQTTClient(Settings.LOCALRUN ? Settings.BROKER_URL_LOCAL : Settings.BROKER_URL,
                    "mqttProxyLocalClient", Settings.MQTT_USERNAME, Settings.MQTT_PASSWORD_MD5, new MainMQTTCallback());
            publicBrokerClient = MQTTClientProxy.connectMQTTClient(Settings.PUBLICBROKER,
                    "mqttProxyPublicClient", "", "", new SimpleMqttCallback());

            // Subscribe to the requested topic
            // The QoS specified is the maximum level that messages will be sent to the mqttClient at.
            // For instance if QoS 1 is specified, any messages originally published at QoS 2 will
            // be downgraded to 1 when delivering to the mqttClient but messages published at 1 and 0
            // will be received at the same level they were published at.
            int subQoS = 2;

            mqttClient.subscribe(Settings.MQTT_RX_TOPIC, subQoS);

            try {
                while (true) {
                    Thread.sleep(5000000);
                    //do forever
                }
            } catch(Exception e) {
                mqttClient.disconnect();
                if (Settings.PUBLISHPUBLIC) {
                    publicBrokerClient.disconnect();
                }
                e.printStackTrace();
            }
            log.info("Disconnected");
            System.exit(0);
        } catch(MqttException me){
            log.error("reason " + me.getReasonCode());
            log.error("msg " + me.getMessage());
            log.error("loc " + me.getLocalizedMessage());
            log.error("cause " + me.getCause());
            log.error("except " + me);
            me.printStackTrace();
        }
    }

    static MqttClient connectMQTTClient(String brokerURL, String clientId, String username, String password, MqttCallback mqttCallback) {
        MqttClient mqttClientInt = null;
        try {
            MemoryPersistence persistence = new MemoryPersistence();

            mqttClientInt = new MqttClient(brokerURL, clientId, persistence);
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(false);
            connOpts.setAutomaticReconnect(true);
            if (username!=null && !username.equals("")) {
                connOpts.setUserName(username);
            }
            if (password!=null && !password.equals("")) {
                connOpts.setPassword(password.toCharArray());
            }
            mqttClientInt.setCallback(mqttCallback);
            log.info("Connecting to broker: {}", brokerURL);
            mqttClientInt.connect(connOpts);

        } catch(MqttException me){
            log.error("reason " + me.getReasonCode());
            log.error("msg " + me.getMessage());
            log.error("loc " + me.getLocalizedMessage());
            log.error("cause " + me.getCause());
            log.error("except " + me);
            me.printStackTrace();
        }
        return mqttClientInt;
    }

}