import threading
from datetime import datetime,timedelta
from .models import Dados

def analizador():
    while True:
        a = Dados.objects.all()
        a = a[0]
        if(a.ultima_mensagem + timedelta( seconds=a.tempo_de_espera ) > datetime.now()):
            a.status = "desconectado"
            a.save()

threading.Thread(target=analizador).start()
#    ultima_mensagem = models.DateTimeField()
#    tempo_de_espera = models.IntegerField()
#    status = models.CharField(max_length=100)