import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
from monitorador.models import *
import json
from django.utils import timezone

client = AWSIoTPyMQTT.AWSIoTMQTTClient("meu_client")
#C:\\Users\\dfc15\\Desktop\\materias\\SD\\Problema-3\\pbl_sd3\\monitoramento\\MQTT\\certificados\\
#/home/ubuntu/sd_pbl2/IoT/
client.configureEndpoint("a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com", 8883)
client.configureCredentials("C:\\Users\\dfc15\\Desktop\\materias\\SD\\Problema-3\\pbl_sd3\\monitoramento\\MQTT\\certificados\\AmazonRootCA1.pem", 
                            "C:\\Users\\dfc15\\Desktop\\materias\\SD\\Problema-3\\pbl_sd3\\monitoramento\\MQTT\\certificados\\a504e91ba6-private.pem.key",
                            "C:\\Users\\dfc15\\Desktop\\materias\\SD\\Problema-3\\pbl_sd3\\monitoramento\\MQTT\\certificados\\a504e91ba6-certificate.pem.crt")

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
    b = Status.objects.all()[0]
    m={
        "status":b.status,
        "state":"Alarme" if b.state else "Detector de acidentes"
    }
    publish("send_status_alexa",m)

def __set_state_alexa__(client, userdata, mensage):
    msg = json.loads(mensage.payload.decode("utf-8"))
    publish("set_state",msg)

def __set_timer_alexa__(client, userdata, mensage):
    a = Dados.objects.all()[0]
    msg = json.loads(mensage.payload.decode("utf-8"))
    a.tempo_de_espera = msg["timer"]
    a.save()
    publish("set_timer",msg)

def __set_state__(client, userdata, mensage):
    a = Dados.objects.all()[0]
    msg = json.loads(mensage.payload.decode("utf-8"))
    a.state = (msg["modo"] == 0)
    a.save()
    print(f'estado mudado para {a.state}')

subscribe(f'Status',0, __status__)
subscribe(f'inicia',0, __iniciar__)
subscribe(f'get_status_alexa',0, __get_status_alexa__)
subscribe(f'set_state_alexa',0, __set_state_alexa__)
subscribe(f'set_timer_alexa',0, __set_timer_alexa__)
subscribe(f'state',0, __set_state__)