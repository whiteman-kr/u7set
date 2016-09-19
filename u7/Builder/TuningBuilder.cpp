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
								 QString projectName, QString userName, BuildResultWriter* buildWriter):
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_tuningDataStorage(tuningDataStorage),
		m_log(log),
		m_buildWriter(buildWriter),
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
		assert(m_buildWriter);

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

		Hardware::ModuleFirmwareCollection firmwareCollection(m_projectName, m_userName, buildNo(), debug(), changesetId());

		std::vector<Hardware::DeviceModule*> lmModules;
		findLmModules(m_deviceRoot, lmModules);

		QString errorString;

		for (Hardware::DeviceModule* m : lmModules)
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

			if (m->propertyExists("TuningFrameSize") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property TuningFrameSize found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("TuningFrameCount") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property TuningFrameCount found in LM %1")).arg(m->caption()));
				return false;
			}

			QString subsysStrID = m->propertyValue("SubsystemID").toString();

			int channel = m->propertyValue("LMNumber").toInt();

			int frameSize = m->propertyValue("TuningFrameSize").toInt();

			int frameCount = m->propertyValue("TuningFrameCount").toInt();

			int subsysID = m_subsystems->ssKey(subsysStrID);

			if (subsysID == -1)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(m->caption()));
				return false;
			}

			Hardware::ModuleFirmwareWriter* firmware = (Hardware::ModuleFirmwareWriter*)firmwareCollection.jsGet(tr("LM-1"), subsysStrID, subsysID, 0x104, frameSize, frameCount);
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

				firmware->setDescriptionFields(tuningData->metadataFields());
				descriptionData = tuningData->metadata();
			}

			if (firmware->setChannelData(m->propertyValue("EquipmentID").toString(), channel, frameSize, frameCount, uniqueID, data, descriptionData, m_log) == false)
			{
				return false;
			}
		}

		std::map<QString, Hardware::ModuleFirmwareWriter>& firmwares = firmwareCollection.firmwares();
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

			if (m_buildWriter->addFile(path, fileName + ".tub", data) == false)
			{
				return false;
			}
		}


		return true;
	}

	void TuningBuilder::findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule *> &modules)
	{
		if (object == nullptr)
		{
			assert(object);
			return;
		}

		for (int i = 0; i < object->childrenCount(); i++)
		{
			Hardware::DeviceObject* child = object->child(i);

			if (child->deviceType() == Hardware::DeviceType::Module)
			{
				Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);

				if (module->moduleFamily() == Hardware::DeviceModule::LM)
				{
					modules.push_back(module);
				}
			}

			findLmModules(child, modules);
		}

		return;
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

