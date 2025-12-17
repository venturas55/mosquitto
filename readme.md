#1.- SE EJECUTA EL SOFTWARE EN EL LOLIN WEMOS MINI:

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi
const char* ssid = "SSID_WIFI_2_CONNECT";
const char* password = "PASSWORD_OF_WIFI_TO_CONNECT";

// MQTT
const char* mqtt_server = "IP";  // IP del broker Mosquitto
const int mqtt_port = 1883;
const char* mqtt_topic = "sensores/temperatura";

// DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

// Conectar WiFi
void setup_wifi() {
  delay(100);
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Conectar MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("WemosD1mini")) {
      Serial.println(" conectado.");
    } else {
      Serial.print(" error ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // Crear payload JSON
  String payload = "{\"temperatura\": ";
  payload += temp;
  payload += ", \"timestamp\": ";
  payload += millis();
  payload += "}";

  client.publish(mqtt_topic, payload.c_str());
  Serial.println("Enviado: " + payload);

  delay(5000); // enviar cada 5 segundos
}


==============================================================
#2.- MQTT:

Se instala con:

      sudo apt update
      sudo apt install mosquitto mosquitto-clients
      sudo systemctl enable mosquitto


Se configura MQTT CON: sudo nano /etc/mosquitto/mosquitto.conf

      pid_file /run/mosquitto/mosquitto.pid

      #persistence true
      per_listener_settings true

      #include_dir /etc/mosquitto/conf.d
      # Para Wemos (TCP normal)
      listener 1883
      protocol mqtt
      allow_anonymous true
      persistence_location /home/ubuntu/mosquitto/data/
      log_dest file /home/ubuntu/mosquitto/mosquitto.log
      log_type error
      log_type warning
      log_type notice
      log_type information


      # Para web (WebSockets)
      listener 9001
      protocol websockets
      allow_anonymous true

      cafile /etc/mosquitto/certs/fullchain.pem
      certfile /etc/mosquitto/certs/fullchain.pem
      keyfile /etc/mosquitto/certs/privkey.pem



================================================================
#3.- La base de datos se ha creado una db: MQTT:
donde hay una tabla "temperaturas":
      CREATE TABLE temperaturas (
          id INT AUTO_INCREMENT PRIMARY KEY,
          lectura FLOAT,
          fecha DATETIME DEFAULT CURRENT_TIMESTAMP
      );


================================================================

#4.- Se ejecuta el "NODE-RED", que en su defecto se ha creado un programita en nodejs que se ejecuta con:
	pm2 start mqtt2mysql.js --name mosquitto
Y ya esta vertiendo datos en la base de datos. Necesitaras un .env

      HOST_MQTT_IP=
      HOST_MQTT_PORT=
      HOST_MQTT_WEBSOCKET_PORT=9001
      DB_HOST=
      DB_USER=root
      DB_PASS=
      DB_NAME=MQTT
      port=3333


===================================================================
#5.- Falta la web de visionado: (YA INCLUIDA EN la app nodejs ) mqtt2mysql.js con plantilla ejs
Plantilla dentro de "views"




====================================================================
El proyecto viene de : 
https://github.com/venturas55/mosquitto.git