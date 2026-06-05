#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "5B1390"; //5B1396
const char* WIFI_PASSWORD = "L2ULAEG329035";

const char* MQTT_BROKER = "192.168.0.8";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastPublishTime = 0;

// put function declarations here:
void connectToWiFi();
void connectToMQTT();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("ESP32-CAM MQTT Telemetry Test");

  connectToWiFi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  connectToMQTT();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    connectToMQTT();
  }

  mqttClient.loop();

  unsigned long now = millis();

  if (now - lastPublishTime >= 3000)
  {
    lastPublishTime = now;

    String payload = "{";
    payload += "\"device\":\"esp32cam\",";
    payload += "\"uptime_ms\":";
    payload += now;
    payload += ",";
    payload += "\"wifi_rssi\":";
    payload += WiFi.RSSI();
    payload += "}";

    mqttClient.publish("telemetry/esp32cam", payload.c_str());

    Serial.print("Published: ");
    Serial.println(payload);
  }
}



// put function definitions here:
void connectToWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker... ");

    if (mqttClient.connect("esp32cam-client"))
    {
      Serial.println("connected!");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 2 seconds");
      delay(2000);
    }
  }
}