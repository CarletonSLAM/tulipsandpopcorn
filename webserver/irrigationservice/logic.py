from irrigation import *
from models import MoistureKVPair, ViewStates, Status


'''
Calculates the irrigation amount for the specified sector
sector - MoistureProfileDict used to store data on that sector

ret - a list of field positions with a irrigation amount value corrisponding to it
'''
def getIrrigationAmountFor(sector):
    return [[kvtup[0], IrrigationAmount(kvtup[1], isess.botany.field_capacity).value] \
            for kvtup in sector.getPairsIterable()]


def parseMoistDataList(dataKVList, sector):
            for tupli in dataKVList:
                m, created = MoistureKVPair.objects.update_or_create(sector_key = tupli[0],
                                                                     profile = sector,
                                                                     defaults = {'sector_value' : tupli[1]})
                m.save()


def calculateSleepTime(isess):
    lst = LowerSoilMoistTarger(isess.botany.field_capacity,
                               isess.botany.perm_wilt_point,
                               isess.botany.max_depl).value
    moist_delta = isess.botany.dry_point - lst
    vwater = moist_delta*SECTORVOLUME
    return vwater/Evapotranspiration("""referneces needed for the values in here""").value

def time_till_lst(isess):
    return calculateSleepTime(isess)

def getLowestMoistureVal(isess):
    low = []
    l = isess.sector1_moist_prof.getValuesIterable()
    l.sort()
    low.append(l[0])
    l = isess.sector2_moist_prof.getValuesIterable()
    l.sort()
    low.append(l[0])
    l = isess.sector3_moist_prof.getValuesIterable()
    l.sort()
    low.append(l[0])
    low.sort()
    return low[0]
