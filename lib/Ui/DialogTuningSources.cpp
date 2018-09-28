#include "DialogTuningSources.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Ui/DialogTuningSourceInfo.h"

#include <QTreeWidget>

const QString DialogTuningSources::m_singleLmControlEnabledString("Single LM control mode is enabled");
const QString DialogTuningSources::m_singleLmControlDisabledString("Single LM control mode is disabled");

DialogTuningSources::DialogTuningSources(TuningTcpClient* tcpClient, bool hasActivationControls, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_tuningTcpClient(tcpClient),
	m_hasActivationControls(hasActivationControls),
	m_parent(parent)
{
	if (m_tuningTcpClient == nullptr)
	{
		assert(m_tuningTcpClient);
		return;
	}

	setAttribute(Qt::WA_DeleteOnClose);

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	connect(m_treeWidget, &QTreeWidget::doubleClicked, this, &DialogTuningSources::on_btnDetails_clicked);
	connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &DialogTuningSources::on_treeWidget_itemSelectionChanged);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	m_btnDetails = new QPushButton(tr("Details..."));
	m_btnDetails->setEnabled(false);
	connect(m_btnDetails, &QPushButton::clicked, this, &DialogTuningSources::on_btnDetails_clicked);
	bottomLayout->addWidget(m_btnDetails);

	if (m_hasActivationControls == true)
	{
		m_btnEnableControl = new QPushButton(tr("Activate Control..."));
		m_btnEnableControl->setEnabled(false);
		connect(m_btnEnableControl, &QPushButton::clicked, this, &DialogTuningSources::on_btnEnableControl_clicked);
		bottomLayout->addWidget(m_btnEnableControl);

		m_btnDisableControl = new QPushButton(tr("Deactivate Control..."));
		m_btnDisableControl->setEnabled(false);
		connect(m_btnDisableControl, &QPushButton::clicked, this, &DialogTuningSources::on_btnDisableControl_clicked);
		bottomLayout->addWidget(m_btnDisableControl);

		m_labelSingleControlMode = new QLabel(m_singleLmControlEnabledString);
		bottomLayout->addWidget(m_labelSingleControlMode);
	}

	bottomLayout->addStretch();

	QPushButton* b = new QPushButton(tr("Close"));
	connect(b, &QPushButton::clicked, this, &DialogTuningSources::on_btnClose_clicked);
	bottomLayout->addWidget(b);

	setLayout(mainLayout);

	setFixedSize(1024, 300);
	//


	QStringList headerLabels;
	headerLabels << tr("EquipmentId");
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

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

	update(false);

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);// sort by EquipmentID

	if (m_hasActivationControls == true)
	{
		if (m_btnEnableControl == nullptr || m_btnDisableControl == nullptr || m_labelSingleControlMode == nullptr)
		{
			assert(m_btnEnableControl);
			assert(m_btnDisableControl);
			assert(m_labelSingleControlMode);
			return;
		}

		m_btnEnableControl->setEnabled(false);
		m_btnDisableControl->setEnabled(false);
	}

	connect(m_tuningTcpClient, &TuningTcpClient::tuningSourcesArrived, this, &DialogTuningSources::slot_tuningSourcesArrived);

	m_updateStateTimerId = startTimer(250);
}

DialogTuningSources::~DialogTuningSources()
{
	emit dialogClosed();
}

void DialogTuningSources::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update(true);
	}
}

bool DialogTuningSources::passwordOk()
{
	return true;
}

void DialogTuningSources::slot_tuningSourcesArrived()
{
	update(false);
}

void DialogTuningSources::update(bool refreshOnly)
{
	if (m_tuningTcpClient == nullptr)
	{
		assert(m_tuningTcpClient);
		return;
	}

	std::vector<TuningSource> tsi = m_tuningTcpClient->tuningSourcesInfo();
	int count = static_cast<int>(tsi.size());

	if (m_treeWidget->topLevelItemCount() != count)
	{
		refreshOnly = false;
	}

	if (refreshOnly == false)
	{
		m_treeWidget->clear();

		for (int i = 0; i < count; i++)
		{
			QStringList connectionStrings;

			TuningSource& ts = tsi[i];

			connectionStrings << ts.info.lmequipmentid().c_str();
			connectionStrings << ts.info.lmip().c_str();
			connectionStrings << QString::number(ts.info.lmport());

			connectionStrings << ts.info.lmsubsystemchannel().c_str();

			connectionStrings << ts.info.lmsubsystem().c_str();
			connectionStrings << QString::number(ts.info.lmnumber());

			QTreeWidgetItem* item = new QTreeWidgetItem(connectionStrings);

			item->setData(columnIndex_Hash, Qt::UserRole, ::calcHash(ts.equipmentId()));
			item->setData(columnIndex_EquipmentId, Qt::UserRole, ts.equipmentId());

			m_treeWidget->addTopLevelItem(item);
		}

		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		m_treeWidget->setColumnWidth(State, 120);

		if (m_hasActivationControls == true)
		{
			// Single control mode controls

			if (m_btnEnableControl == nullptr || m_btnDisableControl == nullptr || m_labelSingleControlMode == nullptr)
			{
				assert(m_btnEnableControl);
				assert(m_btnDisableControl);
				assert(m_labelSingleControlMode);
				return;
			}

			if (m_singleControlMode != m_tuningTcpClient->singleLmControlMode())
			{
				m_singleControlMode = m_tuningTcpClient->singleLmControlMode();

				m_labelSingleControlMode->setText(m_singleControlMode == true ? m_singleLmControlEnabledString : m_singleLmControlDisabledString);

				m_btnEnableControl->setEnabled(m_singleControlMode == true);
				m_btnDisableControl->setEnabled(m_singleControlMode == true);
			}
		}
	}

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);

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
					//item->setBackground(State, QBrush(Qt::red));
					//item->setForeground(State, QBrush(Qt::white));
					item->setForeground(State, QBrush(Qt::red));

					item->setText(State, tr("No Reply"));
				}
				else
				{
					int errorsCount = ts.getErrorsCount();

					if (errorsCount == 0)
					{
						//item->setBackground(State, QBrush(Qt::white));
						item->setForeground(State, QBrush(Qt::black));

						item->setText(State, tr("Active"));
					}
					else
					{
						//item->setBackground(State, QBrush(Qt::red));
						//item->setForeground(State, QBrush(Qt::white));
						item->setForeground(State, QBrush(Qt::red));

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
	if (m_tuningTcpClient == nullptr)
	{
		assert(m_tuningTcpClient);
		return;
	}

	QTreeWidgetItem* item = m_treeWidget->currentItem();

	if (item == nullptr)
	{
		return;
	}

	Hash hash = item->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

	DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClient, this, hash);
	dlg->show();
}

void DialogTuningSources::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* item = m_treeWidget->currentItem();

	m_btnDetails->setEnabled(item != nullptr);

	if (m_hasActivationControls == true)
	{
		// Single control mode controls

		if (m_btnEnableControl == nullptr || m_btnDisableControl == nullptr || m_labelSingleControlMode == nullptr)
		{
			assert(m_btnEnableControl);
			assert(m_btnDisableControl);
			assert(m_labelSingleControlMode);
			return;
		}
		if (item == nullptr)
		{
			m_btnEnableControl->setEnabled(false);
			m_btnDisableControl->setEnabled(false);
		}
		else
		{
			m_btnEnableControl->setEnabled(m_singleControlMode);
			m_btnDisableControl->setEnabled(m_singleControlMode);
		}
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
	if (m_tuningTcpClient == nullptr)
	{
		assert(m_tuningTcpClient);
		return;
	}

	QList<QTreeWidgetItem*> items = m_treeWidget->selectedItems();
	if (items.size() != 1)
	{
		QMessageBox::warning(this, qAppName(), tr("Please select a tuning source!"));
		return;
	}

	if (passwordOk() == false)
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


