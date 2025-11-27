#pragma once

#include <AppSignalLibStd/IAppSignalManagerT.h>
#include "ISignalManager.h"

using IAppSignalManager = IAppSignalManagerT<QString, QStringList>;
