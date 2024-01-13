#include "TuningSignalInfo.h"
#include "ui_TuningSignalInfo.h"

#include "../AppSignalLib/TuningSignalState.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "Settings.h"
#include <QActionGroup>

TuningSignalInfo::TuningSignalInfo(TuningConfigController& configController,
								   const TuningSignalManager& signalManager,
								   const ClientLib::TuningConnection& tuningConnection,
								   Hash appSignalHash,
								   E::AnalogFormat analogFormat,
								   QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::TuningSignalInfo),
	m_configController(configController),
	m_appSignalHash(appSignalHash),
	m_analogFormat(analogFormat),
	m_signalManager(signalManager),
	m_tuningConnection(tuningConnection),
	m_clientHash(::calcHash(configController.softwareInfo().equipmentID())),
	m_lmStatusFlagMode(m_configController.lmStatusFlagMode())
{
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	bool found = false;
	m_asp = m_signalManager.signalParam(m_appSignalHash, &found);

	m_precision = m_asp.precision();

	ui->m_lineAppSignalId->setText(m_asp.appSignalId());
	ui->m_lineCustomAppSignalId->setText(m_asp.customSignalId());
	ui->m_lineCaption->setText(m_asp.caption());
	ui->m_lineTags->setText(m_asp.tagStringList().join(' '));
	ui->m_lineEquipmentId->setText(m_asp.equipmentId());
	ui->m_lineLmEquipmentId->setText(m_asp.lmEquipmentId());

	ui->editValue->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->editValue, &QLineEdit::customContextMenuRequested,  this, &TuningSignalInfo::onValueContextMenu);

	ui->treeProperties->setHeaderLabels(QStringList() << tr("Property") << tr("Value"));
	ui->treeProperties->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeProperties, &QTreeWidget::customContextMenuRequested,  this, &TuningSignalInfo::onPropertiesContextMenu);

	setWindowTitle(tr("%1 - %2").arg(m_asp.customSignalId()).arg(m_asp.caption()));

	updateInfo();

	m_timerId = startTimer(500);

	return;
}

TuningSignalInfo::~TuningSignalInfo()
{
	delete ui;
}

void TuningSignalInfo::timerEvent(QTimerEvent* event)
{
	assert(event);

	if (event->timerId() != m_timerId)
	{
		assert(false);
		return;
	}

	updateInfo();
}

void TuningSignalInfo::updateInfo()
{
	QStringList validStrings;
	QStringList outOfRangeStrings;
	QStringList writeInProgressStrings;
	QStringList controlIsEnabledStrings;
	QStringList isTuningDefaultStrings;
	QStringList writingIsEnabledStrings;

	QStringList stateServices;
	QStringList writeClientHashes;
	QStringList writeErrorCodes;
	QStringList lmTimes;
	QStringList successfulReadTimes;
	QStringList writeRequestTimes;
	QStringList successfulWriteTimes;
	QStringList unsuccessfulWriteTimes;

	bool found = false;

	// Fill the data received from TCP clients
	//

	for (const SoftwareEndpoint::TuningService& tuns : m_configController.configuration().clientSettings.tuningServices)
	{
		const TuningSignalState clientState = m_signalManager.state(m_appSignalHash, ::calcHash(tuns.equipmentId), &found);
		if (found == false)
		{
			continue;
		}

		stateServices.push_back(tuns.equipmentId);

		validStrings.push_back(clientState.valid() == true ? tr("Yes") : tr("No"));
		outOfRangeStrings.push_back(clientState.outOfRange() == true ? tr("Yes") : tr("No"));
		writeInProgressStrings.push_back(clientState.writeInProgress() == true ? tr("Yes") : tr("No"));
		controlIsEnabledStrings.push_back(clientState.controlIsEnabled() == true ? tr("Yes") : tr("No"));
		isTuningDefaultStrings.push_back(clientState.isTuningDefault() == true ? tr("Yes") : tr("No"));

		if (m_lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::AccessKey)
		{
			writingIsEnabledStrings.push_back(clientState.writingIsEnabled() == true ? tr("Yes") : tr("No"));
		}

		QString hashString = QString("%1h").arg(QString::number(clientState.writeClient(), 16));
		if (clientState.writeClient() == m_clientHash)
		{
			hashString += tr(" (this client)");
		}
		writeClientHashes.push_back(hashString);

		writeErrorCodes.push_back(E::valueToString(static_cast<E::NetworkError>(clientState.writeErrorCode())));

		lmTimes.push_back(clientState.lmTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulReadTimes.push_back(clientState.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		writeRequestTimes.push_back(clientState.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulWriteTimes.push_back(clientState.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		unsuccessfulWriteTimes.push_back(clientState.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	}


	// Fill the data that is received from TuningSignalManager

	TuningSignalState managerState = m_signalManager.state(m_appSignalHash, &found);
	{
		stateServices.push_back(tr("Common"));

		validStrings.push_back(managerState.valid() == true ? tr("Yes") : tr("No"));
		outOfRangeStrings.push_back(managerState.outOfRange() == true ? tr("Yes") : tr("No"));
		writeInProgressStrings.push_back(managerState.writeInProgress() == true ? tr("Yes") : tr("No"));
		controlIsEnabledStrings.push_back(managerState.controlIsEnabled() == true ? tr("Yes") : tr("No"));
		isTuningDefaultStrings.push_back(managerState.isTuningDefault() == true ? tr("Yes") : tr("No"));

		if (m_lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::AccessKey)
		{
			writingIsEnabledStrings.push_back(managerState.writingIsEnabled() == true ? tr("Yes") : tr("No"));
		}

		QString hashString = QString("%1h").arg(QString::number(managerState.writeClient(), 16));
		if (managerState.writeClient() == m_clientHash)
		{
			hashString += tr(" (this client)");
		}
		writeClientHashes.push_back(hashString);

		writeErrorCodes.push_back(E::valueToString(static_cast<E::NetworkError>(managerState.writeErrorCode())));

		lmTimes.push_back(managerState.lmTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulReadTimes.push_back(managerState.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		writeRequestTimes.push_back(managerState.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulWriteTimes.push_back(managerState.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		unsuccessfulWriteTimes.push_back(managerState.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	}


	// Print data to the dialog
	//

	AppSignalParam asp = m_signalManager.signalParam(m_appSignalHash, &found);
	{
		QString text;

		if (managerState.controlIsEnabled() == false)
		{
			text = tr("Disabled");
		}
		else
		{
			if (managerState.valid() == false)
			{
				text = tr("?");
			}
			else
			{
				if (asp.isAnalog() == true)
				{
					text = managerState.value().toString(m_analogFormat, m_precision) + " " + asp.unit();
				}
				else
				{
					text = managerState.value().toString();
				}
			}
		}

		if (ui->editValue->text() != text)
		{
			ui->editValue->setText(text);
		}
	}

	if (ui->treeProperties->topLevelItemCount() == 0)
	{
		QStringList propertiesNames;
		propertiesNames << tr("New Value:");
		propertiesNames << tr("Source:");
		propertiesNames << tr("Valid:");
		propertiesNames << tr("OutOfRange:");
		propertiesNames << tr("WriteInProgress:");
		propertiesNames << tr("ControlIsEnabled:");
		propertiesNames << tr("TuningDefault:");
		if (m_lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::AccessKey)
		{
			propertiesNames << tr("WritingIsEnabled:");
		}
		propertiesNames << tr("WriteClientHash:");
		propertiesNames << tr("WriteErrorCode:");
		propertiesNames << tr("LM Time:");

		propertiesNames << tr("SuccessfulReadTime:");
		propertiesNames << tr("WriteRequestTime:");
		propertiesNames << tr("SuccessfulWriteTime:");
		propertiesNames << tr("UnsuccessfulWriteTime:");

		for (const QString& p : propertiesNames)
		{
			ui->treeProperties->addTopLevelItem(new QTreeWidgetItem(QStringList() << p));
		}
		ui->treeProperties->resizeColumnToContents(0);

	}

	// Properties
	//
	QStringList propertiesValues;

	if (managerState.controlIsEnabled() == false)
	{
		propertiesValues.push_back(tr("Disabled"));
	}
	else
	{
		if (m_signalManager.isUnapplied(m_appSignalHash) == true)
		{
			int precision = 0;

			if (asp.isAnalog() == true)
			{
				precision = asp.precision();
			}

			propertiesValues.push_back(m_signalManager.unappliedValue(m_appSignalHash).toString(m_analogFormat, precision));
		}
		else
		{
			propertiesValues.push_back("-");
		}
	}

	propertiesValues.push_back(stateServices.join(" / "));	// Sources

	propertiesValues.push_back(validStrings.join(" / "));
	propertiesValues.push_back(outOfRangeStrings.join(" / "));
	propertiesValues.push_back(writeInProgressStrings.join(" / "));
	propertiesValues.push_back(controlIsEnabledStrings.join(" / "));
	propertiesValues.push_back(isTuningDefaultStrings.join(" / "));

	if (m_lmStatusFlagMode == TuningClientSettings::LmStatusFlagMode::AccessKey)
	{
		propertiesValues.push_back(writingIsEnabledStrings.join(" / "));
	}

	propertiesValues.push_back(writeClientHashes.join(" / "));
	propertiesValues.push_back(writeErrorCodes.join(" / "));
	propertiesValues.push_back(lmTimes.join(" / "));

	propertiesValues.push_back(successfulReadTimes.join(" / "));
	propertiesValues.push_back(writeRequestTimes.join(" / "));
	propertiesValues.push_back(successfulWriteTimes.join(" / "));
	propertiesValues.push_back(unsuccessfulWriteTimes.join(" / "));

	int i = 0;
	for (const QString& p : propertiesValues)
	{
		QTreeWidgetItem* item = ui->treeProperties->topLevelItem(i++);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}
		item->setText(1, p);
	}

	return;
}


void TuningSignalInfo::onValueContextMenu()
{
	QMenu menu;

	// Precision
	//
	if (m_asp.isAnalog() == true)
	{
		QMenu* submenuV = menu.addMenu(tr("Precision"));
		QString strPrecision = ".";

		QActionGroup* precisionGroup = new QActionGroup(this);
		precisionGroup->setExclusive(true);

		for (int i = 0; i < 10; i++)
		{
			QAction* a = new QAction(strPrecision, &menu);

			auto f = [this, i]() -> void
			{
				m_precision = i;

				if (m_analogFormat == E::AnalogFormat::g_9_or_9e || m_analogFormat == E::AnalogFormat::G_9_or_9E)
				{
					m_analogFormat = E::AnalogFormat::f_9;
				}
			};

			connect(a, &QAction::triggered, this, f);

			a->setCheckable(true);

			if (i == m_precision)
			{
				a->setChecked(true);
			}

			precisionGroup->addAction(a);

			strPrecision += "0";
		}

		submenuV->addActions(precisionGroup->actions());
	}

	// Format
	//
	if (m_asp.isAnalog() == true)
	{
		QMenu* submenuV = menu.addMenu(tr("Format"));

		QAction* a = new QAction(tr("Auto-select"), &menu);
		a->setCheckable(true);
		a->setChecked(m_analogFormat == E::AnalogFormat::g_9_or_9e || m_analogFormat == E::AnalogFormat::G_9_or_9E);
		connect(a, &QAction::triggered, this, [this]()
				{
					m_analogFormat = E::AnalogFormat::g_9_or_9e;
				});
		submenuV->addAction(a);

		a = new QAction(tr("Decimal (as [-]9.9)"), &menu);
		a->setCheckable(true);
		a->setChecked(m_analogFormat == E::AnalogFormat::f_9);
		connect(a, &QAction::triggered, this, [this]()
				{
					m_analogFormat = E::AnalogFormat::f_9;
				});
		submenuV->addAction(a);

		a = new QAction(tr("Exponential (as [-]9.9e[+|-]999)"), &menu);
		a->setCheckable(true);
		a->setChecked(m_analogFormat == E::AnalogFormat::e_9e || m_analogFormat == E::AnalogFormat::E_9E);
		connect(a, &QAction::triggered, this, [this]()
				{
					m_analogFormat = E::AnalogFormat::e_9e;
				});
		submenuV->addAction(a);
	}

	//
	if (menu.isEmpty() == false)
	{
		QAction* separator2 = new QAction(&menu);
		separator2->setSeparator(true);
		menu.addAction(separator2);
	}

	// Copy
	//
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				clipboard->setText(ui->editValue->text());
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	// --
	//
	menu.exec(QCursor::pos());
}


void TuningSignalInfo::onPropertiesContextMenu()
{
	QMenu menu(this);

	QTreeWidgetItem* item = ui->treeProperties->currentItem();
	if (item == nullptr)
	{
		return;
	}

	// Copy
	QAction* actionCopy = new QAction(tr("Copy"), &menu);

	auto f = [this]() -> void
			 {
				QClipboard *clipboard = QApplication::clipboard();
				QTreeWidgetItem* item = ui->treeProperties->currentItem();
				if (item == nullptr)
				{
					return;
				}
				clipboard->setText(item->text(1));
			};

	connect(actionCopy, &QAction::triggered, this, f);

	menu.addAction(actionCopy);

	//
	menu.exec(QCursor::pos());
}