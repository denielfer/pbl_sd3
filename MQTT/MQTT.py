import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
from monitorador.models import Dados
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
    a = a[0]
    a.status= msg["estado"]
    a.ultima_mensagem= timezone.now()
    a.save()
    print(type(msg),"  ",msg)

subscribe(f'Status',0, __status__)