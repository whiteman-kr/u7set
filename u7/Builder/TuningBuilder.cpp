#include "TuningBuilder.h"

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		TuningBuilder
	//
	// ------------------------------------------------------------------------

	TuningBuilder::TuningBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems,
								 Tuning::TuningDataStorage *tuningDataStorage, IssueLogger *log, int buildNo, int changesetId, bool debug,
                                 QString projectName, QString userName, const std::vector<Hardware::DeviceModule *> lmModules, const LmDescriptionSet* lmDescriptionSet):
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_tuningDataStorage(tuningDataStorage),
		m_log(log),
        m_lmModules(lmModules),
		m_lmDescriptionSet(lmDescriptionSet),
		m_buildNo(buildNo),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName)
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_tuningDataStorage);
		assert(m_log);
        assert(m_lmDescriptionSet);

		return;
	}

	TuningBuilder::~TuningBuilder()
	{
	}

	bool TuningBuilder::build()
	{
		if (db() == nullptr || log() == nullptr)
		{
			assert(db());
			assert(log());
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		m_firmwareCollection.init(m_projectName, m_userName, buildNo(), debug(), changesetId());

		for (Hardware::DeviceModule* m : m_lmModules)
		{

			if (m->propertyExists("SubsystemID") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property SubsystemID found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("LMNumber") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property LMNumber found in LM %1")).arg(m->caption()));
				return false;
			}

            std::shared_ptr<LogicModule> lmDescription = m_lmDescriptionSet->get(m);
            if (lmDescription == nullptr)
            {
                assert(lmDescription);
                LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No LMDescription for in LM %1")).arg(m->caption()));
                return false;
            }

			QString subsysStrID = m->propertyValue("SubsystemID").toString();

			int channel = m->propertyValue("LMNumber").toInt();

			int frameSize = lmDescription->flashMemory().m_tuningFrameSize;

			int frameCount = lmDescription->flashMemory().m_tuningFrameCount;

			int subsysID = m_subsystems->ssKey(subsysStrID);

			if (subsysID == -1)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(m->caption()));
				return false;
			}

			Hardware::ModuleFirmwareWriter* firmware = (Hardware::ModuleFirmwareWriter*)m_firmwareCollection.jsGet(tr("LM-1"), subsysStrID, subsysID, 0x104, frameSize, frameCount);
			if (firmware == nullptr)
			{
				assert(firmware);
				return false;
			}

			QByteArray data;
			quint64 uniqueID = 0;

			Tuning::TuningDataStorage::iterator it = m_tuningDataStorage->find(m->equipmentId());

			std::vector<QVariantList> descriptionData;

			if (it == m_tuningDataStorage->end())
			{
				data.fill(0, 100);
			}
			else
			{
				Tuning::TuningData *tuningData = it.value();
				if (tuningData == nullptr)
				{
					assert(tuningData);
					return false;
				}

				tuningData->getTuningData(&data);
				uniqueID = tuningData->uniqueID();

				int metadataFieldsVersion = 0;

				QStringList metadataFields;

				tuningData->getMetadataFields(metadataFields, &metadataFieldsVersion);

				firmware->setDescriptionFields(metadataFieldsVersion, metadataFields);
				descriptionData = tuningData->metadata();
			}

			if (firmware->setChannelData(m->propertyValue("EquipmentID").toString(), channel, frameSize, frameCount, uniqueID, data, descriptionData, m_log) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool TuningBuilder::writeBinaryFiles(BuildResultWriter &buildResultWriter)
	{
		std::map<QString, Hardware::ModuleFirmwareWriter>& firmwares = m_firmwareCollection.firmwares();
		for (auto it = firmwares.begin(); it != firmwares.end(); it++)
		{
			Hardware::ModuleFirmwareWriter& f = it->second;

			QByteArray data;

			if (f.save(data, m_log) == false)
			{
				return false;
			}

			QString path = f.subsysId();
			QString fileName = f.caption();

			if (path.isEmpty())
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, subsystemId is empty."));
				return false;
			}
			if (fileName.isEmpty())
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, module type string is empty."));
				return false;
			}

			if (buildResultWriter.addFile(path, fileName + ".tub", data) == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	quint64 TuningBuilder::getFirmwareUniqueId(const QString &subsystemID, int lmNumber)
	{
		return m_firmwareCollection.getFirmwareUniqueId(subsystemID, lmNumber);
	}

	void TuningBuilder::setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId)
	{
		m_firmwareCollection.setGenericUniqueId(subsystemID, lmNumber, genericUniqueId);
	}


	DbController* TuningBuilder::db()
	{
		return m_db;
	}

	OutputLog* TuningBuilder::log() const
	{
		return m_log;
	}

	int TuningBuilder::buildNo() const
	{
		return m_buildNo;
	}

	int TuningBuilder::changesetId() const
	{
		return m_changesetId;
	}

	bool TuningBuilder::debug() const
	{
		return m_debug;
	}

	bool TuningBuilder::release() const
	{
		return !debug();
	}
}

