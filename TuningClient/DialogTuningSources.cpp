#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"
#include "TcpTuningClient.h"

DialogTuningSources::DialogTuningSources(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTuningSources)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	ui->tableWidget->setColumnCount(static_cast<int>(Columns::ColumnCount));

	QStringList headerLabels;
	headerLabels<<"Id";
	headerLabels<<"EquipmentId";
	headerLabels<<"Caption";
	headerLabels<<"DataType";
	headerLabels<<"Ip";
	headerLabels<<"Port";
	headerLabels<<"Channel";
	headerLabels<<"SubsystemID";
	headerLabels<<"Subsystem";
	headerLabels<<"LmNumber";
	headerLabels<<"LmModuleType";
	headerLabels<<"LmAdapterID";
	headerLabels<<"LmDataEnable";
	headerLabels<<"LmDataID";

	headerLabels<<"Uptime";
	headerLabels<<"ReceivedDataSize";
	headerLabels<<"DataReceivingRate";
	headerLabels<<"Respond";

	ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui->tableWidget->resizeColumnsToContents();

	connect(theTcpTuningClient, &TcpTuningClient::tuningSourcesArrived, this, &DialogTuningSources::slot_tuningSourcesArrived);

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
		update(true);
	}
}

void DialogTuningSources::slot_tuningSourcesArrived()
{
	update(false);
}

void DialogTuningSources::update(bool refreshOnly)
{

	std::vector<TuningSource> tsi = theTcpTuningClient->tuningSourcesInfo();
	int count = static_cast<int>(tsi.size());

	if (refreshOnly == false)
	{
		ui->tableWidget->setRowCount(count);
	}
	else
	{
		if (ui->tableWidget->rowCount() != count)
		{
			assert(false);
			ui->tableWidget->setRowCount(count);
		}
	}

	for (int i = 0; i < count; i++)
	{
		TuningSource& ts = tsi[i];

		ui->tableWidget->item(i, static_cast<int>(Columns::Uptime))->setText(QString::number(ts.m_uptime));
		ui->tableWidget->item(i, static_cast<int>(Columns::ReceivedDataSize))->setText(QString::number(ts.m_receivedDataSize));
		ui->tableWidget->item(i, static_cast<int>(Columns::DataReceivingRate))->setText(QString::number(ts.m_dataReceivingRate));
		ui->tableWidget->item(i, static_cast<int>(Columns::Respond))->setText(QString::number(ts.m_respond));

		if (refreshOnly == false)
		{
			ui->tableWidget->item(i, static_cast<int>(Columns::Id))->setText(QString::number(ts.m_id));
			ui->tableWidget->item(i, static_cast<int>(Columns::EquipmentId))->setText(ts.m_equipmentId);
			ui->tableWidget->item(i, static_cast<int>(Columns::Caption))->setText(ts.m_caption);
			ui->tableWidget->item(i, static_cast<int>(Columns::DataType))->setText(QString::number(ts.m_dataType));
			ui->tableWidget->item(i, static_cast<int>(Columns::Ip))->setText(ts.m_ip);
			ui->tableWidget->item(i, static_cast<int>(Columns::Port))->setText(QString::number(ts.m_port));
			ui->tableWidget->item(i, static_cast<int>(Columns::Channel))->setText(QString::number(ts.m_channel));
			ui->tableWidget->item(i, static_cast<int>(Columns::SubsystemID))->setText(QString::number(ts.m_subsystemID));
			ui->tableWidget->item(i, static_cast<int>(Columns::Subsystem))->setText(ts.m_subsystem);

			ui->tableWidget->item(i, static_cast<int>(Columns::LmNumber))->setText(QString::number(ts.m_lmNumber));
			ui->tableWidget->item(i, static_cast<int>(Columns::LmModuleType))->setText(QString::number(ts.m_lmModuleType));
			ui->tableWidget->item(i, static_cast<int>(Columns::LmAdapterID))->setText(ts.m_lmAdapterID);
			ui->tableWidget->item(i, static_cast<int>(Columns::LmDataEnable))->setText(QString::number(ts.m_lmDataEnable));
			ui->tableWidget->item(i, static_cast<int>(Columns::LmDataID))->setText(QString::number(ts.m_lmDataID));

			ui->tableWidget->resizeColumnsToContents();
		}
	}
}

DialogTuningSources* theDialogTuningSources = nullptr;
