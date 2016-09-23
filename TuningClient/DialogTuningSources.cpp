#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"
#include "TcpTuningClient.h"

DialogTuningSources::DialogTuningSources(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogTuningSources)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	ui->tableWidget->setColumnCount(20);

	m_updateStateTimerId = startTimer(250);

}

DialogTuningSources::~DialogTuningSources()
{
	theDialogTuningSources = nullptr;
	delete ui;
}

void DialogTuningSources::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{

		std::vector<TuningSource> tsi = theTcpTuningClient->tuningSourcesInfo();

		if (tsi.size() == 0)
		{
			if (ui->tableWidget->rowCount() > 0)
			{
				ui->tableWidget->clear();
			}
			return;
		}

		int count = static_cast<int>(tsi.size());
		if (static_cast<int>(ui->tableWidget->rowCount()) != count)
		{
			ui->tableWidget->setRowCount(static_cast<int>(tsi.size()));
		}

		for (int i = 0; i < count; i++)
		{
			TuningSource& ts = tsi[i];

			int c = 0;

			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_id));
			ui->tableWidget->item(i, c++)->setText(ts.m_equipmentId);
			ui->tableWidget->item(i, c++)->setText(ts.m_caption);
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_dataType));
			ui->tableWidget->item(i, c++)->setText(ts.m_ip);
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_port));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_channel));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_subsystemID));
			ui->tableWidget->item(i, c++)->setText(ts.m_subsystem);

			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_lmNumber));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_lmModuleType));
			ui->tableWidget->item(i, c++)->setText(ts.m_lmAdapterID);
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_lmDataEnable));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_lmDataID));

			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_uptime));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_receivedDataSize));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_dataReceivingRate));
			ui->tableWidget->item(i, c++)->setText(QString::number(ts.m_respond));
		}
	}
}

DialogTuningSources* theDialogTuningSources = nullptr;
