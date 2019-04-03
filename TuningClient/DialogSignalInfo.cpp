#include "DialogSignalInfo.h"
#include "ui_DialogSignalInfo.h"

#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"
#include "Settings.h"

DialogSignalInfo::DialogSignalInfo(Hash appSignalHash, Hash instanceIdHash, TuningSignalManager* signalManager, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalInfo),
	m_appSignalHash(appSignalHash),
	m_instanceIdHash(instanceIdHash),
	m_signalManager(signalManager)
{
	ui->setupUi(this);

	ui->m_textEdit->setReadOnly(true);

	setAttribute(Qt::WA_DeleteOnClose);

	assert(m_signalManager);

	bool found = false;

	AppSignalParam asp = m_signalManager->signalParam(m_appSignalHash, &found);

	ui->m_lineAppSignalId->setText(asp.appSignalId());
	ui->m_lineCustomAppSignalId->setText(asp.customSignalId());
	ui->m_lineCaption->setText(asp.caption());
	ui->m_lineEquipmentId->setText(asp.equipmentId());

	updateInfo();

	setWindowTitle(tr("%1 - %2").arg(asp.customSignalId()).arg(asp.caption()));

	m_timerId = startTimer(500);

}

DialogSignalInfo::~DialogSignalInfo()
{
	delete ui;
}

void DialogSignalInfo::timerEvent(QTimerEvent* event)
{
	assert(event);

	if (event->timerId() != m_timerId)
	{
		assert(false);
		return;
	}

	updateInfo();
}

void DialogSignalInfo::updateInfo()
{
	bool found = false;

	AppSignalParam asp = m_signalManager->signalParam(m_appSignalHash, &found);

	TuningSignalState state = m_signalManager->state(m_appSignalHash, &found);

	QString text;

	if (state.controlIsEnabled() == false)
	{
		text = tr("Disabled");
	}
	else
	{
		if (state.valid() == false)
		{
			text = tr("?");
		}
		else
		{
			if (asp.isAnalog() == true)
			{
				text = state.value().toString(asp.precision()) + " " + asp.unit();
			}
			else
			{
				text = state.value().toString();
			}
		}
	}

	ui->m_labelValue->setText(text);

	// Flags and info

	text.clear();

	if (state.controlIsEnabled() == false)
	{
		text += tr("NewValue:\t\tDisabled\n");
	}
	else
	{
		if (m_signalManager->newValueIsUnapplied(m_appSignalHash) == true)
		{
			int precision = 0;

			if (asp.isAnalog() == true)
			{
				precision = asp.precision();
			}

			text += tr("NewValue:\t\t%1\n").arg(m_signalManager->newValue(m_appSignalHash).toString(precision));
		}
		else
		{
			text += tr("NewValue:\t\t-\n");
		}
	}

	text += "\n";

	text += tr("Valid:\t\t%1\n").arg(state.valid() == true ? tr("Yes") : tr("No"));
	text += tr("OutOfRange:\t\t%1\n").arg(state.outOfRange() == true ? tr("Yes") : tr("No"));
	text += tr("WriteInProgress:\t%1\n").arg(state.writeInProgress() == true ? tr("Yes") : tr("No"));
	text += tr("ControlIsEnabled:\t%1\n").arg(state.controlIsEnabled() == true ? tr("Yes") : tr("No"));

	if (theConfigSettings.useAccessFlag == true)
	{
		text += tr("WritingIsEnabled:\t%1\n").arg(state.writingIsEnabled() == true ? tr("Yes") : tr("No"));
	}

	text += "\n";

	QString hashString = QString("%1h").arg(QString::number(state.writeClient(), 16));

	if (state.writeClient() == m_instanceIdHash)
	{
		hashString += tr(" (this client)");
	}

	text += tr("WriteClientHash:\t%1\n").arg(hashString);
	text += tr("WriteErrorCode:\t\t%1\n").arg(getNetworkErrorStr(static_cast<NetworkError>(state.writeErrorCode())));

	text += "\n";

	text += tr("SuccessfulReadTime:\t%1\n").arg(state.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("WriteRequestTime:\t%1\n").arg(state.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("SuccessfulWriteTime:\t%1\n").arg(state.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("UnsuccessfulWriteTime:\t%1\n").arg(state.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));;

	if (m_textEditText != text)
	{
		m_textEditText = text;

		ui->m_textEdit->setPlainText(text);
	}
}
