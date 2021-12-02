#include "TuningSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../UtilsLib/Ui/UiTools.h"

#include <QTreeWidget>

DialogTuningSourceInfo::DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, Hash sourceHash) :
	DialogSourceInfo(parent, sourceHash),
	m_tcpClients(tcpClients)
{
	QHBoxLayout* l = new QHBoxLayout();

	m_treeWidget = new QTreeWidget();

	m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,this, &DialogSourceInfo::prepareContextMenu);

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
	createDataItem(stateItem, "WritingDisabled");

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
	createDataItem(errorsFotipFlagItem, "FotipFlagWritingDisabled");

	m_treeWidget->addTopLevelItem(errorsFotipFlagItem);

	updateData();

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

	findActiveTuningTcpClient();
}

DialogTuningSourceInfo::~DialogTuningSourceInfo()
{

}

Hash DialogTuningSourceInfo::sourceHash() const
{
	return m_sourceHash;
}

void DialogTuningSourceInfo::setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients)
{
	m_tcpClients = tcpClients;

	m_activeTcpClient = nullptr;

	findActiveTuningTcpClient();
}

bool DialogTuningSourceInfo::findActiveTuningTcpClient()
{
	for (TuningTcpClient* client : m_tcpClients)
	{
		std::vector<Hash> hashes = client->tuningSourcesEquipmentHashes();

		if (std::find(hashes.begin(), hashes.end(), m_sourceHash) != hashes.end())
		{
			m_activeTcpClient = client;
			return true;
		}
	}

	return false;
}

void DialogTuningSourceInfo::updateData()
{
	static const QString noTuningSourceString = tr("Tuning Source - ") + "?";

	if (m_activeTcpClient == nullptr)
	{
		// Try to find active client
		//
		if (findActiveTuningTcpClient() == false)
		{
			if (windowTitle() != noTuningSourceString)
			{
				setWindowTitle(noTuningSourceString);
			}
			return;
		}
	}

	if (m_activeTcpClient == nullptr)
	{
		Q_ASSERT(m_activeTcpClient);
		return;
	}

	TuningSource ts;

	if (m_activeTcpClient->tuningSourceInfo(m_sourceHash, &ts) == false)
	{
		if (windowTitle() != noTuningSourceString)
		{
			setWindowTitle(noTuningSourceString);
		}
		return;
	}

	// Update Window title

	if (m_sourceEquipmentId.isEmpty() == true)
	{
		m_sourceEquipmentId = ts.equipmentId();
	}

	QString title = tr("Tuning Source - ") + m_sourceEquipmentId;

	if (windowTitle() != title)
	{
		setWindowTitle(title);
	}

	// info

	QTreeWidgetItem* item = m_treeWidget->topLevelItem(0);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(0, Qt::UserRole, 0);

	setDataItemText("ID", tr("%1 (%2h)").arg(QString::number(ts.info.id())).arg(QString::number(ts.info.id(), 16)));
	setDataItemText("EquipmentID", ts.info.moduleequipmentid().c_str());
	setDataItemText("Caption", ts.info.modulecaption().c_str());
	setDataItemNumber("DataType", ts.info.lancontrollerinfo()[0].lancontrollertype());
	setDataItemText("IP", ts.info.lancontrollerinfo()[0].tuningip().c_str());
	setDataItemNumber("Port", ts.info.lancontrollerinfo()[0].tuningport());
	setDataItemText("Channel", ts.info.subsystemchannel().c_str());
	setDataItemNumber("SubsystemID", ts.info.subsystemkey());
	setDataItemText("Subsystem", ts.info.subsystemid().c_str());

	setDataItemNumber("LmNumber", ts.info.lmnumber());
	setDataItemText("LmModuleType", tr("%1 (%2h)").arg(QString::number(ts.info.moduletype())).arg(QString::number(ts.info.moduletype(), 16)));
	setDataItemText("LmAdapterID", ts.info.lancontrollerinfo()[0].equipmentid().c_str());
	setDataItemNumber("LmDataEnable", ts.info.lancontrollerinfo()[0].tuningenable());
	setDataItemText("LmDataID", tr("%1 (%2h)").
					arg(QString::number(ts.info.lancontrollerinfo()[0].tuningdatauid())).
			arg(QString::number(ts.info.lancontrollerinfo()[0].tuningdatauid(), 16)));

	// state

	item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}


	item->setData(0, Qt::UserRole, 0);

	setDataItemText("IsReply", ts.state.isreply() ? "Yes" : "No");

	{
		QTreeWidgetItem* isReplyItem = dataItem("IsReply");
		if (isReplyItem == nullptr)
		{
			assert(isReplyItem);
			return;
		}

		if (ts.state.isreply() == false)
		{
			isReplyItem->setForeground(1, QBrush(DialogSourceInfo::dataItemErrorColor));
		}
		else
		{
			isReplyItem->setForeground(1, QBrush(Qt::black));
		}
	}

	setDataItemNumber("RequestCount", ts.state.requestcount());
	setDataItemNumber("ReplyCount", ts.state.replycount());
	setDataItemNumber("CommandQueueSize", ts.state.commandqueuesize());
	setDataItemText("ControlIsActive", ts.state.controlisactive() ? "Yes" : "No");
	setDataItemText("SetSOR", ts.state.setsor() ? "Yes" : "No");
	setDataItemText("WritingDisabled", ts.state.writingdisabled() ? "Yes" : "No");

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
	setDataItemNumber("FotipFlagWritingDisabled", ts.state.fotipflagwritingdisabled());

	updateParentItemState(item);
}





//
// ---
//


TuningSourcesWidget::TuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, bool hasCloseButton, QWidget* parent) :
	QWidget(parent),
	m_tuningTcpClients(tcpClients),
	m_hasActivationControls(hasActivationControls),
	m_parent(parent)
{
	setWindowTitle(tr("Tuning Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TuningSourcesWidget::treeWidget_itemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &TuningSourcesWidget::treeWidget_itemSelectionChanged);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	m_btnDetails = new QPushButton(tr("Details..."));
	m_btnDetails->setEnabled(false);
	connect(m_btnDetails, &QPushButton::clicked, this, &TuningSourcesWidget::detailsClicked);
	bottomLayout->addWidget(m_btnDetails);

	if (m_hasActivationControls == true)
	{
		m_btnEnableControl = new QPushButton(tr("Activate Control..."));
		m_btnEnableControl->setEnabled(false);
		connect(m_btnEnableControl, &QPushButton::clicked, this, &TuningSourcesWidget::enableControl_clicked);
		bottomLayout->addWidget(m_btnEnableControl);

		m_btnDisableControl = new QPushButton(tr("Deactivate Control..."));
		m_btnDisableControl->setEnabled(false);
		connect(m_btnDisableControl, &QPushButton::clicked, this, &TuningSourcesWidget::disableControl_clicked);
		bottomLayout->addWidget(m_btnDisableControl);
	}

	bottomLayout->addStretch();

	if (hasCloseButton == true)
	{
		QPushButton* b = new QPushButton(tr("Close"));
		connect(b, &QPushButton::clicked, this, &TuningSourcesWidget::closeClicked);
		bottomLayout->addWidget(b);
	}

	setLayout(mainLayout);

	//


	QStringList headerLabels;
	headerLabels << tr("EquipmentId");
	headerLabels << tr("IP");
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

	m_updateStateTimerId = startTimer(250);
}

TuningSourcesWidget::~TuningSourcesWidget()
{
}

void TuningSourcesWidget::setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients)
{
	m_tuningTcpClients = tcpClients;

	// Re-fill sources information list

	update(true);

	// Set tuningTcpClients for open Details dialogs

	for (auto it : m_sourceInfoDialogsMap)
	{
		DialogTuningSourceInfo* d = it.second;
		if (d == nullptr)
		{
			Q_ASSERT(d);
			return;
		}

		d->setTuningTcpClients(m_tuningTcpClients);
	}
}

void TuningSourcesWidget::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update(true);
	}
}

bool TuningSourcesWidget::login()
{
	return true;
}

bool TuningSourcesWidget::checkTuningSourcesChanged() const
{
	int sourcesCount = 0;

	for (const TuningTcpClient* client : m_tuningTcpClients)
	{
		std::vector<TuningSource> clientSources = client->tuningSourcesInfo();

		for (const TuningSource& ts : clientSources)
		{
			sourcesCount++;

			Hash hash = ::calcHash(client->tuningServiceId() + ts.equipmentId());

			if (m_tuningClientsSourcesHashes.find(hash) == m_tuningClientsSourcesHashes.end())
			{
				return true;	// Unknown source has been occured
			}
		}
	}

	if (sourcesCount != m_tuningClientsSourcesHashes.size())
	{
		return true;	// Number of sources changed
	}

	return false;
}

void TuningSourcesWidget::update(bool refreshOnly)
{
	if (checkTuningSourcesChanged() == true)
	{
		refreshOnly = false;
	}

	if (refreshOnly == false)
	{
		m_treeWidget->clear();

		m_tuningClientsSourcesHashes.clear();

		for (const TuningTcpClient* client : m_tuningTcpClients)
		{
			// Add top-level item with client

			QTreeWidgetItem* clientItem = new QTreeWidgetItem();

			QString clientCaption = client->tuningServiceId();
			if (client->singleLmControlMode() == true)
			{
				clientCaption += tr(" (Single LM Control)");
			}
			clientItem->setText(static_cast<int>(Columns::EquipmentId), clientCaption);

			clientItem->setText(static_cast<int>(Columns::Ip), client->currentServerAddressPort().addressStr());
			clientItem->setText(static_cast<int>(Columns::Port), client->currentServerAddressPort().portStr());

			clientItem->setData(columnIndex_Hash, Qt::UserRole, ::calcHash(client->tuningServiceId()));

			// Add child items with client's sources

			std::vector<TuningSource> clientSources = client->tuningSourcesInfo();

			for (const TuningSource& ts : clientSources)
			{
				QStringList connectionStrings;

				connectionStrings << ts.info.moduleequipmentid().c_str();
				connectionStrings << ts.info.lancontrollerinfo()[0].tuningip().c_str();
				connectionStrings << QString::number(ts.info.lancontrollerinfo()[0].tuningport());

				connectionStrings << ts.info.subsystemchannel().c_str();

				connectionStrings << ts.info.subsystemid().c_str();
				connectionStrings << QString::number(ts.info.lmnumber());

				QTreeWidgetItem* sourceItem = new QTreeWidgetItem(connectionStrings);
				clientItem->addChild(sourceItem);

				sourceItem->setData(columnIndex_Hash, Qt::UserRole, ::calcHash(ts.equipmentId()));
				sourceItem->setData(columnIndex_EquipmentId, Qt::UserRole, ts.equipmentId());

				m_tuningClientsSourcesHashes.insert(::calcHash(client->tuningServiceId() + ts.equipmentId()));
			}

			m_treeWidget->addTopLevelItem(clientItem);
			clientItem->setExpanded(true);
		}

		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		m_treeWidget->sortByColumn(static_cast<int>(Columns::EquipmentId), Qt::AscendingOrder);

		m_treeWidget->setColumnWidth(static_cast<int>(Columns::State), 120);

		if (m_hasActivationControls == true)
		{
			// Single control mode controls

			if (m_btnEnableControl == nullptr || m_btnDisableControl == nullptr)
			{
				Q_ASSERT(m_btnEnableControl);
				Q_ASSERT(m_btnDisableControl);
				return;
			}

			m_btnEnableControl->setEnabled(false);
			m_btnDisableControl->setEnabled(false);
		}
	}

	// Refresh
	//

	for (int ci = 0; ci < m_treeWidget->topLevelItemCount(); ci++)
	{
		QTreeWidgetItem* clientItem = m_treeWidget->topLevelItem(ci);
		if (clientItem == nullptr)
		{
			assert(false);
			continue;
		}

		Hash clientHash = clientItem->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

		for (const TuningTcpClient* client : m_tuningTcpClients)
		{
			if (clientHash != ::calcHash(client->tuningServiceId()))
			{
				continue;
			}

			int count = clientItem->childCount();
			for (int si = 0; si < count; si++)
			{
				QTreeWidgetItem* sourceItem = clientItem->child(si);
				if (sourceItem == nullptr)
				{
					assert(false);
					continue;
				}

				Hash sourceHash = sourceItem->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

				TuningSource ts;

				if (client->tuningSourceInfo(sourceHash, &ts) == false)
				{
					continue;
				}

				if (ts.state.controlisactive() == true)
				{
					if (ts.state.isreply() == false)
					{
						sourceItem->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

						sourceItem->setText(static_cast<int>(Columns::State), tr("No Reply"));
					}
					else
					{
						int errorsCount = ts.getErrorsCount();

						if (errorsCount == 0)
						{
							sourceItem->setForeground(static_cast<int>(Columns::State), QBrush(Qt::black));

							sourceItem->setText(static_cast<int>(Columns::State), tr("Active"));
						}
						else
						{
							sourceItem->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

							sourceItem->setText(static_cast<int>(Columns::State), tr("E: %1").arg(errorsCount));
						}
					}
				}
				else
				{
					sourceItem->setText(static_cast<int>(Columns::State), tr("Inactive"));

					sourceItem->setForeground(static_cast<int>(Columns::State), QBrush(Qt::black));
				}

				sourceItem->setText(static_cast<int>(Columns::IsActive), ts.state.controlisactive() ? tr("Yes") : tr("No"));
				sourceItem->setText(static_cast<int>(Columns::HasUnappliedParams), ts.state.hasunappliedparams() ? tr("Yes") : tr("No"));
				sourceItem->setText(static_cast<int>(Columns::RequestCount), QString::number(ts.state.requestcount()));
				sourceItem->setText(static_cast<int>(Columns::ReplyCount), QString::number(ts.state.replycount()));
			}
		}
	}

	if (m_hasActivationControls == true)
	{
		TuningTcpClient* client = selectedClient();

		std::optional<TuningSource> tso = selectedSource();
		{
			m_btnEnableControl->setEnabled(client != nullptr && client->singleLmControlMode() == true && tso.has_value() == true && tso.value().state.controlisactive() == false);
			m_btnDisableControl->setEnabled(client != nullptr && client->singleLmControlMode() == true && tso.has_value() == true && tso.value().state.controlisactive() == true);
		}
	}
}

void TuningSourcesWidget::closeClicked()
{
	emit closeButtonPressed();
}

void TuningSourcesWidget::detailsClicked()
{
	std::optional<TuningSource> tso = selectedSource();
	if (tso.has_value() == false)
	{
		return;
	}

	Hash hash = ::calcHash(tso.value().equipmentId());

	auto it = m_sourceInfoDialogsMap.find(hash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClients, this, hash);
		connect(dlg, &DialogTuningSourceInfo::dialogClosed, this, &TuningSourcesWidget::detailsDialogClosed);
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

void TuningSourcesWidget::treeWidget_itemSelectionChanged()
{
	std::optional<TuningSource> source = selectedSource();

	m_btnDetails->setEnabled(source.has_value() == true);

	// Single control mode controls
	//
	if (m_hasActivationControls == true)
	{
		TuningTcpClient* client = selectedClient();

		{
			m_btnEnableControl->setEnabled(client != nullptr && client->singleLmControlMode() == true && source.has_value() == true && source.value().state.controlisactive() == false);
			m_btnDisableControl->setEnabled(client != nullptr && client->singleLmControlMode() == true && source.has_value() == true && source.value().state.controlisactive() == true);
		}
	}
}

void TuningSourcesWidget::treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	QTimer::singleShot(10, this, &TuningSourcesWidget::detailsClicked);
}

void TuningSourcesWidget::enableControl_clicked()
{
	activateControl(true);
}

void TuningSourcesWidget::disableControl_clicked()
{
	activateControl(false);
}

void TuningSourcesWidget::detailsDialogClosed(Hash hash)
{
	auto it = m_sourceInfoDialogsMap.find(hash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		assert(false);
		return;
	}

	m_sourceInfoDialogsMap.erase(it);

}

void TuningSourcesWidget::activateControl(bool enable)
{
	std::optional<TuningSource> source = selectedSource();

	TuningTcpClient* client = selectedClient();

	if (client == nullptr || source.has_value() == false)
	{
		Q_ASSERT(false);
		return;
	}

	if (login() == false)
	{
		return;
	}

	QString action = enable ? tr("activate") : tr("deactivate");

	bool forceTakeControl = false;

	if (client->singleLmControlMode() == true && client->clientIsActive() == false)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!\n\nCurrent client is not selected as active now.\n\nAre you sure you want to take control and %1 the source %2?")
								 .arg(action)
								 .arg(source.value().equipmentId()),
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
								 tr("Are you sure you want to %1 the source %2?").arg(action).arg(source.value().equipmentId()),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	if (client->activateTuningSourceControl(source.value().equipmentId(), enable, forceTakeControl) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("An error has been occured!"));
	}
}

TuningTcpClient* TuningSourcesWidget::selectedClient() const
{
	QTreeWidgetItem* item = m_treeWidget->currentItem();
	if (item == nullptr)
	{
		return nullptr;
	}

	if (item->parent() != nullptr)
	{
		item = item->parent();
		Q_ASSERT(item->parent() == nullptr);
	}

	Hash clientHash = item->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

	for (TuningTcpClient* client : m_tuningTcpClients)
	{
		if (clientHash == ::calcHash(client->tuningServiceId()))
		{
			return client;
		}
	}

	return nullptr;
}

const std::optional<TuningSource> TuningSourcesWidget::selectedSource() const
{
	TuningTcpClient* client = selectedClient();
	if (client == nullptr)
	{
		return {};
	}

	QTreeWidgetItem* item = m_treeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr)
	{
		return {};
	}

	Hash sourceHash = item->data(columnIndex_Hash, Qt::UserRole).value<Hash>();

	TuningSource ts;

	if (client->tuningSourceInfo(sourceHash, &ts) == true)
	{
		return ts;
	}

	return {};

}

