#include "DialogSignalInfo.h"
#include "ui_DialogSignalInfo.h"

#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"

DialogSignalInfo::DialogSignalInfo(Hash appSignalHash, TuningSignalManager* signalManager, QWidget *parent) :
	m_appSignalHash(appSignalHash),
	m_signalManager(signalManager),
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalInfo)
{
	ui->setupUi(this);

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
		text += tr("ModifiedValue:\t\tDisabled\r\n");
	}
	else
	{
		int precision = 0;

		if (asp.isAnalog() == true)
		{
			precision = asp.precision();
		}

		text += tr("ModifiedValue:\t\t%1\r\n").arg(state.modifiedValue().toString(precision));
	}

	text += "\r\n";

	text += tr("Valid:\t\t%1\r\n").arg(state.valid() == true ? tr("Yes") : tr("No"));
	text += tr("OutOfRange:\t\t%1\r\n").arg(state.outOfRange() == true ? tr("Yes") : tr("No"));
	text += tr("WriteInProgress:\t%1\r\n").arg(state.writeInProgress() == true ? tr("Yes") : tr("No"));
	text += tr("ControlIsEnabled:\t%1\r\n").arg(state.controlIsEnabled() == true ? tr("Yes") : tr("No"));
	text += tr("UserModified:\t\t%1\r\n").arg(state.userModified() == true ? tr("Yes") : tr("No"));

	text += "\r\n";

	text += tr("WriteClientHash:\t%1\r\n").arg(state.writeClient());
	text += tr("WriteErrorCode:\t\t%1\r\n").arg(getNetworkErrorStr(static_cast<NetworkError>(state.writeErrorCode())));

	text += "\r\n";

	text += tr("SuccessfulReadTime:\t%1\r\n").arg(state.successfulReadTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("WriteRequestTime:\t%1\r\n").arg(state.writeRequestTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("SuccessfulWriteTime:\t%1\r\n").arg(state.successfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	text += tr("UnsuccessfulWriteTime:\t%1\r\n").arg(state.unsuccessfulWriteTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));;

	if (ui->m_textEdit->toPlainText() != text)
	{
		ui->m_textEdit->setPlainText(text);
	}
}
