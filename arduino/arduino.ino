#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

/*
  SETUP PLACA
*/

struct dispositivos
{
  int x;
  int y;
  int z;
};


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

dispositivos acelerometro, old_ace;
dispositivos giroscopio, old_giro;

unsigned long wait_time = 60;
unsigned long last_mensage_time = 0;

bool stat_is_alarm = true;
String problema = "";
/*
  FUNÇOES
*/

/*
  Esta função é responsavel por publica o porblema pro broker
*/
void problema_confirmado() {
  if (stat_is_alarm) {
    MQTT.publish("Status", "{\"estado\":\"Roubo\"}");
  } else if (problema != "") {
    String m = "{\"estado\":\"" + problema + "\"}";
    char s[100];
    m.toCharArray(s,100);
    Serial.println(s);
    MQTT.publish("Status", s);
  } else {
    MQTT.publish("Status", "{\"estado\":\"Problema não identificado\"}");
  }
  unsigned int now = tempo_obj.getEpochTime();
  last_mensage_time = now;
}

/*
  Função que muda o estado de funcionamento da placa
  @param new_state, bool, uma booleana sendo este o true se o sistema deve esta no modo de alarme e false para dector de colisão;
*/
void change_state(bool new_state) {
  Serial.print("Estado mudado para: ");
  String s = new_state ? "Alarme" : "Detector de acidente";
  Serial.println(s);
  stat_is_alarm = new_state;
}

/*
  Nesta função salvados os dados  de "leitura" nas variaveis ( foi feito assim pra ser facil troca o modulo caso a foram de adiciona dados mude)
  @param s, String, sendo a uma String que vai esta indicando qual dado deve ser escrito ("acelerometro" para leitura do acelerometro e "giroscopio" para leituras do giroscopio)
  @return um objeto do tipo dispositivos contendo os novos valor para o tipo passado em s
*/
dispositivos salva_dados_leiura( char* s) {
  int x, y, z;
  dispositivos d;
  if ( strcmp(s, "acelerometro") == 0 ) {
    x = 200;
    y = 200;
    z = 220;
  } else if ( strcmp(s, "giroscopio") == 0 ) {
    x = 220;
    y = 220;
    z = 220;
  }
  d.x = x;
  d.y = y;
  d.z = z;
  return d;
}


/*
  Nesta função alteramos os dados de "leitura" sempre acrecentando 10 para testa dodos os valores
  @param d, dispositivo, variavel contendo o estado antigo
  @return um objeto do tipo dispositivos contendo os novos valores
*/
dispositivos varre_dados_leiura( dispositivos d) {
  dispositivos c;
  c.x = d.x+1;
  if(c.x >300)
    Serial.println("reset");
    c.y = d.y+1;
    c.x = 200;
  if(c.y > 300)
    Serial.println("reset");
    c.z = d.z+1;
    c.y = 200;
  Serial.print("x: ");
  Serial.print(d.x);
  Serial.print(" | y: ");
  Serial.print(d.y);
  Serial.print(" | z: ");
  Serial.println(d.z);
  return c;
}


/*
  Função que configura o tempo de espera para envia a mensagem
  @param tempo, unsigned int, salva o tempo que deve ser esperado entre as mensagens
*/
void set_wait_time(unsigned int tempo) {
  //Serial.println("tempo configurado");
  wait_time = tempo > 1 ? tempo - 1 : 1;
  //Serial.println(tempo);
  //    wait_time = tempo;
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
  DynamicJsonDocument doc(512);
  deserializeJson(doc, payload);
  serializeJson(doc, Serial);
  Serial.println();
  if ( strcmp(topic, "set_timer") == 0 ) {
    set_wait_time(doc["timer"]);
  }
}
/**
  Este procedimento é responsavel por realizar a conecção com o wifi
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
  função para Conectar com o MQTT
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
  Função padrao do arduino, é executada 1 vez quando o despositivo entra em execução
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

  acelerometro = salva_dados_leiura("acelerometro");
  giroscopio = salva_dados_leiura("giroscopio");
  Serial.print("x: ");
  Serial.print(acelerometro.x);
  Serial.print(" | y: ");
  Serial.print(acelerometro.y);
  Serial.print(" | z: ");
  Serial.println(acelerometro.z);
  Serial.print("x: ");
  Serial.print(giroscopio.x);
  Serial.print(" | y: ");
  Serial.print(giroscopio.y);
  Serial.print(" | z: ");
  Serial.println(giroscopio.z);
  //  delay(1000000);
  //  Serial.println(tempo_obj.getFormattedTime());
  MQTT.publish("inicia", "{}");
  MQTT.publish("Status", "{\"estado\":\"ok\"}");
}
int mensagem_tipo = 0;
bool was_button_pushed = false, was_save = true, alarm_on = false;
/**
  É uma função padrao do arduino e sera executada infinitamente apos o setup até o despositivo ser desligado
**/
void loop() {
  // se nao estiver conectado a amazon conecte
  if (!MQTT.connected()) {
    conectar_mqtt();
  }
  MQTT.loop();

  if ( was_button_pushed && !was_save) {
    // Coloca codigo do botao precionado
    if (!alarm_on) {
      change_state(stat_is_alarm ? false : true);
      Serial.println("mudando modo operação");
    }
  }
  unsigned long now = tempo_obj.getEpochTime();/*
  Serial.print("x: ");
  Serial.print(acelerometro.x);
  Serial.print(" | y: ");
  Serial.print(acelerometro.y);
  Serial.print(" | z: ");
  Serial.println(acelerometro.z);
  Serial.print("x: ");
  Serial.print(giroscopio.x);
  Serial.print(" | y: ");
  Serial.print(giroscopio.y);
  Serial.print(" | z: ");
  Serial.println(giroscopio.z);*/
  if (alarm_on) {
    Serial.println("Alarme ligado");
    MQTT.publish("Status", "{\"estado\":\"alarme ativado\"}");
    unsigned long espera = now + 60;
    while ( now < espera ) {
      if (!MQTT.connected()) {
        conectar_mqtt();
      }
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      if ( was_button_pushed && !was_save) {
        Serial.print("botao precionado");
        MQTT.publish("Status", "{\"estado\":\"ok\"}");
        last_mensage_time = now;
        alarm_on = false;
        delay(10000);
        break;
      }
      was_save = was_button_pushed;
      was_button_pushed = digitalRead(0) == 0 ? true : false;
      now = tempo_obj.getEpochTime();
      if (now > wait_time + last_mensage_time) {
        MQTT.publish("Status", "{\"estado\":\"alarme\"}");
        last_mensage_time = now;
        Serial.println("publicando alarme ligado");
      }
    }
    if (alarm_on) {
      Serial.println("Problema confirmado/ botao nao apertado");
      problema_confirmado();
      alarm_on = false;
      digitalWrite(LED_BUILTIN, LOW);
      delay(1500);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  was_save = was_button_pushed;
  was_button_pushed = digitalRead(0) == 0 ? true : false;
  old_ace = acelerometro;
  old_giro = giroscopio;
  acelerometro = varre_dados_leiura(acelerometro);
  giroscopio = varre_dados_leiura(giroscopio);
  if (stat_is_alarm) {
    if (old_ace.x != acelerometro.x || old_ace.y != acelerometro.y || old_ace.z != acelerometro.z) {
      alarm_on = true;
      problema = "Roubo";
      Serial.println("roubo detectado");
    }
  } else {
    if (acelerometro.x > 380 && acelerometro.z < 360) {
      problema = "Tombou para a esquerda";
      alarm_on = true;
      Serial.println("acidente detectado");
    } else if (acelerometro.x < 270 && acelerometro.z < 360) {
      problema = "Tombou para a direita";
      alarm_on = true;
      Serial.println("acidente detectado");
    } else if (acelerometro.x < 270 && acelerometro.z < 360) {
      problema = "Tombou para trás";
      alarm_on = true;
      Serial.println("acidente detectado");
    } else if (acelerometro.x > 380 && acelerometro.z < 360) {
      problema = "Tombou para frente";
      alarm_on = true;
      Serial.println("acidente detectado");
    } else if (acelerometro.z > 360) {
      problema = "Capotado";
      alarm_on = true;
      Serial.println("acidente detectado");
    } else {
      problema = "";
    }
  }
  if (now > wait_time + last_mensage_time) {
    Serial.println("publicando estatus");
    if ( problema != "") {
      MQTT.publish("Status", "{\"estado\":\"ok\"}");
    } else {
      MQTT.publish("Status", "{\"estado\":\"alarme\"}");
    }
      last_mensage_time = now;
  }
}
  /*
    else if(now > wait_time + last_mensage_time){
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
    }*/
