#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "globals.hpp"
#include "wifi.hpp"
#include "module.hpp"
#include "save.hpp"

void setupWebServer();
void setupMDNS(const char *hostname);
void setupAccessPoint(const char *ssid, const char *password);
void updateLightInfo(int lightinfo);

// Variables globales initialisées
String enteredName = "";
String enteredPassword = "";
bool conectedToWifi = false;
String theJwt = "";
bool login = false;

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
}

void loop()
{
  if (!enteredName.isEmpty() && !enteredPassword.isEmpty())
  {
    // Appel de la fonction pour se connecter au Wi-Fi
    connectToWiFi(enteredName, enteredPassword);

    // Réinitialiser les variables pour éviter des connexions répétées
    enteredName = "";
    enteredPassword = "";
  }
  updateLightInfo(getLightInfo());
}