from httplib import HTTPConnection, HTTP_PORT, HTTPS_PORT
import socket
import ssl
import enum
import cookielib
from urlparse import urlsplit

http_port_scheme = {
    "http" : HTTP_PORT,
    "https" : HTTPS_PORT
}

http_method_scheme = ("GET", "POST")
    #"DELETE" TODO: Support this in the future.

class SimpleHTTPSConnection(HTTPConnection, object):

    def __init__(self,
                 host,
                 port =  http_port_scheme.get("https"),
                 strict = None,
                 timeout = socket._GLOBAL_DEFAULT_TIMEOUT,
                 source_address = None,
                 socket_options = [(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)],
                 key_file = None,
                 cert_file = None,
                 cert_reqs = 'CERT_NONE',
                 ca_certs = None,
                 context = None,
                 *args, **kwargs):

        self.host = host
        self.port = port
        self.key_file = key_file
        self.cert_file = cert_file
        self.source_address = source_address
        self.socket_options = socket_options
        self.timeout = timeout

        ssl_req_scheme = {
            'CERT_NONE': ssl.CERT_NONE,
            'CERT_OPTIONAL': ssl.CERT_OPTIONAL,
            'CERT_REQUIRED': ssl.CERT_REQUIRED
        }
        self.cert_reqs =  ssl_req_scheme.get(cert_reqs) or ssl.CERT_NONE
        self.ca_certs = ca_certs

        HTTPConnection.__init__(self, self.host, self.port, self.strict, self.timeout,
                                self.source_address)

        self.connect()


    def connect(self):
        sock_nossl = socket.create_connection((self.host, self.port), self.timeout, self.source_address)

        if self.socket_options:
            for opt in self.socket_options:
                sock_nossl.setsockopt(*opt)

        self.sock = ssl.wrap_socket(sock_nossl, keyfile = self.key_file, certfile = self.cert_file,
                                    cert_reqs = self.cert_reqs, ca_certs = self.ca_certs)
        if self.ca_certs:
            match_hostname(self.sock.getpeercert(), self.host)


class RequestContext(object):

    def __init__(self, burl, method, headers = None, body=None,
                 timeout=None, retries=0):
        scheme, host, api_url, query, nil = urlsplit(burl)
        self.host = host
        self.url = '%s?%s' % (api_url, query) if query else api_url

        self.scheme = scheme if http_port_scheme.get(scheme) else None

        if method.upper() in http_method_scheme:
            self.method = method.upper()
        else:
            raise NotImplementedError("Unsupported http methods, only GET and POST available.")

        self.timeout = timeout
        self.retries = retries
        self.headers = headers or {}
        if body is dict:
            import json
            self.body = json.dumps(body)
        else:
            self.body = body

    def add_body(self, data):
        if type(data) is dict():
            import json
            data = json.dumps(data)

        if self.body is None:
            self.body = data
        else:
            self.body += data

    def _set_header_field(self, head_name, head_field):
        self.headers[head_name] = head_field

    def set_headers(self, header_kv_list):
        for head_k,head_v in header_kv_list:
            if type(head_k) is not str or type(head_v) is not str:
                self._set_header_field(head_k, head_v)
            else:
                raise TypeError("header list must be populated of (str,str).")


class RequestContextFactory():

    @staticmethod
    def default_context(url, method):
        return RequestContext(url, method)

    @staticmethod
    def custom_context(url, method, *args, **kwargs):
        return RequestContext(url, method, *args, **kwargs)


class Response():

    @staticmethod
    def good_status(http_res):
        if http_res.status == 200 and http_res.reason == 'OK':
            return True
        return False


class Cookie(object):

    def __init__(self, value, domain='', path='/', expiry='session'):
        self.value = value
        self.domain = domain
        self.path = path
        self.expiry = expiry


class Session(object):

    conn_pool = {}
    cookies_pool = {}
    conn_scheme = {
        HTTP_PORT : HTTPConnection,
        HTTPS_PORT : SimpleHTTPSConnection
    }

    @classmethod
    def execute(cls, context):
        cls._prepare(context)
        conn = cls.conn_pool[context.host]

        Session._make_request(conn, context, context.retries)
        res = conn.getresponse()

        Session._set_cookies(res)

        return res

    @staticmethod
    def _make_request(conn, req_con, retries = 0):
        try:
            conn.request(method = req_con.method, url = req_con.url,
                            body = req_con.body, headers = req_con.headers)
        except Exception as e:
            if retries > 0:
                retries -= 1
                Session._make_request(conn, req_con, retries)

    @classmethod
    def _set_cookies(cls, res):
        if res.getheader('set-cookie'):
            #TODO: implement this
            return

    @classmethod
    def _prepare(cls, req_con):
        if req_con.host not in cls.conn_pool:
            cls.conn_pool[req_con.host] = cls._new_conn(req_con)

        if req_con.host not in cls.cookies_pool:
            cls.cookies_pool[req_con.host] = {}

    @classmethod
    def _new_conn(cls, req_con):
        p = http_port_scheme[req_con.scheme]
        c = cls.conn_scheme[p](**req_con.__dict__)
        return c

    @classmethod
    def close(cls):
        for conn in cls.conn_pool.values():
            conn.close()
        cls.conn_pool.clear()
        cls.cookies_pool.clear()

