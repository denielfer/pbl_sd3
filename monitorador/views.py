#from django.shortcuts import render
from .models import *
from django.template import loader
from django.http import HttpResponse, JsonResponse
from django.shortcuts import redirect
from MQTT import MQTT
from django.views.decorators.csrf import csrf_exempt
#from . import analista

def tela(request):
    a = get_dados()
    ts,s = a.tempo_de_espera,a.status
    t = f'{int(ts/3600):02}:{int((ts%3600)/60):02}:{int(ts%60):02}'
    template = loader.get_template("base.html")
    context = {
        "status":s,
        "tempo_espera":t,
        "tempo_espera_s":ts,
        "last_mensage":a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
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

def get_update(request):
    a = get_dados()
    print(a.status,"  ",a.ultima_mensagem)
    response = {
        'status' : a.status,
        "last_mensage": a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
        "status_code": 200,
    }
    print(response)
    return JsonResponse(response,safe=True)