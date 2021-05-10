import threading
from datetime import timedelta
from django.utils import timezone
from .models import *

def analizador():
    print("analista iniciado")
    while True:
        a = Dados.objects.all()
        a = a[0]
        if(a.ultima_mensagem + timedelta( seconds=a.tempo_de_espera ) > timezone.now()):
            a.status = "desconectado"
            a.save()

threading.Thread(target=analizador).start()
#    ultima_mensagem = models.DateTimeField()
#    tempo_de_espera = models.IntegerField()
#    status = models.CharField(max_length=100)