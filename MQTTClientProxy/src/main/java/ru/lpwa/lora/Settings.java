package ru.lpwa.lora;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

class Settings {
    static boolean IS_DB_POSTGRES_USED = false;
    static String BROKER_URL;
    static String BROKER_URL_LOCAL;
    static String TXTFILENAME = "/home/innolabs/data.txt";
    static String TXTFILENAME_LOCAL = "data.txt";
    static String MQTT_RX_TOPIC = "";
    static String MQTT_USERNAME = "loraroot";
    static String MQTT_PASSWORD_MD5 = "innolabs";
    static String CLIENTID     = "innolabslocal";
    static boolean txtOverrideTrackOnStartup;

    static boolean LOCALRUN;
    static boolean PUBLISHPUBLIC;
    static String PUBLICBROKER;
    static String PUBLICMQTTTOPICPREFIX;

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
            Settings.TXTFILENAME_LOCAL = property.getProperty("txt.FileName_LOCAL");
            Settings.BROKER_URL = property.getProperty("mqqt.brokerURL");
            Settings.BROKER_URL_LOCAL = property.getProperty("mqtt.brokerURL_LOCAL");
            Settings.MQTT_USERNAME = property.getProperty("mqtt.user");
            Settings.MQTT_PASSWORD_MD5 = property.getProperty("mqtt.pass");
            Settings.MQTT_RX_TOPIC = property.getProperty("mqtt.RXTOPIC");
            Settings.CLIENTID = property.getProperty("mqtt.CLIENTID");
            Settings.PUBLISHPUBLIC = Boolean.parseBoolean(property.getProperty("mqttPublishPublic"));
            Settings.PUBLICBROKER = property.getProperty("publicBroker");
            Settings.PUBLICMQTTTOPICPREFIX = property.getProperty("publicMQTTTopicPrefix");
            try {
                Settings.txtOverrideTrackOnStartup = Boolean.parseBoolean(property.getProperty("txt.OverrideTrackOnStartup"));
            } catch (Exception e) {
                MQTTClientProxy.log.error("Property 'txt.OverrideTrackOnStartup' error. Set as normal 'true' value.");
                Settings.txtOverrideTrackOnStartup = true;
            }

        } catch (IOException e) {
            MQTTClientProxy.log.error("ERROR: Property file is missing! Use '-config <fileName>' to set file on runtime.");
            System.exit(1);
        }
    }
}
