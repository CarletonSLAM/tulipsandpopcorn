from django.db import models
from model_utils.managers import PassThroughManager
from model_utils import Choices

class SoilQuerySet(models.QuerySet):

    def getDataForSoilType(self, soil):
        return self.filter(soil_type=soil)


class SoilData(models.Model):
    soil_type = models.CharField(max_length=20)
    perm_wilt_point = models.DecimalField(max_digits=3, decimal_places=2, default = 0)
    field_capacity = models.DecimalField(max_digits=3, decimal_places=2, default = 0)
    infiltration_rate = models.DecimalField(max_digits=5, decimal_places=2, default =0)

    objects = PassThroughManager.for_queryset_class(SoilQuerySet)()


class CropQuerySet(models.QuerySet):

    def getDataForCropType(self, crop):
        return self.filter(crop_name=crop)


class CropData(models.Model):
    crop_name = models.CharField(max_length=20)
    root_depth = models.IntegerField()
    max_depl = models.DecimalField(max_digits=3, decimal_places=2)
    crop_coeff = models.DecimalField(max_digits=4, decimal_places=3, default=0)

    objects = PassThroughManager.for_queryset_class(CropQuerySet)()


class ReferenceEvapotranspirateion(models.Model):
    transpiration_val = models.DecimalField(max_digits=4, decimal_places=2)
    month_choices = Choices('Jan',
                            'Feb',
                            'Mar',
                            'Apr',
                            'May',
                            'Jun',
                            'Jul',
                            'Aug',
                            'Sep',
                            'Oct',
                            'Nov',
                            'Dec')
    month = models.CharField(choices = month_choices, max_length=3)
    stateUSA = models.CharField(max_length = 20)

    class ReferenceEvapotranspirateionQuerySet(models.QuerySet):

        def getValuesByMonth(self, month):
            return self.filter(month=month)

        def getValuesByState(self, state):
            return self.filter(stateUSA = state)

    objects = PassThroughManager.for_queryset_class(ReferenceEvapotranspirateionQuerySet)()


# Create your models here.
