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

// variaveis pra guarda o acelerometro e giroscopio e mais 2 para guarda o estado anterior
dispositivos acelerometro, old_ace;
dispositivos giroscopio, old_giro;

unsigned long wait_time = 60; // quantos segundos serao esperados para enviar uma nova, por defaut botamos 60 mas no setup recebemos o valor salvo no banco de dados do site
unsigned long last_mensage_time = 0; // epoch no qual foi enviado a ultima mensagem

bool stat_is_alarm = true;// bool pra indicar se estamos no modo de alarme ou de detecção de acidentes
String problema = ""; // String contendo o problema identificado, "" para nenhum problema

bool arquivo = true; // bool pra indicar qual arquivo deve escrever o historico ( por decisao de equipe sera salvo o historico do dia anterior entao temos 2 arquivos e escrevemos nele alternadamente a cada dia )
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
  String s = new_state ? "alarme" : "Detector de acidente";
  Serial.println(s);
  new_state ? MQTT.publish("state", "{\"modo\":1}") : MQTT.publish("state", "{\"modo\":0}");
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
  }else if(strcmp(topic, "set_state") == 0){
    change_state(doc["estado"]==1);
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
      MQTT.subscribe("set_state");
    } else {
      Serial.print("Nao conectado, rc=");
      Serial.print(MQTT.state());
      Serial.println(" Nova tentativa em 1 segundo");
      delay(1000);
    }
  }
}

/*
  Função que le todo o arquivo passado, printando o conteudo lido
  @param nome, String sendo o nome do arquivo que sera lido
*/
void le_arquivo_full(String nome){
  File f = SPIFFS.open(nome, "r");
  Serial.print("tentando ler : ");
  Serial.println(nome);
  Serial.println("Conteudo:");
  while(f.available()){
    String retorno = f.readStringUntil('\n');
    Serial.println(retorno);
  }
  Serial.println("Fim do conteudo");
  f.close(); 
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
  
  // caregamos do arquivo sys.txt qual arquivo deve ser aberto
  Serial.println("lendo sys.txt");
  File sys = SPIFFS.open("sys.txt", "r");
  if(!sys){// se nao existir criamos e escrevemos 1;
    Serial.println("sys.txt nao encontrado, criando com valor '1'");  
    sys = SPIFFS.open("sys.txt", "w");
    sys.printf("1\n");
    arquivo = true;
  }else{// se existir salvamos e lemos qual arquivo deve ser escrito
    Serial.print("sys.txt encontrado valor lido : ");  
    String qual_arquivo = sys.readStringUntil('\n');
    arquivo = ( qual_arquivo == "1" );    
    Serial.println(arquivo? '1':'0');  
  }
  sys.close();
  Serial.print("Escrevendo encima do arquivo : ");
  Serial.println(arquivo? "arquivo1.txt":"arquivo2.txt");
  Serial.println("Conteudo antigo : ");
  le_arquivo_full(arquivo? "arquivo1.txt":"arquivo2.txt");
  Serial.println("");
  File file;
  // limpamos o arquivo que sera escrito
  if(arquivo){
    file = SPIFFS.open("arquivo1.txt", "w");
  }else{
    file = SPIFFS.open("arquivo2.txt", "w");
  }  
  file.close();

  tempo_obj.begin(); 
  
  conectar_mqtt();

  acelerometro = salva_dados_leiura("acelerometro");
  giroscopio = salva_dados_leiura("giroscopio");
  /*
   Serial.print("acelerometro x: ");
  Serial.print(acelerometro.x);
  Serial.print(" | y: ");
  Serial.print(acelerometro.y);
  Serial.print(" | z: ");
  Serial.println(acelerometro.z);
  Serial.print("giroscopio   x: ");
  Serial.print(giroscopio.x);
  Serial.print(" | y: ");
  Serial.print(giroscopio.y);
  Serial.print(" | z: ");
  Serial.println(giroscopio.z);
  */
  //  delay(1000000);
  //  Serial.println(tempo_obj.getFormattedTime());
  MQTT.publish("inicia", "{}");
  MQTT.publish("Status", "{\"estado\":\"ok\"}");
  digitalWrite(LED_BUILTIN, HIGH);
}
int mensagem_tipo = 0,i=0;
bool was_button_pushed = false, was_save = true, alarm_on = false;

/*
*  Essa função escreve em 1 dos arquivos de historico, Sendo eles 2 e em qual arquivo escrever é definido pela bool, sendo escrito o instante que a escrita foi feita e a mensagem passada no text
* @param arquivo, bool indicando qual arquivo deve ser escrito, por decisao de projeto exitem 2 arquivos ( true para "arquivo1.txt" e false para "arquivo2.txt" )
* @param textm String contendo o texto que sera esrito
*/
void escreve_no_arquivo( bool arquivo, String text){
  File file;
  if(arquivo){
    file = SPIFFS.open("arquivo1.txt", "a");
  }else{
    file = SPIFFS.open("arquivo2.txt", "a");
  }
  
  String datetime = tempo_obj.getFormattedTime();
  /*int T_index = datetime.indexOf("T");
  String day_String = datetime.substring(0, T_index);
  Serial.print(day_String);
  Serial.print(" ");*/
  file.printf("%s %s\n",datetime.c_str(),text.c_str());
  Serial.print(datetime);
  Serial.print(" ");
  Serial.println(text);  
  file.close();
}


bool day_change = false;

/**
  É uma função padrao do arduino e sera executada infinitamente apos o setup até o despositivo ser desligado
**/
void loop() {
  // se nao estiver conectado a amazon conecte
  if (!MQTT.connected()) {
    conectar_mqtt();
  }
  MQTT.loop();
  while(!tempo_obj.update()) {
    tempo_obj.forceUpdate();
  }
  was_save = was_button_pushed;
  was_button_pushed = digitalRead(0) == 0 ? true : false;
  if ( was_button_pushed && !was_save) {
    // Coloca codigo do botao precionado
    if (!alarm_on) {
      change_state(stat_is_alarm ? false : true);
//      Serial.println("mudando modo operação");
    }
  }
  old_ace = acelerometro; // salvamos o ultimo esta das variaveis acelerometro
  old_giro = giroscopio; // e giroscopio
  while (Serial.available() > 0) { // caso exista algo escrito no buffer de entrada
    String str = Serial.readStringUntil(' ');// lemos esse buffer quebrando a string nos ' '
    int a = str.toInt(); // transformamos esse valor em int
    // adepender da variavel i salvamos o x,y,z do acelerometro ou giroscopio
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
      i = -1;// quando chegamos no z do giroscopio resetamos i pra -1 pq na linha abaixo via soma 1 e ir pra 0
    }
    i++;
  }
/*
  Serial.print("acelerometro x: ");
  Serial.print(acelerometro.x);
  Serial.print(" | y: ");
  Serial.print(acelerometro.y);
  Serial.print(" | z: ");
  Serial.println(acelerometro.z);
  Serial.print("giroscopio   x: ");
  Serial.print(giroscopio.x);
  Serial.print(" | y: ");
  Serial.print(giroscopio.y);
  Serial.print(" | z: ");
  Serial.println(giroscopio.z);
  */
  unsigned long now = tempo_obj.getEpochTime();// verificamos em qual instante estamos
  was_save = was_button_pushed;
  was_button_pushed = digitalRead(0) == 0 ? true : false;
  if (stat_is_alarm) {// se estivermos no modo de alarme verificamos se os valores do estado antigo é igual ao do novo estado lido
    if (old_ace.x != acelerometro.x || old_ace.y != acelerometro.y || old_ace.z != acelerometro.z) {// casso os valores sejam diferentes indicamos o roubo, tornando true a flag do alarme
      alarm_on = true;
      problema = "Roubo";
      Serial.println("roubo detectado");
    }else{
      problema = "";
    }
  } else {// caso o modo nao seja o alarme ( sendo assim detecção de acidentes )
    if (acelerometro.y > 270 && acelerometro.z < 360) {// verificamos se os sensores indicam algum acidente
      problema = "Tombou para a esquerda"; // caso os sensores indiquem algum acidente o alarme é acionado e é salvo uma indicação de qual o acidente detectado
      alarm_on = true;
    } else if (acelerometro.y < 180 && acelerometro.z < 360) {
      problema = "Tombou para a direita";
      alarm_on = true;
    } else if (acelerometro.x < 180 && acelerometro.z < 360) {
      problema = "Tombou para trás";
      alarm_on = true;
    } else if (acelerometro.x > 270 && acelerometro.z < 360) {
      problema = "Tombou para frente";
      alarm_on = true;
    } else if (acelerometro.z < 300) {
      problema = "Capotado";
      alarm_on = true;
    } else {
      problema = "";
    }
    problema != ""? Serial.println(problema):Serial.print(problema);
  }
// Se o alarme tiver sido ligado por alguma situação detectada nos sensores
if (alarm_on) { // se o alarme estiver ligado
    Serial.println("Alarme ligado");
    MQTT.publish("Status", "{\"estado\":\"alarme\"}"); // publicamos alarme ligado
    escreve_no_arquivo(arquivo, "alarme ligado por : "+problema);
    unsigned long espera = now + 60;// definimos o tempo defaut de espera de 60 segundos para o usuario aperta o botao
    while ( now < espera ) {// emquanto esperamos caso se desconecte do broker do MQTT vamos reconectar
      if (!MQTT.connected()) {
        conectar_mqtt();
      }
      //piscamos o led para indicar alarme ligado
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      //se o botao for apertado quebramos o loop e mudamos a flag do alarme pra falso assim pulando o envio do pedido de socorro
      if ( was_button_pushed && !was_save) {
        Serial.println("botao precionado");
        MQTT.publish("Status", "{\"estado\":\"ok\"}");// publicamos que esta tudo ok
        escreve_no_arquivo(arquivo, "botao precionado");
        last_mensage_time = now;
        alarm_on = false;
        break;
      }
      was_save = was_button_pushed;
      was_button_pushed = digitalRead(0) == 0 ? true : false;
      now = tempo_obj.getEpochTime();
      if (now > wait_time + last_mensage_time) {// se em quanto esperamos o tempo do usuario aperta o botao for nescessario envia uma mensagem essa parte fara isso
        MQTT.publish("Status", "{\"estado\":\"alarme\"}");
        last_mensage_time = now;
        Serial.println("publicando alarme ligado");
      }
    }
    if (alarm_on) {// quando o loop acaba se o usuario nao apertou o botao a flag ainda é true entao "fazemos" a ligação
      Serial.println("Problema confirmado/ botao nao apertado");
        escreve_no_arquivo(arquivo, "botao nao precionado precionado, Problema detectado!!!");
      problema_confirmado(); // para publicar problema e salver no historico de eventos
      alarm_on = false;
      // para indicar a ligação
      digitalWrite(LED_BUILTIN, LOW);
      delay(3000);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  
  if (now > wait_time + last_mensage_time) {// caso esteja na hora de manda a mensagem pra confirma que a placa esta conectada
    Serial.println("publicando estatus");
    if ( problema == "") {// se nenhum problema tenha sido identificado publicamos ok
      MQTT.publish("Status", "{\"estado\":\"ok\"}");
    } else {// se nao publicamos que o alarme esta ligado
      MQTT.publish("Status", "{\"estado\":\"alarme\"}");
        escreve_no_arquivo(arquivo, problema);
    }
      last_mensage_time = now;
  }
  if( tempo_obj.getFormattedTime() == "00:00:00" && day_change ){
    arquivo = !arquivo;
    day_change = !day_change;
    File sys = SPIFFS.open("sys.txt", "w");
    sys.printf("%d\n",arquivo);
    sys.close();
    // fazemos isso pro caso do arquivo nao ter sido criado ainda
    File file;
    // limpamos o arquivo que sera escrito
    if(arquivo){
      file = SPIFFS.open("arquivo1.txt", "w");
    }else{
      file = SPIFFS.open("arquivo2.txt", "w");
    }  
    file.close();
  }else if( tempo_obj.getFormattedTime() == "00:00:10" && !day_change ){
    day_change = !day_change;
  }
  //le_arquivo_full( arquivo? "arquivo1.txt":"arquivo2.txt" );
}
