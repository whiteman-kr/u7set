#pragma once

#include "ISignalManager.h"
#include <AppSignalLibStd/IAppSignalManagerT.h>

using IAppSignalManager = IAppSignalManagerT<AppSignalParam, AppSignalState, QString, QStringList, E::SignalType>;
