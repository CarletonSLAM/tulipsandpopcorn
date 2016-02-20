import urllib2
import json
import re
from functools import wraps

#constant decorcator
def constant(f):
    def fset(self, value):
        raise TypeError
    @wraps(f)
    def fget(self):
        return f(self)
    return property(fget, fset)


#user authentication
class Authenticator(object):

    def __init__(self, username, password):
        from django.contrib.auth import authenticate
        self._user = authenticate(username = username, password = password)

    @classmethod
    def fromDeserializedRequest(cls, request):
        body = json.loads(request.body)
        try:
            u = body["username"]
            p = body["password"]
            return cls(u, p)
        except KeyError:
            return None

    @property
    def user(self):
        return self._user

    @user.setter
    def user(self, value):
        self._user = value


#facade for http get/posts because its annoying as fuck to use urllib2
"""
url - url to do an httpRequest on
data -  None if method is GET and str if method is POST
method - http method used for the httpRequest
"""
class Response(object):

    def __init__(self, url, data=None, method="GET"):
        self._method = method
        if (not url.endswith('/')):
            self._url = url + '/'
        else:
            self._url = url

        if (method == "POST"):
            if (type(data) == str):
                self._data = data
            elif(type(data) == dict):
                import urllib
                self._data = urllib.urlencode(data)
            else:
                raise TypeError("invalid data type entered for request.data, use type dict or str")

        req = urllib2.Request(url = self._url, data = self._data if method == "POST" else None)
        res = urllib2.urlopen(req)

        self._res_is_json = False
        try:
            self._res_data = res.read()
            self._res_json_data = json.loads(self._res_data)
            self._res_is_json = True
        except ValueError, e:
            pass

    @property
    def resData(self):
        if(self._res_is_json):
            return self._res_json_data
        else:
            return self._res_data

    @constant
    def url(self):
        return self._url

    @constant
    def method(self):
        return self._method


class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration

    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: 
            self.fall = True
            return True
        else:
            return False
