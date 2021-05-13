from django.contrib import admin
from .models import *
# Register your models here.
@admin.register(Dados)
class DadosAdmin(admin.ModelAdmin):
    list_display = ( "ultima_mensagem","tempo_de_espera",)
@admin.register(Status)
class StatusAdmin(admin.ModelAdmin):
    list_display = ( "status",)