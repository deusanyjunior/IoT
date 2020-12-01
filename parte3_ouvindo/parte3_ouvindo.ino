/*
  Introdução à Internet das Coisas com ESP8266

  Parte 3 - Escutando a Internet

*/

#include "keys.h"
#include <ESP8266WiFi.h> // Versão 2.6.0

// WiFi
WiFiClient client;

// A Coisa
String nomeDaCoisa = "node";

// A Fonte
const char* host = "dweet.io";
const int httpPort = 80;

// O Led
int led = BUILTIN_LED;

void setup(void)
{
  Serial.begin(9600);

  // WiFi
  conectarWiFi(); // Configura o Wifi
  
  // O Led
  pinMode(led, OUTPUT);

  delay(1000); // Aguarda um tempo antes de iniciar
}

void loop(void)
{
  // Verifica a conexão WiFi
  while (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }
  
  Serial.println("Olá, Internet!");

  // Solicita o último valor enviado para o Dweet.io
  dweetPedido();

  // Pisca O Led
  digitalWrite(led, LOW);
  delay(1000);
  digitalWrite(led, HIGH);
  delay(1000);
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

// Conecta via TCP ao Dweet.io
// Inicia comunicação, faz  pedido e finaliza a conexão
void dweetPedido() {
  Serial.println("\nInício da conexão.");

  WiFiClient client; 
  client.setTimeout(10000);
  if (!client.connect(host, httpPort)) { 
    Serial.println("Falha de conexão com o Dweet.io!"); 
    return; 
  }

  Serial.println("Conectado!");
  
  // Prepara a mensagem de solicitação de dados e envia
  client.print(msgReceberHTTPDweet());

  // Decodifica a mensagem recebida
  decodificaRespostaDweet(client);
  
  // Desconectar
  client.stop();
  
  Serial.println();  
  Serial.println("Fim da conexão.");
  delay(1000); 
} 

// Prepara a mensagem de envio para o Dweet.io
// Segue o padrão: https://dweet.io/get/latest/dweet/for/my-thing-name
String msgReceberHTTPDweet() {
  String dweetHttpGet = "GET /get/latest/dweet/for/";
  dweetHttpGet = dweetHttpGet + String( nomeDaCoisa );
  dweetHttpGet = dweetHttpGet+" HTTP/1.1\r\n"+ 
                 "Host: " +  
                 host +  
                 "\r\n" +  
                 "Connection: close\r\n\r\n";
  Serial.println("\nTexto a enviar:");
  Serial.print(dweetHttpGet);
  return dweetHttpGet;
}

// Recebe o retorno em formato JSON e decodifica o dado lido
void decodificaRespostaDweet(WiFiClient client) {
  // Verifica resposta HTTP
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print("Resposta inesperada: ");
    Serial.println(status);
    return;
  }

  // Pula os cabecalhos HTTP
  char fimDosCabecalhos[] = "\r\n\r\n";
  if (!client.find(fimDosCabecalhos)) {
    Serial.println("Resposta inválida!");
    return;
  }

  // Salva o objeto JSON dinamicamente e imprime o dado na saída Serial
  // Use arduinojson.org/v6/assistant para definir esta capacidade
  const size_t capacity = JSON_ARRAY_SIZE(1) 
                        + JSON_OBJECT_SIZE(1) 
                        + JSON_OBJECT_SIZE(3) 
                        + JSON_OBJECT_SIZE(4) 
                        + 130;
  DynamicJsonDocument doc(capacity);
  DeserializationError erro = deserializeJson(doc, client);
  if (erro) {
    Serial.print("Falha em deserializeJson(): ");
    Serial.println(erro.c_str());
    return;
  }  
  JsonObject conteudo = doc["with"][0];
  const char* coisa = conteudo["thing"];
  const char* dataDeCriacao = conteudo["created"];
  int dado = conteudo["content"]["dado"];
  Serial.print("Dado lido: ");
  Serial.println(dado);
}
