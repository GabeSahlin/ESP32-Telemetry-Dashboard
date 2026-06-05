from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import paho.mqtt.client as mqtt
import json
import time
from threading import Lock

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

latest_telemetry = {
    "device": None,
    "uptime_ms": None,
    "wifi_rssi": None,
    "last_seen": None,
    "connected": False,
}

lock = Lock()

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "telemetry/esp32cam"


def on_connect(client, userdata, flags, reason_code, properties=None):
    print("Connected to MQTT broker")
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    global latest_telemetry

    try:
        payload = json.loads(msg.payload.decode())

        with lock:
            latest_telemetry.update(payload)
            latest_telemetry["last_seen"] = time.time()
            latest_telemetry["connected"] = True

        print("Received:", payload)

    except Exception as e:
        print("Bad MQTT message:", e)


mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_start()


@app.get("/")
def root():
    return {"message": "ESP32 Telemetry Backend Running"}


@app.get("/api/telemetry")
def get_telemetry():
    with lock:
        data = latest_telemetry.copy()

    if data["last_seen"] is not None:
        seconds_since_seen = time.time() - data["last_seen"]
        data["connected"] = seconds_since_seen < 10

    return data