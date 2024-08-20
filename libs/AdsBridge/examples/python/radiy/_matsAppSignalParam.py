import ctypes
from ._mats import *


class MatsAppSignalParam:
    '''Represents the parameters of a MatsAppSignal.'''

    def __init__(self,
                 hash: int,
                 appSignalId: str = None,
                 customSignalId: str = None,
                 caption: str = None,
                 equipmentId: str = None,
                 lmEquipmentId: str = None,
                 unit: str = None,
                 tags: str = None,
                 channel: MatsChannel = MatsChannel.ChannelA,
                 inOutType: MatsSignalInOutType = MatsSignalInOutType.Input,
                 type: MatsSignalType = MatsSignalType.Analog,
                 decimalPlaces: int = 0,
                 lowValidRange: float = 0,
                 highValidRange: float = 0,
                 tuning: bool = False
                 ) -> None:
        self.hash = hash
        self.appSignalId = appSignalId
        self.customSignalId = customSignalId
        self.caption = caption
        self.equipmentId = equipmentId
        self.lmEquipmentId = lmEquipmentId
        self.unit = unit
        self.tags = tags
        self.channel = channel
        self.inOutType = inOutType
        self.type = type
        self.decimalPlaces = decimalPlaces
        self.lowValidRange = lowValidRange
        self.highValidRange = highValidRange
        self.tuning = tuning


class c_MatsAppSignalParam(ctypes.Structure):
    _fields_ = [("hash", ctypes.c_ulonglong),
                ("appSignalId", ctypes.c_char_p),
                ("customSignalId", ctypes.c_char_p),
                ("caption", ctypes.c_char_p),
                ("equipmentId", ctypes.c_char_p),
                ("lmEquipmentId", ctypes.c_char_p),
                ("unit", ctypes.c_char_p),
                ("tags", ctypes.c_char_p),
                ("channel", ctypes.c_int),  # enum
                ("inOutType", ctypes.c_int),  # enum
                ("type", ctypes.c_int),  # enum
                ("decimalPlaces", ctypes.c_int),
                ("lowValidRange", ctypes.c_double),
                ("highValidRange", ctypes.c_double),
                ("tuning", ctypes.c_bool)]

    def toUserMatsAppSignalParam(self) -> MatsAppSignalParam:
        if self.appSignalId is None:
            return MatsAppSignalParam(hash=int(self.hash))
        else:
            return MatsAppSignalParam(hash=int(self.hash),
                                      appSignalId=bytes(
                                          self.appSignalId).decode('utf-8'),
                                      customSignalId=self.customSignalId.decode(
                                          'utf-8'),
                                      caption=self.caption.decode('utf-8'),
                                      equipmentId=self.equipmentId.decode(
                                          'utf-8'),
                                      lmEquipmentId=self.lmEquipmentId.decode(
                                          'utf-8'),
                                      unit=self.unit.decode('utf-8'),
                                      tags=self.tags.decode('utf-8'),
                                      channel=MatsChannel(self.channel),
                                      inOutType=MatsSignalInOutType(
                                          self.inOutType),
                                      type=MatsSignalType(self.type),
                                      decimalPlaces=int(self.decimalPlaces),
                                      lowValidRange=float(self.lowValidRange),
                                      highValidRange=float(
                                          self.highValidRange),
                                      tuning=bool(self.tuning))
