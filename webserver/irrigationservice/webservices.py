from django.core.validators import URLValidator
from django.core.exceptions import ValidationError
import re
import enum
from utils import constant
import abc
import http
import json

class RequestFailed(Exception):
    pass

class HTTPMethods(enum.Enum):
    GET = 0
    POST = 1
    DELETE = 2


class BaseWebService(object):

    def __init__(self, namespace, **kwargs):
        validate = URLValidator()
        try:
            validate(namespace)
        except ValidationError, e:
            #TODO there has to be something better to do here...
            raise ValueError

        self._namespace = namespace
        self._apitoken = kwargs.get("apitoken", None)
        self._req_format = kwargs.get("req_format", None)
        self._method = kwargs.get("method", str(HTTPMethods.GET))

    @constant
    def namespace(self):
        return self._namespace

    @constant
    def apitoken(self):
        return self._apitoken

    @constant
    def req_format(self):
        return self._req_format

    @constant
    def method(self):
        return self._method

    @staticmethod
    def verify(response):
        if not http.Response.good_status(response):
            raise RequestFailed

    @abc.abstractmethod
    def buildURL(self):
        raise NotImplementedError('Class does not implement this method.')

    @abc.abstractmethod
    def call(self):
        raise NotImplementedError('Class does not implement this method.')


class DarkSkyWebService(BaseWebService):

    _NAMESPACE = 'https://api.forecast.io/forecast/'
    _APITOKEN = '95078e3382b74bed6aac7e4a81e8713f'
    _REQFORMAT = '<accesspoint><apikey>/<lat>,<long>'

    def __init__(self, latitude, longtitude):
        super(DarkSkyWebService, self).__init__(namespace = self._NAMESPACE, apitoken = self._APITOKEN, req_format = self._REQFORMAT)
        self._latitude = str(latitude)
        self._longtitude = str(longtitude)
        self._buildURL()

    @property
    def latitude(self):
        return self._latitude

    @property
    def longtitude(self):
        return self._longtitude

    def _buildURL(self):
        url = re.sub(r'\<accesspoint>', self.namespace, self.req_format)
        url = re.sub(r'\<apikey>', self.apitoken, url)
        url = re.sub(r'\<lat>', self.latitude, url)
        url = re.sub(r'\<long>', self.longtitude, url)
        self.url = url

    def call(self):
        cont = http.RequestContextFactory.default_context(self.url, 'get')
        res = http.Session.execute(cont)
        BaseWebService.verify(res)
        self._resData = json.loads(res.read())

    def precip_amount(self):
        rain = 0
        for i in range(49):
            try:
                if self._resData["hourly"]["data"][i]["precipProbability"] > .3:
                    rain += self._resData["hourly"]["data"][""]
            except KeyError:
                continue
        return rain

    def time_till_rain(self):
        for i in range(49):
            try:
                if self._resData["hourly"]["data"][i]["precipProbability"] > .45:
                    return i
            except KeyError:
                continue


class GoogleMapsWebService(BaseWebService):

    _NAMESPACE = 'https://maps.googleapis.com/maps/api/geocode/json?'
    _APITOKEN = 'AIzaSyD4F9LEYOSsJg0MfzzMGAGRc5eB0sFXD2M'
    _REQFORMAT = '<accesspoint>address=<addr>&key=<apikey>'

    def __init__(self, address):
        super(GoogleMapsWebService, self).__init__(namespace = self._NAMESPACE, apitoken = self._APITOKEN, req_format = self._REQFORMAT)
        self._address = address
        self._buildURL()

    @property
    def address(self):
        return self._address

    def _buildURL(self):
        url = re.sub(r'\<accesspoint>', self.namespace, self.req_format)
        url = re.sub(r'\<apikey>', self.apitoken, url)
        url = re.sub(r'\<addr>', self.address, url)
        self.url = url

    def call(self):
        cont = http.RequestContextFactory.default_context(self.url, 'get')
        res = http.Session.execute(cont)
        BaseWebService.verify(res)
        self._resData = json.loads(res.read())

    def getLatitude(self):
        return self._resData['results'][0]['geometry']['location']['lat']

    def getLongtitude(self):
        return self._resData['results'][0]['geometry']['location']['lng']
