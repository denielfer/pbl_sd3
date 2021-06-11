import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
from monitorador.models import *
import json
from django.utils import timezone
from os import getcwd

client = AWSIoTPyMQTT.AWSIoTMQTTClient("meu_client")
#/home/ubuntu/sd_pbl2/IoT/
pwd = getcwd()
client.configureEndpoint("a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com", 8883)
client.configureCredentials(f"{pwd}/MQTT/certificados/AmazonRootCA_site.pem", 
                            f"{pwd}/MQTT/certificados/f1a9b69ac4-private.pem.key",
                            f"{pwd}/MQTT/certificados/f1a9b69ac4-certificate.pem.crt")

client.connect()
#print("Client Connected")

def publish(topic, msg):
    client.publish(topic, str(msg), 0)

def subscribe(topic, qos, callback):
    client.subscribe(topic, qos, callback)

def __status__(client, userdata, mensage):
    msg = json.loads(mensage.payload.decode("utf-8"))
    print(msg)
    a = Dados.objects.all()
    b = Status.objects.all()
    a,b = a[0],b[0]
    b.status= msg["estado"]
    if(msg["should_update"]):
        a.tempo_de_espera = msg["tempo"]+1
        a.state = msg["modo"]
    a.ultima_mensagem= timezone.now()
    a.save()
    b.save()
    m = {"estado":1 if a.state else 0,"is_not_site":0}
    publish("set_state",m)
    
    #print(type(msg),"  ",msg)

def __iniciar__(client, userdata, mensage):
    a = Dados.objects.all()
    a = a[0]
    m = {"timer":a.tempo_de_espera,"is_not_site":0}
    publish("set_timer",m)
    m = {"estado":1 if a.state else 0,"is_not_site":0}
    publish("set_state",m)
    print(m)

def __set_state__(client, userdata, mensage):
    a = Dados.objects.all()[0]
    msg = json.loads(mensage.payload.decode("utf-8"))
    a.state = (msg["modo"] == 1)
    a.save()
    print(f'estado mudado para {a.state}')

subscribe(f'Status',0, __status__)
subscribe(f'inicia',0, __iniciar__)
subscribe(f'state',0, __set_state__)
