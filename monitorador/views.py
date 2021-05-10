#from django.shortcuts import render
from .models import *
from django.template import loader
from django.http import HttpResponse
from django.shortcuts import redirect
from MQTT import MQTT
from django.views.decorators.csrf import csrf_exempt
import analista.py

def tela(request):
    a = get_dados()
    t,s = a.tempo_de_espera,a.status
    t = f'{int(t/3600):02}:{int((t%3600)/60):02}:{int(t%60):02}'
    template = loader.get_template("base.html")
    context = {
        "status":s,
        "tempo_espera":t,
        "last_mensage":a.ultima_mensagem,
    }
#    print(a)
    return HttpResponse(template.render(context,request))

def set_tempo(request):
    if(request.method == "POST"):
#        print(request.POST)
        a = get_dados()
        t = request.POST.get("tempo")
        t = int(t[0:2])*3600+int(t[3:5])*60+int(t[6:8])
        a.tempo_de_espera = t
        a.save()
        m = {"timer":t}
        MQTT.publish("set_timer",m)
    return redirect('monitorador:tela_inicial')

def get_dados():
    a = Dados.objects.all()
    return a[0]

def get_update():
    a = get_dados()
    retorno = {
        'status' : a.status,
        "last_mensage": a.ultima_mensagem,
    }
    print(retorno)
    return retorno