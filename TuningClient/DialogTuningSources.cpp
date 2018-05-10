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
	m_tuningTcpClient(tcpClient),
	m_parent(parent)
{
	assert(m_tuningTcpClient);

	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	ui->labelSingleControlMode->setText(m_singleLmControlEnabledString);

	QStringList headerLabels;
	headerLabels << tr("EquipmentId");
	headerLabels << tr("Caption");
	headerLabels << tr("Ip");
	headerLabels << tr("Port");
	headerLabels << tr("Channel");
	headerLabels << tr("SubsystemID");
	headerLabels << tr("LmNumber");

	headerLabels << tr("State");
	headerLabels << tr("IsActive");
	headerLabels << tr("HasUnapplied");
	headerLabels << tr("RequestCount");
	headerLabels << tr("ReplyCount");

	ui->treeWidget->setColumnCount(headerLabels.size());
	ui->treeWidget->setHeaderLabels(headerLabels);

	update(false);

	ui->treeWidget->setSortingEnabled(true);
	ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);// sort by EquipmentID

	ui->btnEnableControl->setEnabled(false);
	ui->btnDisableControl->setEnabled(false);

	connect(m_tuningTcpClient, &TuningTcpClient::tuningSourcesArrived, this, &DialogTuningSources::slot_tuningSourcesArrived);

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
	std::vector<TuningSource> tsi = m_tuningTcpClient->tuningSourcesInfo();
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

			connectionStrings << ts.info.lmequipmentid().c_str();
			connectionStrings << ts.info.lmcaption().c_str();
			connectionStrings << ts.info.lmip().c_str();
			connectionStrings << QString::number(ts.info.lmport());

			connectionStrings << ts.info.lmsubsystemchannel().c_str();

			connectionStrings << ts.info.lmsubsystem().c_str();
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

		ui->treeWidget->setColumnWidth(State, 120);

		// Single control mode controls
		if (m_singleControlMode != m_tuningTcpClient->singleLmControlMode())
		{
			m_singleControlMode = m_tuningTcpClient->singleLmControlMode();

			ui->labelSingleControlMode->setText(m_singleControlMode == true ? m_singleLmControlEnabledString : m_singleLmControlDisabledString);

			ui->btnEnableControl->setEnabled(m_singleControlMode == true);
			ui->btnDisableControl->setEnabled(m_singleControlMode == true);
		}
	}

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

		if (m_tuningTcpClient->tuningSourceInfo(hash, &ts) == true)
		{
			if (ts.state.controlisactive() == true)
			{
				if (ts.state.isreply() == false)
				{
					item->setBackground(State, QBrush(Qt::red));
					item->setForeground(State, QBrush(Qt::white));

					item->setText(State, tr("No Reply"));
				}
				else
				{
					int errorsCount = ts.getErrorsCount();

					if (errorsCount == 0)
					{
						item->setBackground(State, QBrush(Qt::white));
						item->setForeground(State, QBrush(Qt::black));

						item->setText(State, tr("Active"));
					}
					else
					{
						item->setBackground(State, QBrush(Qt::red));
						item->setForeground(State, QBrush(Qt::white));

						item->setText(State, tr("E: %1").arg(errorsCount));
					}
				}
			}
			else
			{
				item->setText(State, tr("Inactive"));
			}

			item->setText(IsActive, ts.state.controlisactive() ? tr("Yes") : tr("No"));
			item->setText(HasUnappliedParams, ts.state.hasunappliedparams() ? tr("Yes") : tr("No"));
			item->setText(RequestCount, QString::number(ts.state.requestcount()));
			item->setText(ReplyCount, QString::number(ts.state.replycount()));
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

	DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClient, m_parent, hash);
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

	bool forceTakeControl = false;

	if (m_tuningTcpClient->singleLmControlMode() == true && m_tuningTcpClient->clientIsActive() == false)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!\r\n\r\nCurrent client is not selected as active now.\r\n\r\nAre you sure you want to take control and %1 the source %2?").arg(action).arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}

		forceTakeControl = true;
	}
	else
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Are you sure you want to %1 the source %2?").arg(action).arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	if (m_tuningTcpClient->activateTuningSourceControl(equipmentId, enable, forceTakeControl) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("An error has been occured!"));
	}
}


