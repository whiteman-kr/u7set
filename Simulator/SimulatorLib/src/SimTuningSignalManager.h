#pragma once
#include <ClientLib/TuningSignalManager.h>
#include "SimScopedLog.h"

namespace Sim
{

	class TuningSignalManager : public ClientLib::TuningSignalManager
	{
		Q_OBJECT

	public:
		explicit TuningSignalManager(ScopedLog log, QObject* parent = nullptr);
		virtual ~TuningSignalManager() = default;

	private:
		ScopedLog m_log;
	};

}
