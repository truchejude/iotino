#include <HTTPClient.h>
#include "globals.hpp"

// Fonction pour envoyer une requête POST et récupérer le JWT
void loginToAPI(const String &identifier, const String &password) {
  if (identifier.isEmpty() || password.isEmpty()) {
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
  if (httpResponseCode > 0) {
    Serial.printf("Code de réponse : %d\n", httpResponseCode);

    // Lire la réponse
    String response = http.getString();
    Serial.println("Réponse : " + response);

    // Extraire le JWT du JSON
    int jwtIndex = response.indexOf("jwt");
    if (jwtIndex > 0) {
      int start = response.indexOf("\"", jwtIndex) + 1; // Index du début de la valeur JWT
      int end = response.indexOf("\"", start); // Index de la fin de la valeur JWT
      if (start > 0 && end > start) {
        theJwt = response.substring(start, end); // Stocker le JWT
        Serial.println("JWT récupéré : " + theJwt);
      } else {
        Serial.println("Erreur lors de l'extraction du JWT");
      }
    } else {
      Serial.println("JWT non trouvé dans la réponse");
    }
  } else {
    Serial.printf("Erreur lors de la requête : %s\n", http.errorToString(httpResponseCode).c_str());
  }

  // Finir la requête
  http.end();
}
