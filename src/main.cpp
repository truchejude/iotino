#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <PubSubClient.h> // Bibliothèque pour MQTT
#include "globals.hpp"
#include "wifi.hpp"
#include "module.hpp"
#include "save.hpp"
#include <ArduinoJson.h>

void setupWebServer();
void setupMDNS(const char *hostname);
void setupAccessPoint(const char *ssid, const char *password);
void publishLightInfo(int lightinfo, String Email);
void connectToMQTT();
String getUniqueID();

WiFiClient espClient;
PubSubClient client(espClient);

// Variables globales initialisées
String enteredName = "";
String enteredPassword = "";
bool conectedToWifi = false;
String theJwt = "";
bool login = false;
const int ledPin1 = 33;
const int ledPin2 = 32;

const char *mqttServer = "31.34.150.223"; // Adresse du serveur Mosquitto
const int mqttPort = 1883;                // Port MQTT standard

void setupAccessPoint(const char *ssid, const char *password)
{
  // Configuration du point d'accès Wi-Fi
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);
}

void setupMDNS(const char *hostname)
{
  if (!MDNS.begin(hostname))
  {
    Serial.println("Error setting up mDNS responder!");
    return;
  }
  Serial.printf("mDNS responder started: http://%s.local\n", hostname);
}

void handleJsonMessage(String message) {
  // Créer un document JSON avec une capacité appropriée
  StaticJsonDocument<200> doc;

  // Analyser le JSON reçu
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("Erreur de parsing JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Vérifier et contrôler les LEDs en fonction du JSON
  const char* led1State = doc["led1"];
  const char* led2State = doc["led2"];

  if (strcmp(led1State, "on") == 0) {
    digitalWrite(ledPin1, HIGH); // Allumer la LED 1
    Serial.println("LED 1 allumée");
  } else if (strcmp(led1State, "off") == 0) {
    digitalWrite(ledPin1, LOW); // Éteindre la LED 1
    Serial.println("LED 1 éteinte");
  }

  if (strcmp(led2State, "on") == 0) {
    digitalWrite(ledPin2, HIGH); // Allumer la LED 2
    Serial.println("LED 2 allumée");
  } else if (strcmp(led2State, "off") == 0) {
    digitalWrite(ledPin2, LOW); // Éteindre la LED 2
    Serial.println("LED 2 éteinte");
  }
}

// Fonction de rappel pour les messages entrants
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Convertir le payload en chaîne de caractères
  String message;
  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.print("Message: ");
  Serial.println(message);
  handleJsonMessage(message);
}

void publishLightInfo(int lightinfo, String Email)
{
  // Formater le message au format JSON
  String message = "{\"" + getUniqueID() + "\": {\"lightinfo\": " + String(lightinfo) + ", \"email\": \"" + Email + "\"}}";

  // Publier le message sur le topic 'esplighter'
  if (client.publish("esplighter", message.c_str())) // c_str() pour convertir String en char*
  {
  }
  else
  {
    Serial.println("Message publication failed");
  }
}

void connectToMQTT()
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback); // Définir la fonction de rappel pour recevoir les messages

  while (!client.connected())
  {
    Serial.print("Connecting to MQTT... ");
    if (client.connect(("ESP32Client" + getUniqueID()).c_str())) // Utilise un client ID unique pour se connecter
    {
      Serial.println("connected");

      // Souscription au topic getUniqueID() après une connexion réussie
      if (client.subscribe(getUniqueID().c_str()))
      {
        Serial.println("Subscribed to topic");
      }
      else
      {
        Serial.println("Failed to subscribe to topic");
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Attendre avant de réessayer
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // Création du point d'accès Wi-Fi
  initPhotoresistor();
  const char *ssid = "ESP32_AP";     // Nom du point d'accès
  const char *password = "12345678"; // Mot de passe (min. 8 caractères)
  setupAccessPoint(ssid, password);

  // Démarrage de mDNS
  setupMDNS("esp32ap");

  // Démarrage du serveur web
  setupWebServer();

  readFromMemory("wifiSSID", enteredName);
  readFromMemory("wifiPass", enteredPassword);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);
}

void loop()
{
  String loginIdentifier = "";
  readFromMemory("loginIdentifier", loginIdentifier);

  if (!enteredName.isEmpty() && !enteredPassword.isEmpty())
  {
    // Appel de la fonction pour se connecter au Wi-Fi
    connectToWiFi(enteredName, enteredPassword);

    // Réinitialiser les variables pour éviter des connexions répétées
    enteredName = "";
    enteredPassword = "";
  }

  int lightinfo = getLightInfo();
  if (!client.connected())
  {
    connectToMQTT(); // Connecte-toi si tu n'es pas connecté
  }

  publishLightInfo(lightinfo, loginIdentifier);
  client.loop(); // Maintenir la connexion MQTT active et vérifier les messages entrants
  delay(200);
}
