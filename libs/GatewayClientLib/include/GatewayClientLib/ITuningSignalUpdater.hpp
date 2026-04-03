#pragma once

#include "ISignalUpdater.hpp"
#include "TuningGwProtocol.hpp"

namespace GatewayClientLib
{
	using ITuningSignalUpdater = ISignalUpdater<GwAppSignalParam, GwTuningSignalState>;
} // namespace GatewayClientLib