#include <WiFi.h>
#include "wifi.hpp"
#include "globals.hpp"

// Fonction pour se connecter au Wi-Fi
void connectToWiFi(const String name, const String password) {
  if (name.isEmpty() || password.isEmpty()) {
    Serial.println("Nom ou mot de passe Wi-Fi manquant");
    return;
  }

  Serial.println("Tentative de connexion au Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(name);
  
  WiFi.begin(name.c_str(), password.c_str());

  // Attendre la connexion
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
    delay(1000);
    Serial.print(".");
    retryCount++;
  }

  // Vérification de la connexion
  if (WiFi.status() == WL_CONNECTED) {
    conectedToWifi = true;
    Serial.println("\nConnecté au Wi-Fi !");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nImpossible de se connecter au Wi-Fi.");
  }
}

// Fonction pour se déconnecter du Wi-Fi
void wifiDeconnection() {
  if (conectedToWifi) {
    WiFi.disconnect();
    conectedToWifi = false;
    Serial.println("Déconnecté du Wi-Fi.");
  } else {
    Serial.println("Déjà déconnecté du Wi-Fi.");
  }
}
