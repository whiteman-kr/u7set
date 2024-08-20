import ctypes


class MatsAppSignalState:
    def __init__(self,
                 hash: int,
                 plantTime: int,
                 serverTime: int,
                 value: float,
                 flags: int) -> None:
        self.hash = hash
        self.plantTime = plantTime
        self.serverTime = serverTime
        self.value = value
        self.flags = flags


class c_MatsAppSignalState(ctypes.Structure):
    _fields_ = [("hash", ctypes.c_ulonglong),
                ("plantTime", ctypes.c_ulonglong),
                ("serverTime", ctypes.c_ulonglong),
                ("value", ctypes.c_double),
                ("flags", ctypes.c_uint32)]

    def toUserMatsAppSignalState(self) -> MatsAppSignalState:
        hash = int(self.hash)
        plantTime = int(self.plantTime)
        serverTime = int(self.serverTime)
        value = float(self.value)
        flags = int(self.flags)

        return MatsAppSignalState(
            hash=hash,
            plantTime=plantTime,
            serverTime=serverTime,
            value=value,
            flags=flags)