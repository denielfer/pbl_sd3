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
        "tempo_espera_s":ts*500 if ts <10 else 5000,
        "last_mensage":a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
        "modo": a.state,
    }
    #print(context)
    return HttpResponse(template.render(context,request))

@csrf_exempt
def set_tempo(request):
    if(request.method == "POST"):
#        print(request.POST)
        a,b = get_dados()
        t = request.POST.get("tempo")
        t = str(t)
        t = int(t[0:2])*3600+int(t[3:5])*60+int(t[6:8])
        a.tempo_de_espera = t
        a.save()
        m = {"timer":t,"is_not_site":0}
        MQTT.publish("set_timer",m)
        #print(m)
    return redirect('monitorador:tela_inicial')

def get_dados():
    a = Dados.objects.all()
    b = Status.objects.all()
    return a[0],b[0]

@csrf_exempt
def get_update(request):
    a,b = get_dados()
#    print(b.status,"  ",a.ultima_mensagem)
    a.ultima_mensagem +=timedelta( hours= -3 )
    response = {
        'status' : b.status,
        "last_mensage": a.ultima_mensagem.strftime("%d/%m/%Y, %H:%M:%S"),
        "state": a.state
    }
#    print(response)
    return JsonResponse(response,safe=True)

@csrf_exempt
def mudar_modo(request):
    if(request.method == "POST"):
        state = (request.POST.get("estado") == "true")
        #print(state)
#        a.state = state
#        a.save()
        m = { "estado":1 if(state) else 0,"is_not_site":0}
        MQTT.publish("set_state",m)
    return redirect('monitorador:tela_inicial')
