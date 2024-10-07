#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "save.hpp"

const char *ap_ssid = "alibaba";
const char *ap_password = "123456789";
const String ipServeur = "http://31.34.150.223:1337/";
AsyncWebServer server(80);                                 // Serveur Web
String esp32ID = String((uint32_t)ESP.getEfuseMac(), HEX); // ID unique de l'ESP32

// Stockage de l'identifiant et du mot de passe dans SPIFFS
String identifier = "";
String password = "";
String jwt = "";
boolean alrediKnow = false;
int idDataBas = -1;

void creatAEsplighters(String route, String UID)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(route);

    // Header
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + jwt);

    /*
    {
      "UID": l'uid unique de l'esp
    }
    */
    String postData = "{\"UID\": \"" + UID + "\"}";

    // Envoyer la requête POST
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0)
    {
      String response = http.getString();      // Lire la réponse du serveur
      Serial.println("Response: " + response); // Afficher la réponse complète

      // Utiliser ArduinoJson pour analyser la réponse JSON
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, response);

      // Vérifier si la désérialisation a réussi
      if (!error)
      {
        // Récupérer l'ID à partir du JSON
        idDataBas = doc["id"];
        idDataBas -= 1; //-1 parce que strapi mon pote
        Serial.println("YES le truc a bien été crée");
      }
      else
      {
        Serial.println("Erreur de désérialisation JSON");
      }
    }
    else
    {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    // Fin de la connexion HTTP
    http.end();
  }
  else
  {
    Serial.println("Error in WiFi connection");
  }
}

void sendLightInfo(int lightInfo, const String &id, const String &jwt)
{
  HTTPClient http;
  String url = ipServeur + "api/esplighters/lightinfo";

  // Commencer la requête HTTP
  http.begin(url);

  // Ajouter le header Authorization avec le JWT
  http.addHeader("Authorization", "Bearer " + jwt);
  http.addHeader("Content-Type", "application/json");

  // Créer le corps JSON
  String payload = "{\"id\":\"" + id + "\", \"lightinfo\":" + String(lightInfo) + "}";

  // Envoyer la requête PUT
  int httpResponseCode = http.PUT(payload);

  // Vérifier la réponse HTTP
  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.print("c'est un gg: ");
    Serial.println(lightInfo);
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Response: " + response);
  }
  else
  {
    Serial.println("Error on sending PUT request: " + String(httpResponseCode));
  }

  // Terminer la connexion HTTP
  http.end();
}

// Fonction pour envoyer une requête HTTP avec JWT
void parseIdsAndUIDs(const String &jsonString, int ids[], String uids[], int &count)
{
  count = 0;

  // Variables temporaires
  int index = 0;

  // Boucle à travers la chaîne JSON
  while (index >= 0)
  {
    // Chercher "id" et extraire la valeur
    index = jsonString.indexOf("\"id\":", index);
    if (index >= 0)
    {
      int idEnd = jsonString.indexOf(",", index);
      ids[count] = jsonString.substring(index + 5, idEnd).toInt();

      // Chercher "UID" et extraire la valeur
      index = jsonString.indexOf("\"UID\":\"", idEnd);
      if (index >= 0)
      {
        int uidEnd = jsonString.indexOf("\"", index + 7);
        uids[count] = jsonString.substring(index + 7, uidEnd);

        // Incrémenter le compteur
        count++;
      }
    }
  }
}

String sendRequest(const String &url, const String &requestType)
{
  HTTPClient http;
  String response;

  // Initialiser la requête HTTP
  http.begin(url);

  // Ajouter le header Authorization avec le JWT
  http.addHeader("Authorization", "Bearer " + jwt);

  // Envoyer la requête selon le type spécifié
  int httpResponseCode;
  if (requestType == "GET")
  {
    httpResponseCode = http.GET();
  }
  else if (requestType == "POST")
  {
    httpResponseCode = http.POST("");
  }
  else if (requestType == "PUT")
  {
    httpResponseCode = http.PUT("");
  }
  else if (requestType == "DELETE")
  {
    httpResponseCode = http.sendRequest("DELETE");
  }
  else
  {
    Serial.println("Unsupported request type");
    return "";
  }

  // Vérifier le code de réponse HTTP
  if (httpResponseCode > 0)
  {
    response = http.getString(); // Récupérer la réponse en tant que chaîne
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
  }
  else
  {
    Serial.println("Error on HTTP request: " + String(httpResponseCode));
    response = "";
  }

  // Fermer la connexion
  http.end();

  // Retourner la réponse
  return response;
}

// Fonction pour se connecter à un réseau Wi-Fi
int setup_wifi(const String &ssid, const String &password)
{
  int i = 0;
  Serial.print("Connecting to ");
  Serial.println(ssid + " " + password);

  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED && i < 10)
  {
    delay(500);
    Serial.print(".");
    i++;
  }
  if (i == 10)
  {
    return 0;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return 1;
}

String getJwtFromResponse(const String &jsonResponse)
{
  // Créer un buffer pour contenir le JSON
  StaticJsonDocument<512> doc;

  // Parser la réponse JSON
  DeserializationError error = deserializeJson(doc, jsonResponse);

  // Vérifier si le parsing a réussi
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  // Extraire le JWT
  const char *jwt = doc["jwt"];

  // Retourner le JWT sous forme de chaîne
  return String(jwt);
}

// Fonction pour envoyer une requête POST à l'API
int connectToAPI()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(ipServeur + "api/auth/local"); // Adresse de l'API
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"identifier\": \"" + identifier + "\", \"password\": \"" + password + "\"}";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      jwt = getJwtFromResponse(response);
      Serial.println("Response: " + jwt);
      Serial.println("Response: " + response);
      Serial.println(httpResponseCode);
      // Traite ici la réponse (comme récupérer le JWT)
    }
    else
    {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    http.end();
    if (jwt.length() > 0)
    {
      int ids[25];
      String uids[25];
      int count = 0;

      // Extraire les id et UID
      parseIdsAndUIDs(sendRequest(ipServeur + "api/esplighters", "GET"), ids, uids, count);

      // Afficher les résultats
      int isExisting = 0;
      for (int i = 0; i < count; i++)
      {
        if (uids[i] == esp32ID)
        {
          alrediKnow = true;
          idDataBas = ids[i];
          Serial.print(idDataBas);
          Serial.println("alredy know");
          isExisting = 1;
          break;
        }
        Serial.println("ID: " + String(ids[i]) + ", UID: " + uids[i]);
      }
      if (isExisting == 0)
      {
        creatAEsplighters(ipServeur + "api/esplighters", esp32ID);
      }
    }
  }
  return 0;
}

void setup()
{
  Serial.begin(115200);

  // Initialiser SPIFFS pour lire les identifiants stockés
  String ssidvalue = "";
  String passvalue = "";
  readFromMemory("SSID", ssidvalue);
  readFromMemory("wifiPassword", passvalue);
  if (ssidvalue && passvalue)
  {
    if (setup_wifi(ssidvalue, passvalue))
      Serial.println("conecter au wifi");
  }
  // Configurer le point d'accès
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point 'alibaba' started");

  // Initialiser mDNS pour l'accès avec esp32.local
  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error starting mDNS");
    return;
  }

  // Serveur Web - page principale pour entrer les informations Wi-Fi et de connexion API Debut
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String html = "<!DOCTYPE html><html lang='en'>";
        html += "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>Connectez l'ESP32 à un réseau Wi-Fi</title>";
        html += "<style>body{font-family:Arial;text-align:center;padding:50px;}";
        html += "input{padding:10px;margin:10px;width:80%;max-width:400px;border:1px solid #ccc;border-radius:5px;}";
        html += "button{padding:10px 20px;background-color:#28a745;color:white;border:none;border-radius:5px;cursor:pointer;}";
        html += "button:hover{background-color:#218838;}</style></head>";
        html += "<body><h1>Configurer la connexion Wi-Fi</h1>";
        html += "<form action='/setWifi' method='POST'>";
        html += "<label>SSID du réseau Wi-Fi:</label><br>";
        html += "<input type='text' name='ssid' placeholder='Entrez le SSID' required><br>";
        html += "<label>Mot de passe:</label><br>";
        html += "<input type='password' name='password' placeholder='Entrez le mot de passe' required><br>";
        html += "<button type='submit'>Connecter</button>";
        html += "</form><br>";
        html += "<h2>ID Unique de l'ESP32: " + esp32ID + "</h2>";
        html += "<h1>Connexion API</h1>";
        html += "<form action='/setApiCredentials' method='POST'>";
        html += "<label>Identifiant:</label><br>";
        html += "<input type='text' name='identifier' value='" + identifier + "' placeholder='Entrez l'identifiant' required><br>";
        html += "<label>Mot de passe:</label><br>";
        html += "<input type='password' name='password' value='" + password + "' placeholder='Entrez le mot de passe' required><br>";
        html += "<button type='submit'>Se connecter</button>";
        html += "</form></body></html>";

        request->send(200, "text/html", html); });
  // fin de la création d'html

  String emailSaved = "";
  String passSaved = "";
  readFromMemory("email", emailSaved);
  readFromMemory("pass", passSaved);
  identifier = emailSaved;
  password = passSaved;
  connectToAPI();
  // Route pour traiter les informations Wi-Fi
  server.on("/setWifi", HTTP_POST, [](AsyncWebServerRequest *request)
            {
        String ssid = request->arg("ssid");
        String wifiPassword = request->arg("password");

        // Afficher les informations reçues dans le terminal
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + wifiPassword);
        //sauvgarde les information
        saveToMemory("SSID", ssid);
        saveToMemory("wifiPassword", wifiPassword);
        Serial.println("le pass:" + wifiPassword + " et le ssid: " + ssid + "ont bien étéstoquer");
        // Connecter au réseau Wi-Fi
        setup_wifi(ssid, wifiPassword);

        request->send(200, "text/html", "<h1>Connexion en cours...</h1><p>Retournez sur esp32.local dans quelques instants.</p>"); });

  // Route pour traiter les identifiants API et les stocker
  server.on("/setApiCredentials", HTTP_POST, [](AsyncWebServerRequest *request)
            {
        identifier = request->arg("identifier");
        password = request->arg("password");
        // Sauvegarder les identifiants dans SPIFFS
        saveToMemory("email", identifier);
        saveToMemory("pass", password);
        connectToAPI();
        request->send(200, "text/html", "<h1>Connexion API en cours...</h1>"); });

  // Démarrer le serveur
  server.begin();
}

void loop()
{
  // Lecture et envoi périodique de la valeur de la photorésistance
  delay(1000); // Envoyer toutes les 1 secondes
  if (idDataBas > -1)
  {
    int lightInfo = analogRead(34);
    sendLightInfo(lightInfo, (String)idDataBas, jwt);
    Serial.println("send");
  }
}
