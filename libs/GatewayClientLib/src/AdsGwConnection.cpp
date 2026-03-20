#include <GatewayClientLib/AdsGwConnection.hpp>
#include <GatewayClientLib/ISignalUpdater.hpp>

#include "AdsGwConnImpl.hpp"


namespace GatewayClientLib
{
	AdsGwConnection::AdsGwConnection(ISignalUpdater& signalUpdater, ILogger& logger) :
		m_signalUpdater{signalUpdater},
		m_logger{logger}
	{
	}

	AdsGwConnection::~AdsGwConnection()
	{
		close();
	}

	void AdsGwConnection::connect(std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		if (m_thread.joinable() == true)
		{
			close();
		}

		m_thread =
			std::jthread{[addressStr = std::string{address}, port, equipmentIdStr = std::string{equipmentId}, this](std::stop_token stoken)
						 {
							 AdsGwConnImpl conn{m_signalUpdater, m_logger};
							 conn.run(stoken, addressStr, port, equipmentIdStr);
						 }};
	}

	void AdsGwConnection::close()
	{
		if (m_thread.joinable() == false)
		{
			return;
		}

		m_thread.request_stop();
		m_thread.join();

		m_signalUpdater.reset();
		return;
	}
} // namespace GatewayClientLib