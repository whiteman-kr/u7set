#include "DialogTuningSources.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "UiTools.h"

#include <QTreeWidget>

DialogTuningSourceInfo::DialogTuningSourceInfo(TuningTcpClient* tcpClient, QWidget* parent, Hash sourceHash) :
	DialogSourceInfo(parent, sourceHash),
	m_tcpClient(tcpClient)
{
	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_sourceHash, &ts) == true)
	{
		setWindowTitle(tr("Tuning Source - ") + ts.info.lmequipmentid().c_str());
	}
	else
	{
		setWindowTitle("???");
	}

	//

	QHBoxLayout* l = new QHBoxLayout();

	m_treeWidget = new QTreeWidget();

	l->addWidget(m_treeWidget);

	setLayout(l);

	setMinimumSize(640, 600);

	QStringList headerLabels;
	headerLabels << tr("Parameter");
	headerLabels << tr("Value");
	headerLabels << QString();

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

	QTreeWidgetItem* infoItem = new QTreeWidgetItem(QStringList() << tr("1-Source Information"));

	createDataItem(infoItem, "ID");
	createDataItem(infoItem, "EquipmentID");
	createDataItem(infoItem, "Caption");
	createDataItem(infoItem, "DataType");
	createDataItem(infoItem, "IP");
	createDataItem(infoItem, "Port");
	createDataItem(infoItem, "Channel");
	createDataItem(infoItem, "SubsystemID");
	createDataItem(infoItem, "Subsystem");

	createDataItem(infoItem, "LmNumber");
	createDataItem(infoItem, "LmModuleType");
	createDataItem(infoItem, "LmAdapterID");
	createDataItem(infoItem, "LmDataEnable");
	createDataItem(infoItem, "LmDataID");

	m_treeWidget->addTopLevelItem(infoItem);

	infoItem->setExpanded(true);

	QTreeWidgetItem* stateItem = new QTreeWidgetItem(QStringList() << tr("2-Source State"));

	createDataItem(stateItem, "IsReply");
	createDataItem(stateItem, "RequestCount");
	createDataItem(stateItem, "ReplyCount");
	createDataItem(stateItem, "CommandQueueSize");
	createDataItem(stateItem, "ControlIsActive");
	createDataItem(stateItem, "SetSOR");

	createDataItem(stateItem, "ErrUntimelyReplay");
	createDataItem(stateItem, "ErrSent");
	createDataItem(stateItem, "ErrPartialSent");
	createDataItem(stateItem, "ErrReplySize");
	createDataItem(stateItem, "ErrNoReply");
	createDataItem(stateItem, "ErrAnalogLowBoundCheck");
	createDataItem(stateItem, "ErrAnalogHighBoundCheck");

	m_treeWidget->addTopLevelItem(stateItem);

	stateItem->setExpanded(true);

	QTreeWidgetItem* errorsRUPItem = new QTreeWidgetItem(QStringList() << tr("3-Errors in Reply RupFrameHeader"));

	createDataItem(errorsRUPItem, "ErrRupProtocolVersion");
	createDataItem(errorsRUPItem, "ErrRupFrameSize");
	createDataItem(errorsRUPItem, "ErrRupNoTuningData");
	createDataItem(errorsRUPItem, "ErrRupModuleType");
	createDataItem(errorsRUPItem, "ErrRupFramesQuantity");
	createDataItem(errorsRUPItem, "ErrRupFrameNumber");
	createDataItem(errorsRUPItem, "ErrRupCRC");

	m_treeWidget->addTopLevelItem(errorsRUPItem);

	QTreeWidgetItem* errorsFotipItem = new QTreeWidgetItem(QStringList() << tr("4-Errors in Reply FotipHeader"));

	createDataItem(errorsFotipItem, "ErrFotipProtocolVersion");
	createDataItem(errorsFotipItem, "ErrFotipUniqueID");
	createDataItem(errorsFotipItem, "ErrFotipLmNumber");
	createDataItem(errorsFotipItem, "ErrFotipSubsystemCode");

	createDataItem(errorsFotipItem, "ErrFotipOperationCode");
	createDataItem(errorsFotipItem, "ErrFotipFrameSize");
	createDataItem(errorsFotipItem, "ErrFotipRomSize");
	createDataItem(errorsFotipItem, "ErrFotipRomFrameSize");

	m_treeWidget->addTopLevelItem(errorsFotipItem);

	QTreeWidgetItem* errorsFotipFlagItem = new QTreeWidgetItem(QStringList() << tr("5-Errors Reported by LM in Reply FotipHeader.flags"));

	createDataItem(errorsFotipFlagItem, "FotipFlagBoundsCheckSuccess");
	createDataItem(errorsFotipFlagItem, "FotipFlagWriteSuccess");
	createDataItem(errorsFotipFlagItem, "FotipFlagDataTypeErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagOpCodeErr");

	createDataItem(errorsFotipFlagItem, "FotipFlagStartAddrErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagRomSizeErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagRomFrameSizeErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagFrameSizeErr");

	createDataItem(errorsFotipFlagItem, "FotipFlagProtocolVersionErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagSubsystemKeyErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagUniueIDErr");
	createDataItem(errorsFotipFlagItem, "FotipFlagOffsetErr");

	createDataItem(errorsFotipFlagItem, "FotipFlagApplySuccess");
	createDataItem(errorsFotipFlagItem, "FotipFlagSetSOR");

	m_treeWidget->addTopLevelItem(errorsFotipFlagItem);

	updateData();

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

}

DialogTuningSourceInfo::~DialogTuningSourceInfo()
{

}

void DialogTuningSourceInfo::updateData()
{
	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	TuningSource ts;

	if (m_tcpClient->tuningSourceInfo(m_sourceHash, &ts) == false)
	{
		return;
	}

	// info

	QTreeWidgetItem* item = m_treeWidget->topLevelItem(0);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	int c = 0;

	item->setData(0, Qt::UserRole, 0);

	setDataItemText("ID", QString::number(ts.info.id(), 16));
	setDataItemText("EquipmentID", ts.info.lmequipmentid().c_str());
	setDataItemText("Caption", ts.info.lmcaption().c_str());
	setDataItemNumber("DataType", ts.info.lmdatatype());
	setDataItemText("IP", ts.info.lmip().c_str());
	setDataItemNumber("Port", ts.info.lmport());
	setDataItemText("Channel", ts.info.lmsubsystemchannel().c_str());
	setDataItemNumber("SubsystemID", ts.info.lmsubsystemid());
	setDataItemText("Subsystem", ts.info.lmsubsystem().c_str());

	setDataItemNumber("LmNumber", ts.info.lmnumber());
	setDataItemNumber("LmModuleType", ts.info.lmmoduletype());
	setDataItemText("LmAdapterID", ts.info.lmadapterid().c_str());
	setDataItemNumber("LmDataEnable", ts.info.lmdataenable());
	setDataItemNumber("LmDataID", ts.info.lmdataid());

	// state

	item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setDataItemText("IsReply", ts.state.isreply() ? "Yes" : "No");

	{
		QTreeWidgetItem*  item = dataItem("IsReply");
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		if (ts.state.isreply() == false)
		{
			item->setForeground(1, QBrush(DialogSourceInfo::dataItemErrorColor));
		}
		else
		{
			item->setForeground(1, QBrush(Qt::black));
		}
	}

	setDataItemNumber("RequestCount", ts.state.requestcount());
	setDataItemNumber("ReplyCount", ts.state.replycount());
	setDataItemNumber("CommandQueueSize", ts.state.commandqueuesize());
	setDataItemText("ControlIsActive", ts.state.controlisactive() ? "Yes" : "No");
	setDataItemText("SetSOR", ts.state.setsor() ? "Yes" : "No");

	setDataItemNumber("ErrUntimelyReplay", ts.state.erruntimelyreplay());
	setDataItemNumber("ErrSent", ts.state.errsent());
	setDataItemNumber("ErrPartialSent", ts.state.errpartialsent());
	setDataItemNumber("ErrReplySize", ts.state.errreplysize());
	setDataItemNumberCompare(item, "ErrNoReply", ts.state.errnoreply(), ts.previousState().errnoreply());
	setDataItemNumber("ErrAnalogLowBoundCheck", ts.state.erranaloglowboundcheck());
	setDataItemNumber("ErrAnalogHighBoundCheck", ts.state.erranaloghighboundcheck());

	updateParentItemState(item);

	// RupFrameHeader

	item = m_treeWidget->topLevelItem(2);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumberCompare(item, "ErrRupProtocolVersion", ts.state.errrupprotocolversion(), ts.previousState().errrupprotocolversion());
	setDataItemNumberCompare(item, "ErrRupFrameSize", ts.state.errrupframesize(), ts.previousState().errrupframesize());
	setDataItemNumberCompare(item, "ErrRupNoTuningData", ts.state.errrupnontuningdata(), ts.previousState().errrupnontuningdata());
	setDataItemNumberCompare(item, "ErrRupModuleType", ts.state.errrupmoduletype(), ts.previousState().errrupmoduletype());
	setDataItemNumberCompare(item, "ErrRupFramesQuantity", ts.state.errrupframesquantity(), ts.previousState().errrupframesquantity());
	setDataItemNumberCompare(item, "ErrRupFrameNumber", ts.state.errrupframenumber(), ts.previousState().errrupframenumber());
	setDataItemNumberCompare(item, "ErrRupCRC", ts.state.errrupcrc(), ts.previousState().errrupcrc());

	updateParentItemState(item);

	// FotipHeader

	item = m_treeWidget->topLevelItem(3);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumberCompare(item, "ErrFotipProtocolVersion", ts.state.errfotipprotocolversion(), ts.previousState().errfotipprotocolversion());
	setDataItemNumberCompare(item, "ErrFotipUniqueID", ts.state.errfotipuniqueid(), ts.previousState().errfotipuniqueid());
	setDataItemNumberCompare(item, "ErrFotipLmNumber", ts.state.errfotiplmnumber(), ts.previousState().errfotiplmnumber());
	setDataItemNumberCompare(item, "ErrFotipSubsystemCode", ts.state.errfotipsubsystemcode(), ts.previousState().errfotipsubsystemcode());

	setDataItemNumberCompare(item, "ErrFotipOperationCode", ts.state.errfotipoperationcode(), ts.previousState().errfotipoperationcode());
	setDataItemNumberCompare(item, "ErrFotipFrameSize", ts.state.errfotipframesize(), ts.previousState().errfotipframesize());
	setDataItemNumberCompare(item, "ErrFotipRomSize", ts.state.errfotipromsize(), ts.previousState().errfotipromsize());
	setDataItemNumberCompare(item, "ErrFotipRomFrameSize", ts.state.errfotipromframesize(), ts.previousState().errfotipromframesize());

	updateParentItemState(item);

	// FotipFlags

	item = m_treeWidget->topLevelItem(4);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	c = 0;

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumber("FotipFlagBoundsCheckSuccess", ts.state.fotipflagboundschecksuccess());
	setDataItemNumber("FotipFlagWriteSuccess", ts.state.fotipflagwritesuccess());
	setDataItemNumberCompare(item, "FotipFlagDataTypeErr", ts.state.fotipflagdatatypeerr(), ts.previousState().fotipflagdatatypeerr());
	setDataItemNumberCompare(item, "FotipFlagOpCodeErr", ts.state.fotipflagopcodeerr(), ts.previousState().fotipflagopcodeerr());

	setDataItemNumberCompare(item, "FotipFlagStartAddrErr", ts.state.fotipflagstartaddrerr(), ts.previousState().fotipflagstartaddrerr());
	setDataItemNumberCompare(item, "FotipFlagRomSizeErr", ts.state.fotipflagromsizeerr(), ts.previousState().fotipflagromsizeerr());
	setDataItemNumberCompare(item, "FotipFlagRomFrameSizeErr", ts.state.fotipflagromframesizeerr(), ts.previousState().fotipflagromframesizeerr());
	setDataItemNumberCompare(item, "FotipFlagFrameSizeErr", ts.state.fotipflagframesizeerr(), ts.previousState().fotipflagframesizeerr());

	setDataItemNumberCompare(item, "FotipFlagProtocolVersionErr", ts.state.fotipflagprotocolversionerr(), ts.previousState().fotipflagprotocolversionerr());
	setDataItemNumberCompare(item, "FotipFlagSubsystemKeyErr", ts.state.fotipflagsubsystemkeyerr(), ts.previousState().fotipflagsubsystemkeyerr());
	setDataItemNumberCompare(item, "FotipFlagUniueIDErr", ts.state.fotipflaguniueiderr(), ts.previousState().fotipflaguniueiderr());
	setDataItemNumberCompare(item, "FotipFlagOffsetErr", ts.state.fotipflagoffseterr(), ts.previousState().fotipflagoffseterr());

	setDataItemNumber("FotipFlagApplySuccess", ts.state.fotipflagapplysuccess());
	setDataItemNumber("FotipFlagSetSOR", ts.state.fotipflagsetsor());

	updateParentItemState(item);
}





//
// ---
//

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

	setWindowTitle(tr("Tuning Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &DialogTuningSources::on_treeWidget_itemDoubleClicked);
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

	setMinimumSize(1024, 300);
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
					item->setForeground(State, QBrush(DialogSourceInfo::dataItemErrorColor));

					item->setText(State, tr("No Reply"));
				}
				else
				{
					int errorsCount = ts.getErrorsCount();

					if (errorsCount == 0)
					{
						item->setForeground(State, QBrush(Qt::black));

						item->setText(State, tr("Active"));
					}
					else
					{
						item->setForeground(State, QBrush(DialogSourceInfo::dataItemErrorColor));

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

	auto it = m_sourceInfoDialogsMap.find(hash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClient, this, hash);
		connect(dlg, &DialogTuningSourceInfo::dialogClosed, this, &DialogTuningSources::onDetailsDialogClosed);
		dlg->show();
		dlg->activateWindow();

		m_sourceInfoDialogsMap[hash] = dlg;
	}
	else
	{
		DialogTuningSourceInfo* dlg = it->second;
		dlg->activateWindow();

		UiTools::adjustDialogPlacement(dlg);
	}
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

void DialogTuningSources::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	QTimer::singleShot(10, this, &DialogTuningSources::on_btnDetails_clicked);
}

void DialogTuningSources::on_btnEnableControl_clicked()
{
	activateControl(true);
}

void DialogTuningSources::on_btnDisableControl_clicked()
{
	activateControl(false);
}

void DialogTuningSources::onDetailsDialogClosed(Hash hash)
{
	auto it = m_sourceInfoDialogsMap.find(hash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		assert(false);
		return;
	}

	m_sourceInfoDialogsMap.erase(it);

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


