import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT
import json

client = AWSIoTPyMQTT.AWSIoTMQTTClient("meu_client")

def iniciar():
    client.configureEndpoint("a1hhzdnhqam0eu-ats.iot.us-east-1.amazonaws.com", 8883)
    client.configureCredentials("AmazonRootCA1.pem", 
                                "a504e91ba6-private.pem.key",
                                "a504e91ba6-certificate.pem.crt")

def conectar():
    client.connect()

def publish(topic, msg):
    client.publish(topic, str(msg), 0)

