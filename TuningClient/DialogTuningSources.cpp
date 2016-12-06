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
	headerLabels<<"Ip";
	headerLabels<<"Port";
	headerLabels<<"Channel";
	headerLabels<<"SubsystemID";
	headerLabels<<"Subsystem";
	headerLabels<<"LmNumber";

    headerLabels<<"IsReply";
    headerLabels<<"RequestCount";
    headerLabels<<"ReplyCount";
    headerLabels<<"CommandQueueSize";

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

        ui->tableWidget->item(i, static_cast<int>(Columns::IsReply))->setText(ts.m_state.isreply() ? "Yes" : "No");
        ui->tableWidget->item(i, static_cast<int>(Columns::RequestCount))->setText(QString::number(ts.m_state.requestcount()));
        ui->tableWidget->item(i, static_cast<int>(Columns::ReplyCount))->setText(QString::number(ts.m_state.replycount()));
        ui->tableWidget->item(i, static_cast<int>(Columns::CommandQueueSize))->setText(QString::number(ts.m_state.commandqueuesize()));

        if (refreshOnly == false)
		{
            ui->tableWidget->item(i, static_cast<int>(Columns::Id))->setText(QString::number(ts.m_info.id()));
            ui->tableWidget->item(i, static_cast<int>(Columns::EquipmentId))->setText(ts.m_info.equipmentid().c_str());
            ui->tableWidget->item(i, static_cast<int>(Columns::Caption))->setText(ts.m_info.caption().c_str());
            ui->tableWidget->item(i, static_cast<int>(Columns::Ip))->setText(ts.m_info.ip().c_str());
            ui->tableWidget->item(i, static_cast<int>(Columns::Port))->setText(QString::number(ts.m_info.port()));
            ui->tableWidget->item(i, static_cast<int>(Columns::Channel))->setText(QString::number(ts.m_info.channel()));
            ui->tableWidget->item(i, static_cast<int>(Columns::SubsystemID))->setText(QString::number(ts.m_info.subsystemid()));
            ui->tableWidget->item(i, static_cast<int>(Columns::Subsystem))->setText(ts.m_info.subsystem().c_str());

            ui->tableWidget->item(i, static_cast<int>(Columns::LmNumber))->setText(QString::number(ts.m_info.lmnumber()));

			ui->tableWidget->resizeColumnsToContents();
		}
	}
}

DialogTuningSources* theDialogTuningSources = nullptr;
