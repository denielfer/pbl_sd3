#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

/*
  SETUP PLACA
*/

//setando as informaçoes da rede
const char* id_rede = "FIBRA-1032";
const char* senha_rede = "0Z38019489";

WiFiUDP udp;
NTPClient tempo_obj(udp, "b.ntp.br", -3 * 3600, 60000);
//endpoint AWS
const char* AWS_endpoint = "a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com";
//setando a função que vai ser chamada quando uma mensagem chegar
void callback(char* topic, byte* payload, unsigned int length);

WiFiClientSecure espClient;
PubSubClient MQTT(AWS_endpoint, 8883, callback, espClient);

/*
  SETUP VARIAVEIS GLOBAIS
*/
// Variaveis pra o codigo

unsigned long wait_time=60;
unsigned long last_mensage_time=0;

//Nome dispostivo
const String nome_dispositivo = "Lampada1";

/*
  FUNÇOES
*/

/*
  Função que configura o tempo de espera para envia a mensagem
  @param tempo, unsigned int, salva o tempo que deve ser esperado entre as mensagens 
*/
 void set_wait_time(unsigned int tempo){
    Serial.println("tempo configurado");
    wait_time = tempo>1? tempo-1:1;
  }
/*
  Função que recebe a mensagem do MQTT e chama os devidos procedimentos para serem executados
  @param topic é um vetor de char que contem o titulo do topico a qual a mensagme pertence e sera usadao para chamar o procedimento que lidara com essa mensagem
  @param payload é um vetor de bytes e consiste no binario contendo a mensagem enviada
  @param length é um unsigned int que tem o tamanho do payload
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  DynamicJsonDocument doc(length);
  deserializeJson(doc, payload);
  serializeJson(doc, Serial);
  Serial.println();
  if ( strcmp(topic,"set_timer") == 0 ) {
    set_wait_time(doc["timer"]);
  }
}
/**
* Este procedimento é responsavel por realizar a conecção com o wifi
**/
void conectar_wifi() {
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Tentando conectar com: ");
  Serial.println(id_rede);

  WiFi.begin(id_rede, senha_rede);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  tempo_obj.begin();
  while (!tempo_obj.update()) {
    tempo_obj.forceUpdate();
  }
  espClient.setX509Time(tempo_obj.getEpochTime());

}
/**
*função para Conectar com o MQTT
**/
void conectar_mqtt() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o MQTT: ");
    // parque que faz o request pra conectar
    if (MQTT.connect("ESPthing")) {
      Serial.println("Conectado");
      //se inscrevendo nos topicos que serao constantemente ouvidos pela placa
      MQTT.subscribe("set_timer");
    } else {
      Serial.print("Nao conectado, rc=");
      Serial.print(MQTT.state());
      Serial.println(" Nova tentativa em 1 segundo");
      delay(1000);
    }
  }
}
/**
* Função padrao do arduino, é executada 1 vez quando o despositivo entra em execução
**/
void setup() {
  Serial.begin(9600); // inicia "console"
  Serial.setDebugOutput(true);
  // Pino do led como saida.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  //tenta conectar com wifi
  conectar_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Falha na inicialização");
    return;
  }
  // Carregando certificados do AWS
  File cert = SPIFFS.open("/crt.der", "r");
  cert ? Serial.println("Certificado aberto com sucesso") : Serial.println("Falha ao abrir o certificado");
  espClient.loadCertificate(cert) ? Serial.println("Certificado caregado") : Serial.println("Falha ao caregar o certificado");

  File private_key = SPIFFS.open("/private.der", "r");
  private_key ? Serial.println("Chave privada aberta com sucesso") : Serial.println("Falha em abrir a chave privada");
  espClient.loadPrivateKey(private_key) ? Serial.println("Chave privada carregada com sucesso") : Serial.println("Chave privada não carregada");

  File ca = SPIFFS.open("/ca.der", "r");
  ca ? Serial.println("AWS CA aberto com sucesso") : Serial.println("AWS CA não foi abertoaberto");
  espClient.loadCACert(ca) ? Serial.println("AWS CA caregado") : Serial.println("AWS CA não carregado");

  conectar_mqtt();
//  Serial.println(tempo_obj.getFormattedTime());
  MQTT.publish("Status","{\"estado\":\"ok\"}");
}
int mensagem_tipo=0;
/**
* É uma função padrao do arduino e sera executada infinitamente apos o setup até o despositivo ser desligado
**/
void loop() {
  // se nao estiver conectado a amazon conecte
  if (!MQTT.connected()) {
    conectar_mqtt();
  }
  MQTT.loop();
  //coletamos qual o millis de agora
  unsigned long now = tempo_obj.getEpochTime();
  if(now > wait_time + last_mensage_time){
    Serial.println("publicado estatus ");
    last_mensage_time = now;
    if(mensagem_tipo==0){
      MQTT.publish("Status","{\"estado\":\"ok\"}");
      mensagem_tipo++;
    }else if(mensagem_tipo==1){
      MQTT.publish("Status","{\"estado\":\"emergencia\"}");
      mensagem_tipo++;
    }else{
      mensagem_tipo=0;
      MQTT.publish("Status","{\"estado\":\"erro\"}");
    }
  }
}
