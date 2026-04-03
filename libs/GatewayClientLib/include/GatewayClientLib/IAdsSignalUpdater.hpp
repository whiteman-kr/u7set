#pragma once

#include "AdsGwProtocol.hpp"
#include "ISignalUpdater.hpp"

namespace GatewayClientLib
{
	using IAdsSignalUpdater = ISignalUpdater<GwAppSignalParam, GwAppSignalState>;
} // namespace GatewayClientLib