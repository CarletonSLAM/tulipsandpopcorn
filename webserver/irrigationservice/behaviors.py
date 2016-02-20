from django.db import models
from django_extensions.db.fields import UUIDField, CharField
from django.contrib.auth.models import User

class Ownable(models.Model):
    user = models.ForeignKey(User)

    class Meta:
        abstract = True

class FarmAnalyzable(models.Model):
    water_saved = models.DecimalField(default = 0, max_digits=5, decimal_places = 2)
    water_used = models.DecimalField(default = 0, max_digits=5, decimal_places = 2)
    power_consumed = models.DecimalField(default = 0, max_digits=5, decimal_places = 2)

    class Meta:
        abstract = True

class MositureSensible(models.Model):
    
