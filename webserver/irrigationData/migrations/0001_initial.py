# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='CropData',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('crop_name', models.CharField(max_length=20)),
                ('root_depth', models.IntegerField()),
                ('max_depl', models.DecimalField(max_digits=3, decimal_places=2)),
            ],
        ),
        migrations.CreateModel(
            name='SoilData',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('soil_type', models.CharField(max_length=20)),
                ('perm_wilt_point', models.DecimalField(max_digits=3, decimal_places=2)),
                ('field_capacity', models.DecimalField(max_digits=3, decimal_places=2)),
            ],
        ),
    ]
