#include <GatewayClientLib/AdsGwConnection.hpp>
#include <GatewayClientLib/AdsSignalManager.hpp>
#include <GatewayClientLib/Logger.hpp>

#include <iostream>
#include <string>
#include <vector>


void printHelp()
{
	std::osyncstream(std::cout) << "Command line parameters:\n"
								<< "  -address=<ip address> - specifies the gateway service IP address, default value: 127.0.0.1\n"
								<< "  Example: -address=192.168.0.1\n"
								<< "  -port=<port> - specifies the gateway service port, default value: " << GatewayClientLib::ADS_GW_PORT
								<< "\n"
								<< "  Example: -port=5566\n"
								<< "  -id=<Client ID> - specifies the gateway service client ID address, default value: CLIENTID\n"
								<< "  Example: -id=CLIENTID\n"
								<< std::endl;

	std::osyncstream(std::cout) << "Available commands:\n"
								<< "  help, h, ? - Show this help message\n"
								<< "  exit, bye, quit, q - Exit the program\n"
								<< "  tt, t - Toggle trace logging\n"
								<< "  value, v #SIGNALID - Get the current value of a signal\n"
								<< "  param, p #SIGNALID - Get the parameters of a signal\n"
								<< "  <Enter> - Repeat last command\n"
								<< std::endl;
}

void printSignalValue(const GatewayClientLib::AdsSignalManager& signalManager, std::string_view signalId)
{
	auto signalState = signalManager.getSignalState(signalId);
	if (signalState.has_value() == false)
	{
		std::cout << "Signal state not available: " << signalId << std::endl;
		return;
	}

	std::cout << "Signal ID: " << signalId << "\n"
			  << "  Value: " << signalState->value << "\n"
			  << "  Flags: " << to_string(static_cast<GatewayClientLib::GwAppSignalStateFlags>(signalState->flags)) << "\n"
			  << "  System Time: " << signalState->systemTime << "\n"
			  << "  Local Time: " << signalState->localTime << "\n"
			  << "  Plant Time: " << signalState->plantTime << "\n";

	return;
}

void printSignalParam(const GatewayClientLib::AdsSignalManager& signalManager, std::string_view signalId)
{
	auto signalParam = signalManager.getSignalParam(signalId);
	if (signalParam.has_value() == false)
	{
		std::cout << "Signal param not available: " << signalId << std::endl;
		return;
	}

	std::string_view ioTypeStr;
	switch (signalParam->inOutType)
	{
	case GatewayClientLib::InOutType::Input:
		ioTypeStr = "Input";
		break;
	case GatewayClientLib::InOutType::Output:
		ioTypeStr = "Output";
		break;
	case GatewayClientLib::InOutType::Internal:
		ioTypeStr = "Internal";
		break;
	case GatewayClientLib::InOutType::SoftwareCalculated:
		ioTypeStr = "SoftwareCalculated";
		break;
	default:
		assert(false);
		ioTypeStr = "Unknown";
		break;
	}

	std::string_view typeStr;
	switch (signalParam->type)
	{
	case GatewayClientLib::SignalType::Discrete:
		typeStr = "Discrete";
		break;
	case GatewayClientLib::SignalType::SignedInt32:
		typeStr = "Analog(SignedInt32)";
		break;
	case GatewayClientLib::SignalType::Float32:
		typeStr = "Analog(Float32)";
		break;
	case GatewayClientLib::SignalType::Bus:
		typeStr = "Bus";
		break;
	default:
		assert(false);
		typeStr = "Unknown";
		break;
	}

	std::cout << "Signal ID: " << signalId << "\n"
			  << "  Caption: " << signalParam->caption << "\n"
			  << "  Equipment ID: " << signalParam->equipmentId << "\n"
			  << "  LogicModule Equipment ID: " << signalParam->lmEquipmentId << "\n"
			  << "  Units: " << signalParam->units << "\n"
			  << "  Tags: " << signalParam->tags << "\n"
			  << "  Channel: " << ('A' + static_cast<char>(signalParam->channel)) << "\n"
			  << "  I/O Type: " << ioTypeStr << "\n"
			  << "  Type: " << typeStr << "\n"
			  << "  Decimal Places: " << static_cast<int>(signalParam->decimalPlaces) << "\n"
			  << "  Tuning: " << (signalParam->tuning != 0 ? "Tunable" : "Non-tunable") << "\n"
			  << "  Low Valid Range: " << signalParam->lowValidRange << "\n"
			  << "  High Valid Range: " << signalParam->highValidRange << "\n"
			  << "  Tuning Default Value: " << signalParam->tuningDefaultValue << "\n"
			  << "  Tuning Low Bound: " << signalParam->tuningLowBound << "\n"
			  << "  Tuning High Bound: " << signalParam->tuningHighBound << "\n";
	return;
}

struct AppParams
{
	std::string_view address;
	uint16_t port = 0;
	std::string_view clientID;
};

bool parseCommandLine(const std::vector<std::string_view>& args, AppParams& params)
{
	const std::string_view paramAddress = "-address=";
	const std::string_view paramPort = "-port=";
	const std::string_view paramClientID = "-id=";

	for (const auto& arg : args)
	{
		if (arg.starts_with(paramAddress) == true)
		{
			params.address = arg.substr(paramAddress.length(), arg.length() - paramAddress.length());
			if (params.address.empty() == true)
			{
				std::cout << "Error: specified address is empty." << std::endl;
				return false;
			}
		}

		if (arg.starts_with(paramPort) == true)
		{
			const auto [ptr, ec] = std::from_chars(arg.data() + paramPort.length(), arg.data() + arg.size(), params.port);
			if (ec != std::errc())
			{
				std::cout << "Error: specified port is incorrect." << std::endl;
				return false;
			}
		}

		if (arg.starts_with(paramClientID) == true)
		{
			params.clientID = arg.substr(paramClientID.length(), arg.length() - paramClientID.length());
			if (params.clientID.empty() == true)
			{
				std::cout << "Error: specified client ID is empty." << std::endl;
				return false;
			}
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	AppParams params = {.address = "127.0.0.1", .port = GatewayClientLib::ADS_GW_PORT, .clientID = "CLIENTID"};

	std::cout << "ADS Gateway Client Example\n\n";
	printHelp();

	std::vector<std::string_view> args(argv + 1, argv + argc);
	if (parseCommandLine(args, params) == false)
	{
		return -1;
	}

	GatewayClientLib::AdsSignalManager signalManager{};

	GatewayClientLib::ConsoleLogger logger{};
	logger.setTraceEnabled(false);

	{
		std::cout << "Creating TCP connection...\n";
		std::cout << "\t address: " << params.address << "\n";
		std::cout << "\t port: " << params.port << "\n";
		std::cout << "\t client ID: " << params.clientID << "\n";

		GatewayClientLib::AdsGwConnection conn{signalManager, logger};
		conn.connect(params.address, params.port, params.clientID);

		std::string lastLine;

		while (true)
		{
			std::string line;
			std::getline(std::cin, line);
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);

			if (line.empty() == true)
			{
				line = lastLine;
			}

			lastLine = line;

			if (line == "help" || line == "h" || line == "?")
			{
				printHelp();
				continue;
			}

			if (line == "exit" || line == "bye" || line == "quit" || line == "q")
			{
				break;
			}

			if (line == "tt" || line == "t")
			{
				bool traceEnabled = logger.isTraceEnabled();
				logger.setTraceEnabled(!traceEnabled);
				std::cout << "Trace logging " << (traceEnabled ? "disabled." : "enabled.") << std::endl;
				continue;
			}

			if (line.starts_with("value ") || line.starts_with("v "))
			{
				size_t spacePos = line.find(' ');
				if (spacePos == std::string::npos || spacePos + 1 >= line.size())
				{
					std::cout << "Invalid command format. Usage: value <signal_id>" << std::endl;
					continue;
				}

				std::string signalId = line.substr(spacePos + 1);
				printSignalValue(signalManager, signalId);
				continue;
			}

			if (line.starts_with("param ") || line.starts_with("p "))
			{
				size_t spacePos = line.find(' ');
				if (spacePos == std::string::npos || spacePos + 1 >= line.size())
				{
					std::cout << "Invalid command format. Usage: param <signal_id>" << std::endl;
					continue;
				}

				std::string signalId = line.substr(spacePos + 1);
				printSignalParam(signalManager, signalId);
				continue;
			}
		}
	}

	std::cout << "Finished." << std::endl;
	return 0;
}