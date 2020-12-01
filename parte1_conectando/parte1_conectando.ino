/*
  Introdução à Internet das Coisas com ESP8266

  Parte 1 - Conectando-se à Internet
*/

#include "keys.h"
#include <ESP8266WiFi.h> // Versão 2.6.0

// WiFi
WiFiClient client;

void setup(void)
{
  Serial.begin(9600);

  // WiFi
  conectarWiFi(); // Configura o Wifi

  delay(1000); // Aguarda um tempo antes de iniciar
}

void loop(void)
{
  // Verifica a conexão WiFi
  while (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  
  Serial.println("Olá, Internet!");
  
  delay(10000);
}

// Método para (re)conectar à rede WiFi
void conectarWiFi() {
  Serial.println("");
  Serial.print("Conectando-se à rede: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}
