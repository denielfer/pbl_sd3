# Generated by Django 3.1.5 on 2021-05-09 21:29

from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Dados',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('ultima_mensagem', models.DateTimeField()),
                ('tempo_de_espera', models.IntegerField()),
            ],
        ),
    ]
