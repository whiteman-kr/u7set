#include "AdsGwConnection.hpp"

#include <array>
#include <iostream>

void printHelp()
{
	std::cout << "Available commands:\n";
	std::cout << "  help - Show this help message\n";
	std::cout << "  exit - Exit the program\n";
}

int main()
{
	std::string_view address = "127.0.0.1";
	uint16_t port = 5566;

	{
		std::cout << "Creating TCP connection..." << std::endl;
		std::cout << "\t address: " << address << std::endl;
		std::cout << "\t port: " << port << std::endl;

		adsgw::AdsGwConnection conn;

		conn.connect(address, port, "CLIENTID");

		while (true)
		{
			std::string line;
			std::getline(std::cin, line);

			if (line == "help")
			{
				printHelp();
				continue;
			}

			if (line == "exit")
			{
				break;
			}
		}
	}

	std::cout << "Finished." << std::endl;
	return 0;
}