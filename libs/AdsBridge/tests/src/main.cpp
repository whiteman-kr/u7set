#include "TestSettings.h"

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	if (argc == 1)
	{
		std::cout << "Usage: AdsBridgeTest [-config=configuration_file] [-profile=profile_name]\n\n";
		return EXIT_SUCCESS;
	}

	// --help
	//
	if (argc == 2 && (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h"))
	{
		std::cout << "\nAdsBridgeTest\n";
		std::cout << "Usage: AdsBridgeTest [--config=configuration_file] [--profile=profile_name]\n\n";
		std::cout << "--config - must be specified, path to the configuration file.\n";
		std::cout << "--profile - optional, profile name to use.\n\n";

		// Google Test will print help message.
		//
		return RUN_ALL_TESTS();
	}

	for (int i = 0; i < argc; i++)
	{
		std::string_view arg{argv[i]};

		if (arg.starts_with("-config=") == true)
		{
			TestSettings::ConfigurationFile = arg.substr(8);
			continue;
		}

		if (arg.starts_with("--config=") == true)
		{
			TestSettings::ConfigurationFile = arg.substr(9);
			continue;
		}

		if (arg.starts_with("-profile=") == true)
		{
			TestSettings::SettingsProfile = arg.substr(9);
			continue;
		}

		if (arg.starts_with("--profile=") == true)
		{
			TestSettings::SettingsProfile = arg.substr(10);
			continue;
		}
	}

	if (TestSettings::ConfigurationFile.empty() == true)
	{
		std::cout << "Error: Configuration file is not specified.\n";
		std::cout << "./AdsBridgeTest [-config=configuration_file] [-profile=profile_name]\n\n";
		return EXIT_FAILURE;
	}

	std::cout << "Configuration file: " << TestSettings::ConfigurationFile << "\n";
	std::cout << "Profile name: " << TestSettings::SettingsProfile << "\n";

	return RUN_ALL_TESTS();
}
