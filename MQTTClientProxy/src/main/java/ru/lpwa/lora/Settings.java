package ru.lpwa.lora;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

class Settings {
    static String BROKER_URL;
    static String BROKER_URL_LOCAL;
    static String TXTFILENAME = "/home/innolabs/data.txt";
    static String MQTT_RX_TOPIC = "";

    static boolean LOCALRUN;
    static boolean PUBLISHPUBLIC;
    static String PUBLICBROKER;

    static void populateProperties(String propertyFile) {
        FileInputStream fis;
        Properties property = new Properties();

        try {
            fis = new FileInputStream(propertyFile);
            property.load(fis);

            try {
                Settings.LOCALRUN = Boolean.parseBoolean(property.getProperty("localRun"));
            } catch (Exception e) {
                MQTTClientProxy.log.error("Property 'LocalRun' error. Set as normal 'false' value.");
                Settings.LOCALRUN = false;
            }

            Settings.TXTFILENAME = property.getProperty("txt.FileName");
            Settings.BROKER_URL = property.getProperty("mqqt.brokerURL");
            Settings.BROKER_URL_LOCAL = property.getProperty("mqtt.brokerURL_LOCAL");
            Settings.MQTT_RX_TOPIC = property.getProperty("mqtt.RXTOPIC");
            Settings.PUBLISHPUBLIC = Boolean.parseBoolean(property.getProperty("mqttPublishPublic"));
            Settings.PUBLICBROKER = property.getProperty("publicBroker");

        } catch (IOException e) {
            MQTTClientProxy.log.error("ERROR: Property file is missing! Use '-config <fileName>' to set file on runtime.");
            System.exit(1);
        }
    }
}
