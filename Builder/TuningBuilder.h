#pragma once

#include "../DbLib/DbController.h"
#include "../HardwareLib/DeviceObject.h"
#include "../HardwareLib/ModuleFirmware.h"
#include "../TuningService/TuningDataStorage.h"
#include "IssueLogger.h"
#include "LmDescriptionSet.h"
#include "ModuleFirmwareWriter.h"
#include "Context.h"

namespace Builder
{
	class TuningBuilder : public QObject
	{
		Q_OBJECT

	public:
		TuningBuilder() = delete;
		TuningBuilder(Context* context);
		virtual ~TuningBuilder();

		bool build();

	private:
		DbController* db();
		OutputLog* log() const;

	private:
		Hardware::ModuleFirmwareWriter* m_firmwareWriter = nullptr;

		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		SignalSet* m_signalSet = nullptr;
		SubsystemStorage* m_subsystems = nullptr;
		Tuning::TuningDataStorage *m_tuningDataStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		std::vector<Hardware::DeviceModule*> m_lmModules;
        const LmDescriptionSet* m_lmDescriptionSet = nullptr;
	};
}
