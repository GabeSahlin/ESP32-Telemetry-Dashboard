#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "esp_camera.h"

const char *WIFI_SSID = "5B1390"; // 5B1396
const char *WIFI_PASSWORD = "L2ULAEG329035";
const char *MQTT_BROKER = "192.168.0.8";
const int MQTT_PORT = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastPublishTime = 0;
uint32_t publishCount = 0;
uint32_t mqttReconnects = 0;
uint32_t frameCount = 0;
bool cameraActive = false;
unsigned long lastCaptureTimeMs = 0;
size_t lastImageSize = 0;

// put function declarations here:
void connectToWiFi();
void connectToMQTT();
void initializeCamera();
void captureCameraTelemetry();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("ESP32-CAM MQTT Telemetry Test");

  connectToWiFi();

  initializeCamera();
  mqttClient.setBufferSize(1024);

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

    captureCameraTelemetry();

    publishCount++;

    String payload = "{";
    payload += "\"device\":\"esp32cam\",";
    payload += "\"uptime_ms\":";
    payload += now;
    payload += ",";
    payload += "\"wifi_rssi\":";
    payload += WiFi.RSSI();
    payload += ",";
    payload += "\"free_heap\":";
    payload += ESP.getFreeHeap();
    payload += ",";
    payload += "\"min_heap\":";
    payload += ESP.getMinFreeHeap();
    payload += ",";
    payload += "\"cpu_freq\":";
    payload += ESP.getCpuFreqMHz();
    payload += ",";
    payload += "\"wifi_channel\":";
    payload += WiFi.channel();
    payload += ",";
    payload += "\"mac\":\"";
    payload += WiFi.macAddress();
    payload += "\",";
    payload += "\"ip\":\"";
    payload += WiFi.localIP().toString();
    payload += "\",";
    payload += "\"camera_active\":";
    payload += cameraActive ? "true" : "false";
    payload += ",";
    payload += "\"frames_captured\":";
    payload += frameCount;
    payload += ",";
    payload += "\"capture_time_ms\":";
    payload += lastCaptureTimeMs;
    payload += ",";
    payload += "\"image_size\":";
    payload += lastImageSize;
    payload += ",";
    payload += "\"messages_sent\":";
    payload += publishCount;
    payload += ",";
    payload += "\"mqtt_reconnects\":";
    payload += mqttReconnects;
    payload += ",";
    payload += "\"last_publish\":";
    payload += now;
    payload += "}";

    Serial.print("Payload Size: ");
    Serial.println(payload.length());
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
      mqttReconnects++;
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

void initializeCamera()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sccb_sda = 26;
  config.pin_sccb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);

  if (err == ESP_OK)
  {
    cameraActive = true;
    Serial.println("Camera initialized successfully");
  }
  else
  {
    cameraActive = false;
    Serial.print("Camera init failed: ");
    Serial.println(err);
  }
}

void captureCameraTelemetry()
{
  if (!cameraActive)
  {
    return;
  }

  unsigned long start = millis();

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  lastCaptureTimeMs = millis() - start;
  lastImageSize = fb->len;
  frameCount++;

  esp_camera_fb_return(fb);
}