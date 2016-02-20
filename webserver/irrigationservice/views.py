from django.shortcuts import render
from . import models
from models import ViewStates, Status
from django.http import HttpResponse, JsonResponse, HttpResponseBadRequest
import django.contrib.auth
from django.core import serializers
from utils import Authenticator, switch
import logic
from django.utils import timezone
from datetime import timedelta
from webservices import GoogleMapsWebService, DarkSkyWebService
from django.views.decorators.http import require_http_methods
import json
import http


@require_http_methods(["POST"])
def Hello(request):
    body = json.loads(request.body)
    try:
        rsoil_type = models.BotanicData.soil_choices[body["soilType"]]
        rcrop_name = models.BotanicData.seed_choices[body["cropName"]]
        passw = body["password"]
        loc = body["location"]
    except KeyError:
        return HttpResponseBadRequest("Request data parameters are invalid")

    gmws = GoogleMapsWebService(loc)
    gmws.call()
    lat = gmws.getLatitude()
    lng = gmws.getLongtitude()

    response = dict()

    #initalizing models for new user
    import uuid
    user_name = str(uuid.uuid1())
    f = models.Farmer.objects.create_user(username = user_name,
                                          password = passw,
                                          state = str(ViewStates.hello),
                                          last_service_request = request.body, 
                                          loc_latitude = lat,
                                          loc_longtitude = lng)
    mps1 = models.MoistureProfileDict(name = "moist_prof_sect1")
    mps2 = models.MoistureProfileDict(name = "moist_prof_sect2")
    mps3 = models.MoistureProfileDict(name = "moist_prof_sect3")
    bd = models.BotanicData.objects.create(soil_type = rsoil_type, seed = rcrop_name)
    a = models.Analytics.objects.create()

    #saving models into db so they can be related to irrigationSession
    bd.save()
    a.save()
    f.save()
    mps1.save()
    mps2.save()
    mps3.save()

    isess = models.IrrigationSession.objects.create(user = f,
                                                    botany = bd,
                                                    analytics = a,
                                                    sector1_moist_prof = mps1,
                                                    sector2_moist_prof = mps2,
                                                    sector3_moist_prof = mps3)
    isess.save()
    response["Response"] = {}
    response["Response"]["Username"] = user_name
    response["Response"]["Password"]  = passw
    response["Status"] = str(Status.OK)
    response["Next_State"] = str(ViewStates.viewStateDelegator)
    return JsonResponse(response)


@require_http_methods(["POST"])
def viewStateDelegator(request):
    response = dict()
    user = Authenticator.fromDeserializedRequest(request).user
    if user:
        isess = user.irrigationsession

        response["Next_State"] = str(ViewStates.giveMoistProf)

        if user.state != str(ViewStates.giveMoistProf):
            dsws = DarkSkyWebService(user.loc_latitude, user.loc_longtitude)
            dsws.call()
            ttr = dsws.time_till_rain()
            ttl = logic.time_till_lst(isess)
            if ttr and ttl:
                if ttr > ttl:
                    response["Next_State"] = str(ViewStates.checkMositureProfile)
                else:
                    response["Next_State"] = str(ViewStates.calcSleepTime)

        response["Status"] = str(Status.OK)
        return JsonResponse(response)

    return HttpResponseBadRequest("Client not found")


@require_http_methods(["POST"])
def giveMoistureProfie(request):
    user = Authenticator.fromDeserializedRequest(request).user
    if user:
        body = json.loads(request.body)
        isess = user.irrigationsession

        response = dict()
        logic.parseMoistDataList(body["sector1"], isess.sector1_moist_prof)
        logic.parseMoistDataList(body["sector2"], isess.sector2_moist_prof)
        logic.parseMoistDataList(body["sector3"], isess.sector3_moist_prof)

        dry_val = logic.getLowestMoistureVal(isess)
        dsws = DarkSkyWebService(user.loc_latitude, user.loc_longtitude)
        dsws.call()
        isess.botany.dry_point = dry_val + dsws.precip_amount()
        user.state = str(ViewStates.giveMoistProf)
        isess.save()
        response["Status"] = str(Status.OK)
        response["Next_State"] = str(ViewStates.viewStateDelegator)
        return JsonResponse(response)

    return HttpResponseBadRequest('Client not found.')


@require_http_methods(["POST"])
def checkMoistureProfile(request):
    user = Authenticator.fromDeserializedRequest(request).user
    if user:
        isess = user.irrigationsession

        response = dict()

        response["Response"]["sector1"]  = logic.getIrrigationAmountFor(isess.sector1_moist_prof)
        response["Response"]["sector2"]  = logic.getIrrigationAmountFor(isess.sector2_moist_prof)
        response["Response"]["sector3"]  = logic.getIrrigationAmountFor(isess.sector3_moist_prof)

        user.state = str(ViewStates.checkMositureProfile)
        user.save()
        response["Status"] = str(Status.OK)
        response["Next_State"] = str(ViewStates.viewStateDelegator)
        return JsonResponse(response)

    return HttpResponseBadRequest("Client not found.")


@require_http_methods(["POST"])
def calcSleepTime(request):
    user = Authenticator.fromDeserializedRequest(request).user
    if user:
        isess = user.irrigationsession
        user.last_irrigation_cycle = datetime.now()

        response = dict()
        response["Response"]["Hours"] = logic.calculateSleepTime()
        user.state = str(ViewStates.calcSleepTime)
        isess.save()
        response["Status"] = str(Status.OK)
        response["Next_State"] = str(ViewStates.viewStateDelegator)
        return JsonResponse(response)

    return HttpResponseBadRequest('Client not found.')

# Create your views here.
