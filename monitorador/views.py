#from django.shortcuts import render
from .models import *
from django.template import loader
from django.http import HttpResponse, JsonResponse
from django.shortcuts import redirect
from MQTT import MQTT
from datetime import timedelta
from django.views.decorators.csrf import csrf_exempt
from . import analista

def tela(request):
    a,b = get_dados()
    ts,s = a.tempo_de_espera,b.status
    t = f'{int(ts/3600):02}:{int((ts%3600)/60):02}:{int(ts%60):02}'
    template = loader.get_template("base.html")
    context = {
        "status":s,
        "tempo_espera":t,
        "tempo_espera_s":ts*500,
        "last_mensage":a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
    }
#    print(a)
    return HttpResponse(template.render(context,request))

def set_tempo(request):
    if(request.method == "POST"):
#        print(request.POST)
        a,b = get_dados()
        t = request.POST.get("tempo")
        t = int(t[0:2])*3600+int(t[3:5])*60+int(t[6:8])
        a.tempo_de_espera = t
        a.save()
        m = {"timer":t}
        MQTT.publish("set_timer",m)
        print(m)
    return redirect('monitorador:tela_inicial')

def get_dados():
    a = Dados.objects.all()
    b = Status.objects.all()
    return a[0],b[0]

def get_update(request):
    a,b = get_dados()
#    print(b.status,"  ",a.ultima_mensagem)
    a.ultima_mensagem +=timedelta( hours= -3 )
    response = {
        'status' : b.status,
        "last_mensage": a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
        "status_code": 200,
    }
#    print(response)
    return JsonResponse(response,safe=True)