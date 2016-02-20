from utils import constant
import abc

#CONSTANTS
FLOWRATE = 12.83 #inches^3/second/solinoid
SECTORVOLUME = 340 #inches^3

class AbstractCalculation(object):

    @abc.abstractmethod
    def calculate(self):
        raise NotImplementedError('Class does not implement method')


class LowerSoilMoistTarger(AbstractCalculation):

    def __init__(self, fc, pwp, mad):
        self._fc = fc
        self._pwp = pwp
        self._mad = mad
        self.calculate()

    def calculate(self):
        self._value = self.fc - (self.fc - self.pwp)*self.mad

    @constant
    def value(self):
        return self._value

    @property
    def fc(self):
        return self._fc

    @fc.setter
    def fc(self, value):
        self._fc = value

    @property
    def pwp(self):
        return self._pwp

    @pwp.setter
    def pwp(self, value):
        self._pwp = value

    @property
    def mad(self):
        return self._mad

    @mad.setter
    def mad(self, value):
        self._mad = value


class Evapotranspiration(AbstractCalculation):

    def __init__(self, et0, kc, ef):
        self._et0 = et0
        self._kc = kc
        self._ef = ef
        self.calculate()

    def calculate(self):
        self._value = self.et0*self.kc/self.ef

    @constant
    def value(self):
        return self._value

    @property
    def et0(self):
        return self._et0

    @et0.setter
    def et0(self, value):
        self._et0 = value

    @property
    def kc(self):
        return self._kc

    @kc.setter
    def kc(self, value):
        self._kc = value

    @property
    def ef(self):
        return self._ef

    @ef.setter
    def ef(self, value):
        self._ef = value


class IrrigationAmount(AbstractCalculation):

    def __init__(self, wvf, fc):
        self._wvf = float(wvf)
        self._fc = float(fc)
        self.calculate()

    def calculate(self):
        v = (self.fc*SECTORVOLUME - self.wvf*SECTORVOLUME)/FLOWRATE
        self._value = v if v > 0 else 0 #assure value is always positive

    @constant
    def value(self):
        return self._value

    @property
    def wvf(self):
        return self._wvf

    @wvf.setter
    def wvf(self, value):
        self._wvf = value

    @property
    def fc(self):
        return self._fc

    @fc.setter
    def fc(self, value):
        self._fc = value

