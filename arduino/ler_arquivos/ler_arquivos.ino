#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600); // inicia "console"
  // Pino do led como saida.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  Serial.println();
  SPIFFS.begin();
  
  File sys;
  sys = SPIFFS.open("sys.txt", "r");
  Serial.println("sys.txt \n Condetudo : ");
  String s="";
  while(sys.available()){
    char a = sys.read();
    Serial.println(a);
    s+=a;
  }
  Serial.println("Fim do arquivo");
  Serial.println(s);
  sys.close();
  
  ler("arquivo1.txt");
  ler("arquivo2.txt");
  
}
void loop() {
}

void escrever( String s ){
  Serial.println("escrevendo?");
  File file;
  file = SPIFFS.open("arquivo1.txt","w");
  file.printf("%s\n",s.c_str());
  file.close();
  Serial.println("escrito?");
}

void ler( String s ){
  Serial.print("lendo arquivo : ");
  Serial.println(s);
  File file;
  file = SPIFFS.open(s,"r");
  if(file){
    Serial.println("Conteudo");
    String retorno;
    while(file.available()){
      retorno = file.readStringUntil('\n');
      Serial.println(retorno);
    }
    Serial.println("Final do arquivo");
    file.close(); 
  }else{
    Serial.println("arquivo nao encontrado");
  }
}
