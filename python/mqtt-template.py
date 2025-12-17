import ssl
import paho.mqtt.client as mqtt

# -------------------------------
# Configuración del broker
# -------------------------------
BROKER_HOST = "MOSQUITTO SERVER"  # o IP del servidor
BROKER_PORT = 9001                      # puerto WSS del broker
TOPIC = "test/ws"

# -------------------------------
# Callbacks
# -------------------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("✅ Conectado al broker")
        client.subscribe(TOPIC)
    else:
        print("❌ Error de conexión, rc =", rc)

def on_message(client, userdata, msg):
    print(f"📩 Mensaje recibido: {msg.topic} -> {msg.payload.decode()}")

def on_disconnect(client, userdata, rc):
    print("🔌 Desconectado, rc =", rc)

# -------------------------------
# Cliente MQTT con WebSockets y TLS
# -------------------------------
client = mqtt.Client(transport="websockets")
client.tls_set(ca_certs=None, certfile=None, keyfile=None,
               cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLS, ciphers=None)
client.tls_insecure_set(True)  # desactiva verificación de certificado si estás usando IP en lugar de dominio

client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect

try:
    print(f"Conectando a wss://{BROKER_HOST}:{BROKER_PORT} ...")
    client.connect(BROKER_HOST, BROKER_PORT)
except mqtt.WebsocketConnectionError as e:
    print("❌ Error WebSocket:", e)
except Exception as e:
    print("❌ Error inesperado:", e)

# -------------------------------
# Loop para recibir mensajes
# -------------------------------
client.loop_forever()
