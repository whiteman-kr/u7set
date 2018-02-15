#include "DialogTuningSources.h"
#include "ui_DialogTuningSources.h"
#include "MainWindow.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "DialogTuningSourceInfo.h"

const QString DialogTuningSources::m_singleLmControlEnabledString("Single LM control mode is enabled");
const QString DialogTuningSources::m_singleLmControlDisabledString("Single LM control mode is disabled");

DialogTuningSources::DialogTuningSources(TuningClientTcpClient* tcpClient, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTuningSources),
	m_tcpClient(tcpClient),
	m_parent(parent)
{
	assert(m_tcpClient);

	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	ui->labelSingleControlMode->setText(m_singleLmControlEnabledString);

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
	headerLabels << tr("ControlIsActive");
	headerLabels << tr("RequestCount");
	headerLabels << tr("ReplyCount");

	ui->treeWidget->setColumnCount(headerLabels.size());
	ui->treeWidget->setHeaderLabels(headerLabels);

	update(false);

	ui->treeWidget->setSortingEnabled(true);
	ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);// sort by EquipmentID

	ui->btnEnableControl->setEnabled(false);
	ui->btnDisableControl->setEnabled(false);

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

			item->setData(columnIndex_Hash, Qt::UserRole, ::calcHash(ts.equipmentId()));
			item->setData(columnIndex_EquipmentId, Qt::UserRole, ts.equipmentId());

			ui->treeWidget->addTopLevelItem(item);
		}

		for (int i = 0; i < ui->treeWidget->columnCount(); i++)
		{
			ui->treeWidget->resizeColumnToContents(i);
		}

		// Single control mode controls
		if (m_singleControlMode != m_tcpClient->singleLmControlMode())
		{
			m_singleControlMode = m_tcpClient->singleLmControlMode();

			ui->labelSingleControlMode->setText(m_singleControlMode == true ? m_singleLmControlEnabledString : m_singleLmControlDisabledString);

			ui->btnEnableControl->setEnabled(m_singleControlMode == true);
			ui->btnDisableControl->setEnabled(m_singleControlMode == true);
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

		Hash hash = item->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

		TuningSource ts;

		if (m_tcpClient->tuningSourceInfo(hash, &ts) == true)
		{
			int col = dynamicColumn;

			item->setText(col++, ts.state.isreply() ? tr("Yes") : tr("No"));
			item->setText(col++, ts.state.controlisactive() ? tr("Yes") : tr("No"));
			item->setText(col++, QString::number(ts.state.requestcount()));
			item->setText(col++, QString::number(ts.state.replycount()));
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

	Hash hash = item->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

	DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tcpClient, m_parent, hash);
	dlg->show();
}

void DialogTuningSources::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	ui->btnDetails->setEnabled(item != nullptr);

	if (item == nullptr)
	{
		ui->btnEnableControl->setEnabled(false);
		ui->btnDisableControl->setEnabled(false);
	}
	else
	{
		ui->btnEnableControl->setEnabled(m_singleControlMode);
		ui->btnDisableControl->setEnabled(m_singleControlMode);
	}
}

void DialogTuningSources::on_btnEnableControl_clicked()
{
	activateControl(true);
}

void DialogTuningSources::on_btnDisableControl_clicked()
{
	activateControl(false);
}

void DialogTuningSources::activateControl(bool enable)
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (items.size() != 1)
	{
		QMessageBox::warning(this, qAppName(), tr("Please select a tuning source!"));
		return;
	}

	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	QTreeWidgetItem* item = items[0];
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QString equipmentId = item->data(columnIndex_EquipmentId, Qt::UserRole).value<QString>();

	QString action = enable ? tr("activate") : tr("deactivate");

	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want to %1 the source %2?").arg(action).arg(equipmentId),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	if (m_tcpClient->activateTuningSourceControl(equipmentId, enable) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("An error has been occured!"));
	}
}


