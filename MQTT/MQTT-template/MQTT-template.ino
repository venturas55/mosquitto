#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi
const char* ssid = "iPhone-H4YJ1BKB0F0X";
const char* password = "1234567890";
// MQTT
const char *mqtt_server = "152.228.133.198"; // IP del broker Mosquitto
const int mqtt_port = 1883;
const char *mqtt_topic = "sensores/temperatura";
const int pinSalida = 5; // ES EL 5? DEL WEMOS LOLIN D1 MINI V4.0
const int pinSalida2 = 0; // ES EL 0? DEL WEMOS LOLIN D1 MINI V4.0

// DS18B20
#define ONE_WIRE_BUS 2 // ES EL 2 DEL WEMOS LOLIN D1 MINI V4.0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

// Conectar WiFi
void setup_wifi()
{
  delay(100);
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Conectar MQTT
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Conectando MQTT...");
    if (client.connect("WemosD1mini"))
    {
      Serial.println(" conectado.");
      client.subscribe("wemos/salida1");
      client.subscribe("wemos/salida2");
    }
    else
    {
      Serial.print(" error MQTT:");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup()
{
  pinMode(pinSalida, OUTPUT);
  digitalWrite(pinSalida, LOW);
  pinMode(pinSalida2, OUTPUT);
  digitalWrite(pinSalida2, LOW);
  Serial.begin(115200);
  sensors.begin();

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String mensaje;
  for (int i = 0; i < length; i++)
    mensaje += (char)payload[i];
  mensaje.trim();
  mensaje.toUpperCase();

  if (String(topic) == "wemos/salida1")
  {
    if (mensaje == "ON")
    {
      digitalWrite(pinSalida, HIGH);
      Serial.println("Recibido: ON ");
    }
    else if (mensaje == "OFF")
    {
      Serial.println("Recibido: OFF ");
      digitalWrite(pinSalida, LOW);
    }
  }else 
  if (String(topic) == "wemos/salida2")
  {
    if (mensaje == "ON")
    {
      digitalWrite(pinSalida2, HIGH);
      Serial.println("Recibido: ON ");
    }
    else if (mensaje == "OFF")
    {
      Serial.println("Recibido: OFF ");
      digitalWrite(pinSalida2, LOW);
    }
  }
}


unsigned long lastSend = 0;
void loop()
{
    if (!client.connected())
        reconnect();

    client.loop();

    if (millis() - lastSend > 5000)
    {
        lastSend = millis();

        sensors.requestTemperatures();
        float temp = sensors.getTempCByIndex(0);

        String payload = "{\"temperatura\": ";
        payload += temp;
        payload += ", \"timestamp\": ";
        payload += millis();
        payload += "}";

        client.publish(mqtt_topic, payload.c_str());

        Serial.println("Enviado: " + payload);
    }
}