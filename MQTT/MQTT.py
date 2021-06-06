import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
from monitorador.models import *
import json
from django.utils import timezone
from os import getcwd

client = AWSIoTPyMQTT.AWSIoTMQTTClient("meu_client")
#/home/ubuntu/sd_pbl2/IoT/
pwd = getcwd()
client.configureEndpoint("a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com", 8883)
client.configureCredentials(f"{pwd}\\MQTT\\certificados\\AmazonRootCA_site.pem", 
                            f"{pwd}\\MQTT\\certificados\\f1a9b69ac4-private.pem.key",
                            f"{pwd}\\MQTT\\certificados\\f1a9b69ac4-certificate.pem.crt")

client.connect()
#print("Client Connected")

def publish(topic, msg):
    client.publish(topic, str(msg), 0)

def subscribe(topic, qos, callback):
    client.subscribe(topic, qos, callback)

def __status__(client, userdata, mensage):
    msg = json.loads(mensage.payload.decode("utf-8"))
    a = Dados.objects.all()
    b = Status.objects.all()
    a,b = a[0],b[0]
    b.status= msg["estado"]
    a.ultima_mensagem= timezone.now()
    a.save()
    b.save()
    #print(type(msg),"  ",msg)

def __iniciar__(client, userdata, mensage):
    a = Dados.objects.all()
    a = a[0]
    m = {"timer":a.tempo_de_espera}
    publish("set_timer",m)
    m = {"estado":1 if a.state else 0}
    publish("set_state",m)
    print(m)

def __get_status_alexa__(client, userdata, mensage):
    print("Chegou request da alexa pra obter informações de status e state")
    b = Status.objects.all()[0]
    a = Dados.objects.all()[0]
    m={
        "status":b.status,
        "state":"Alarme" if a.state else "Detector de acidentes"
    }
    publish("send_status_alexa",m)

def __set_state_alexa__(client, userdata, mensage):
    print("Chegou mensagem da alexa pra seta State")
    msg = json.loads(mensage.payload.decode("utf-8"))
    publish("set_state",msg)

def __set_timer_alexa__(client, userdata, mensage):
    print("Chegou mensagem da alexa pra seta tempo")
    a = Dados.objects.all()[0]
    msg = json.loads(mensage.payload.decode("utf-8"))
    a.tempo_de_espera = msg["timer"]
    a.save()
    publish("set_timer",msg)

def __set_state__(client, userdata, mensage):
    a = Dados.objects.all()[0]
    msg = json.loads(mensage.payload.decode("utf-8"))
    a.state = (msg["modo"] == 1)
    a.save()
    print(f'estado mudado para {a.state}')

subscribe(f'Status',0, __status__)
subscribe(f'inicia',0, __iniciar__)
subscribe(f'set_timer_alexa_p',0, __set_timer_alexa__)
subscribe(f'state',0, __set_state__)