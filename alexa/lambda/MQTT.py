import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
import json

client = AWSIoTPyMQTT.AWSIoTMQTTClient("meu_client")
status = "Desconectado"
state = "Alarme"
flag = False

def publish(topic, msg):
    client.publish(topic, str(msg), 0)

def subscribe(topic, qos, callback):
    client.subscribe(topic, qos, callback)

def iniciar():
    client.configureEndpoint("a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com", 8883)
    client.configureCredentials("AmazonRootCA.pem", 
                                "665e74a714-private.pem.key",
                                "665e74a714-certificate.pem.crt")

def conectar():
    client.connect()

def __set_status__(client, userdata, mensage):
    msg = json.loads(mensage.payload.decode("utf-8"))
    status = msg["status"]
    status = msg["state"]
    flag = True

def subs():
    subscribe("send_status_alexa",0,__set_status__)