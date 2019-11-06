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
                    "mqttProxyLocalClient", "", "", new MainMQTTCallback());
            publicBrokerClient = connectMQTTClient(Settings.PUBLICBROKER,
                    "mqttProxyPublicClient", "", "", new SimpleMqttCallback());

            int subQoS = 2;
            mqttClient.subscribe(Settings.MQTT_RX_TOPIC, subQoS);

            try {
                while (true) {
                    Thread.sleep(5000000);
                    //do forever
                }
            } catch(Exception e) {
                e.printStackTrace();
                mqttClient.disconnect();
                if (Settings.PUBLISHPUBLIC) {
                    publicBrokerClient.disconnect();
                }
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

    static MqttClient connectMQTTClient(String brokerURL, String clientId, String username,
                                        String password, MqttCallback mqttCallback) {
        MqttClient mqttClientInt = null;
        try {
            MemoryPersistence persistence = new MemoryPersistence();

            clientId = MqttClient.generateClientId();
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