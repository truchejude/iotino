#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "globals.hpp"

void loginToAPI(const String &identifier, const String &password);
void registerToAPI(const String &username, const String &email, const String &password);

// Déclaration du serveur HTTP
AsyncWebServer server(80);
void wifiDeconnection();

// Fonction pour gérer la soumission du formulaire
void handleFormSubmit(AsyncWebServerRequest *request)
{
  if (request->hasParam("name", true) && request->hasParam("password", true))
  {
    enteredName = request->getParam("name", true)->value();
    enteredPassword = request->getParam("password", true)->value();

    Serial.println("Nom: " + enteredName);
    Serial.println("Mot de passe: " + enteredPassword);

    // Réponse avec une page de confirmation
    request->send(200, "text/html", "<h1>Informations enregistrées</h1><p>Nom: " + enteredName + "</p><p>Mot de passe: " + enteredPassword + "</p>");
  }
  else
  {
    request->send(400, "text/plain", "Données manquantes");
  }
}

void setupWebServer()
{
  // Route pour la page d'accueil "/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Si l'utilisateur est déjà connecté au Wi-Fi
    if (conectedToWifi) {
      // Contenu HTML pour l'utilisateur connecté
      String html = R"rawliteral(
        <!DOCTYPE html>
        <html lang="fr">
        <head>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <title>Connecté au Wi-Fi</title>
          <style>
            body {
              font-family: Arial, sans-serif;
              background-color: #f4f4f9;
              display: flex;
              justify-content: center;
              align-items: center;
              height: 100vh;
              margin: 0;
            }
            .container {
              background-color: white;
              padding: 20px;
              border-radius: 10px;
              box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
            }
            h1 {
              color: #333;
            }
            button {
              background-color: #f44336;
              color: white;
              padding: 10px 15px;
              border: none;
              border-radius: 5px;
              cursor: pointer;
              width: 100%;
              margin-top: 10px;
            }
            button:hover {
              background-color: #d32f2f;
            }
            .login-form {
              margin-top: 20px;
            }
            input[type="text"], input[type="password"] {
              width: 100%;
              padding: 10px;
              margin: 10px 0;
              border: 1px solid #ccc;
              border-radius: 5px;
            }
          </style>
        </head>
        <body>
          <div class="container">
            <h1>Vous êtes déjà connecté au Wi-Fi !</h1>
            <form action="/disconnect" method="POST">
              <button type="submit">Se déconnecter</button>
            </form>
            <form action="/register" method="POST">
              <h2>1er fois? Crée un compte!</h2>
              <label for="identifier">nom :</label>
              <input type="text" id="identifier" name="identifier" placeholder="Entrer votre nom">
              <label for="email">Email :</label>
              <input type="text" id="email" name="email" placeholder="Entrer votre email">
              <label for="password">Mot de passe :</label>
              <input type="password" id="password" name="password" placeholder="Entrer votre mot de passe">
              <button type="submit">S'inscrire</button>
            </form>
            <div class="login-form">
              <h2>Vous avez deja un compte ? Connecter vous</h2>
              <form action="/login" method="POST">
                <label for="identifier">Identifiant :</label>
                <input type="text" id="identifier" name="identifier" placeholder="Entrer votre identifiant" required>
                
                <label for="password">Mot de passe :</label>
                <input type="password" id="password" name="password" placeholder="Entrer votre mot de passe" required>
                
                <button type="submit">Se connecter</button>
              </form>
            </div>
          </div>
        </body>
        </html>
      )rawliteral";

      request->send(200, "text/html", html);
    } else {
      // Contenu HTML avec le formulaire si non connecté
      String html = R"rawliteral(
        <!DOCTYPE html>
        <html lang="fr">
        <head>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <title>ESP32 Form</title>
          <style>
            body {
              font-family: Arial, sans-serif;
              background-color: #f4f4f9;
              display: flex;
              justify-content: center;
              align-items: center;
              height: 100vh;
              margin: 0;
            }
            .container {
              background-color: white;
              padding: 20px;
              border-radius: 10px;
              box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
            }
            h1 {
              color: #333;
            }
            input[type="text"], input[type="password"] {
              width: 100%;
              padding: 10px;
              margin: 10px 0;
              border: 1px solid #ccc;
              border-radius: 5px;
            }
            button {
              background-color: #4CAF50;
              color: white;
              padding: 10px 15px;
              border: none;
              border-radius: 5px;
              cursor: pointer;
              width: 100%;
            }
            button:hover {
              background-color: #45a049;
            }
          </style>
        </head>
        <body>
          <div class="container">
            <h1>Entrer vos informations</h1>
            <form action="/submit" method="POST">
              <label for="name">Nom :</label>
              <input type="text" id="name" name="name" placeholder="Entrer votre nom">
              
              <label for="password">Mot de passe :</label>
              <input type="password" id="password" name="password" placeholder="Entrer votre mot de passe">
              
              <button type="submit">Valider</button>
            </form>
          </div>
        </body>
        </html>
      )rawliteral";

      request->send(200, "text/html", html);
    } });

  server.on("/register", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String identifier = request->arg("identifier");
    String password = request->arg("password");
    String email = request->arg("email");

    // Appel de la fonction pour s'inscrire à l'API
    registerToAPI(identifier, email, password);

    // Rediriger vers la page d'accueil après l'inscription
    request->redirect("/"); });
  // Route pour traiter le formulaire à l'envoi (POST)
  server.on("/submit", HTTP_POST, handleFormSubmit);

  // Route pour la connexion à l'API
  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String identifier = request->arg("identifier");
    String password = request->arg("password");

    // Appel de la fonction pour se connecter à l'API
    loginToAPI(identifier, password);

    // Rediriger vers la page d'accueil après la tentative de connexion
    request->redirect("/"); });

  // Route pour déconnexion du Wi-Fi
  server.on("/disconnect", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              wifiDeconnection();     // Appel de la fonction de déconnexion
              request->redirect("/"); // Rediriger vers la page d'accueil après la déconnexion
            });

  // Démarrage du serveur
  server.begin();
  Serial.println("HTTP server started");
}