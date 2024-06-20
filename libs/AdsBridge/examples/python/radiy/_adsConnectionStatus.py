import ctypes
from ._mats import *


class AdsConnectionStatus:
    '''Represents the status of an AppDataService connection.'''

    def __init__(self,
                 id: int,
                 status: bool,
                 setConnectionResult: MatsConnectionResult,
                 connectionType: str,
                 port: int,
                 address: str,
                 adsEquipmentId: str,
                 received: int,
                 sent: int,
                 requestCount: int,
                 replyCount: int) -> None:
        self.id = id
        self.status = status
        self.setConnectionResult = setConnectionResult
        self.connectionType = connectionType
        self.port = port
        self.address = address
        self.adsEquipmentId = adsEquipmentId
        self.received = received
        self.sent = sent
        self.requestCount = requestCount
        self.replyCount = replyCount


class c_AdsConnectionStatus(ctypes.Structure):
    _fields_ = [("id", ctypes.c_ulonglong),
                ("status", ctypes.c_bool),
                ("setConnectionResult", ctypes.c_int),
                ("connectionType", ctypes.c_char_p),
                ("port", ctypes.c_int),
                ("address", ctypes.c_char_p),
                ("adsEquipmentId", ctypes.c_char_p),
                ("received", ctypes.c_size_t),
                ("sent", ctypes.c_size_t),
                ("requestCount", ctypes.c_size_t),
                ("replyCount", ctypes.c_size_t)]    # Total of 80 bytes

    def toUserAdsConnectionStatus(self) -> AdsConnectionStatus:
        # Convert C data types to Python data types
        id = int(self.id)
        status = bool(self.status)
        setConnectionResult = MatsConnectionResult(self.setConnectionResult)
        connectionType = self.connectionType.decode('utf-8')
        port = int(self.port)
        address = self.address.decode('utf-8')
        adsEquipmentId = self.adsEquipmentId.decode('utf-8')
        received = int(self.received)
        sent = int(self.sent)
        requestCount = int(self.requestCount)
        replyCount = int(self.replyCount)

        # Create and return a user_AdsConnectionStatus object
        return AdsConnectionStatus(
            id=id,
            status=status,
            setConnectionResult=setConnectionResult,
            connectionType=connectionType,
            port=port,
            address=address,
            adsEquipmentId=adsEquipmentId,
            received=received,
            sent=sent,
            requestCount=requestCount,
            replyCount=replyCount)