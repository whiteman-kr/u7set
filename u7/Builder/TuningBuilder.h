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
                      Tuning::TuningDataStorage *tuningDataStorage, IssueLogger* log, int buildNo, int changesetId, bool debug,
                      QString projectName, QString userName, const std::vector<Hardware::DeviceModule *> lmModules, const LmDescriptionSet *lmDescriptionSet);
		virtual ~TuningBuilder();

		bool build();

		bool writeBinaryFiles(BuildResultWriter &buildResultWriter);

		quint64 getFirmwareUniqueId(const QString &subsystemID, int lmNumber);

		void setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId);

	private:
		DbController* db();
		OutputLog* log() const;
		int buildNo() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:


	private:
		Hardware::ModuleFirmwareCollection m_firmwareCollection;

		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Tuning::TuningDataStorage *m_tuningDataStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		std::vector<Hardware::DeviceModule*> m_lmModules;
        const LmDescriptionSet* m_lmDescriptionSet = nullptr;

		int m_buildNo = 0;
		int m_changesetId = 0;
		int m_debug = false;
		QString m_projectName;
		QString m_userName;
	};

}

#endif // TUNINGBUILDER_H
