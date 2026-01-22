#include "TcpConnection.hpp"

#include <array>
#include <iostream>

int main()
{
	{
		adsgw::TcpConnection conn;

		bool res = conn.connect("127.0.0.1", 1111);
		std::cout << "Connect result: " << res << ", LasteError: " << conn.lastError() << "\n";

		if (res == true)
		{
			const std::string_view msg = "Hello, TCP Server!";
			std::span<const std::byte> dataSpan{reinterpret_cast<const std::byte*>(msg.data()), msg.size()};

			res = conn.send(dataSpan);
			std::cout << "Send result: " << res << ", LasteError: " << conn.lastError() << "\n";

			std::array<std::byte, 10> dataBuffer{};
			bool received = conn.receive(dataBuffer);
			if (received == true)
			{
				std::string receivedStr{reinterpret_cast<const char*>(dataBuffer.data()), dataBuffer.size()};
				std::cout << "Received " << dataBuffer.size() << " bytes: " << receivedStr << "\n";
			}
			else
			{
				std::cout << "Receive failed, LasteError: " << conn.lastError() << "\n";
			}

			conn.close();
		}
	}

	std::cout << "Hello, World!" << std::endl;
	return 0;
}