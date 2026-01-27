#include "TestSettings.hpp"

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	// --help
	//
	if (argc == 2 && (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h"))
	{
		std::cout << "\nAdsGatewayTests\n";
		std::cout << "Usage: AdsGatewayTests [--address=address] [--port=port]\n\n";

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
	}

	std::cout << "AdsGateway:\n";
	std::cout << "\tAddress: " << TestSettings::Address << "\n";
	std::cout << "\tPort: " << TestSettings::Port << "\n";

	return RUN_ALL_TESTS();
}