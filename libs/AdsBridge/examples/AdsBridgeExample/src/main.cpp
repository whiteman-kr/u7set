#include <AdsBridge/AdsBridge.h>

#include "dump.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <stop_token>
#include <string>
#include <syncstream>
#include <thread>


struct AppDataService
{
	std::string equipmentId;
	std::string address;
	int port;
};

// EquipmentID of this part of the system.
constexpr std::string_view g_equipmentId = "AZPZ_WS1_ADSBRIDGE";

// Set to true if this is a Qt application and it runs message loop, otherwise message loop will be run in separate thread by AdsBridge.
constexpr bool g_isQtApplication = false;

// The list of AppDataServices to connect to.
const std::array g_appDataServices = {
	AppDataService{"AZPZ_WS1_ADS", "127.0.0.1", 13323} /*, AppDataService{"AZPZ_WS2_ADS", "127.0.0.2", 13323}*/};


extern "C" void log_handler(MatsLogLevel level, const char* message)
{
	const char* logLevelString = nullptr;
	switch (level)
	{
	case MATS_LOG_LEVEL_DEBUG:
		logLevelString = "DBG: ";
		break;
	case MATS_LOG_LEVEL_WARNING:
		logLevelString = "WRN: ";
		break;
	case MATS_LOG_LEVEL_ERROR:
		logLevelString = "ERR: ";
		break;
	default:
		logLevelString = "UNK: ";
		break;
	}

	std::osyncstream{std::cerr} << logLevelString << message << "\n";
	return;
}


void adsLoop(std::stop_token stoken)
{
	// Wait until all signal params and states are loaded or timeout occurs.
	//
	using namespace std::chrono_literals;
	const auto timeout = 5s;
	auto start = std::chrono::steady_clock::now();

	while (std::chrono::steady_clock::now() - start < timeout)
	{
		if (stoken.stop_requested() == true)
		{
			return;
		}

		if (AdsSignalParamsLoaded() == true && AdsSignalStatesLoaded() == true)
		{
			// All signal params and states are loaded.
			//
			break;
		}

		std::this_thread::sleep_for(250ms);
	}

	// Get signal list.
	//
	size_t signalCount = AdsGetSignalCount();
	std::cout << "Signal count: " << signalCount << "\n";

	auto signalHashes = std::make_unique<MatsSignalHash[]>(signalCount);
	auto signalParams = std::make_unique<MatsAppSignalParam[]>(signalCount);

	int32_t getOk = AdsGetSignalList(signalHashes.get(), signalCount);

	if (getOk == 0)
	{
		signalHashes.reset();
		signalParams.reset();

		std::cerr << "Failed to get signal list.\n";
	}

	if (signalHashes != nullptr)
	{
		// Get signal params.
		//
		getOk = AdsGetSignalParams(signalHashes.get(), signalParams.get(), signalCount);

		if (getOk != 0)
		{
#if 1
			for (size_t i = 0; i < signalCount; ++i)
			{
				dumpAppSignalParam(signalParams[i]);
			}
#endif
		}
		else
		{
			signalHashes.reset();
			signalParams.reset();

			std::cerr << "Failed to get signal params.\n";
		}
	}

	// Loop, print some info, exit if user hits Enter.
	//

	// Signal IDs to get states for.
	constexpr std::array<std::string_view, 2> signalIds{"#AZPZ_RACK1_CH01_MD00_CTRLIN_INH02A", "#AZPZ_RACK1_CH01_MD00_CTRLIN_INH03A"};
	std::array<MatsSignalHash, 2> hashes{AdsCalcHash(signalIds[0].data()), AdsCalcHash(signalIds[1].data())};
	std::array<MatsAppSignalState, hashes.size()> states{};

	while (stoken.stop_requested() == false)
	{
#if 0
		dumpConnectionStatus();
#endif
		AdsGetSignalStates(hashes.data(), states.data(), states.size());

		dumpAppSignalState(states[0], signalIds[0]);
		dumpAppSignalState(states[1], signalIds[1]);

		std::cout << "\nHit Enter to exit...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return;
}

int main(int argc, char* argv[])
{
	AdsSetLogHandler(log_handler);
	AdsSetLogLevel(MATS_LOG_LEVEL_DEBUG);

#if 1
	// Set configuration manually
	//

	// Initialize, create QCoreApplication, start Qt message loop if g_isQtApplication is false!
	//
	bool initOk = AdsInit(g_equipmentId.data());
	if (initOk == false)
	{
		std::cerr << "Failed to initialize AdsBridge.\n";
		return EXIT_FAILURE;
	}

	// Adds connections does not start communication, it just adds connections to the list.
	//
	for (const auto& ads : g_appDataServices)
	{
		AdsAddService(ads.equipmentId.c_str(), ads.address.c_str(), ads.port);
	}
#else
	// Load configuration from file.
	//
	bool loadOk = AdsLoadConfiguration("Configuration.xml");
	if (loadOk == false)
	{
		std::cerr << "Failed to load configuration.\n";
		return EXIT_FAILURE;
	}

	bool setProfileOk = AdsSetConfigurationProfile("Local");
	if (setProfileOk == false)
	{
		std::cerr << "Failed to set configuration profile.\n";
		return EXIT_FAILURE;
	}

	bool initOk = AdsInit(AdsGetSoftwareId());
	if (initOk == false)
	{
		std::cerr << "Failed to initialize AdsBridge.\n";
		return EXIT_FAILURE;
	}
#endif

	// Connects to all added services.
	//
	AdsConnect();

	// Process signal params and states in a separate thread.
	//
	{
		std::jthread exitProgramThread{adsLoop};

		// Wait for user input to exit (hit Enter) then signal to stop the thread.
		//
		std::string input;
		std::getline(std::cin, input);

		exitProgramThread.request_stop();
	}

	// Close all connections.
	//
	AdsCloseConnection();

	// Shutdown, stop Qt message loop.
	//
	AdsShutdown();

	return EXIT_SUCCESS;
}