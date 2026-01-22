#pragma once

#include <string>
#include <thread>

namespace adsgw
{
	class AdsGwConnection
	{
	public:
		void connect(std::string_view address, uint16_t port, std::string_view equipmentId);
		void close();

	private:
		std::jthread m_thread;
	};
} // namespace adsgw