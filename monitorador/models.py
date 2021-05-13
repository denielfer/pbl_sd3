from django.db import models

# Create your models here.
class Dados(models.Model):
    ultima_mensagem = models.DateTimeField()
    tempo_de_espera = models.IntegerField()
    state = models.BooleanField( default = True )

class Status(models.Model):
    status = models.CharField(max_length=100)