#include "AdsGwConnection.hpp"
#include "Logger.hpp"
#include "SignalManager.hpp"

#include <AdsGatewayLib/AdsGwProtocol.hpp>

#include <iostream>
#include <string>


void printHelp()
{
	std::osyncstream(std::cout) << "Available commands:\n"
								<< "  help, h - Show this help message\n"
								<< "  exit, bye, quit, q - Exit the program\n\n";
}

int main()
{
	std::cout << "ADS Gateway Client Example\n";
	printHelp();

	std::string_view address = "127.0.0.1";
	uint16_t port = AdsGatewayLib::ADSGW_PORT;

	AdsGatewayLib::SignalManager signalManager;

	AdsGatewayLib::ConsoleLogger logger;
	logger.setTraceEnabled(true);

	{
		std::cout << "Creating TCP connection...\n";
		std::cout << "\t address: " << address << "\n";
		std::cout << "\t port: " << port << "\n";

		AdsGatewayLib::AdsGwConnection conn{signalManager, logger};
		conn.connect(address, port, "CLIENTID");

		while (true)
		{
			std::string line;
			std::getline(std::cin, line);
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);

			if (line == "help" || line == "h")
			{
				printHelp();
				continue;
			}

			if (line == "exit" || line == "bye" || line == "quit" || line == "q")
			{
				break;
			}
		}
	}

	std::cout << "Finished." << std::endl;
	return 0;
}