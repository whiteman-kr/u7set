#pragma once

#include "SimLanInterface.h"

namespace Sim
{
	class Simulator;

	class DiagDataLanInterface : public LanInterface
	{
		Q_OBJECT

	public:
		explicit DiagDataLanInterface(const ::LanControllerInfo& lci, Lans* lans);
		virtual ~DiagDataLanInterface();

	public:
	private:
	};

}
