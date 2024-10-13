#ifndef WIFI_HPP
#define WIFI_HPP
#include <Arduino.h>
#include <WiFi.h>

void connectToWiFi(const String name, const String password);
void wifiDeconnection();

#endif // WIFI_HPP
