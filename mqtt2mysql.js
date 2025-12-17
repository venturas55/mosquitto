import mqtt from "mqtt";
import mysql from "mysql2";
import express from "express";
import cors from "cors";

import 'dotenv/config'
const app = express();
app.set("view engine", "ejs");
app.set("views", "./views");
app.use(cors());
const {
  port = 3333,
  HOST_MQTT_IP = "hostMQTT",
  HOST_MQTT_NAME = "hostMQTT",
  HOST_MQTT_WEBSOCKET_PORT = "hostMQTTport",
  HOST_MQTT_PORT = "hostMQTTport",
  DB_HOST = "localhost",
  DB_USER = "usuario",
  DB_PASS = "contrasena",
  DB_NAME = "dbname",
  DB_PORT = 3306,

} = process.env;


// -------------------
// CONFIGURACIÓN MQTT
// -------------------
const mqttUrl = "mqtt://" + HOST_MQTT_IP; // O IP del broker
const topic = "sensores/temperatura";

// -------------------
// CONFIGURACIÓN MYSQL
// -------------------
const db = mysql.createPool({
  host: DB_HOST,
  user: DB_USER,
  password: DB_PASS,
  database: DB_NAME,
});

// Conexión al broker MQTT
const client = mqtt.connect(mqttUrl);

client.on("connect", () => {
  console.log("🟢 Conectado a MQTT");
  client.subscribe(topic, (err) => {
    if (!err) console.log(`📡 Suscrito a: ${topic}`);
  });
});

// Al recibir mensaje
client.on("message", (topic, message) => {
  try {
    const data = JSON.parse(message.toString());

    const temperatura = data.temperatura;
    const timestamp = data.timestamp || Math.floor(Date.now() / 1000);

    db.query(
      "INSERT INTO temperaturas (lectura) VALUES (?)",
      [temperatura],
      (err) => {
        if (err) return console.error("❌ Error MySQL:", err.message);
        console.log("📥 Insertado:", temperatura, timestamp);
      }
    );
  } catch (e) {
    console.error("❌ Error procesando mensaje:", e);
  }
});


// -------------------
// Express API
// -------------------

app.get("/", (req, res) => {
  res.render("index", {
    serverIP: HOST_MQTT_IP,
    serverName: HOST_MQTT_NAME,
    mqttPort: HOST_MQTT_WEBSOCKET_PORT
  });
});

// Último valor
app.get("/api/ultimo", (req, res) => {
  db.query(
    "SELECT lectura, fecha FROM temperaturas ORDER BY fecha DESC LIMIT 1",
    (err, results) => {
      if (err) return res.status(500).json({ error: err.message });
      res.json(results[0]);
    }
  );
});

// Histórico
app.get("/api/historico", (req, res) => {
  db.query(
    "SELECT lectura, fecha FROM temperaturas ORDER BY fecha ASC",
    (err, results) => {
      if (err) return res.status(500).json({ error: err.message });
      res.json(results);
    }
  );
});

// RUTA PARA limpiar de valores dispares en la BBDD
app.post("/api/clean", (req, res) => {
  console.log("Limpiando");
  db.query(
    "delete from temperaturas where lectura>75 or lectura <0",
    (err, results) => {
      if (err) return res.status(500).json({ error: err.message });
      res.redirect("/");
    }
  );
});

app.listen(port, () => {
  console.log(`Servidor web/API corriendo en http://localhost:${port}`);
});
