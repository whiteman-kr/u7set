#pragma once

#include "SimLanInterface.h"

namespace Sim
{
	class AppDataLanInterface : public LanInterface
	{
		Q_OBJECT

	public:
		explicit AppDataLanInterface(const ::LanControllerInfo& lci, Lans* lans);
		virtual ~AppDataLanInterface() = default;
	};

}
