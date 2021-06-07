import threading
from datetime import timedelta
from django.utils import timezone
from .models import *
from time import sleep

def analizador():
    print("analista iniciado")
    while True:
        a = Dados.objects.all()
        b = Status.objects.all()
        a,b = a[0],b[0]
        if(a.ultima_mensagem + timedelta( seconds=a.tempo_de_espera+1 ) < timezone.now()):
#            print()
#            print()
#            print(b.ultima_mensagem + timedelta( seconds=b.tempo_de_espera ), timezone.now())
#            print("editado")
#            print()
#            print()
            if(b.status != "desconectado"):
                b.status = "desconectado"
                b.save()
            sleep(int(a.tempo_de_espera/2))

threading.Thread(target=analizador).start()
#    ultima_mensagem = models.DateTimeField()
#    tempo_de_espera = models.IntegerField()
#    status = models.CharField(max_length=100)