#include "MainWindow.h"
#include "TuningClientFilterStorage.h"

TuningClientFilterStorage::TuningClientFilterStorage()
{

}

void TuningClientFilterStorage::checkAndRemoveFilterSignals(const std::vector<Hash>& signalHashes, bool& removedNotFound, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parentWidget)
{
	removedNotFound = false;

	m_root->checkSignals(signalHashes, notFoundSignalsAndFilters);

	if (notFoundSignalsAndFilters.empty() == true)
	{
		return;
	}

	DialogCheckFilterSignals d(notFoundSignalsAndFilters, parentWidget);
	if (d.exec() == QDialog::Accepted)
	{
		int removedCounter = 0;

		m_root->removeNotExistingSignals(signalHashes, removedCounter);

		removedNotFound = true;

		QMessageBox::warning(parentWidget, qApp->applicationName(), QObject::tr("%1 signals have been removed.").arg(removedCounter));
	}

}
void TuningClientFilterStorage::updateCounters(const TuningSignalManager* objects, const TuningClientTcpClient* tcpClient, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		filter = m_root.get();
	}

	if (filter->isEmpty() == false)
	{
		TuningFilterCounters counters;

		// Equipment counters

		std::vector<Hash> equipmentHashes = filter->equipmentHashes();

		for (Hash& equipmentHash : equipmentHashes)
		{
			if (tcpClient->tuningSourceCounters(equipmentHash, &counters) == false)
			{
				continue;
			}
		}

		// Signals counters

		const std::vector<Hash>& appSignalsHashes = filter->signalsHashes();

		bool found = false;

		for (const Hash& appSignalHash : appSignalsHashes)
		{
			TuningSignalState state = objects->state(appSignalHash, &found);
			if (found == false)
			{
				continue;
			}

			if (state.controlIsEnabled() == true)
			{
				counters.controlEnabledCounter++;
			}

			if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
			{
				counters.discreteCounter++;
			}
		}

		filter->setCounters(counters);
	}

	int count = filter->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		updateCounters(objects, tcpClient, filter->childFilter(i).get());
	}
}

void TuningClientFilterStorage::createAutomaticFilters(const TuningSignalManager* objects, bool bySchemas, bool byEquipment, const QStringList& tuningSourcesEquipmentIds)
{
	if (objects == nullptr)
	{
		assert(objects);
		return;
	}

	if (bySchemas == true)
	{
		m_root->removeChild("%AUTOFILTER%_SCHEMA");

		// Filter for Schema
		//
		std::shared_ptr<TuningFilter> ofSchema = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
		ofSchema->setID("%AUTOFILTER%_SCHEMA");
		ofSchema->setCaption(QObject::tr("Schemas"));
		ofSchema->setSource(TuningFilter::Source::Schema);

		for (const VFrame30::SchemaDetails& schemasDetails : m_schemasDetails)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			for (const QString& appSignalID : schemasDetails.m_signals)
			{
				// find if this signal is a tuning signal
				//
				Hash hash = ::calcHash(appSignalID);

				if (objects->signalExists(hash) == false)
				{
					continue;
				}

				TuningFilterValue ofv;
				ofv.setAppSignalId(appSignalID);
				ofTs->addValue(ofv);
			}

			if (ofTs->valuesCount() == 0)
			{
				// Do not add empty filters
				//
				continue;
			}

			ofTs->setID("%AUFOFILTER%_SCHEMA_" + schemasDetails.m_schemaId);

			//QString s = QString("%1 - %2").arg(schemasDetails.m_Id).arg(schemasDetails.m_caption);
			ofTs->setCaption(schemasDetails.m_caption);
			ofTs->setSource(TuningFilter::Source::Schema);

			ofSchema->addChild(ofTs);
		}

		m_root->addTopChild(ofSchema);
	}

	if (byEquipment == true)
	{
		m_root->removeChild("%AUTOFILTER%_EQUIPMENT");

		// Filter for EquipmentId
		//
		std::shared_ptr<TuningFilter> ofEquipment = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
		ofEquipment->setID("%AUTOFILTER%_EQUIPMENT");
		ofEquipment->setCaption(QObject::tr("Equipment"));
		ofEquipment->setSource(TuningFilter::Source::Equipment);

		for (const QString& ts : tuningSourcesEquipmentIds)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofTs->setEquipmentIDMask(ts);
			ofTs->setID("%AUFOFILTER%_EQUIPMENT_" + ts);
			ofTs->setCaption(ts);
			ofTs->setSource(TuningFilter::Source::Equipment);

			ofEquipment->addChild(ofTs);
		}

		m_root->addTopChild(ofEquipment);
	}
}

void TuningClientFilterStorage::removeFilters(TuningFilter::Source sourceType)
{
	m_root->removeChildren(sourceType);

}

void TuningClientFilterStorage::writeLogError(const QString& message)
{
	theLogFile->writeError(message);
}

void TuningClientFilterStorage::writeLogWarning(const QString& message)
{
	theLogFile->writeWarning(message);
}

void TuningClientFilterStorage::writeLogMessage(const QString& message)
{
	theLogFile->writeMessage(message);
}
