#include "TestSettings.hpp"

#include <gtest/gtest.h>

#include <fstream>

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	// --help
	//
	if (argc == 2 && (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h"))
	{
		std::cout << "\nGatewayTests\n";
		std::cout << "Usage: GatewayTests [--address=address] [--port=port] [--signal-ids=<filename.csv>]\n\n";
		std::cout << "Options:\n";
		std::cout << "  --ads-address=address        Address of the AdsGateway server (default: 127.0.0.1)\n";
		std::cout << "  --ads-port=port              Port of the AdsGateway server (default: 5566)\n";
		std::cout << "  --ads-signal-ids=<filename>  CSV file containing signal IDs to be used in tests\n";
		std::cout << "  --tuning-address=address     Address of the TuningGateway server (default: 127.0.0.1)\n";
		std::cout << "  --tuning-port=port           Port of the AdsGateway server (default: 5576)\n";
		std::cout << "\n";
		std::cout << "AppDataService CSV Format:\n";
		std::cout << "  Separator: semicolon (';')\n";
		std::cout << "  Column 1: AppSignalID\n";
		std::cout << "  Column 2: ExpectedValue, can be NaN for skipping value check\n";
		std::cout << "  No header row\n";

		// Google Test will print help message.
		//
		return RUN_ALL_TESTS();
	}

	for (int i = 0; i < argc; i++)
	{
		std::string_view arg{argv[i]};

		if (arg.starts_with("--address=") == true) // Old option, kept for backward compatibility
		{
			AdsTestSettings::Address = arg.substr(10);
			continue;
		}

		if (arg.starts_with("--ads-address=") == true)
		{
			AdsTestSettings::Address = arg.substr(14);
			continue;
		}

		if (arg.starts_with("--port=") == true) // Old option, kept for backward compatibility
		{
			try
			{
				AdsTestSettings::Port = static_cast<uint16_t>(std::stoul(std::string{arg.substr(7)}));
				TuningTestSettings::Port = static_cast<uint16_t>(std::stoul(std::string{arg.substr(7)}));
			}
			catch (const std::exception&)
			{
				std::cout << "Error: Invalid AdsGateway port number specified: " << arg.substr(7) << "\n";
				return EXIT_FAILURE;
			}
			continue;
		}

		if (arg.starts_with("--ads-port=") == true)
		{
			try
			{
				AdsTestSettings::Port = static_cast<uint16_t>(std::stoul(std::string{arg.substr(11)}));
			}
			catch (const std::exception&)
			{
				std::cout << "Error: Invalid AdsGateway port number specified: " << arg.substr(7) << "\n";
				return EXIT_FAILURE;
			}
			continue;
		}

		if (arg.starts_with("--signal-ids=") == true)
		{
			std::string fileName{arg.substr(13)};

			std::ifstream file{fileName};
			if (file.is_open() == false)
			{
				std::cout << "Error: Cannot open file " << fileName << "\n";
				return EXIT_FAILURE;
			}

			AdsTestSettings::projectSignals.reserve(32);

			std::string line;
			while (std::getline(file, line))
			{
				if (line.empty())
				{
					continue;
				}

				auto semicolonPos = line.find_first_of(';');
				if (semicolonPos == std::string::npos || (semicolonPos + 1) == line.length())
				{
					std::cout << "Error: Wrong csv file format, no ';' was found\n";
					return EXIT_FAILURE;
				}

				AdsTestSettings::ProjectSignal signal;
				signal.appSignalId = line.substr(0, semicolonPos);
				signal.hash = Radiy::calcHash(signal.appSignalId);
				try
				{
					signal.expectedValue = std::stod(line.substr(semicolonPos + 1));
				}
				catch (const std::invalid_argument&)
				{
					// If value is "NaN", skip value check by setting to NaN
					//
					if (line.substr(semicolonPos + 1) == "NaN" || line.substr(semicolonPos + 1) == "nan")
					{
						signal.expectedValue = std::numeric_limits<double>::quiet_NaN();
					}
					else
					{
						std::cout << "Error: Invalid expected value in CSV: " << line.substr(semicolonPos + 1) << "\n";
						return EXIT_FAILURE;
					}
				}
				catch (const std::out_of_range&)
				{
					std::cout << "Error: Expected value out of range in CSV: " << line.substr(semicolonPos + 1) << "\n";
					return EXIT_FAILURE;
				}

				AdsTestSettings::projectSignals.push_back(std::move(signal));
			}
		}

		if (arg.starts_with("--tuning-address=") == true)
		{
			TuningTestSettings::Address = arg.substr(17);
			continue;
		}

		if (arg.starts_with("--tuning-port=") == true)
		{
			try
			{
				TuningTestSettings::Port = static_cast<uint16_t>(std::stoul(std::string{arg.substr(14)}));
			}
			catch (const std::exception&)
			{
				std::cout << "Error: Invalid TuningGateway port number specified: " << arg.substr(7) << "\n";
				return EXIT_FAILURE;
			}
			continue;
		}
	}

	std::cout << "AdsGateway:\n";
	std::cout << "\tAddress: " << AdsTestSettings::Address << "\n";
	std::cout << "\tPort: " << AdsTestSettings::Port << "\n";
	std::cout << "\tSignals: " << AdsTestSettings::projectSignals.size() << "\n\n";

	std::cout << "TuningGateway:\n";
	std::cout << "\tAddress: " << TuningTestSettings::Address << "\n";
	std::cout << "\tPort: " << TuningTestSettings::Port << "\n\n";

	return RUN_ALL_TESTS();
}