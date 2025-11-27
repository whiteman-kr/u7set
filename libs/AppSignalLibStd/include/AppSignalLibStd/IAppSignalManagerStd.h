#pragma once

#include <AppSignalLibStd/ISignalManagerT.h>

using ISignalManagerStd = ISignalManagerT<std::string, std::vector<std::string>>;
using IAppSignalManagerStd = IAppSignalManagerT<std::string, std::vector<std::string>>;
