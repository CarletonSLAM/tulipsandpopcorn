# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('irrigationData', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='ReferenceEvapotranspirateion',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('transpiration_val', models.DecimalField(max_digits=4, decimal_places=2)),
                ('month', models.CharField(max_length=3, choices=[(b'Jan', b'Jan'), (b'Feb', b'Feb'), (b'Mar', b'Mar'), (b'Apr', b'Apr'), (b'May', b'May'), (b'Jun', b'Jun'), (b'Jul', b'Jul'), (b'Aug', b'Aug'), (b'Sep', b'Sep'), (b'Oct', b'Oct'), (b'Nov', b'Nov'), (b'Dec', b'Dec')])),
                ('stateUSA', models.CharField(max_length=20)),
            ],
        ),
        migrations.AddField(
            model_name='soildata',
            name='infiltration_rate',
            field=models.DecimalField(default=0, max_digits=5, decimal_places=2),
        ),
        migrations.AlterField(
            model_name='soildata',
            name='field_capacity',
            field=models.DecimalField(default=0, max_digits=3, decimal_places=2),
        ),
        migrations.AlterField(
            model_name='soildata',
            name='perm_wilt_point',
            field=models.DecimalField(default=0, max_digits=3, decimal_places=2),
        ),
    ]
