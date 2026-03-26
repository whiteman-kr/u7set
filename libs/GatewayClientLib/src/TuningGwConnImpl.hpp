#pragma once

#include "GwConnImpl.hpp"

#include <GatewayClientLib/TuningGwProtocol.hpp>


namespace GatewayClientLib
{
	class ISignalUpdater;

	class TuningGwConnImpl : public GwConnImpl<TuningGwRequestId>
	{
	public:
		TuningGwConnImpl(ISignalUpdater& signalUpdater, ILogger& logger) :
			GwConnImpl{logger},
			m_signalUpdater{signalUpdater}
		{
		}

		void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId) override;

		// Requests:
		//
	protected:
		void requestHandshake(std::string_view equipmentId, uint16_t protocolVersion = TUNING_GW_PROTOCOL_VERSION);
		std::vector<char> requestTuningSources();
		//std::vector<GwAppSignalParam> requestSignalParams();
		//void requestStateChanges();
		//void requestSignalStates();

	protected:
		ISignalUpdater& m_signalUpdater;
		std::function<bool()> m_isCancelledFunc;

	protected:
		TuningGwHandshakeResponse m_handshakeResponse{};
		//std::vector<std::string> m_appSignalIds{};
		//std::vector<Radiy::Hash> m_appSignalHashes{};

		// Operative buffers used in requests
		//
		//std::vector<GwAppSignalState> m_statesBuffer{};
		//std::vector<Radiy::Hash> m_hashBuffer{};
		//size_t m_nextStateIndexToRequest{0}; // Used only in requestSignalStates()
	};
} // namespace GatewayClientLib