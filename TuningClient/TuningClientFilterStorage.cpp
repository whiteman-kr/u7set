#include "MainWindow.h"
#include "TuningClientFilterStorage.h"

TuningClientFilterStorage::TuningClientFilterStorage()
{

}

void TuningClientFilterStorage::createSignalsAndEqipmentHashes(const TuningSignalManager* objects,
															   const std::vector<Hash>& allHashes,
															   TuningFilter* filter,
															   bool userFiltersOnly)
{
	if (objects == nullptr)
	{
		assert(objects);
		return;
	}

	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	int allCount = static_cast<int>(allHashes.size());

	std::vector<Hash> signalsHashes;
	signalsHashes.reserve(allCount);

	while (true)
	{
		if (userFiltersOnly == true && filter->isSourceUser() == false)
		{
			// Filter of non-processed type, skip it

			signalsHashes = allHashes;

			break;
		}

		if (filter->isRoot() == true)
		{
			// Root filter

			// All signals ahashes are stored in root filter for discrete counter to work

			signalsHashes = allHashes;

			filter->setSignalsHashes(signalsHashes);

			break;
		}

		if (filter->isEmpty() == true)
		{
			// Filter is empty

			signalsHashes = allHashes;

			break;
		}

		// Filter is not empty

		std::map<Hash, int> equipmentHashesMap;

		for (int i = 0; i < allCount; i++)
		{
			bool ok = false;

			AppSignalParam asp = objects->signalParam(allHashes[i], &ok);
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

			if (filter->isSourceEquipment() == false)	// Skip equipment filters
			{
				Hash aspEquipmentHash = ::calcHash(asp.equipmentId());
				equipmentHashesMap[aspEquipmentHash] = 1;
			}
		}

		filter->setSignalsHashes(signalsHashes);

		if (filter->isSourceEquipment() == false)
		{
			// Set equipment hashes

			std::vector<Hash> equipmentHashes;
			for (auto it : equipmentHashesMap)
			{
				equipmentHashes.push_back(it.first);
			}
			filter->setEquipmentHashes(equipmentHashes);
		}
		else
		{
			// Equipment filters can be processed easier

			std::vector<Hash> equipmentHashes;
			equipmentHashes.push_back(::calcHash(filter->caption()));
			filter->setEquipmentHashes(equipmentHashes);
		}

		break;
	}

	for (int i = 0; i < filter->childFiltersCount(); i++)
	{
		createSignalsAndEqipmentHashes(objects, signalsHashes, filter->childFilter(i).get(), userFiltersOnly);
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

void TuningClientFilterStorage::createSchemaCounterFilters()
{
	std::vector<std::shared_ptr<TuningFilter>> templateFilters;

	m_schemaCounterFiltersCount = 0;
	m_schemaCounterFiltersNames.clear();

	// Save schema counter filters locally and delete them from global storage

	int count = m_root->childFiltersCount();
	for (int i = count - 1; i >= 0; i--)
	{
		std::shared_ptr<TuningFilter> f = m_root->childFilter(i);

		if (f->isCounter() == true && f->counterType() == TuningFilter::CounterType::FilterTree)
		{
			templateFilters.insert(templateFilters.begin(), f);

			m_schemaCounterFiltersNames.insert(0, f->caption());
			m_schemaCounterFiltersCount++;

			m_root->removeChild(i);
		}
	}

	// Add counter filters to every schema and equipment filter

	count = m_root->childFiltersCount();

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = m_root->childFilter(i);

		if (f->isSourceSchema() == true || f->isSourceEquipment() == true) // This is parent schemas or equipment filter
		{
			Q_ASSERT(f->hasDiscreteCounter() == false);

			int schemaCount = f->childFiltersCount();

			for (int s = 0; s < schemaCount; s++)
			{
				std::shared_ptr<TuningFilter> sf = f->childFilter(s);

				Q_ASSERT(sf->hasDiscreteCounter() == false);

				Q_ASSERT(sf->isSourceSchema() == true || sf->isSourceEquipment() == true);

				for (auto tf: templateFilters)
				{
					std::shared_ptr<TuningFilter> cf = std::make_shared<TuningFilter>(*tf);
					sf->addChild(cf);
				}
			}
		}
	}

	return;
}

void TuningClientFilterStorage::updateCounters(const TuningSignalManager* objects, const TuningClientTcpClient* tcpClient, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		filter = m_root.get();
	}

	TuningCounters filterCounters;

	if (filter->isRoot() == true)
	{
		// Root (total) equipment counters

		filterCounters.errorCounter += tcpClient->sourceErrorCount();

		if (theConfigSettings.showSOR == true)
		{
			filterCounters.sorCounter += tcpClient->sourceSorCount(&filterCounters.sorActive, &filterCounters.sorValid);
		}
	}
	else
	{
		if (filter->isEmpty() == false)
		{
			// Equipment counters

			std::vector<Hash> equipmentHashes = filter->equipmentHashes();

			for (Hash& equipmentHash : equipmentHashes)
			{

				filterCounters.errorCounter += tcpClient->sourceErrorCount(equipmentHash);

				if (theConfigSettings.showSOR == true)
				{
					bool sorIsActive = false;
					bool sorIsValid = false;

					filterCounters.sorCounter += tcpClient->sourceSorCount(equipmentHash, &sorIsActive, &sorIsValid);

					if (sorIsActive == true)
					{
						filterCounters.sorActive = true;
					}
					if (sorIsValid == true)
					{
						filterCounters.sorValid = true;
					}
				}
			}

			// Discrete counters

			if (filter->hasDiscreteCounter() == true || filter->isCounter() == true)
			{
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
						filterCounters.discreteCounter++;
					}
				}
			}
		}
	}

	filter->setCounters(filterCounters);

	int count = filter->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		updateCounters(objects, tcpClient, filter->childFilter(i).get());

		// Add child filters' counters
		//

		if (filter->isRoot() == false)
		{
			TuningCounters childCounters = filter->childFilter(i)->counters();

			filterCounters.discreteCounter += childCounters.discreteCounter;
			filterCounters.errorCounter += childCounters.errorCounter;
			filterCounters.sorCounter += childCounters.sorCounter;

			filter->setCounters(filterCounters);
		}
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

int TuningClientFilterStorage::schemaCounterFiltersCount() const
{
	return m_schemaCounterFiltersCount;
}

const QStringList& TuningClientFilterStorage::schemaCounterFiltersNames() const
{
	return m_schemaCounterFiltersNames;
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
