#include <GatewayClientLib/Logger.hpp>
#include <GatewayClientLib/TuningGwConnection.hpp>
#include <GatewayClientLib/TuningGwProtocol.hpp>
#include <GatewayClientLib/TuningSignalManager.hpp>

#include <algorithm>
#include <cctype>
#include <format>
#include <iomanip>
#include <iostream>
#include <span>
#include <string>
#include <syncstream>


void printHelp()
{
	std::osyncstream(std::cout) << "Available commands:\n"
								<< "  help, h, ? - Show this help message\n"
								<< "  exit, bye, quit, q - Exit the program\n"
								<< "  tt, t - Toggle trace logging\n"
								<< "\nSignals:\n"
								<< "  value, v #SIGNALID - Get the current value of a signal\n"
								<< "  param, p #SIGNALID - Get the parameters of a signal\n"
								<< "\nTuning Sources:\n"
								<< "  ts list - Get the list of tuning sources, short 'ts l'\n"
								<< "  ts status LM_EQUIPMENT_ID - Get tuning source information, short 'ts s ID'\n"
								<< "  ts activate LM_EQUIPMENT_ID - Activate tuning source, short 'ts a ID'\n"
								<< "  ts deactivate - Deactivate current tuning source, short 'ts d' \n"
								<< "  \n<Enter> - Repeat last command\n"
								<< std::endl;
	return;
}

std::string activeTuningSourceId(std::span<const GatewayClientLib::GwTuningSourceState> tuningSources)
{
	auto it = std::find_if(tuningSources.begin(),
						   tuningSources.end(),
						   [](const GatewayClientLib::GwTuningSourceState& ts)
						   {
							   return ts.controlIsActive != 0;
						   });

	if (it == tuningSources.end())
	{
		return {};
	}

	return std::string{it->moduleEquipmentId};
}

void printSignalValue(const GatewayClientLib::TuningSignalManager& signalManager, std::string_view signalId)
{
	auto signalState = signalManager.getSignalState(signalId);
	if (signalState.has_value() == false)
	{
		std::cout << "Signal state not available: " << signalId << std::endl;
		return;
	}

	std::cout << "Signal ID: " << signalId << "\n"
			  << "  Hash: " << "0x" << std::hex << std::setw(16) << std::setfill('0') << signalState->hash << std::dec << "\n"
			  << "  ErrorCode: " << to_string(static_cast<GatewayClientLib::GwErrorCode>(signalState->errorCode)) << "\n"
			  << "  Value: " << signalState->value << "\n"
			  << "  Flags: " << to_string(static_cast<GatewayClientLib::GwTuningSignalStateFlags>(signalState->flags)) << "\n"
			  << "  Successful Read Time: " << signalState->successfulReadTime << "\n"
			  << "  Write Request Time: " << signalState->writeRequestTime << "\n"
			  << "  Successful Write Time: " << signalState->successfulWriteTime << "\n"
			  << "  Unsuccessful Write Time: " << signalState->unsuccessfulWriteTime << "\n"
			  << "  LogicModule Time: " << signalState->lmTime << "\n"
			  << "  FOTIP Processing Numerator: " << signalState->fotipProcessingNumerator << "\n";

	return;
}

void printSignalParam(const GatewayClientLib::TuningSignalManager& signalManager, std::string_view signalId)
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

void printTuningSourceState(const GatewayClientLib::GwTuningSourceState& tuningSourceState)
{
	auto yesNo = [](uint8_t value)
	{
		return value != 0 ? "Yes" : "No";
	};

	std::cout << "Tuning Source:\n"
			  << "  Source ID: 0x" << std::hex << std::setw(16) << std::setfill('0') << tuningSourceState.sourceId << std::dec << "\n"
			  << "  Module Equipment ID: " << tuningSourceState.moduleEquipmentId << "\n"
			  << "  LAN Equipment ID: " << tuningSourceState.lanEquipmentId << "\n"
			  << "  Is Replying: " << yesNo(tuningSourceState.isReplying) << "\n"
			  << "  Control Is Active: " << yesNo(tuningSourceState.controlIsActive) << "\n"
			  << "  Set SOR: " << yesNo(tuningSourceState.setSOR) << "\n"
			  << "  Writing Disabled: " << yesNo(tuningSourceState.writingDisabled) << "\n"
			  << "  Build Mismatch: " << yesNo(tuningSourceState.buildMismatch) << "\n"
			  << "  Has Unapplied Params: " << yesNo(tuningSourceState.hasUnappliedParams) << "\n"
			  << "  LogicModule Time: " << tuningSourceState.lmTime << "\n"
			  << std::endl;

	return;
}

void printTuningSourceState(std::span<const GatewayClientLib::GwTuningSourceState> tuningSources, std::string_view equipmentId)
{
	auto it = std::find_if(tuningSources.begin(),
						   tuningSources.end(),
						   [equipmentId](const GatewayClientLib::GwTuningSourceState& ts)
						   {
							   return std::string_view{ts.moduleEquipmentId} == equipmentId;
						   });
	if (it == tuningSources.end())
	{
		std::cout << equipmentId << " not found.\n";
		std::cout << " Available tuning sources:\n";
		for (const auto& ts : tuningSources)
		{
			std::cout << "\t" << std::string_view{ts.moduleEquipmentId} << "\n";
		}
	}
	else
	{
		printTuningSourceState(*it);
	}

	return;
}

void printTuningSourceList(std::span<const GatewayClientLib::GwTuningSourceState> tuningSources)
{
	for (const auto& ts : tuningSources)
	{
		printTuningSourceState(ts);
	}

	return;
}

GatewayClientLib::GwErrorCode printCommandResult(std::future<GatewayClientLib::GwErrorCode> future)
{
	if (future.valid() == false)
	{
		assert(future.valid());
		std::cout << "Invalid command result.\n";
		return GatewayClientLib::GwErrorCode::GWC_CLIENT_INTERNAL_ERROR;
	}

	if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready)
	{
		GatewayClientLib::GwErrorCode errorCode = future.get();
		std::cout << "Command result: " << to_string(errorCode) << "\n";
		return errorCode;
	}
	else
	{
		std::cout << "Command result not available yet.\n";
		return GatewayClientLib::GwErrorCode::GWC_CLIENT_INTERNAL_ERROR;
	}
}

std::string trim(std::string_view s)
{
	std::string result{s};

	// Left trim
	//
	result.erase(result.begin(),
				 std::find_if(result.begin(),
							  result.end(),
							  [](unsigned char ch)
							  {
								  return !std::isspace(ch);
							  }));

	// Right trim
	//
	result.erase(std::find_if(result.rbegin(),
							  result.rend(),
							  [](unsigned char ch)
							  {
								  return !std::isspace(ch);
							  })
					 .base(),
				 result.end());

	return result;
}

int main()
{
	std::cout << "Tuning Gateway Client Example\n";
	printHelp();

	std::string_view address = "127.0.0.1";
	uint16_t port = GatewayClientLib::TUNING_GW_PORT;

	GatewayClientLib::TuningSignalManager signalManager;

	GatewayClientLib::ConsoleLogger logger;
	logger.setTraceEnabled(false);

	{
		std::cout << "Creating TCP connection...\n";
		std::cout << "\t address: " << address << "\n";
		std::cout << "\t port: " << port << "\n";

		GatewayClientLib::TuningGwConnection conn{signalManager, logger};
		conn.connect(address, port, "CLIENTID");

		std::string lastLine;

		while (true)
		{
			// Prompt: "A [TS: SDS_RC11_CH01_LM01]>", "O [TS: SDS_RC11_CH01_LM01]>"
			//
			std::string_view clientStatus = conn.clientIsActive() ? "A" : "O";
			std::string prompt = std::format("{} [TS:{}] >", clientStatus, activeTuningSourceId(conn.tuningSources()));
			std::cout << prompt;

			// Get command
			//
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

			if (line.starts_with("ts "))
			{
				size_t spacePos = line.find(' ');
				if (spacePos == std::string::npos || spacePos + 1 >= line.size())
				{
					std::cout << "Invalid command format." << std::endl;
					continue;
				}

				std::string argument = trim(line.substr(spacePos + 1));

				if (argument == "list" || argument == "l")
				{
					printTuningSourceList(conn.tuningSources());
					continue;
				}

				if (argument.starts_with("status ") == true || argument.starts_with("s ") == true)
				{
					std::string equipmentId = argument.starts_with("status ") ? trim(std::string_view{argument}.substr(7)) :
																				trim(std::string_view{argument}.substr(2));

					if (equipmentId.empty() == true)
					{
						std::cout << "Invalid command format. Usage: ts status LM_EQUIPMENT_ID" << std::endl;
						continue;
					}

					printTuningSourceState(conn.tuningSources(), equipmentId);
					continue;
				}

				if (argument.starts_with("activate ") == true || argument.starts_with("a ") == true)
				{
					std::string equipmentId = argument.starts_with("activate ") ? trim(std::string_view{argument}.substr(9)) :
																				  trim(std::string_view{argument}.substr(2));

					if (equipmentId.empty() == true)
					{
						std::cout << "Invalid command format. Usage: ts activate LM_EQUIPMENT_ID" << std::endl;
						continue;
					}

					auto future = conn.commandActivateTuningSource(equipmentId);
					printCommandResult(std::move(future));
					continue;
				}

				if (argument == "deactivate" || argument == "d")
				{
					std::cout << "Deactivate current tuning source" << std::endl;

					auto future = conn.commandDeactivateTuningSource();
					printCommandResult(std::move(future));
					continue;
				}

				std::cout << "Invalid command format.\n";
				printHelp();
				continue;
			}
		}
	}

	std::cout << "Finished." << std::endl;
	return 0;
}