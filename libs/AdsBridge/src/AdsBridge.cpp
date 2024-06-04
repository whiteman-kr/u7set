#include "AdsBridgeFacade.h"
#include "AdsBridgeLogFile.h"
#include <AdsBridge/AdsBridge.h>

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include <latch>
#include <thread>

#include <QDebug>

namespace
{
	std::jthread g_appThread;
	std::atomic<QCoreApplication*> g_app = nullptr; // Atomic is not necessary, but it is used to make sure that the compiler does not
													// optimize the code and the value is always up to date.
	AdsBridge::LogFile g_log;
	AdsBridge::AdsBridgeFacade g_adsBridge{&g_log};

	QByteArray g_lastLoadedConfiguration;
} // namespace

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
	qDebug() << "AdsTestLogHandler(): BEFORE CALLBACK, message: " << message;
	switch (level)
	{
	case LOG_LEVEL_DEBUG:
		g_log.writeMessage(message);
		break;
	case LOG_LEVEL_WARNING:
		g_log.writeWarning(message);
		break;
	case LOG_LEVEL_ERROR:
		g_log.writeError(message);
		break;
	default:
		g_log.writeError("AdsTestLogHandler(): Unknown message level");
	}

	qDebug() << "AdsTestLogHandler(): AFTER CALLBACK";
	return;
}

bool AdsInitPrivate(int argc, char** argv, const char* equipmentId, bool isQtApplication)
{
	g_log.writeMessage("AdsInit()");

	if (g_appThread.joinable() == true || g_app != nullptr)
	{
		g_log.writeError("AdsInit(): QCoreApplication already exists, call AdsShutdown() first.");
		return false;
	}

	if (isQtApplication == false)
	{
		assert(QCoreApplication::instance() == nullptr);
		g_log.writeMessage("AdsInit(): QCoreApplication::instance() == nullptr, create new thread, start message loop.");

		std::latch done{2};

		g_appThread = std::jthread(
			[argc, argv, &done]()
			{
				int argcc = argc;
				char** argvv = argv;

				g_app.store(new QCoreApplication{argcc, argvv});

				done.count_down();

				g_app.load()->exec();

				delete g_app.exchange(nullptr);
			});

		// wait for QCoreApplication to be created
		//
		done.arrive_and_wait();
	}
	else
	{
		g_log.writeMessage("AdsInit(): QCoreApplication already exists");
	}

	g_adsBridge.setEquipmentId(equipmentId);
	return true;
}

const char* AdsGetSoftwareId()
{
	return g_adsBridge.getStringConstPointer(g_adsBridge.equipmentId());
}

bool AdsLoadConfiguration(const char* fileName)
{
	g_lastLoadedConfiguration.clear();

	QFile file{fileName};
	if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		g_log.writeError(QString("AdsLoadConfiguration(): Cannot open file: %1, error: %2").arg(fileName).arg(file.errorString()));
		return false;
	}

	g_lastLoadedConfiguration = file.readAll();
	return g_adsBridge.setConfiguration(g_lastLoadedConfiguration, SettingsProfile::DEFAULT);
}

bool AdsSetConfiguration(const char* configurationXml, size_t size)
{
	g_lastLoadedConfiguration = QByteArray::fromRawData(configurationXml, size);
	return g_adsBridge.setConfiguration(g_lastLoadedConfiguration, SettingsProfile::DEFAULT);
}

bool AdsSetConfigurationProfile(const char* profile)
{
	if (g_lastLoadedConfiguration.isEmpty() == true)
	{
		g_log.writeError("AdsSetConfigurationProfile(): No configuration loaded.");
		return false;
	}

	return g_adsBridge.setConfiguration(g_lastLoadedConfiguration, profile);
}

void AdsShutdown()
{
	g_log.writeMessage("AdsShutdown()");
	g_adsBridge.close();

	if (g_appThread.joinable() == true)
	{
		assert(g_app != nullptr);

		if (g_app != nullptr)
		{
			// Application will not exit until we call QCoreApplication::quit
			// So we can check g_app for nullptr and then call QCoreApplication::quit
			//
			QTimer::singleShot(0, g_app, &QCoreApplication::quit);
		}

		g_appThread.join();
	}
}

void AdsAddConnection(const char* adsEquipmentId, const char* address, int port)
{
	g_log.writeMessage(QString("AdsAddConnection: %1, %2:%3").arg(adsEquipmentId).arg(address).arg(port));
	g_adsBridge.addAppDataService(adsEquipmentId, address, port);
}

void AdsConnect()
{
	g_log.writeMessage("AdsConnect()");
	g_adsBridge.connect();
}

void AdsClose()
{
	g_log.writeMessage("AdsClose()");
	g_adsBridge.close();
}

size_t AdsGetConnectionCount()
{
	return g_adsBridge.connectionCount();
}

bool AdsGetConnectionStatuses(struct AdsConnectionStatus* out, size_t count)
{
	return g_adsBridge.connectionStatus(out, sizeof(AdsConnectionStatus), count);
}

bool AdsSignalParamsLoaded()
{
	return g_adsBridge.signalParamsLoaded();
}

bool AdsSignalStatesLoaded()
{
	return g_adsBridge.signalStatesLoaded();
}

size_t AdsGetSignalCount()
{
	return g_adsBridge.signalCount();
}

bool AdsGetSignalList(MatsSignalHash* out, size_t count)
{
	return g_adsBridge.signalList(out, count);
}

bool AdsGetSignalParams(const MatsSignalHash* signalHashes, MatsAppSignalParam* out, size_t count)
{
	return g_adsBridge.signalParams(signalHashes, out, count);
}

bool AdsGetSignalStates(const MatsSignalHash* signalHashes, MatsAppSignalState* out, size_t count)
{
	return g_adsBridge.signalStates(signalHashes, out, count);
}

MatsSignalHash AdsCalcHash(const char* string)
{
	return ::calcHash(QString{string});
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
	matsTestAppSignalParam.unit = (char*)0xBBAADDFF7711AA99ull;
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