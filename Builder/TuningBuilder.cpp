#include "TuningBuilder.h"

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		TuningBuilder
	//
	// ------------------------------------------------------------------------
	TuningBuilder::TuningBuilder(Context* context) :
		QObject(nullptr),
		m_firmwareWriter(context->m_buildResultWriter->firmwareWriter()),
		m_db(&context->m_db),
		m_deviceRoot(context->m_equipmentSet->root()),
		m_signalSet(context->m_signalSet.get()),
		m_subsystems(context->m_subsystems.get()),
		m_tuningDataStorage(context->m_tuningDataStorage.get()),
		m_log(context->m_log),
		m_lmModules(context->m_lmModules),
		m_lmDescriptionSet(context->m_lmDescriptions.get())
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_tuningDataStorage);
        assert(m_lmDescriptionSet);
		assert(m_firmwareWriter);
		assert(m_log);

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
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		for (Hardware::DeviceModule* m : m_lmModules)
		{

			if (m->propertyExists("SubsystemID") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, QString(tr("No property SubsystemID found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("LMNumber") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, QString(tr("No property LMNumber found in LM %1")).arg(m->caption()));
				return false;
			}

            std::shared_ptr<LmDescription> lmDescription = m_lmDescriptionSet->get(m);
            if (lmDescription == nullptr)
            {
                assert(lmDescription);
                LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, QString(tr("No LMDescription for in LM %1")).arg(m->caption()));
                return false;
            }

			if (lmDescription->flashMemory().m_tuningWriteBitstream == false)
			{
				return true;
			}

			QString subsysStrID = m->propertyValue("SubsystemID").toString();

			int channel = m->propertyValue("LMNumber").toInt();
			int frameSize = lmDescription->flashMemory().m_tuningFramePayload;
			int frameCount = lmDescription->flashMemory().m_tuningFrameCount;
			int subsysID = m_subsystems->ssKey(subsysStrID);
			int tuningUartId = lmDescription->flashMemory().m_tuningUartId;

			if (subsysID == -1)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(m->caption()));
				return false;
			}

			m_firmwareWriter->createFirmware(subsysStrID,
											 subsysID,
											 tuningUartId,
											 "Tuning",
											 frameSize,
											 frameCount,
											 lmDescription->lmDescriptionFile(m),
											 lmDescription->descriptionNumber());

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

				m_firmwareWriter->setDescriptionFields(subsysStrID, tuningUartId, metadataFieldsVersion, metadataFields);
				descriptionData = tuningData->metadata();
			}

			if (m_firmwareWriter->setChannelData(subsysStrID, tuningUartId, m->propertyValue("EquipmentID").toString(), channel, frameSize, frameCount, uniqueID, data, descriptionData, m_log) == false)
			{
				return false;
			}
		}

		return true;
	}

	DbController* TuningBuilder::db()
	{
		return m_db;
	}

	OutputLog* TuningBuilder::log() const
	{
		return m_log;
	}

}

