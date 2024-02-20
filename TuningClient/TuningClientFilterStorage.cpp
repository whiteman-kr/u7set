#include "TuningClientFilterStorage.h"
#include "../ClientLib/TuningConnection.h"
#include "MainWindow.h"
#include "TuningSourcesHelper.h"

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

void TuningClientFilterStorage::updateCounters(const TuningSignalManager& tunigSignals,
											   const ClientLib::TuningConnection& tuningConnection,
											   const std::vector<ClientLib::TuningSource>& sourceStates,
											   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
											   TuningFilter* filter)
{
	if (filter == nullptr)
	{
		filter = m_root.get();
	}

	TuningCounters filterCounters;

	if (filter->isRoot() == true)
	{
		// Root (total) Error and SOR counters
		//

		filterCounters.errorCounter = ClientLib::TuningSourcesHelper::sourcesErrorsCount(sourceStates);

		if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
		{
			filterCounters.sorCounter = ClientLib::TuningSourcesHelper::sourcesSorCount(sourceStates,
																						&filterCounters.sorActive,
																						&filterCounters.sorValid);
		}
	}
	else
	{
		if (filter->isEmpty() == false)
		{
			// Equipment counters

			std::vector<Hash> equipmentHashes = filter->equipmentHashes();

			// Error and SOR Counter
			//
			for (Hash& equipmentHash : equipmentHashes)
			{
				filterCounters.errorCounter += ClientLib::TuningSourcesHelper::sourceErrorsCount(sourceStates, equipmentHash);

				if (lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::SOR)
				{
					bool sorIsActive = false;
					bool sorIsValid = false;

					filterCounters.sorCounter += ClientLib::TuningSourcesHelper::sourceSorCount(sourceStates,
																								 equipmentHash,
																								 &sorIsActive,
																								 &sorIsValid);

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
			//
			if (filter->hasDiscreteCounter() == true || filter->isCounter() == true)
			{
				const std::vector<Hash>& appSignalsHashes = filter->signalsHashes();

				bool found = false;

				for (const Hash& appSignalHash : appSignalsHashes)
				{
					TuningSignalState state = tunigSignals.queuedState(appSignalHash, &found);
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
		updateCounters(tunigSignals, tuningConnection, sourceStates, lmStatusFlagMode, filter->childFilter(i).get());

		// Add child filters' counters for all empty filters
		//

		if (filter->isRoot() == false && filter->isEmpty() == true)
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
