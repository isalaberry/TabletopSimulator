
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- Definição dos pinos do hardware ---
const int redPin = 18;
const int greenPin = 19;
const int bluePin = 21;

// --- Configurações da Rede Wi-Fi ---
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

// --- Configurações do Broker MQTT ---
const char* mqtt_server = "broker.emqx.io";
const char* mqtt_topic_input = "IPB/IoT/tabletop/ambiente";
const char* mqtt_client_id = "esp32_tabletop_client";

// --- Objetos de comunicação ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Função para configurar e conectar ao Wi-Fi ---
void setup_wifi() {
  Serial.println("Starting Setup Wifi");
  delay(10); 
  Serial.println();
  Serial.print("Connecting to Wifi ");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// --- Função de callback para mensagens MQTT recebidas ---
void callback(char* topic_char, byte* payload, unsigned int length) {
  String topic = String(topic_char); 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Converte o payload bruto em uma String para facilitar o debug inicial
  String rawPayloadString = "";
  for (int i = 0; i < length; i++) {
    rawPayloadString += (char)payload[i];
  }
  Serial.print("Raw Payload: ");
  Serial.println(rawPayloadString);

  String command = "";

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, payload, length);

  if (!error) {
    if (doc.containsKey("_payload") && doc["_payload"].containsKey("payload")) {
      command = doc["_payload"]["payload"].as<String>();
    } 
  }

  if (command.length() == 0) { 
    command = rawPayloadString;
  }

  command.toLowerCase(); 

  Serial.print("Processed Command: ");
  Serial.println(command);

  // --- Lógica de controle dos LEDs e Buzzer ---

  digitalWrite(redPin, HIGH); 
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  if (command == "floresta") {
    digitalWrite(greenPin, LOW);
    Serial.println("Ambiente: Floresta - LEDs verdes acesos.");
  } else if (command == "combate") {
    digitalWrite(redPin, LOW);
    Serial.println("Ambiente: Combate - LEDs vermelhos acesos.");
  } else if (command == "vitoria") {
    digitalWrite(bluePin, LOW);
    Serial.println("Ambiente: Vitoria - LEDs azuis acesos.");
  } else if (command == "taverna") {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    Serial.println("Ambiente: Taverna - LEDs amarelos acesos.");
  } else if (command == "dungeon") {
    digitalWrite(redPin, LOW);
    digitalWrite(bluePin, LOW);
    Serial.println("Ambiente: Dungeon - LEDs roxos acesos.");
  } else if (command == "stop") {
    digitalWrite(redPin, HIGH);
    digitalWrite(bluePin, HIGH);
    Serial.println("Comando: STOP - Desligando todos os LEDs.");
  }
  else {
    Serial.println("Comando de ambiente desconhecido ou inválido.");
    // Pisca todos os LEDs para indicar erro
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    delay(200); 
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }
  Serial.println();
}

// --- Função para reconectar ao broker MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected to the Broker");
      client.subscribe(mqtt_topic_input);
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic_input);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000); 
    }
  }
}

// --- Função de configuração inicial do ESP32 ---
void setup() {
  Serial.begin(115200); 
  Serial.println("\n--- Starting Tabletop Imersivo Setup ---");

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  digitalWrite(redPin, HIGH); 
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  setup_wifi(); 
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
}

// --- Loop principal do ESP32 ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
    delay(1000); 
  }

  if (!client.connected()) {
    Serial.println("Node disconnected from Broker. Trying to connect...");
    reconnect();
  }

  client.loop();

}