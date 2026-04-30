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

		m_conn = std::make_unique<GatewayClientLib::TuningGwConnImpl>(m_signalUpdater, m_logger);

		m_thread =
			std::jthread{[addressStr = std::string{address}, port, equipmentIdStr = std::string{equipmentId}, this](std::stop_token stoken)
						 {
							 m_conn->run(stoken, addressStr, port, equipmentIdStr);
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

		assert(m_conn != nullptr);
		m_conn.reset();

		m_signalUpdater.reset();
		return;
	}

	std::future<GwErrorCode> TuningGwConnection::commandActivateTuningSource(std::string_view tuningSourceId)
	{
		if (m_conn == nullptr)
		{
			std::promise<GatewayClientLib::GwErrorCode> promise;
			promise.set_value(GwErrorCode::GWC_COMMUNICATION_ERROR);
			return promise.get_future();
		}

		return m_conn->commandActivateTuningSource(tuningSourceId, true);
	}

	std::future<GwErrorCode> TuningGwConnection::commandDeactivateTuningSource(std::string_view tuningSourceId)
	{
		if (m_conn == nullptr)
		{
			std::promise<GatewayClientLib::GwErrorCode> promise;
			promise.set_value(GwErrorCode::GWC_COMMUNICATION_ERROR);
			return promise.get_future();
		}

		return m_conn->commandActivateTuningSource(tuningSourceId, false);
	}

	std::future<WriteValueResult> TuningGwConnection::commandWriteSignalValues(std::span<const GwTuningWriteValue> states,
																			   std::string_view user,
																			   bool apply)
	{
		if (m_conn == nullptr)
		{
			GatewayClientLib::WriteValueResult result{};
			result.errorCode = GwErrorCode::GWC_COMMUNICATION_ERROR;

			std::promise<GatewayClientLib::WriteValueResult> promise;
			promise.set_value(result);
			return promise.get_future();
		}

		return m_conn->commandWriteSignalValues(states, user, apply);
	}

	std::future<GwErrorCode> TuningGwConnection::commandApplyWrittenSignals()
	{
		if (m_conn == nullptr)
		{
			std::promise<GatewayClientLib::GwErrorCode> promise;
			promise.set_value(GwErrorCode::GWC_COMMUNICATION_ERROR);
			return promise.get_future();
		}

		return m_conn->commandApplyWrittenSignals();
	}

	bool TuningGwConnection::clientIsActive() const
	{
		if (m_conn == nullptr)
		{
			return {};
		}

		return m_conn->clientIsActive();
	}

	std::vector<GatewayClientLib::GwTuningSourceState> TuningGwConnection::tuningSources() const
	{
		if (m_conn == nullptr)
		{
			return {};
		}

		return m_conn->tuningSources();
	}
} // namespace GatewayClientLib