from django.shortcuts import render
import json
from django.core import serializers
from django.http import JsonResponse, HttpResponse, HttpResponseNotAllowed, HttpResponseBadRequest
from models import *

def getSoilData(request):
    if request.method == "POST":
        json_request_body = json.loads(request.body)
        print json_request_body
        soil_data = SoilData.objects.getDataForSoilType(soil=json_request_body["soilType"])
        if soil_data != False:
            json_soil_data = serializers.serialize('json', [soil_data.first(),])
            o = json_soil_data.strip("[]")
            return JsonResponse(json.loads(o))
        else:
            return HttpResponse('Soil Type was not found in database.')
    else:
        return HttpResponseNotAllowed(['POST'])

def getCropData(request):
    if request.method == "POST":
        json_request_body = json.loads(request.body)
        crop_data = CropData.objects.getDataForCropType(crop=json_request_body["cropName"])
        if crop_data != False:
            json_crop_data = serializers.serialize('json', [crop_data,])
            o = json_crop_data.strip("[]")
            return JsonResponse(json.loads(o))
        else:
            return HttpResponse("Crop Name was not found in database")
    else:
        return HttpResponseNotAllowed(['GET'])

def getEvapotranspiration(request):
    if request.method == "POST":
        body = json.loads(request.body)
        query_list = ReferenceEvapotranspirateion.objects.getValuesByState(state = body["state"]).getValuesByMonth(month = body["month"])
        trans_val = query_list.first()
        if trans_val:
            json_trans_data = serializers.serialize('json', [trans_val,])
            o = json_trans_data.strip("[]")
            return JsonResponse(json.loads(o))
        else:
            return HttpResponseBadRequest()
    else:
        return HttpResponseNotAllowed(['GET'])
# Create your views here.
