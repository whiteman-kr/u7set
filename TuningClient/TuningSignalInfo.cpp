#include "TuningSignalInfo.h"
#include "ui_TuningSignalInfo.h"

#include "../AppSignalLib/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "Settings.h"

TuningSignalInfo::TuningSignalInfo(Hash appSignalHash,
								   E::AnalogFormat analogFormat,
								   Hash instanceIdHash,
								   TuningSignalManager& signalManager,
								   std::vector<ClientLib::TuningTcpClient*> tuningTcpClients,
								   LmStatusFlagMode lmStatusFlagMode,
								   QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::TuningSignalInfo),
	m_appSignalHash(appSignalHash),
	m_analogFormat(analogFormat),
	m_instanceIdHash(instanceIdHash),
	m_signalManager(signalManager),
	m_lmStatusFlagMode(lmStatusFlagMode)
{
	ui->setupUi(this);

	ui->m_textEdit->setReadOnly(true);

	setAttribute(Qt::WA_DeleteOnClose);

	setTuningTcpClients(tuningTcpClients);

	bool found = false;
	AppSignalParam asp = m_signalManager.signalParam(m_appSignalHash, &found);

	ui->m_lineAppSignalId->setText(asp.appSignalId());
	ui->m_lineCustomAppSignalId->setText(asp.customSignalId());
	ui->m_lineCaption->setText(asp.caption());
	ui->m_lineTags->setText(asp.tagStringList().join(' '));
	ui->m_lineEquipmentId->setText(asp.equipmentId());
	ui->m_lineLmEquipmentId->setText(asp.lmEquipmentId());

	setWindowTitle(tr("%1 - %2").arg(asp.customSignalId()).arg(asp.caption()));

	updateInfo();

	m_timerId = startTimer(500);

	return;
}

TuningSignalInfo::~TuningSignalInfo()
{
	delete ui;
}

void TuningSignalInfo::setTuningTcpClients(std::vector<ClientLib::TuningTcpClient*> tuningTcpClients)
{
	m_signalTcpClients.clear();

	// Get TCP clients that process this signal
	//
	for (ClientLib::TuningTcpClient* client : tuningTcpClients)
	{
		if (client->hasTuningSignal(m_appSignalHash) == true)
		{
			m_signalTcpClients.push_back(client);
		}
	}
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
	bool found = false;

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

	// Fill the data received from TCP clients
	//
	for (ClientLib::TuningTcpClient* client : m_signalTcpClients)
	{
		TuningSignalState clientState = client->state(m_appSignalHash, &found);

		stateServices.push_back(client->tuningServiceId());

		validStrings.push_back(clientState.valid() == true ? tr("Yes") : tr("No"));
		outOfRangeStrings.push_back(clientState.outOfRange() == true ? tr("Yes") : tr("No"));
		writeInProgressStrings.push_back(clientState.writeInProgress() == true ? tr("Yes") : tr("No"));
		controlIsEnabledStrings.push_back(clientState.controlIsEnabled() == true ? tr("Yes") : tr("No"));
		isTuningDefaultStrings.push_back(clientState.isTuningDefault() == true ? tr("Yes") : tr("No"));

		if (m_lmStatusFlagMode == LmStatusFlagMode::AccessKey)
		{
			writingIsEnabledStrings.push_back(clientState.writingIsEnabled() == true ? tr("Yes") : tr("No"));
		}

		QString hashString = QString("%1h").arg(QString::number(clientState.writeClient(), 16));
		if (clientState.writeClient() == m_instanceIdHash)
		{
			hashString += tr(" (this client)");
		}
		writeClientHashes.push_back(hashString);

		writeErrorCodes.push_back(getNetworkErrorStr(static_cast<NetworkError>(clientState.writeErrorCode())));

		lmTimes.push_back(clientState.lmTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulReadTimes.push_back(clientState.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		writeRequestTimes.push_back(clientState.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulWriteTimes.push_back(clientState.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		unsuccessfulWriteTimes.push_back(clientState.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	}

	// Fill the data that is received from TuningSignalManager

	TuningSignalState managerState = m_signalManager.state(m_appSignalHash, &found);

	{
		stateServices.push_back(tr("TuningSignalManager"));

		validStrings.push_back(managerState.valid() == true ? tr("Yes") : tr("No"));
		outOfRangeStrings.push_back(managerState.outOfRange() == true ? tr("Yes") : tr("No"));
		writeInProgressStrings.push_back(managerState.writeInProgress() == true ? tr("Yes") : tr("No"));
		controlIsEnabledStrings.push_back(managerState.controlIsEnabled() == true ? tr("Yes") : tr("No"));
		isTuningDefaultStrings.push_back(managerState.isTuningDefault() == true ? tr("Yes") : tr("No"));

		if (m_lmStatusFlagMode == LmStatusFlagMode::AccessKey)
		{
			writingIsEnabledStrings.push_back(managerState.writingIsEnabled() == true ? tr("Yes") : tr("No"));
		}

		QString hashString = QString("%1h").arg(QString::number(managerState.writeClient(), 16));
		if (managerState.writeClient() == m_instanceIdHash)
		{
			hashString += tr(" (this client)");
		}
		writeClientHashes.push_back(hashString);

		writeErrorCodes.push_back(getNetworkErrorStr(static_cast<NetworkError>(managerState.writeErrorCode())));

		lmTimes.push_back(managerState.lmTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulReadTimes.push_back(managerState.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		writeRequestTimes.push_back(managerState.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		successfulWriteTimes.push_back(managerState.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
		unsuccessfulWriteTimes.push_back(managerState.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	}

	// Print data to the dialog
	//

	AppSignalParam asp = m_signalManager.signalParam(m_appSignalHash, &found);

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
				text = managerState.value().toString(m_analogFormat, asp.precision()) + " " + asp.unit();
			}
			else
			{
				text = managerState.value().toString();
			}
		}
	}

	ui->m_labelValue->setText(text);

	// New value
	//
	text.clear();

	if (managerState.controlIsEnabled() == false)
	{
		text += tr("NewValue:\t\tDisabled\n");
	}
	else
	{
		if (m_signalManager.newValueIsUnapplied(m_appSignalHash) == true)
		{
			int precision = 0;

			if (asp.isAnalog() == true)
			{
				precision = asp.precision();
			}

			text += tr("NewValue:\t\t%1\n").arg(m_signalManager.newValue(m_appSignalHash).toString(m_analogFormat, precision));
		}
		else
		{
			text += tr("NewValue:\t\t-\n");
		}
	}

	text += "\n";

	text += tr("Source:\t\t%1\n").arg(stateServices.join(" / "));

	text += "\n";

	text += tr("Valid:\t\t%1\n").arg(validStrings.join(" / "));
	text += tr("OutOfRange:\t\t%1\n").arg(outOfRangeStrings.join(" / "));
	text += tr("WriteInProgress:\t%1\n").arg(writeInProgressStrings.join(" / "));
	text += tr("ControlIsEnabled:\t%1\n").arg(controlIsEnabledStrings.join(" / "));
	text += tr("TuningDefault:\t\t%1\n").arg(isTuningDefaultStrings.join(" / "));

	text += "\n";

	if (m_lmStatusFlagMode == LmStatusFlagMode::AccessKey)
	{
		text += tr("WritingIsEnabled:\t%1\n").arg(writingIsEnabledStrings.join(" / "));
	}

	text += tr("WriteClientHash:\t%1\n").arg(writeClientHashes.join(" / "));
	text += tr("WriteErrorCode:\t\t%1\n").arg(writeErrorCodes.join(" / "));

	text += "\n";

	text += tr("LM Time:\t\t%1\n").arg(lmTimes.join(" / "));

	text += "\n";

	text += tr("SuccessfulReadTime:\t%1\n").arg(successfulReadTimes.join(" / "));
	text += tr("WriteRequestTime:\t%1\n").arg(writeRequestTimes.join(" / "));
	text += tr("SuccessfulWriteTime:\t%1\n").arg(successfulWriteTimes.join(" / "));
	text += tr("UnsuccessfulWriteTime:\t%1\n").arg(unsuccessfulWriteTimes.join(" / "));

	if (m_textEditText != text)
	{
		m_textEditText = text;

		ui->m_textEdit->setPlainText(text);
	}

	return;
}
