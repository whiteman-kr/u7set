#ifndef TUNINGBUILDER_H
#define TUNINGBUILDER_H

#include "Builder.h"

namespace Builder
{
	class TuningBuilder : QObject
	{
		Q_OBJECT
	public:
		TuningBuilder() = delete;
        TuningBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems,
					  Tuning::TuningDataStorage *tuningDataStorage, const std::vector<Hardware::DeviceModule*> lmModules,
					  const LmDescriptionSet* lmDescriptionSet, Hardware::ModuleFirmwareWriter* firmwareWriter, IssueLogger* log);
		virtual ~TuningBuilder();

		bool build();

	private:
		DbController* db();
		OutputLog* log() const;

	private:


	private:
		Hardware::ModuleFirmwareWriter* m_firmwareWriter = nullptr;

		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Tuning::TuningDataStorage *m_tuningDataStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		std::vector<Hardware::DeviceModule*> m_lmModules;
        const LmDescriptionSet* m_lmDescriptionSet = nullptr;
	};

}

#endif // TUNINGBUILDER_H
