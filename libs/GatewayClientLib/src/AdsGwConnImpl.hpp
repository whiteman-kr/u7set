#pragma once

#include "GwConnImpl.hpp"

#include <GatewayClientLib/IAdsSignalUpdater.hpp>


namespace GatewayClientLib
{
	class AdsGwConnImpl : public GwConnImpl<AdsGwRequestId>
	{
	public:
		AdsGwConnImpl(IAdsSignalUpdater& signalUpdater, ILogger& logger);

		void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId) override;

		// Requests
		//
	protected:
		void requestHandshake(std::string_view equipmentId, uint16_t protocolVersion = ADS_GW_PROTOCOL_VERSION);
		std::vector<std::string> requestSignalList();
		std::vector<GwAppSignalParam> requestSignalParams();
		void requestStateChanges();
		void requestSignalStates();

	protected:
		IAdsSignalUpdater& m_signalUpdater;
		std::function<bool()> m_isCancelledFunc;

	protected:
		AdsGwHandshakeResponse m_handshakeResponse{};
		std::vector<std::string> m_appSignalIds{};
		std::vector<Radiy::Hash> m_appSignalHashes{};

		// Operative buffers used in requests
		//
		std::vector<GwAppSignalState> m_statesBuffer{};
		std::vector<Radiy::Hash> m_hashBuffer{};
		size_t m_nextStateIndexToRequest{0}; // Used only in requestSignalStates()
	};
} // namespace GatewayClientLib