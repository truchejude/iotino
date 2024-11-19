# Documentation du Projet IoT Esplighter

## Introduction

L'objectif du projet Esplighter est de créer un dispositif IoT capable de gérer automatiquement des LED en fonction de la luminosité ambiante mesurée par une photorésistance. Le dispositif envoie les informations collectées à un serveur Mostiko et offre des fonctionnalités de contrôle à distance via une interface web.

## Matériel Utilisé

- **ESP32**
- **LED bleue** : connectée au pin GPIO 33
- **LED rouge** : connectée au pin GPIO 32
- **Photorésistance** : connectée au pin GPIO 34
- Serveur **Mostiko**
- Serveur **Strapi**
- Serveur **Page web en react**

## Fonctionnalités Principales

### Gestion Automatique des LED

1. **LED bleue (GPIO 33)** :
   - S'allume lorsque la luminosité ambiante est inférieure à un seuil minimum.
2. **LED rouge (GPIO 32)** :
   - S'allume lorsque la luminosité ambiante dépasse un seuil maximum.

### Envoi des Informations au Serveur

- Les données mesurées par la photorésistance sont envoyées périodiquement au serveur Mostiko.
- Ces données incluent :
  - La luminosité actuelle
  - l'email de l'utilisateur

### Contrôle via Interface Web

L'interface web connectée au serveur Mostiko permet :

- De définir des seuils de luminosité :
  - **Luminosité minimale** (pour déclencher la LED bleue)
  - **Luminosité maximale** (pour déclencher la LED rouge)
- De renommer l'Esplighter pour faciliter son identification dans un environnement multi-dispositifs.

## Fonctionnement du Système

1. La photorésistance mesure la luminosité ambiante en continu.
2. Les données sont traitées sur le serveur strapi pour déterminer l'état des LED :
   - Si la luminosité est inférieure au seuil minimum, la LED bleue s'allume.
   - Si la luminosité est supérieure au seuil maximum, la LED rouge s'allume.
3. Les données des led sont envoyer sur le chanel moskito qui a pour topic UID de cette ESP

## API et Configuration

### Données Transmises au Serveur Mostiko
conextion au topic puis envoie des information via un json en string

### Paramètres Configurables

1. **Seuils de luminosité** :
   - Luminosité minimale
   - Luminosité maximale
2. **Nom de l'Esplighter** :
   - Permet de personnaliser le nom du dispositif pour une meilleure organisation.

## Schéma de Connexion

- **LED bleue** : Pin GPIO 33 (avec résistance)
- **LED rouge** : Pin GPIO 32 (avec résistance)
- **Photorésistance** : Pin GPIO 34 (avec un diviseur de tension)

## Instructions de Déploiement

1. Deploiment du serveur Strapi.
2. Deploiment du serveur React.
3. Déployer le firmware sur l'ESP32 via PlatformIO ou Arduino IDE.

## Améliorations Futures

- Ajouter une alerte via notification en cas de luminosité extrême.
- Intégrer des capteurs supplémentaires (température, humidité) pour étendre les fonctionnalités.

---

Cette documentation couvre les aspects techniques et fonctionnels du projet Esplighter pour une gestion de la luminosité et des LED via un dispositif IoT.
