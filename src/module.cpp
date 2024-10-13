#include <Arduino.h>
#include <WiFi.h>
#define photoresistorPin 34

// Variable pour stocker le seuil de lumière
int lightThreshold = 500;

// Fonction d'initialisation de la photorésistance
void initPhotoresistor() {
  // Configuration du pin en mode d'entrée
  pinMode(photoresistorPin, INPUT);
}

// Fonction pour récupérer la valeur lue par la photorésistance
int getLightInfo() {
  // Lire la valeur analogique de la photorésistance
  int lightValue = analogRead(photoresistorPin);
  return lightValue;
}

// Fonction pour définir un seuil de lumière
void setLightThreshold(int threshold) {
  lightThreshold = threshold;
}

// Fonction pour vérifier si la lumière est au-dessus du seuil
bool isLightAboveThreshold() {
  int lightValue = getLightInfo();
  return lightValue > lightThreshold;
}


// Fonction pour obtenir l'UID unique de l'ESP32
String getUniqueID() {
  uint64_t chipID = ESP.getEfuseMac();  // Récupère l'adresse MAC de l'ESP32
  String uniqueID = String((uint16_t)(chipID >> 32), HEX) + String((uint32_t)chipID, HEX);
  
  // Formater l'UID en majuscules
  uniqueID.toUpperCase();
  
  return uniqueID;
}
