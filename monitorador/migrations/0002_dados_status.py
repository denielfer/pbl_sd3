# Generated by Django 3.1.5 on 2021-05-10 10:37

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('monitorador', '0001_initial'),
    ]

    operations = [
        migrations.AddField(
            model_name='dados',
            name='status',
            field=models.CharField(default='', max_length=100),
            preserve_default=False,
        ),
    ]
