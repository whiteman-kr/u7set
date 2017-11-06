#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"
#include "MainWindow.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "DialogTuningSourceInfo.h"

DialogTuningSources::DialogTuningSources(TuningClientTcpClient* tcpClient, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTuningSources),
	m_tcpClient(tcpClient)
{
	assert(m_tcpClient);

	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	QStringList headerLabels;
	headerLabels << tr("Id");
	headerLabels << tr("EquipmentId");
	headerLabels << tr("Caption");
	headerLabels << tr("Ip");
	headerLabels << tr("Port");
	headerLabels << tr("Channel");
	headerLabels << tr("SubsystemID");
	headerLabels << tr("Subsystem");
	headerLabels << tr("LmNumber");

	headerLabels << tr("IsReply");
	headerLabels << tr("RequestCount");
	headerLabels << tr("ReplyCount");
	headerLabels << tr("CommandQueueSize");

	ui->treeWidget->setColumnCount(headerLabels.size());
	ui->treeWidget->setHeaderLabels(headerLabels);

	update(false);

	ui->treeWidget->setSortingEnabled(true);
	ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);// sort by EquipmentID

	connect(m_tcpClient, &TuningTcpClient::tuningSourcesArrived, this, &DialogTuningSources::slot_tuningSourcesArrived);

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
	std::vector<TuningSource> tsi = m_tcpClient->tuningSourcesInfo();
	int count = static_cast<int>(tsi.size());

	if (ui->treeWidget->topLevelItemCount() != count)
	{
		refreshOnly = false;
	}

	if (refreshOnly == false)
	{
		ui->treeWidget->clear();

		for (int i = 0; i < count; i++)
		{
			QStringList connectionStrings;

			TuningSource& ts = tsi[i];

			connectionStrings << QString::number(ts.info.id());
			connectionStrings << ts.info.equipmentid().c_str();
			connectionStrings << ts.info.caption().c_str();
			connectionStrings << ts.info.ip().c_str();
			connectionStrings << QString::number(ts.info.port());

			QChar chChannel = 'A' + ts.info.channel();
			connectionStrings << chChannel;

			connectionStrings << QString::number(ts.info.subsystemid());
			connectionStrings << ts.info.subsystem().c_str();
			connectionStrings << QString::number(ts.info.lmnumber());

			QTreeWidgetItem* item = new QTreeWidgetItem(connectionStrings);

			item->setData(0, Qt::UserRole, static_cast<quint64>(ts.info.id()));

			ui->treeWidget->addTopLevelItem(item);
		}

		for (int i = 0; i < ui->treeWidget->columnCount(); i++)
		{
			ui->treeWidget->resizeColumnToContents(i);
		}
	}

	const int dynamicColumn = 9;

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);

		if (item == nullptr)
		{
			assert(false);
			continue;
		}

		quint64 id = item->data(0, Qt::UserRole).value<quint64>();

		TuningSource ts;

		if (m_tcpClient->tuningSourceInfo(id, &ts) == true)
		{
			int col = dynamicColumn;

			item->setText(col++, ts.state.isreply() ? tr("Yes") : tr("No"));
			item->setText(col++, QString::number(ts.state.requestcount()));
			item->setText(col++, QString::number(ts.state.replycount()));
			item->setText(col++, QString::number(ts.state.commandqueuesize()));
		}
	}
}

DialogTuningSources* theDialogTuningSources = nullptr;

void DialogTuningSources::on_treeWidget_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	on_btnDetails_clicked();
}

void DialogTuningSources::on_btnClose_clicked()
{
	reject();
}

void DialogTuningSources::on_btnDetails_clicked()
{
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	if (item == nullptr)
	{
		return;
	}

	quint64 id = item->data(0, Qt::UserRole).value<quint64>();

	DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tcpClient, this, id);
	dlg->exec();
}

void DialogTuningSources::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	ui->btnDetails->setEnabled(item != nullptr);
}
