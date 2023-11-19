#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_client_id = "mqtt-explorer-36e0e042";

int PinoAnalogico = A0;
int PinoDigital = D2;
int Rele = D1;
int EstadoSensor = 0;
int UltimoEstSensor = 0;
int ValAnalogIn;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
const long interval = 10000;  // Intervalo de 10 segundos

void setup() {
  Serial.begin(9600);
  pinMode(Rele, OUTPUT);
  pinMode(PinoDigital, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
    Serial.println("Conectado ao servidor MQTT");
    client.subscribe("casa/jardim/umidade");
    client.subscribe("casa/jardim/status");
  } else {
    Serial.println("Falha na conexão MQTT");
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "casa/jardim/umidade") {
    Serial.print("Changing umidade to ");
    if(messageTemp == "Irrigando Planta"){
      Serial.println("Irrigando Planta");
      digitalWrite(Rele, HIGH);
    }
    else if(messageTemp == "Planta Irrigada"){
      Serial.println("Planta Irrigada");
      digitalWrite(Rele, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("casa/jardim/umidade");
      client.subscribe("casa/jardim/status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  ValAnalogIn = analogRead(PinoAnalogico);
  int Porcento = map(ValAnalogIn, 1023, 0, 0, 100);
  Serial.print("Umidade: ");
  Serial.print(Porcento);
  Serial.println("%");

  if (Porcento <= 75) {
    Serial.println("Irrigando Planta");
    digitalWrite(Rele, HIGH);
    client.publish("casa/jardim/umidade", "Irrigando Planta");
  } else {
    Serial.println("Planta Irrigada");
    digitalWrite(Rele, LOW);
    client.publish("casa/jardim/status", "Planta Irrigada");
  }

  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Código que precisa ser executado periodicamente a cada 10 segundos
  }
}
