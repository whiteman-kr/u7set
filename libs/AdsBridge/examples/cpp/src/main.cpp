#include <AdsBridge/AdsBridge.h>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <span>
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
const std::string g_equipmentId = "AZPZ_WS1_ADSBRIDGE";

// Set to true if this is a Qt application and it runs message loop, otherwise message loop will be run in separate thread by AdsBridge.
const bool g_isQtApplication = false;

// The list of AppDataServices to connect to.
const std::array g_appDataServices = {AppDataService{"AZPZ_WS1_ADS", "127.0.0.1", 13323}/*, AppDataService{"AZPZ_WS2_ADS", "127.0.0.2", 13323}*/};

// Exit program flag.
//
std::atomic<bool> g_exitProgram{false};


extern "C" void log_handler(MatsLogLevel level, const char* message)
{
	const char* logLevelString = nullptr;
	switch (level)
	{
	case LOG_LEVEL_DEBUG:
		logLevelString = "DBG: ";
		break;
	case LOG_LEVEL_WARNING:
		logLevelString = "WRN: ";
		break;
	case LOG_LEVEL_ERROR:
		logLevelString = "ERR: ";
		break;
	default:
		logLevelString = "UNK: ";
		break;
	}

	std::osyncstream{std::cerr} << logLevelString << message << "\n";
	return;
}

void dumpConnectionStatus(const AdsConnectionStatus& status)
{
	std::string setConnectionResultString;
	switch (status.setConnectionResult)
	{
	case ADS_SET_CONNECTION_RESULT_UNDEFINED:
		setConnectionResultString = "Undefined";
		break;
	case ADS_SET_CONNECTION_RESULT_OK:
		setConnectionResultString = "Ok";
		break;
	case ADS_SET_CONNECTION_RESULT_UNKNOWN_CLIENT_ID:
		setConnectionResultString = "UnknownClientId";
		break;
	case ADS_SET_CONNECTION_RESULT_WRONG_CLIENT_HOST_NAME:
		setConnectionResultString = "WrongClientHostName";
		break;
	case ADS_SET_CONNECTION_RESULT_WRONG_SERVER_ID:
		setConnectionResultString = "WrongServerId";
		break;
	default:
		assert(false);
		setConnectionResultString = "Unknown";
		break;
	}

	std::cout << "Connection: " << status.connectionType << "\n";
	std::cout << "\tid: " << std::hex << status.id << std::dec << "\n";
	std::cout << "\tConnection status: " << (status.status ? "Ok" : "NoConnection") << "\n";
	std::cout << "\tSetConnectionResult: " << setConnectionResultString << "\n";
	std::cout << "\tPort: " << status.port << "\n";
	std::cout << "\tAddress: " << status.address << "\n";
	std::cout << "\tEquipment ID: " << status.adsEquipmentId << "\n";
	std::cout << "\tReceived: " << status.received << "\n";
	std::cout << "\tSent: " << status.sent << "\n";
	std::cout << "\tRequest count: " << status.requestCount << "\n";
	std::cout << "\tReply count: " << status.replyCount << "\n";
	return;
}

void dumpConnectionStatus()
{
	std::cout << "------< ConnectionStatus >------\n";

	const auto count = AdsGetConnectionCount();

	auto stats = std::make_unique<AdsConnectionStatus[]>(count);
	bool getOk = AdsGetConnectionStatuses(stats.get(), count);

	if (getOk == true)
	{
		for (const auto& status : std::span{stats.get(), count})
		{
			dumpConnectionStatus(status);
		}
	}

	return;
}

void dumpAppSignalParam(const MatsAppSignalParam& signalParam)
{
	std::cout << "AppSignalParam: " << signalParam.appSignalId << "\n";
	std::cout << "\thash: " << std::hex << signalParam.hash << std::dec << "\n";
	std::cout << "\tcustomSignalId: " << signalParam.customSignalId << "\n";
	std::cout << "\tcaption: " << signalParam.caption << "\n";
	std::cout << "\tequipmentId: " << signalParam.equipmentId << "\n";
	std::cout << "\tlmEquipmentId: " << signalParam.lmEquipmentId << "\n";

	std::cout << "\tunit: " << signalParam.unit << "\n";
	std::cout << "\ttags: " << signalParam.tags << "\n";

	std::cout << "\tchannel: " << signalParam.channel << "\n";
	std::cout << "\tinOutType: " << signalParam.inOutType << "\n";
	std::cout << "\ttype: " << signalParam.type << "\n";
	std::cout << "\tdecimalPlaces: " << signalParam.decimalPlaces << "\n";

	std::cout << "\tlowValidRange: " << signalParam.lowValidRange << "\n";
	std::cout << "\thighValidRange: " << signalParam.highValidRange << "\n";

	std::cout << "\ttuning: " << signalParam.tuning << "\n";

	return;
}

void dumpAppSignalState(const MatsAppSignalState& state, std::string_view appSignalId)
{
	std::cout << "AppSignalState: " << state.value << (state.flags & MATS_FLAG_VALID ? " (VALID)" : " (NOT_VALID)");
	if (appSignalId.empty() == false)
	{
		std::cout << " - " << appSignalId;
	}
	std::cout << "\n";

	std::cout << "\thash: " << std::hex << state.hash << std::dec << "\n";

	std::cout << "\tflags: ";
	std::cout << (state.flags & MATS_FLAG_VALID ? "VALID" : "NOT_VALID ");
	std::cout << (state.flags & MATS_FLAG_STATE_AVAILABLE ? "" : "STATE_NOT_AVAILABLE ");
	std::cout << (state.flags & MATS_FLAG_SIMULATED ? "SIMULATED " : "");
	std::cout << (state.flags & MATS_FLAG_BLOCKED ? "BLOCKED " : "");
	std::cout << (state.flags & MATS_FLAG_MISMATCH ? "MISMATCH " : "");
	std::cout << (state.flags & MATS_FLAG_ABOVE_HIGH_LIMIT ? "ABOVE_HIGH_LIMIT " : "");
	std::cout << (state.flags & MATS_FLAG_BELOW_LOW_LIMIT ? "BELOW_LOW_LIMIT " : "");
	std::cout << (state.flags & MATS_FLAG_SW_SIMULATED ? "SW_SIMULATED " : "");
	std::cout << (state.flags & MATS_FLAG_TUNING_DEFAULT ? "TUNING_DEFAULT " : "");
	std::cout << "\n";

	std::cout << "\tplantTime: " << state.plantTime << "\n";
	std::cout << "\tserverTime: " << state.serverTime << "\n";

	return;
}

int main(int argc, char* argv[])
{
	AdsSetLogHandler(log_handler);
	AdsSetLogLevel(LOG_LEVEL_DEBUG);

#if 1
	// Set configuration manually
	//

	// Initialize, create QCoreApplication, start Qt message loop if g_isQtApplication is false!
	//
	bool initOk = AdsInit(argc, argv, g_equipmentId.c_str(), g_isQtApplication);
	if (initOk == false)
	{
		std::cerr << "Failed to initialize AdsBridge.\n";
		return EXIT_FAILURE;
	}

	// Adds connections does not start communication, it just adds connections to the list.
	//
	for (const auto& ads : g_appDataServices)
	{
		AdsAddConnection(ads.equipmentId.c_str(), ads.address.c_str(), ads.port);
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

	bool initOk = AdsInit(argc, argv, AdsGetSoftwareId(), g_isQtApplication);
	if (initOk == false)
	{
		std::cerr << "Failed to initialize AdsBridge.\n";
		return EXIT_FAILURE;
	}
#endif

	// Connects to all added services.
	//
	AdsConnect();

	// This thread waits for user input to exit the program.
	//
	std::jthread exitProgramThread{[]()
								   {
									   std::string input;
									   std::getline(std::cin, input);
									   g_exitProgram.store(true);
								   }};

	// Wait until all signal params and states are loaded or timeout occurs.
	//
	{
		using namespace std::chrono;
		const auto timeout = seconds(5);
		auto start = steady_clock::now();

		while (steady_clock::now() - start < timeout && g_exitProgram.load() == false)
		{
			if (AdsSignalParamsLoaded() == true && AdsSignalStatesLoaded() == true)
			{
				// All signal params and states are loaded.
				//
				break;
			}

			std::this_thread::sleep_for(milliseconds(250));
		}
	}

	// Get signal list.
	//
	size_t signalCount = AdsGetSignalCount();
	std::cout << "Signal count: " << signalCount << "\n";

	auto signalHashes = std::make_unique<MatsSignalHash[]>(signalCount);
	auto signalParams = std::make_unique<MatsAppSignalParam[]>(signalCount);

	bool getOk = AdsGetSignalList(signalHashes.get(), signalCount);

	if (getOk == false)
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

		if (getOk == true)
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
	std::array<const char*, 2> signalIds{"#AZPZ_RACK1_CH01_MD00_CTRLIN_INH02A", "#AZPZ_RACK1_CH01_MD00_CTRLIN_INH03A"};
	std::array<MatsSignalHash, 2> hashes{AdsCalcHash(signalIds[0]), AdsCalcHash(signalIds[1])};
	std::array<MatsAppSignalState, hashes.size()> states{};

	while (g_exitProgram.load() == false)
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

	// Close all connections.
	//
	AdsClose();

	// Shutdown, stop Qt message loop.
	//
	AdsShutdown();

	return EXIT_SUCCESS;
}