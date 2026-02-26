#pragma once

#include "ISignalManager.h"
#include <AppSignalLibStd/IAppSignalManagerT.h>
#include "AppSignalParam.h"
#include "AppSignalState.h"

using IAppSignalManager = IAppSignalManagerT<AppSignalParam, AppSignalState, QString, QStringList, E::SignalType>;
