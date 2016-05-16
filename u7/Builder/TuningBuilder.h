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
		TuningBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems, Tuning::TuningDataStorage *tuningDataStorage, IssueLogger* log, int changesetId, bool debug, QString projectName, QString userName, BuildResultWriter* buildWriter);
		virtual ~TuningBuilder();

		bool build();

	private:
		void findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>& modules);
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:


	private:
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Tuning::TuningDataStorage *m_tuningDataStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		BuildResultWriter* m_buildWriter = nullptr;

		int m_changesetId = 0;
		int m_debug = false;
		QString m_projectName;
		QString m_userName;
	};

}

#endif // TUNINGBUILDER_H
