from django.db import models
from django.conf import settings
from django.contrib.auth.models import User, AbstractUser, BaseUserManager
from model_utils.fields import StatusField, MonitorField
from model_utils import Choices, FieldTracker
from jsonfield import JSONField
from django.db.models.signals import pre_save
from django.utils.translation import gettext as _
from utils import Response, constant
from model_utils.managers import PassThroughManager, PassThroughManagerMixin
import enum

class ViewStates(enum.Enum):
    hello = 1
    viewStateDelegator = 2
    giveMoistProf = 3
    checkMositureProfile = 4
    calcSleepTime = 5

class Status(enum.Enum):
    OK = 0
    ERROR = 0

class LastRequestStorable(models.Model):
    last_service_request = JSONField()
    req_tracker = FieldTracker()
    last_request_changed = MonitorField(monitor='last_service_request')

    def getlastReqAsDict(self):
        return self.last_service_request

    def lastServiceRequestHasChanged(self):
        return self.req_tracker.has_changed('last_service_request')

    def getPreviousLastReqAsDict(self):
        return self.req_tracker.previous('last_service_request')

    def getChangedTime(self):
        return self.last_request_changed

    class Meta:
        abstract = True


class MapPointable(models.Model):
    _loc_latitude = models.FloatField(null = True)
    _loc_longtitude = models.FloatField(null = True)

    @property
    def loc_latitude(self):
        return self._loc_latitude

    @loc_latitude.setter
    def loc_latitude(self, value):
        if self._loc_latitude == None:
            self._loc_latitude = value
        else:
            raise ValueError("Read-only field, the value cannot be set")

    @property
    def loc_longtitude(self):
        return self._loc_longtitude

    @loc_longtitude.setter
    def loc_longtitude(self, value):
        if self._loc_longtitude == None:
            self._loc_longtitude = value
        else:
            raise ValueError("Read-only field, the value cannot be set")

    class Meta:
        abstract = True

class FarmStateble(models.Model):
    STATUS = Choices(str(ViewStates.hello),
                     str(ViewStates.viewStateDelegator),
                     str(ViewStates.giveMoistProf),
                     str(ViewStates.checkMositureProfile),
                     str(ViewStates.calcSleepTime))
    state = StatusField()
    state_tracker = FieldTracker(fields=['state'])

    class Meta:
        abstract = True

class PassThroughFarmerManager(PassThroughManagerMixin, BaseUserManager):
    use_in_migrations = False

    def create_user(self, username, last_service_request, loc_latitude, loc_longtitude, password=None):
        user = self.model(username=username,
                          last_service_request=last_service_request, 
                          loc_latitude=loc_latitude, 
                          loc_longtitude=loc_longtitude)
        user.set_password(password)
        user.save(using=self._db)
        return user

class Farmer(FarmStateble, MapPointable, LastRequestStorable, AbstractUser):
    last_irrigation_cycle = models.DateTimeField(auto_now_add=False, auto_now=False, null = True, blank=True)

    class FarmerQuerySet(models.QuerySet):

        #td - timedelta object of 'how long ago' to filter requests from
        #eg. models.Farmer.objects.getOverdueIrrigationCycleCheck(datetime.timedelta(minutes=5))
        def getOverdueIrrigationCycleCheck(self, td):
            from datetime import timedelta
            from django.utils import timezone
            search_delta = timezone.now() - td
            return self.filter(last_irrigation_cycle__lte = search_delta)

        def getOverdueJsonRequests(self, td):
            from datetime import timedelta
            from django.utils import timezone
            search_delta = timezone.now() - td
            return self.filter(last_request_changed__lte= search_delta)

        def getOverdueGeneral(self, td):
            return self.getOverdueIrrigationCycleCheck(td).getOverdueJsonRequests(td)

    objects = PassThroughFarmerManager.for_queryset_class(FarmerQuerySet)()

    def getLastIrrigationCycleDate(self):
        return self.last_irrigation_cycle

class MoistureProfileDict(models.Model):
    name = models.CharField(max_length=30)

    class MoistureProfileDict(models.QuerySet):

        def getBelowMADKVPairs(self, owner, mad):
            mdict = self.get(name = owner).select_related()
            return mdict.MoistureKVPair__set.filter(sector_value__lte = mad)

        def getItemsAsQsetInDict(self, owner):
            mdict = self.get(name = owner).select_related()
            return mdict.MoistureKVPair__set.all()

    objects = PassThroughManager.for_queryset_class(MoistureProfileDict)

    def count(self):
        return self.MoistureKVPair__set.count()

    def containsKey(self, key):
        try:
            self.MoistureKVPair__set.get(sector_key = key)
            return True
        except MoistureKVPair.DoesNotExist:
            return False

    def clear(self):
        self.MoistureKVPair__set.all().delete()

    def delete(self, key):
        self.MoistureKVPair__set.get(sector_key = key).delte()

    def getAsDict(self):
        profile = dict()
        for moist_kvp in self.MoistureKVPair__set.all():
            profile[moist_kvp.sector_key] = moist_kvp.sector_value
        return profile

    def getPairsIterable(self):
        return [(moist_kvp.sector_key, moist_kvp.sector_value) for moist_kvp in self.moisturekvpair_set.all()]

    def getKeysIterable(self):
        return [moist_kvp.sector_key for moist_kvp in self.moisturekvpair_set.all()]

    def getValuesIterable(self):
        return [moist_kvp.sector_value for moist_kvp in self.moisturekvpair_set.all()]

class MoistureKVPair(models.Model):
    profile = models.ForeignKey(MoistureProfileDict)
    sector_key = models.CharField(max_length=30)
    sector_value = models.DecimalField(max_digits=4, decimal_places=3, default=0)


class BotanicData(models.Model):
    soil_choices = Choices( ('LOAM', _('Loam')),
                            ('SILT', _('Silt')),
                            ('CLAY', _('Sand')) )
    soil_type = models.CharField(choices= soil_choices, blank=True, max_length=20)
    seed_choices = Choices( 'TOMATOE',
                            'POTATOE',
                            'CARROT',
                            'STRAWBERRY',
                            'CHICKEN' )
    seed = models.CharField(choices=seed_choices, blank = True, max_length=15)
    root_depth = models.IntegerField(default=0)
    dry_point = models.DecimalField(max_digits=4, decimal_places=3, default=0)
    _crop_coeff = models.DecimalField(max_digits=4, decimal_places=3, default=0)
    _field_capacity = models.DecimalField(max_digits=3, decimal_places=3, default=0)
    _perm_wilt_point = models.DecimalField(max_digits=3, decimal_places=3, default=0)
    _max_depl = models.DecimalField(max_digits=3, decimal_places=3, default=0)

    @property
    def field_capacity(self):
        return self._field_capacity

    @field_capacity.setter
    def field_capacity(self, value):
        self._field_capacity = value

    @property
    def perm_wilt_point(self):
        return self._perm_wilt_point

    @perm_wilt_point.setter
    def perm_wilt_point(self, value):
        self._perm_wilt_point= value

    @property
    def max_depl(self):
        return self._max_depl

    @max_depl.setter
    def max_depl(self, value):
        self._max_depl= value

    @property
    def crop_coeff(self):
        return self._crop_coeff

    @crop_coeff.setter
    def field_capacity(self, value):
        self._crop_coeff = value

    @constant
    def _soilDataWebServiceURL(inst):
        return "http://192.168.1.32:3000/getSoilData/"

    @constant
    def _cropDataWebServiceURL(inst):
        return "http://192.168.1.32:3000/getCropData/"

def determineBotanicData(sender, instance, signal, *args, **kwargs):
    #requesting data server for soil data
    if instance.pk is None:
        sb_http_req_data = '{"soilType":"' + instance.soil_type + '\"}'
        res = Response(url = instance._soilDataWebServiceURL, data = sb_http_req_data, method = "POST")
        print "in here"
        instance.field_capacity = float(res.resData["fields"]["field_capacity"])
        instance.perm_wilt_point = float(res.resData["fields"]["perm_wilt_point"])
    #requesting data server for crop data
    '''
    sb_http_req_data = '{"cropName":"' + instance.seed + '\"}'
    res = Response(url = instance._soilDataWebServiceURL, data = sb_http_req_data, method = "POST")
    instance.field_capacity = float(res.resData["fields"]["field_capacity"])
    instance.perm_wilt_point = float(res.resData["fields"]["perm_wilt_point"])
    '''

pre_save.connect(determineBotanicData, sender=BotanicData)

class Analytics(models.Model):
    _power_used = models.DecimalField(max_digits=7, decimal_places=2, default=0)
    _water_used = models.DecimalField(max_digits=7, decimal_places=2, default=0)
    _water_saved = models.DecimalField(max_digits=7, decimal_places=2, default=0)

    @property
    def power_used(self):
        return self._power_used

    @power_used.setter
    def power_used(self, value):
        if self.power_used > value:
            self._power_used = value
        else:
            raise ValueError("Power consumed must always be set to something greater")

    def incWaterSaved(self, value):
        if type(value) == float:
            if value > 0:
                self.water_saved = models.F('water_saved') + value
            else:
                raise ValueError("Water used must always be set to something greater")
        else:
            raise TypeError("can only change value by type float")

    @property
    def water_used(self):
        return self._water_used

    @power_used.setter
    def water_used(self, value):
        if self.water_used > value:
            self._water_used = value
        else:
            raise ValueError("Water used must always be set to something greater")

    def incWaterUsed(self, value):
        if type(value) == float:
            if value > 0:
                self.water_used = models.F('water_used') + value
            else:
                raise ValueError("Water used must always be set to something greater")
        else:
            raise TypeError("can only change value by type float")

    @property
    def water_saved(self):
        return self._water_saved

    @water_saved.setter
    def water_saved(self, value):
        self._water_saved = value

    def changeWaterSavedBy(self, value):
        if type(value) == float:
            self.water_saved = models.F('water_saved') + value
        else:
            raise TypeError("can only change value by type float")


class IrrigationSession(models.Model):
    user = models.OneToOneField(settings.AUTH_USER_MODEL)
    botany = models.OneToOneField(BotanicData)
    analytics = models.OneToOneField(Analytics)
    sector1_moist_prof = models.OneToOneField(MoistureProfileDict, related_name="sess1")
    sector2_moist_prof = models.OneToOneField(MoistureProfileDict, related_name="sess2")
    sector3_moist_prof = models.OneToOneField(MoistureProfileDict, related_name="sess3")

    def getDriestValue(self):
        return self.botany.dry_point
# Create your models here.
