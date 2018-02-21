#include "MainWindow.h"
#include "TuningClientFilterStorage.h"

TuningClientFilterStorage::TuningClientFilterStorage()
{

}

void TuningClientFilterStorage::createSignalsAndEqipmentHashes(const TuningSignalManager* objects, const std::vector<Hash>& allHashes, TuningFilter* filter)
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

	// Root filter

	if (filter->isRoot() == true)
	{
		// All signals ahashes are stored in root filter for discrete counter to work

		signalsHashes = allHashes;

		filter->setSignalsHashes(signalsHashes);
	}

	// Other filters

	if (filter->isEmpty() == false)
	{
		if (filter->isSourceEquipment() == true)	// Equipment filters can be processed easier
		{
			std::vector<Hash> equipmentHashes;
			equipmentHashes.push_back(::calcHash(filter->caption()));
			filter->setEquipmentHashes(equipmentHashes);
		}

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

		if (filter->isSourceEquipment() == false)	// Skip equipment filters
		{
			std::vector<Hash> equipmentHashes;
			for (auto it : equipmentHashesMap)
			{
				equipmentHashes.push_back(it.first);
			}
			filter->setEquipmentHashes(equipmentHashes);
		}
	}
	else
	{
		signalsHashes = allHashes;
	}

	int count = filter->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		createSignalsAndEqipmentHashes(objects, signalsHashes, filter->childFilter(i).get());
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
		TuningCounters counters;

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
		TuningCounters totalCounters;

		// Equipment counters

		std::vector<Hash> equipmentHashes = filter->equipmentHashes();

		for (Hash& equipmentHash : equipmentHashes)
		{
			totalCounters.errorCounter += tcpClient->sourceSorCount(equipmentHash);
			totalCounters.sorCounter += tcpClient->sourceSorCount(equipmentHash);
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
				totalCounters.controlEnabledCounter++;
			}

			if (filter->hasDiscreteCounter() == true)
			{
				if (state.valid() == true && state.value().type() == TuningValueType::Discrete && state.value().discreteValue() != 0)
				{
					totalCounters.discreteCounter++;
				}
			}
		}

		filter->setCounters(totalCounters);
	}

	int count = filter->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		updateCounters(objects, tcpClient, filter->childFilter(i).get());
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
