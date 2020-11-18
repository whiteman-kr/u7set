#pragma once

#include "SimLanInterface.h"

namespace Sim
{
	class Simulator;

	class TuningLanInterface : public LanInterface
	{
		Q_OBJECT

	public:
		explicit TuningLanInterface(const ::LanControllerInfo& lci, Lans* lans);
		virtual ~TuningLanInterface();

	public:
	private:
	};

}

