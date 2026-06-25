#include <gtest/gtest.h>

#include <fstream>

#include <ArchV3Lib/ArchV3Core.h>

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

/* // --help
	//
	if (argc == 2 && (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h"))
	{
		std::cout << "\nAdsGatewayTests\n";
		std::cout << "Usage: AdsGatewayTests [--address=address] [--port=port] [--signal-ids=<filename.csv>]\n\n";
		std::cout << "Options:\n";
		std::cout << "  --address=address       Specify the address of the AdsGateway server (default: 127.0.0.1)\n";
		std::cout << "  --port=port             Specify the port of the AdsGateway server (default: 5566)\n";
		std::cout << "  --signal-ids=<filename> Specify a CSV file containing signal IDs to be used in tests\n";
		std::cout << "\n";
		std::cout << "CSV Format:\n";
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

		if (arg.starts_with("--address=") == true)
		{
			TestSettings::Address = arg.substr(10);
			continue;
		}

		if (arg.starts_with("--port=") == true)
		{
			try
			{
				TestSettings::Port = static_cast<uint16_t>(std::stoul(std::string{arg.substr(7)}));
			}
			catch (const std::exception&)
			{
				std::cout << "Error: Invalid port number specified: " << arg.substr(7) << "\n";
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

			TestSettings::projectSignals.reserve(32);

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

				TestSettings::ProjectSignal signal;
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

				TestSettings::projectSignals.push_back(std::move(signal));
			}
		}
	}

	std::cout << "AdsGateway:\n";
	std::cout << "\tAddress: " << TestSettings::Address << "\n";
	std::cout << "\tPort: " << TestSettings::Port << "\n";
	std::cout << "\tSignals: " << TestSettings::projectSignals.size() << "\n"; */

	//ArchV3::Core("D:\ArchiveTests", "SYSTEM", );

	return RUN_ALL_TESTS();
}