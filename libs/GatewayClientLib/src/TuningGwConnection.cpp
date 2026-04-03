#include <GatewayClientLib/TuningGwConnection.hpp>

#include "TuningGwConnImpl.hpp"


namespace GatewayClientLib
{
	TuningGwConnection::TuningGwConnection(ITuningSignalUpdater& signalUpdater, ILogger& logger) :
		m_signalUpdater{signalUpdater},
		m_logger{logger}
	{
	}

	TuningGwConnection::~TuningGwConnection()
	{
		close();
	}

	void TuningGwConnection::connect(std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		if (m_thread.joinable() == true)
		{
			close();
		}

		m_thread =
			std::jthread{[addressStr = std::string{address}, port, equipmentIdStr = std::string{equipmentId}, this](std::stop_token stoken)
						 {
							 TuningGwConnImpl conn{m_signalUpdater, m_logger};
							 conn.run(stoken, addressStr, port, equipmentIdStr);
						 }};
	}

	void TuningGwConnection::close()
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