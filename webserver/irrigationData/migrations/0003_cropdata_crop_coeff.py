# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('irrigationData', '0002_auto_20151201_0244'),
    ]

    operations = [
        migrations.AddField(
            model_name='cropdata',
            name='crop_coeff',
            field=models.DecimalField(default=0, max_digits=4, decimal_places=3),
        ),
    ]
