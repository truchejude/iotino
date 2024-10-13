#ifndef MODULE_H
#define MODULE_H

#include <Arduino.h>

void initPhotoresistor();
int getLightInfo();
void setLightThreshold(int threshold);
bool isLightAboveThreshold();
String getUniqueID();

#endif
