from django.urls import path
from django.conf.urls import url
from . import views

app_name = "monitorador"

urlpatterns = [
    url(r'^$', views.tela, name="tela_inicial"),
    url(r'^set_tempo/$', views.set_tempo, name="set_tempo"),
    url(r'^get_update/$', views.get_update, name="get_update"),
]