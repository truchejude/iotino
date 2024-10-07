#include <Preferences.h>

Preferences preferences;

void saveToMemory(const String &key1, const String &value1)
{
  preferences.begin("my-app", false); // Ouvre un espace nommé "my-app" en mode lecture/écriture
  preferences.putString(key1.c_str(), value1);
  preferences.end();
  Serial.println("Data saved to memory.");
}

void readFromMemory(const String &key, String &value)
{
  preferences.begin("my-app", true);

  value = preferences.getString(key.c_str(), "");
  preferences.end();
}