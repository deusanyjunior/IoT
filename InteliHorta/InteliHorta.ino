/*
   Código utilizado no projeto InteliHorta

   Desenvolvedores:
      Antonio Deusany de Carvalho Junior
      Janaina da Silva Fortirer
      Carlos Celso Frazão Saraiva Júnior
      Kessia Silva de Oliveira
      Leticia Figueiredo Candido
      Luan Souza da Fonseca
      Silvia Andrade Maia
*/

#include <ESP8266WiFi.h>
#include "ThingSpeak.h" // v 1.5.0
#include "keys.h"
#include <OneWire.h> // v 2.3.4 ou 2.3.5
#include <DallasTemperature.h> // v 3.8.0
#include "DHT.h" // v 1.4.0 DHT Sensor Library (Adafruit)

// WiFi
WiFiClient client;
uint8_t MAC[6] = {0xc8, 0x2a, 0x14, 0x4e, 0xc4, 0x86}; // Endereço de MAC para a placa

// Umidade e temperatura aéreas (DHT22)
#define DHTPIN D3
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

// Temperatura do solo (DS18B20)
#define ONE_WIRE_BUS D4 // Fio para comunicação com o Arduino (Normalmente o fio amarelo)
OneWire oneWire(ONE_WIRE_BUS); // Configura a biblioteca para se comunicar pelo porta selecionada
DallasTemperature sensors(&oneWire); // Configura a biblioteca para medir a temperatura

// Umidade do solo (Sensor capacitivo de umidade do solo)
#define SM_PIN A0 // Fio para comunicação com o Arduino (Normalmente o fio amarelo)
float umidadeZero = 864; // Valor dependente do sensor e deve ser verificado antes do uso
float umidadeCem = 451; // Valor dependente do sensor e deve ser verificado antes do uso

// Bomba d'água
#define BOMBA_PIN D5
float umidadeCritica = 50;
float umidadeBaixa = 60;
float TEMPIDEAL = 30;
int tempoDeRegarMax = 20000; // Tempo máximo para regar em milissegundos
int tempoDeRegarMin = 10000; // Tempo máximo para regar em milissegundos

// Tempo sem coletar dados (em minutos)
int minutosDeDescanso = 1;

void setup(void)
{
  Serial.begin(9600);

  // WiFi
  wifi_set_macaddr(STATION_IF, MAC); // Define novo endereço de MAC
  conectarWiFi(); // Configura o Wifi

  // ThingSpeak
  ThingSpeak.begin(client); // Inicia o cliente IoT

  // Umidade e temperatura aéreas (DHT22)
  dht.begin();
  
  // Temperatura do solo (DS18B20)
  sensors.begin(); // Inicia a biblioteca que mede a temperatura
  pinMode(ONE_WIRE_BUS, INPUT); // Define a porta do sensor como entrada

  // Bomba d'água
  pinMode (BOMBA_PIN, OUTPUT);
  digitalWrite(BOMBA_PIN, HIGH);

  delay(1000); // Aguarda um tempo antes de iniciar a coleta de dados
}

void loop(void)
{
  // Verifica a conexão WiFi
  while (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Umidade e temperatura aéreas (DHT22)
  float dhtH = dht.readHumidity();
  float dhtT = dht.readTemperature(); // Celsius
  //float dhtT = dht.reatTemperature(true); // Fahrenheit
  Serial.print("Umidade do ar:\t\t");
  Serial.print(dhtH, 1);
  Serial.println("%");
  Serial.print("Temperatura do ar:\t");
  Serial.print(dhtT, 1);
  Serial.println("C");

  // Temperatura do solo (DS18B20)
  sensors.requestTemperatures();
  float ds18b20T = sensors.getTempCByIndex(0); // Considera que há apenas um sensor conectado
  Serial.print("Temperatura do solo:\t");
  Serial.print(ds18b20T, 1);
  Serial.println("C");

  // Umidade do solo (Sensor capacitivo de umidade do solo)
  float soilMoisture = analogRead(SM_PIN);
  float smMapped = map(soilMoisture, umidadeZero, umidadeCem, 0, 100); // Mapeia os valores para uma faixa de 0 a 100%
  
  Serial.print("Umidade do solo:\t");
  Serial.print(smMapped, 1);
  Serial.print("% (");
  Serial.print(soilMoisture, 1);
  Serial.println(")");  

  // Controle da da bomba d'água
  int coloqueiAgua = 0;

  if (smMapped <= umidadeBaixa && ds18b20T > TEMPIDEAL)  {
    Serial.println("Colocando mais agua.");
    digitalWrite(BOMBA_PIN, LOW);  // Ligar  a bomba
    delay (tempoDeRegarMax);
    digitalWrite(BOMBA_PIN, HIGH);  // Desligar a bomba
    coloqueiAgua = 1;
    Serial.println("Coloquei muita água.");
  } else if (smMapped <= umidadeCritica)  {
    Serial.println("Colocando água.");
    digitalWrite(BOMBA_PIN, LOW);  // Ligar  a bomba
    delay (tempoDeRegarMin);
    digitalWrite(BOMBA_PIN, HIGH);  // Desligar a bomba
    coloqueiAgua = 1;
    Serial.println("Coloquei água.");
  } else {
    Serial.println("Nao coloquei água.");
  }

  // Define os valores dos campos a serem enviados para a Nuvem
  ThingSpeak.setField(1, coloqueiAgua);
  ThingSpeak.setField(2, dhtH);
  ThingSpeak.setField(3, dhtT);
  ThingSpeak.setField(4, ds18b20T);
  ThingSpeak.setField(5, smMapped);

  // Envia os dados para o ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Envia dados e retorna informação sobre o envio
  if (x == 200) {
    Serial.println("Canal atualizado com sucesso.");
  }
  else {
    Serial.println("Problema ao atualizar o canal. Código do erro no HTTP: " + String(x));
  }

  Serial.println("\nAgora vou dormir!\n");
  delay(minutosDeDescanso * 60e3); // Método para pausar o ESP (tempo em milissegundos)
  //  ESP.deepSleep(minutosDeDescanso * 60e6); // Método para desligar o ESP (tempo em microssegundos)
}

// Método para (re)conectar à rede WiFi
void conectarWiFi() {
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
}
