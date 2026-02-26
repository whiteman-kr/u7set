#pragma once

#include <AppSignalLibStd/ISignalManagerT.h>
#include "AppSignalParam.h"

using ISignalManager = ISignalManagerT<AppSignalParam, QString, QStringList>;
