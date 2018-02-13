#include "MainWindow.h"
#include "TuningClientFilterStorage.h"

TuningClientFilterStorage::TuningClientFilterStorage()
{

}

bool TuningClientFilterStorage::loadSchemasDetails(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_schemasDetails.clear();

	VFrame30::SchemaDetailsSet detailsSet;

	if (detailsSet.Load(data) == false)
	{
		*errorCode = QObject::tr("Failed to parse SchemaDetailsSet.");
		return false;
	}

	m_schemasDetails = detailsSet.schemasDetails();

	return true;

}

int TuningClientFilterStorage::schemaDetailsCount()
{
	return static_cast<int>(m_schemasDetails.size());
}

VFrame30::SchemaDetails  TuningClientFilterStorage::schemaDetails(int index)
{
	if (index < 0 || index >= m_schemasDetails.size())
	{
		assert(false);
		return VFrame30::SchemaDetails();
	}
	return m_schemasDetails[index];
}

void TuningClientFilterStorage::createSignalsAndEqipmentHashes(const TuningSignalManager* objects, TuningFilter* filter)
{
	if (objects == nullptr)
	{
		assert(objects);
		return;
	}

	if (filter == nullptr)
	{
		filter = m_root.get();
	}

	// Root filter

	if (filter->isRoot() == true && theConfigSettings.showDiscreteCounters == true)
	{
		// All signals ahashes are stored in root filter for discrete counter to work

		std::vector<Hash> signalsHashes;

		std::vector<Hash> hashes = objects->signalHashes();

		int count = static_cast<int>(hashes.size());
		for (int i = 0; i < count; i++)
		{
			signalsHashes.push_back(hashes[i]);
		}

		filter->setSignalsHashes(signalsHashes);
	}

	// Other filters

	if (filter->isEmpty() == false)
	{
		std::vector<Hash> signalsHashes;
		std::map<Hash, int> equipmentHashesMap;

		std::vector<Hash> hashes = objects->signalHashes();

		int count = static_cast<int>(hashes.size());
		for (int i = 0; i < count; i++)
		{
			bool ok = false;
			AppSignalParam asp = objects->signalParam(hashes[i], &ok);
			if (ok == false)
			{
				assert(false);
				return;
			}

			if (filter->match(asp) == false)
			{
				continue;
			}

			signalsHashes.push_back(asp.hash());

			Hash aspEquipmentHash = ::calcHash(asp.equipmentId());

			equipmentHashesMap[aspEquipmentHash] = 1;
		}

		filter->setSignalsHashes(signalsHashes);

		std::vector<Hash> equipmentHashes;
		for (auto it : equipmentHashesMap)
		{
			equipmentHashes.push_back(it.first);
		}

		filter->setEquipmentHashes(equipmentHashes);
	}

	int count = filter->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		createSignalsAndEqipmentHashes(objects, filter->childFilter(i).get());
	}
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

	// Root counters

	if (filter->isRoot() == true && theConfigSettings.showDiscreteCounters == true)
	{
		TuningFilterCounters counters;

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

			if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
			{
				counters.discreteCounter++;
			}
		}

		filter->setCounters(counters);
	}

	// Other counters

	if (filter->isEmpty() == false)
	{
		TuningFilterCounters counters;

		// Equipment counters

		std::vector<Hash> equipmentHashes = filter->equipmentHashes();

		for (Hash& equipmentHash : equipmentHashes)
		{
			int errorCounter = 0;
			int sorCounter = 0;

			if (tcpClient->tuningSourceCounters(equipmentHash, &errorCounter, &sorCounter) == false)
			{
				continue;
			}

			counters.errorCounter += errorCounter;
			counters.sorCounter += sorCounter;
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

			if (filter->hasDiscreteCounter() == true)
			{
				if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
				{
					counters.discreteCounter++;
				}
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

void TuningClientFilterStorage::createAutomaticFilters(const TuningSignalManager* objects, bool bySchemas, bool byEquipment, bool discreteCounters, const QStringList& tuningSourcesEquipmentIds)
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
			ofTs->setHasDiscreteCounter(discreteCounters);

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
			ofTs->setHasDiscreteCounter(discreteCounters);

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

//
// DialogCheckFilterSignals
//

DialogCheckFilterSignals::DialogCheckFilterSignals(std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{

	QVBoxLayout* mainLayout = new QVBoxLayout();

	QTextEdit* edit = new QTextEdit();

	QString text = tr("<font size=\"4\">Errors have been occured while loading the database:<br><br>");

	for (const std::pair<QString, QString>& p: notFoundSignalsAndFilters)
	{
		QString msg = tr("Signal with AppSignalID <font color=\"red\">'%1'</font> was not found in the preset '%2'.").arg(p.first).arg(p.second);

		text += msg + "<br>";
	}

	text += tr("<br>Do you wish to remove these signals from presets?</font>");

	edit->setText(text);

	edit->setReadOnly(true);


	QPushButton* yesButton = new QPushButton(tr("Yes"));
	yesButton->setAutoDefault(false);

	QPushButton* noButton = new QPushButton(tr("No"));
	noButton->setDefault(true);

	m_buttonBox = new QDialogButtonBox();

	m_buttonBox->addButton(yesButton, QDialogButtonBox::YesRole);
	m_buttonBox->addButton(noButton, QDialogButtonBox::NoRole);

	m_buttonBox->setFocus();

	connect(m_buttonBox, &QDialogButtonBox::clicked, this, &DialogCheckFilterSignals::buttonClicked);

	mainLayout->addWidget(edit);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	resize(800, 400);
}

void DialogCheckFilterSignals::buttonClicked(QAbstractButton* button)
{
	if (button == nullptr)
	{
		assert(button);
		return;
	}

	QDialogButtonBox::ButtonRole role = m_buttonBox->buttonRole(button);

	if (role == QDialogButtonBox::ButtonRole::YesRole)
	{
		accept();
	}

	if (role == QDialogButtonBox::ButtonRole::NoRole)
	{
		reject();
	}

}
