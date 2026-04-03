#pragma once
#include "SignalManager.hpp"
#include "TuningGwProtocol.hpp"

namespace GatewayClientLib
{
	using TuningSignalManager = SignalManager<GwAppSignalParam, GwTuningSignalState>;
} // namespace GatewayClientLib