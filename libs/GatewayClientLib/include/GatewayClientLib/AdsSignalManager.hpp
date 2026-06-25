#pragma once
#include "AdsGwProtocol.hpp"
#include "SignalManager.hpp"

namespace GatewayClientLib
{
	using AdsSignalManager = SignalManager<GwAppSignalParam, GwAppSignalState>;
} // namespace GatewayClientLib