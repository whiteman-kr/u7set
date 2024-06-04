import sys
import ctypes
from ._mats import *
from ._adsConnectionStatus import *
from ._matsAppSignalParam import *
from ._matsAppSignalState import *


class AdsBridge:
    def __init__(self, equipmentId: str, isQtApplication: bool, sharedLib: str = "/.AdsBridge") -> None:
        '''This function initializes the AdsBridge library and starts the message loop if the application is not Qt-based.'''
        argc = len(sys.argv)
        argv = sys.argv

        self.ads_lib = ctypes.CDLL(sharedLib)

        initialzed = self.init(argc, argv, equipmentId, isQtApplication)

        if not initialzed:
            raise ValueError("Initialization failed: Unable to run adsInit()")

    def init(self, argc: int, argv: list[str], equipmentId: str, isQtApplication: bool) -> bool:
        '''This function initializes the AdsBridge library and starts the message loop if the application is not Qt-based.'''
        c_AdsInit = self.ads_lib.AdsInitPrivate
        c_AdsInit.argtypes = [ctypes.c_int, ctypes.POINTER(
            ctypes.c_char_p), ctypes.c_char_p, ctypes.c_bool]
        c_AdsInit.restype = ctypes.c_bool

        for i in range(argc):
            argv[i] = ctypes.c_char_p(argv[i].encode('utf-8'))

        c_argv = (ctypes.c_char_p * argc)(*argv)

        isInitialized = c_AdsInit(
            ctypes.c_int(argc),
            c_argv,
            ctypes.c_char_p(equipmentId.encode('utf-8')),
            ctypes.c_bool(isQtApplication)
        )

        assert (self.testAdsConnectionStatus() == True)
        assert (self.testMatsAppSignalParam() == True)
        assert (self.testMatsAppSignalState() == True)

        # initialize c instances of widely used functions:
        self.c_AdsGetConnectionCount = self.ads_lib.AdsGetConnectionCount
        self.c_AdsGetConnectionCount.restype = ctypes.c_size_t

        self.c_AdsGetConnectionStatuses = self.ads_lib.AdsGetConnectionStatuses
        self.c_AdsGetConnectionStatuses.argtypes = [
            ctypes.POINTER(c_AdsConnectionStatus), ctypes.c_size_t]
        self.c_AdsGetConnectionStatuses.restype = ctypes.c_bool

        self.c_AdsSignalParamsLoaded = self.ads_lib.AdsSignalParamsLoaded
        self.c_AdsSignalParamsLoaded.restype = ctypes.c_bool

        self.c_AdsSignalStatesLoaded = self.ads_lib.AdsSignalStatesLoaded
        self.c_AdsSignalStatesLoaded.restype = ctypes.c_bool

        self.c_AdsGetSignalCount = self.ads_lib.AdsGetSignalCount
        self.c_AdsGetSignalCount.restype = ctypes.c_size_t

        self.c_AdsGetSignalList = self.ads_lib.AdsGetSignalList
        self.c_AdsGetSignalList.argtypes = [
            ctypes.POINTER(ctypes.c_ulonglong), ctypes.c_size_t]
        self.c_AdsGetSignalList.restype = ctypes.c_bool

        self.c_AdsGetSignalParams = self.ads_lib.AdsGetSignalParams
        self.c_AdsGetSignalParams.argtypes = [ctypes.POINTER(
            ctypes.c_ulonglong), ctypes.POINTER(c_MatsAppSignalParam), ctypes.c_size_t]
        self.c_AdsGetSignalParams.restype = ctypes.c_bool

        self.c_AdsGetSignalStates = self.ads_lib.AdsGetSignalStates
        self.c_AdsGetSignalStates.argtypes = [ctypes.POINTER(
            ctypes.c_ulonglong), ctypes.POINTER(c_MatsAppSignalState), ctypes.c_size_t]
        self.c_AdsGetSignalStates.restype = ctypes.c_bool

        self.c_AdsCalcHash = self.ads_lib.AdsCalcHash
        self.c_AdsCalcHash.argtypes = [ctypes.c_char_p]
        self.c_AdsCalcHash.restype = ctypes.c_ulonglong

        return isInitialized

    def setLogHandler(self, callback_func) -> None:
        '''This function sets the log handler that will be used by the AdsBridge library to handle log messages. The log handler should be thread-safe.'''
        c_AdsSetLogHandler = self.ads_lib.AdsSetLogHandler

        # make funcitons global so they are not stored on Stack
        global _CALLBACKFUNC
        _CALLBACKFUNC = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_char_p)

        global _func
        _func = _CALLBACKFUNC(callback_func)

        c_AdsSetLogHandler.argtypes = [_CALLBACKFUNC]
        c_AdsSetLogHandler.restype = None

        self.ads_lib.AdsSetLogHandler(_func)

    def setLogLevel(self, level: MatsLogLevel) -> None:
        '''This function sets the log level that will be used by the AdsBridge library to filter log messages. The log level determines the severity of the log messages that will be logged. The available log levels are defined in the MatsLogLevel enum. By default, the log level is set to LOG_LEVEL_WARNING.'''
        c_AdsSetLogLevel = self.ads_lib.AdsSetLogLevel

        c_AdsSetLogLevel.argtypes = [ctypes.c_int]
        c_AdsSetLogLevel.restype = None

        c_AdsSetLogLevel(level.value)

    def addConnection(self, adsEquipmentId: str, address: str, port: int) -> None:
        '''This function adds connection to the AppDataService. The connection is identified by the EquipmentID, and the IPv4 address/port of the service. The connection is not established until AdsConnect() is called.'''
        c_AdsAddConnection = self.ads_lib.AdsAddConnection

        c_AdsAddConnection.argtypes = [
            ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        c_AdsAddConnection.restype = None

        c_AdsAddConnection(ctypes.c_char_p(adsEquipmentId.encode('utf-8')),
                           ctypes.c_char_p(address.encode('utf-8')),
                           ctypes.c_int(port))

    def connect(self) -> None:
        '''This funcConnects to the AppDataService(s) using the connections that have been added with AdsAddConnection(). The connection status can be checked with AdsGetConnectionStatuses().'''
        self.ads_lib.argtypes = []
        self.ads_lib.restype = None
        self.ads_lib.AdsConnect()

    def shutdown(self) -> None:
        '''This function shuts down the AdsBridge library and stops the message loop if the application is not Qt-based.'''
        self.ads_lib.AdsShutdown()

    def close(self) -> None:
        '''This function closes the connection to the AppDataService(s).'''
        self.ads_lib.AdsClose()

    def getConnectionCount(self) -> int:
        '''This function returns the number of connections that are established.'''
        connections = self.c_AdsGetConnectionCount()
        return connections

    def getConnectionStatuses(self) -> list[AdsConnectionStatus]:
        '''This function returns the connection statuses of the AppDataService(s).'''

        count = self.getConnectionCount()
        AdsConnectionStatusArray = c_AdsConnectionStatus * count
        c_out = AdsConnectionStatusArray()

        connection_success = self.c_AdsGetConnectionStatuses(
            c_out, ctypes.c_size_t(count))

        connection_statuses: list[AdsConnectionStatus] = []

        for i in range(count):
            connection_statuses.append(c_out[i].toUserAdsConnectionStatus())

        return connection_statuses

    def signalParamsLoaded(self) -> bool:
        '''This function checks if the signal parameters are loaded.'''
        loaded = self.c_AdsSignalParamsLoaded()
        return loaded

    def signalStatesLoaded(self) -> bool:
        '''This function checks if the signal states are loaded.'''

        loaded = self.c_AdsSignalStatesLoaded()
        return loaded

    def getSignalCount(self) -> int:
        '''This function returns the number of signals that are available.'''

        count = int(self.c_AdsGetSignalCount())
        return count

    def getSignalList(self) -> tuple[bool, list[int]]:
        '''This function returns the list of signals that are available.'''

        count = self.getSignalCount()

        AdsSignalsArray = ctypes.c_ulonglong * count
        c_out = AdsSignalsArray()

        signals_success = self.c_AdsGetSignalList(c_out, count)

        signals: list[int] = []     # I believe int to work, if not use str
        for i in range(count):
            signals.append(int(c_out[i]))

        return (signals_success, signals)

    def getSignalParams(self, appSignalIds: list[str]) -> tuple[bool, list[MatsAppSignalParam]]:
        '''This function returns the parameters of the specified signals.'''

        count = len(appSignalIds)
        if count == 0:
            return (False, [])

        signal_hashes = []
        for i in range(count):
            signal_hashes.append(self.calcHash(appSignalIds[i]))

        c_signal_hashes = (ctypes.c_ulonglong * count)(*signal_hashes)

        AdsSignalParamsArray = (c_MatsAppSignalParam * count)
        c_out = AdsSignalParamsArray()

        signals_params_success = self.c_AdsGetSignalParams(c_signal_hashes,
                                                           c_out,
                                                           ctypes.c_size_t(count))

        signal_params: list[MatsAppSignalParam] = []
        for i in range(count):
            signal_params.append(c_out[i].toUserMatsAppSignalParam())

        return (signals_params_success, signal_params)

    def getSignalStates(self, appSignalIds: list[str]) -> tuple[bool, list[MatsAppSignalState]]:
        '''This function returns the states of the specified signals.'''

        count = len(appSignalIds)
        if count == 0:
            return (False, [])

        signal_hashes = []
        for i in range(count):
            signal_hashes.append(self.calcHash(appSignalIds[i]))

        c_signal_hashes = (ctypes.c_ulonglong * count)(*signal_hashes)

        AdsSignalStatesArray = c_MatsAppSignalState * count
        c_out = AdsSignalStatesArray()

        signals_state_success = self.c_AdsGetSignalStates(c_signal_hashes,
                                                          c_out,
                                                          ctypes.c_size_t(count))

        signal_states: list[MatsAppSignalState] = []
        for i in range(count):
            signal_states.append(c_out[i].toUserMatsAppSignalState())

        return (signals_state_success, signal_states)

    def calcHash(self, string: str) -> int:
        '''This function calculates the hash value of the specified string.'''
        hash = self.c_AdsCalcHash(ctypes.c_char_p(string.encode('utf-8')))
        return hash

    def testAdsConnectionStatus(self) -> bool:
        '''Self-test function'''
        connection_status = c_AdsConnectionStatus()
        ctypes.memset(ctypes.pointer(connection_status),
                      0, ctypes.sizeof(connection_status))

        c_AdsTestAdsConnectionStatus = self.ads_lib.AdsTestAdsConnectionStatus

        c_AdsTestAdsConnectionStatus.argtypes = [
            ctypes.c_size_t, ctypes.POINTER(c_AdsConnectionStatus)]
        c_AdsTestAdsConnectionStatus.restype = ctypes.c_bool

        connection_status.id = ctypes.c_ulonglong(2)
        connection_status.status = ctypes.c_bool(True)
        connection_status.setConnectionResult = ctypes.c_int(
            MatsConnectionResult.WrongClientHostName.value)
        connection_status.connectionType = ctypes.c_char_p(0x223344998899AABB)
        connection_status.port = ctypes.c_int(7654)
        connection_status.address = ctypes.c_char_p(0xBBAADDFF7711AA99)
        connection_status.adsEquipmentId = ctypes.c_char_p(0x77223399BBAAEEDD)
        connection_status.received = ctypes.c_size_t(1234567)
        connection_status.sent = ctypes.c_size_t(7654321)
        connection_status.requestCount = ctypes.c_size_t(123)
        connection_status.replyCount = ctypes.c_size_t(456)

        test_passed = c_AdsTestAdsConnectionStatus(ctypes.sizeof(connection_status),
                                                   ctypes.byref(connection_status))
        return test_passed

    def testMatsAppSignalParam(self) -> bool:
        '''Self-test function'''
        signal_param = c_MatsAppSignalParam()
        ctypes.memset(ctypes.pointer(signal_param),
                      0, ctypes.sizeof(signal_param))

        c_AdsTestMatsAppSignalParam = self.ads_lib.AdsTestMatsAppSignalParam

        c_AdsTestMatsAppSignalParam.argtypes = [
            ctypes.c_size_t, ctypes.POINTER(c_MatsAppSignalParam)]
        c_AdsTestMatsAppSignalParam.restype = ctypes.c_bool

        signal_param.hash = ctypes.c_ulonglong(0x123456789abcdef0)
        signal_param.appSignalId = ctypes.c_char_p(0x223344998899AABB)
        signal_param.customSignalId = ctypes.c_char_p(0xBBAADDFF7711AA99)
        signal_param.caption = ctypes.c_char_p(0x77223399BBAAEEDD)
        signal_param.equipmentId = ctypes.c_char_p(0x2233D1998899AA9A)
        signal_param.lmEquipmentId = ctypes.c_char_p(0x3233D199889AAA8B)
        signal_param.unit = ctypes.c_char_p(0xBBAADDFF7711AA99)
        signal_param.tags = ctypes.c_char_p(0x223344998899AABB)
        signal_param.channel = ctypes.c_int(MatsChannel.ChannelD.value)
        signal_param.inOutType = ctypes.c_int(
            MatsSignalInOutType.Internal.value)
        signal_param.type = ctypes.c_int(MatsSignalType.Discrete.value)
        signal_param.decimalPlaces = ctypes.c_int(5)
        signal_param.lowValidRange = ctypes.c_double(-100.0)
        signal_param.highValidRange = ctypes.c_double(100.0)
        signal_param.tuning = ctypes.c_bool(True)

        test_passed = c_AdsTestMatsAppSignalParam(ctypes.sizeof(signal_param),
                                                  ctypes.byref(signal_param))
        return test_passed

    def testMatsAppSignalState(self) -> bool:
        '''Self-test function'''
        signal_state = c_MatsAppSignalState()
        ctypes.memset(ctypes.pointer(signal_state),
                      0, ctypes.sizeof(signal_state))

        c_AdsTestMatsAppSignalState = self.ads_lib.AdsTestMatsAppSignalState

        c_AdsTestMatsAppSignalState.argtypes = [
            ctypes.c_size_t, ctypes.POINTER(c_MatsAppSignalState)]
        c_AdsTestMatsAppSignalState.restype = ctypes.c_bool

        signal_state.hash = ctypes.c_ulonglong(0x123456789abcdef0)
        signal_state.plantTime = ctypes.c_ulonglong(0x123456789abcdef0)
        signal_state.serverTime = ctypes.c_ulonglong(0x123456789abcdef0)
        signal_state.value = ctypes.c_double(123.456)
        signal_state.flags = ctypes.c_uint32(
            MatsFlag.VALID.value | MatsFlag.TUNING_DEFAULT.value)

        test_passed = c_AdsTestMatsAppSignalState(ctypes.sizeof(signal_state),
                                                  ctypes.byref(signal_state))
        return test_passed

    def getSoftwareId(self) -> str:
        '''This function returns an id of the software'''
        c_AdsGetSoftwareId = self.ads_lib.AdsGetSoftwareId
        c_AdsGetSoftwareId.restype = ctypes.c_char_p

        software_id = c_AdsGetSoftwareId()

        return software_id.decode('utf-8')

    def loadConfiguration(self, fileName: str) -> bool:
        '''This function loads configuration'''
        c_AdsLoadConfiguration = self.ads_lib.AdsLoadConfiguration
        c_AdsLoadConfiguration.argtypes = [ctypes.c_char_p]
        c_AdsLoadConfiguration.restype = ctypes.c_bool

        loaded = c_AdsLoadConfiguration(
            ctypes.c_char_p(fileName.encode('utf-8')))

        return loaded

    def setConfiguration(self, configurationXml: str, size: int) -> bool:
        '''This function sets configuration'''
        c_AdsSetConfiguration = self.ads_lib.AdsSetConfiguration
        c_AdsSetConfiguration.argtypes = [ctypes.c_char_p, ctypes.c_size_t]
        c_AdsSetConfiguration.restype = ctypes.c_bool

        configured = c_AdsSetConfiguration(ctypes.c_char_p(
            configurationXml.encode('utf-8')), ctypes.c_size_t(size))

        return configured

    def setConfiguration(self, profile: str) -> bool:
        '''This function sets configuration profile. Possible to use only after AdsLoadConfiguration() or AdsSetConfiguration()'''
        c_AdsSetConfigurationProfile = self.ads_lib.AdsSetConfigurationProfile
        c_AdsSetConfigurationProfile.argtypes = [ctypes.c_char_p]
        c_AdsSetConfigurationProfile.restype = ctypes.c_bool

        profile_set = c_AdsSetConfigurationProfile(
            ctypes.c_char_p(profile.encode('utf-8')))

        return profile_set