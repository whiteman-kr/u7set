#include <AdsGatewayLib/AdsGwConnection.hpp>
#include <AdsGatewayLib/AdsGwProtocol.hpp>
#include <AdsGatewayLib/Logger.hpp>
#include <AdsGatewayLib/SignalManager.hpp>

#include <iostream>
#include <string>


void printHelp()
{
	std::osyncstream(std::cout) << "Available commands:\n"
								<< "  help, h - Show this help message\n"
								<< "  exit, bye, quit, q - Exit the program\n"
								<< "  tt, t - Toggle trace logging\n"
								<< "  value, v #SIGNALID - Get the current value of a signal\n"
								<< "  param, p #SIGNALID - Get the parameters of a signal\n"
								<< "  <Enter> - Repeat last command\n"
								<< std::endl;
}

void printSignalValue(const AdsGatewayLib::SignalManager& signalManager, std::string_view signalId)
{
	auto signalState = signalManager.getSignalState(signalId);
	if (signalState.has_value() == false)
	{
		std::cout << "Signal state not available: " << signalId << std::endl;
		return;
	}

	std::cout << "Signal ID: " << signalId << "\n"
			  << "  Value: " << signalState->value << "\n"
			  << "  Flags: " << to_string(static_cast<AdsGatewayLib::GwAppSignalStateFlags>(signalState->flags)) << "\n"
			  << "  System Time: " << signalState->systemTime << "\n"
			  << "  Local Time: " << signalState->localTime << "\n"
			  << "  Plant Time: " << signalState->plantTime << "\n";

	return;
}

void printSignalParam(const AdsGatewayLib::SignalManager& signalManager, std::string_view signalId)
{
	auto signalParam = signalManager.getSignalParam(signalId);
	if (signalParam.has_value() == false)
	{
		std::cout << "Signal param not available: " << signalId << std::endl;
		return;
	}

	std::cout << "Signal ID: " << signalId << "\n"
			  << "  Caption: " << signalParam->caption << "\n"
			  << "  Equipment ID: " << signalParam->equipmentId << "\n"
			  << "  LogicModule Equipment ID: " << signalParam->lmEquipmentId << "\n"
			  << "  Units: " << signalParam->units << "\n"
			  << "  Tags: " << signalParam->tags << "\n"
			  << "  Channel: " << static_cast<int>(signalParam->channel) << "\n"
			  << "  I/O Type: " << static_cast<int>(signalParam->inOutType) << "\n"
			  << "  Type: " << static_cast<int>(signalParam->type) << "\n"
			  << "  Decimal Places: " << static_cast<int>(signalParam->decimalPlaces) << "\n"
			  << "  Tuning: " << (signalParam->tuning != 0 ? "Tunable" : "Non-tunable") << "\n"
			  << "  Low Valid Range: " << signalParam->lowValidRange << "\n"
			  << "  High Valid Range: " << signalParam->highValidRange << "\n"
			  << "  Tuning Default Value: " << signalParam->tuningDefaultValue << "\n"
			  << "  Tuning Low Bound: " << signalParam->tuningLowBound << "\n"
			  << "  Tuning High Bound: " << signalParam->tuningHighBound << "\n";
	return;
}

int main()
{
	std::cout << "ADS Gateway Client Example\n";
	printHelp();

	std::string_view address = "127.0.0.1";
	uint16_t port = AdsGatewayLib::ADSGW_PORT;

	AdsGatewayLib::SignalManager signalManager;

	AdsGatewayLib::ConsoleLogger logger;
	logger.setTraceEnabled(false);

	{
		std::cout << "Creating TCP connection...\n";
		std::cout << "\t address: " << address << "\n";
		std::cout << "\t port: " << port << "\n";

		AdsGatewayLib::AdsGwConnection conn{signalManager, logger};
		conn.connect(address, port, "CLIENTID");

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

			if (line == "help" || line == "h")
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