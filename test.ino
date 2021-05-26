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



/*
  SETUP VARIAVEIS GLOBAIS
*/
// Variaveis pra o codigo

dispositivos acelerometro, old_ace;
dispositivos giroscopio, old_giro;

/*
  FUNÇOES
*/

/**
  Função padrao do arduino, é executada 1 vez quando o despositivo entra em execução
**/
void setup() {
  Serial.begin(9600); // inicia "console"
  Serial.setDebugOutput(true);
  // Pino do led como saida.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);
}
  /**
  É uma função padrao do arduino e sera executada infinitamente apos o setup até o despositivo ser desligado
**/
  int i = 0;
void loop() {
  // check if data is available
  while (Serial.available() > 0) {
    // read the incoming string:
    String incomingString = Serial.readStringUntil(' ');
    int a = incomingString.toInt();
    if( i==0 ){
      acelerometro.x = a;
    }else if( i == 1 ){
      acelerometro.y = a;
    }else if( i == 2 ){
      acelerometro.z = a;
    }else if( i==3 ){
      giroscopio.x = a;
    }else if( i == 4 ){
      giroscopio.y = a;
    }else if( i == 5 ){
      giroscopio.z = a;
      i = -1;
    }
    i++;
  }

  Serial.print("acelerometro x: ");
  Serial.print(acelerometro.x);
  Serial.print(" | y: ");
  Serial.print(acelerometro.y);
  Serial.print(" | z: ");
  Serial.print(acelerometro.z);
  Serial.print("      giroscopio   x: ");
  Serial.print(giroscopio.x);
  Serial.print(" | y: ");
  Serial.print(giroscopio.y);
  Serial.print(" | z: ");
  Serial.println(giroscopio.z);
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
