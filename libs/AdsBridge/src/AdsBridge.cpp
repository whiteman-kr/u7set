#include "AdsBridgeFacade.h"
#include "AdsBridgeLogFile.h"
#include <AdsBridge/AdsBridge.h>

#include <grpcpp/grpcpp.h>

#include <fstream>


namespace
{
	const std::string DefaultProfile = "Default";

	AdsBridge::Resources g_resources;
	AdsBridge::LogFile g_log;
	std::unique_ptr<AdsBridge::AdsBridgeFacade> g_adsBridge;

	std::vector<char> g_lastLoadedConfiguration;
} // namespace

#define ADSB_API_VERSION 0x00000001 ///< API version of the AdsBridge library.

uint32_t AdsGetInterfaceVersion(void)
{
	return ADSB_API_VERSION;
}

void AdsSetLogHandler(MatsLogHandler handler)
{
	AdsBridge::LogFile::g_logHandler = handler;
}

void AdsSetLogLevel(MatsLogLevel level)
{
	AdsBridge::LogFile::g_logLevel = level;
}

void AdsTestLogHandler(enum MatsLogLevel level, const char* message)
{
	switch (level)
	{
	case MATS_LOG_LEVEL_DEBUG:
		g_log.writeMessage(message);
		break;
	case MATS_LOG_LEVEL_WARNING:
		g_log.writeWarning(message);
		break;
	case MATS_LOG_LEVEL_ERROR:
		g_log.writeError(message);
		break;
	default:
		g_log.writeError("AdsTestLogHandler(): Unknown message level");
	}

	return;
}

bool AdsInit(const char* equipmentId)
{
	g_log.writeMessage("AdsInit()");

	if (g_adsBridge != nullptr)
	{
		g_log.writeError("AdsInit(): AdsBridge is already initialized.");
		return false;
	}

	grpc_init();
	[[maybe_unused]] auto _ = std::chrono::current_zone(); // A call to this function that is the first reference to the time zonedatabase
														   // will cause it to be initialized.

	g_adsBridge = std::make_unique<AdsBridge::AdsBridgeFacade>(g_resources, g_log);

	g_adsBridge->setEquipmentId(equipmentId);
	return true;
}

const char* AdsGetSoftwareId()
{
	return g_resources.getString(g_adsBridge->equipmentId());
}

bool AdsLoadConfiguration(const char* fileName)
{
	g_lastLoadedConfiguration.clear();

	std::ifstream file{fileName, std::ios::binary | std::ios::ate};
	if (file.is_open() == false)
	{
		g_log.writeError(std::format("AdsLoadConfiguration(): Cannot open file: {}", fileName));
		return false;
	}

	auto fileSize = file.tellg();
	file.seekg(0);

	g_lastLoadedConfiguration.resize(fileSize);
	file.read(g_lastLoadedConfiguration.data(), fileSize);

	return g_adsBridge->setConfiguration(g_lastLoadedConfiguration, DefaultProfile);
}

bool AdsSetConfiguration(const char* configurationXml, size_t size)
{
	g_lastLoadedConfiguration = std::vector<char>(configurationXml, configurationXml + size);
	return g_adsBridge->setConfiguration(g_lastLoadedConfiguration, DefaultProfile);
}

bool AdsSetConfigurationProfile(const char* profile)
{
	if (g_lastLoadedConfiguration.empty() == true)
	{
		g_log.writeError("AdsSetConfigurationProfile(): No configuration loaded.");
		return false;
	}

	return g_adsBridge->setConfiguration(g_lastLoadedConfiguration, profile);
}

void AdsShutdown()
{
	g_log.writeMessage("AdsShutdown()");

	g_adsBridge->clearAppDataServices();
	g_adsBridge->close();
	g_adsBridge.reset();

	// Shutdown gRPC AFTER all channels/stubs have been destroyed (main window scope ended)
	//
	grpc_shutdown();

	return;
}

void AdsAddService(const char* adsEquipmentId, const char* address, int port)
{
	g_log.writeMessage(std::format("AdsAddService: {}, {}:{}", adsEquipmentId, address, port));
	g_adsBridge->addAppDataService(adsEquipmentId, address, port);
}

void AdsConnect()
{
	g_log.writeMessage("AdsConnect()");
	g_adsBridge->connect();
}

void AdsCloseConnection()
{
	g_log.writeMessage("AdsCloseConnection()");
	g_adsBridge->close();
}

size_t AdsGetTcpConnectionCount()
{
	return g_adsBridge->connectionCount();
}

bool AdsGetTcpConnectionStatusesPrivate1(size_t structSize, struct AdsConnectionStatus* out, size_t count)
{
	return g_adsBridge->connectionStatus(structSize, out, count);
}

bool AdsSignalParamsLoaded()
{
	return g_adsBridge->signalParamsLoaded();
}

bool AdsSignalStatesLoaded()
{
	return g_adsBridge->signalStatesLoaded();
}

size_t AdsGetSignalCount()
{
	return g_adsBridge->signalCount();
}

bool AdsGetSignalList(MatsSignalHash* out, size_t count)
{
	return g_adsBridge->signalList(out, count);
}

size_t AdsGetSignalParamsPrivate1(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count)
{
	return g_adsBridge->signalParams(structSize, signalHashes, out, count);
}

size_t AdsGetSignalStatesPrivate1(size_t structSize, const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count)
{
	return g_adsBridge->signalStates(structSize, signalHashes, out, count);
}

MatsSignalHash AdsCalcHash(const char* string)
{
	return ::calcHash(std::string_view{string});
}

bool AdsTestAdsConnectionStatus(size_t structSize, const struct AdsConnectionStatus* testValue)
{
	assert(structSize == sizeof(AdsConnectionStatus));
	if (structSize != sizeof(AdsConnectionStatus))
	{
		g_log.writeError("AdsTestAdsConnectionStatus(), structSize != sizeof(AdsConnectionStatus)");
		return false;
	}

	struct AdsConnectionStatus adsTestConnectionStatus;
	std::memset(&adsTestConnectionStatus, 0, sizeof(AdsConnectionStatus));
	adsTestConnectionStatus.id = 2ull;
	adsTestConnectionStatus.status = true;
	adsTestConnectionStatus.setConnectionResult = ADS_SET_CONNECTION_RESULT_WRONG_CLIENT_HOST_NAME;
	adsTestConnectionStatus.connectionType = (char*)0x223344998899AABBull;
	adsTestConnectionStatus.port = 7654;
	adsTestConnectionStatus.address = (char*)0xBBAADDFF7711AA99ull;
	adsTestConnectionStatus.adsEquipmentId = (char*)0x77223399BBAAEEDDull;
	adsTestConnectionStatus.received = 1234567ull;
	adsTestConnectionStatus.sent = 7654321ull;
	adsTestConnectionStatus.requestCount = 123ull;
	adsTestConnectionStatus.replyCount = 456ull;

	assert(&adsTestConnectionStatus != testValue);

	int result = memcmp(&adsTestConnectionStatus, testValue, sizeof(AdsConnectionStatus));
	if (result != 0)
	{
		g_log.writeError("AdsTestAdsConnectionStatus(), data alignment or size mismatch");
	}

	return result == 0;
}

bool AdsTestMatsAppSignalParam(size_t structSize, const struct MatsAppSignalParam* testValue)
{
	assert(structSize == sizeof(MatsAppSignalParam));
	if (structSize != sizeof(MatsAppSignalParam))
	{
		g_log.writeError("AdsTestMatsAppSignalParam(), structSize != sizeof(MatsAppSignalParam)");
		return false;
	}

	struct MatsAppSignalParam matsTestAppSignalParam;
	memset(&matsTestAppSignalParam, 0, sizeof(MatsAppSignalParam));
	matsTestAppSignalParam.hash = 0x123456789abcdef0ull;
	matsTestAppSignalParam.appSignalId = (char*)0x223344998899AABBull;
	matsTestAppSignalParam.customSignalId = (char*)0xBBAADDFF7711AA99ull;
	matsTestAppSignalParam.caption = (char*)0x77223399BBAAEEDDull;
	matsTestAppSignalParam.equipmentId = (char*)0x2233D1998899AA9Aull;
	matsTestAppSignalParam.lmEquipmentId = (char*)0x3233D199889AAA8Bull;
	matsTestAppSignalParam.units = (char*)0xBBAADDFF7711AA99ull;
	matsTestAppSignalParam.tags = (char*)0x223344998899AABBull;
	matsTestAppSignalParam.channel = MATS_CHANNEL_D;
	matsTestAppSignalParam.inOutType = MATS_SIGNAL_INTERNAL;
	matsTestAppSignalParam.type = MATS_SIGNAL_DISCRETE;
	matsTestAppSignalParam.decimalPlaces = 5;
	matsTestAppSignalParam.lowValidRange = -100.0;
	matsTestAppSignalParam.highValidRange = 100.0;
	matsTestAppSignalParam.tuning = true;

	assert(&matsTestAppSignalParam != testValue);

	int result = memcmp(&matsTestAppSignalParam, testValue, sizeof(MatsAppSignalParam));
	if (result != 0)
	{
		g_log.writeError("AdsTestMatsAppSignalParam(), data alignment or size mismatch");
	}

	return result == 0;
}

bool AdsTestMatsAppSignalState(size_t structSize, const struct MatsAppSignalState* testValue)
{
	assert(structSize == sizeof(MatsAppSignalState));
	if (structSize != sizeof(MatsAppSignalState))
	{
		g_log.writeError("AdsTestMatsAppSignalState(), structSize != sizeof(MatsAppSignalState)");
		return false;
	}

	struct MatsAppSignalState matsTestAppSignalState;
	memset(&matsTestAppSignalState, 0, sizeof(MatsAppSignalState));
	matsTestAppSignalState.hash = 0x123456789abcdef0ull;
	matsTestAppSignalState.plantTime = 0x123456789abcdef0ull;
	matsTestAppSignalState.serverTime = 0x123456789abcdef0ull;
	matsTestAppSignalState.value = 123.456;
	matsTestAppSignalState.flags = MATS_FLAG_VALID | MATS_FLAG_TUNING_DEFAULT;

	assert(&matsTestAppSignalState != testValue);

	int result = memcmp(&matsTestAppSignalState, testValue, sizeof(MatsAppSignalState));
	if (result != 0)
	{
		g_log.writeError("AdsTestMatsAppSignalState(), data alignment or size mismatch");
	}

	return result == 0;
}