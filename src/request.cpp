#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "globals.hpp"
#include "save.hpp"
#include <Arduino.h>
#include "save.hpp"

String getUniqueID();
int id = -1;

// Fonction pour extraire l'ID à partir d'une chaîne JSON
int extractId(const String &jsonString) {
  // Créer un objet JsonDocument
  JsonDocument doc; // Utiliser JsonDocument au lieu de StaticJsonDocument

  // Désérialiser la chaîne JSON
  DeserializationError error = deserializeJson(doc, jsonString);

  // Vérifier si la désérialisation a échoué
  if (error) {
    Serial.print("Erreur de désérialisation : ");
    Serial.println(error.f_str());
    return -1; // Retourne -1 en cas d'erreur
  }

  // Extraire l'ID
  int id = doc["id"]; // Extraire l'ID du JSON
  return id; // Retourner l'ID
}

// Fonction pour envoyer un UID en POST à l'API
void postEsplighter(const String &uid)
{
    if (theJwt.isEmpty())
    {
        Serial.println("Le JWT est manquant. Connectez-vous d'abord.");
        return;
    }

    // Créer l'URL complète
    String url = "http://" SERVEUR_IP "/api/esplighters";

    // Initialiser HTTPClient
    HTTPClient http;

    // Commencer la requête POST
    http.begin(url);
    http.addHeader("Content-Type", "application/json");  // Définir le type de contenu
    http.addHeader("Authorization", "Bearer " + theJwt); // Ajouter le JWT au header

    // Créer le JSON pour la requête
    String jsonBody = "{\"UID\":\"" + uid + "\"}";

    // Envoyer la requête POST
    int httpResponseCode = http.POST(jsonBody);

    // Vérifier le code de réponse
    if (httpResponseCode == 200)
    { // Vérifiez si la création a réussi
        String response = http.getString();
        id = extractId(response) - 1;
        Serial.print(id);
        Serial.println("Réponse : " + response);
    }
    else
    {
        Serial.printf("Erreur lors de la requête : %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Finir la requête
    http.end();
}

// get les info pour savoir si il existe deja et savoir son id
void getEsplighters()
{
    String uid = getUniqueID();
    bool existe = false;

    if (theJwt.isEmpty())
    {
        Serial.println("Le JWT est manquant. Connectez-vous d'abord.");
        return;
    }

    // Créer l'URL complète
    String url = "http://" SERVEUR_IP "/api/esplighters";

    // Initialiser HTTPClient
    HTTPClient http;

    // Commencer la requête GET
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + theJwt); // Ajouter le JWT au header

    // Envoyer la requête GET
    int httpResponseCode = http.GET();

    // Vérifier le code de réponse
    if (httpResponseCode == 200)
    {
        Serial.printf("Code de réponse : %d\n", httpResponseCode);

        // Lire la réponse
        String response = http.getString();
        Serial.println("Réponse : " + response);

        // Créer un document JSON pour parser la réponse
        JsonDocument doc; // Utiliser directement JsonDocument
        DeserializationError error = deserializeJson(doc, response);

        if (error)
        {
            Serial.print(F("Erreur de parsing JSON : "));
            Serial.println(error.f_str());
            return;
        }

        // Tableau pour stocker les ids et UIDs
        int ids[10];     // Un tableau de taille 10 pour les IDs
        String uids[10]; // Un tableau de taille 10 pour les UIDs
        int index = 0;   // Index pour le tableau

        // Parcourir le tableau JSON des esplighters
        for (JsonObject esplighter : doc.as<JsonArray>())
        {
            int idDuTruc = esplighter["id"];           // Extraire l'id
            const char *uid = esplighter["UID"]; // Extraire l'UID

            // Ajouter dans les tableaux
            if (index < 10)
            { // S'assurer de ne pas dépasser la taille du tableau
                ids[index] = idDuTruc;
                uids[index] = String(uid);
                index++;
            }
        }

        // Afficher les ids et UIDs
        for (int i = 0; i < index; i++)
        {
            Serial.print("ID: ");
            Serial.print(ids[i]);
            Serial.print(" - UID: ");
            Serial.println(uids[i]);
            if (uid == uids[i])
            {
                existe = true;
                id = ids[i];
                break;
            }
        }
    }
    else
    {
        Serial.printf("Erreur lors de la requête : %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Finir la requête
    http.end();
    if (!existe)
    {
        Serial.print("POST de ");
        Serial.println(uid);
        postEsplighter(uid);
    }
}

// Fonction pour envoyer une requête POST et récupérer le JWT
void loginToAPI(const String &identifier, const String &password)
{
    if (identifier.isEmpty() || password.isEmpty())
    {
        Serial.println("Identifier ou mot de passe manquant");
        return;
    }

    // Créer l'URL complète
    String url = "http://" SERVEUR_IP "/api/auth/local";

    // Initialiser HTTPClient
    HTTPClient http;

    // Commencer la requête
    http.begin(url);
    http.addHeader("Content-Type", "application/json"); // Définir le type de contenu

    // Créer le JSON pour la requête
    String jsonBody = "{\"identifier\":\"" + identifier + "\",\"password\":\"" + password + "\"}";

    // Envoyer la requête POST
    int httpResponseCode = http.POST(jsonBody);

    // Vérifier le code de réponse
    if (httpResponseCode == 200)
    {
        Serial.printf("Code de réponse : %d\n", httpResponseCode);
        saveToMemory("loginIdentifier", identifier);
        saveToMemory("loginPass", password);
        // Lire la réponse
        String response = http.getString();
        Serial.println("Réponse : " + response);

        // Extraire le JWT du JSON
        int jwtIndex = response.indexOf("jwt");
        if (jwtIndex > 0)
        {
            int start = response.indexOf("\"jwt\"", 0) + 7; // Index du début de la valeur JWT
            int end = response.indexOf("\",", start);       // Index de la fin de la valeur JWT
            if (start > 0 && end > start)
            {
                theJwt = response.substring(start, end); // Stocker le JWT
                Serial.println("JWT récupéré : " + theJwt);
                login = true;
                getEsplighters();
            }
            else
            {
                Serial.println("Erreur lors de l'extraction du JWT");
            }
        }
        else
        {
            Serial.println("JWT non trouvé dans la réponse");
        }
    }
    else
    {
        Serial.printf("Erreur lors de la requête : %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Finir la requête
    http.end();
}

// Fonction pour envoyer une requête POST et enregistrer un utilisateur
void registerToAPI(const String &username, const String &email, const String &password)
{
    bool sucesse = false;

    if (username.isEmpty() || email.isEmpty() || password.isEmpty())
    {
        Serial.print(username);
        Serial.print(email);
        Serial.print(password);
        Serial.println("Nom d'utilisateur, email ou mot de passe manquant");
        return;
    }

    // URL de l'API de registration
    String url = "http://" SERVEUR_IP "/api/auth/local/register";

    // Initialiser HTTPClient
    HTTPClient http;

    // Commencer la requête
    http.begin(url);
    http.addHeader("Content-Type", "application/json"); // Définir le type de contenu

    // Créer le JSON pour la requête
    String jsonBody = "{\"username\":\"" + username + "\",\"email\":\"" + email + "\",\"password\":\"" + password + "\"}";
    Serial.println(jsonBody);
    // Envoyer la requête POST
    int httpResponseCode = http.POST(jsonBody);

    // Vérifier le code de réponse
    if (httpResponseCode == 200 || httpResponseCode == 201) // 201 = Created
    {
        sucesse = true;
        Serial.printf("Utilisateur créé avec succès. Code de réponse : %d\n", httpResponseCode);

        // Lire la réponse
        String response = http.getString();
        Serial.println("Réponse : " + response);

        // Extraire le JWT ou d'autres informations de la réponse si nécessaire
        int jwtIndex = response.indexOf("jwt");
        if (jwtIndex > 0)
        {
            int start = response.indexOf("\"jwt\"", 0) + 7; // Index du début de la valeur JWT
            int end = response.indexOf("\",", start);       // Index de la fin de la valeur JWT
            if (start > 0 && end > start)
            {
                theJwt = response.substring(start, end); // Stocker le JWT
                Serial.println("JWT récupéré : " + theJwt);
                login = true; // Indiquer que l'utilisateur est connecté
            }
            else
            {
                Serial.println("Erreur lors de l'extraction du JWT");
            }
        }
        else
        {
            Serial.println("JWT non trouvé dans la réponse");
        }
    }
    else
    {
        Serial.printf("Erreur lors de la requête : %s\n", http.errorToString(httpResponseCode).c_str());
    }

    // Finir la requête
    Serial.println("---------------------------------------------------------");
    http.end();
    if (sucesse) {
        delay(2000);
        Serial.print("CONEXTION AVEC");
        Serial.print(email);
        Serial.println(password);
        loginToAPI(email, password);
    }
}

void updateLightInfo(int lightinfo) {
    if (id <= 0) {
        Serial.println("ID invalide.");
        return;
    }

    // Créer l'URL complète
    String url = "http://" SERVEUR_IP "/api/esplighters/lightinfo";

    // Initialiser HTTPClient
    HTTPClient http;

    // Créer l'objet JSON pour la requête
    JsonDocument doc; // Utiliser JsonDocument

    // Créer le JSON pour la requête
    doc["id"] = String(id); // Utiliser la variable globale id
    doc["lightinfo"] = lightinfo; // Ajouter lightinfo

    // Convertir le JSON en chaîne
    String jsonBody;
    serializeJson(doc, jsonBody);
    Serial.println(jsonBody);
    // Commencer la requête
    http.begin(url);
    http.addHeader("Content-Type", "application/json"); // Définir le type de contenu
    http.addHeader("Authorization", "Bearer " + theJwt); // Ajouter le JWT au header

    // Envoyer la requête PUT
    int httpResponseCode = http.PUT(jsonBody);

    // Vérifier le code de réponse
    if (httpResponseCode == 200) {
        Serial.printf("Code de réponse : %d\n", httpResponseCode);
        Serial.println("Mise à jour réussie !");
    } else {
        Serial.printf("Erreur lors de la mise à jour : %s\n", http.errorToString(httpResponseCode).c_str());
        getEsplighters();
    }

    // Finir la requête
    http.end();
}