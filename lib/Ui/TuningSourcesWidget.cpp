#include "TuningSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../UtilsLib/Ui/UiTools.h"

#include <QTreeWidget>

DialogTuningSourceInfo::DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, Hash sourceHash, int channel) :
	DialogSourceInfo(parent, sourceHash + channel /*This is an unique dialog indentifier, NOT sourceHash!*/),
	m_tcpClients(tcpClients),
	m_tuningSourceHash(sourceHash),
	m_channel(channel)
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

	createDataItem(stateItem, "LanEquipmentID");
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

	DialogTuningSourceInfo::updateData();

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

		if (std::find(hashes.begin(), hashes.end(), m_tuningSourceHash) != hashes.end())
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

	if (m_activeTcpClient->tuningSourceInfo(m_tuningSourceHash, &ts) == false)
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

	QString title;

	if (ts.statesChannelsCount() == 1)
	{
		title = tr("Tuning Source - %1").arg(m_sourceEquipmentId);
	}
	else
	{
		title = tr("Tuning Source - %1, Channel %2").arg(m_sourceEquipmentId).arg(m_channel);
	}

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

	const ::Network::DataSourceInfo& info = ts.info();

	if (m_channel < 0 || m_channel >= ts.controllersCount())
	{
		Q_ASSERT(false);
		return;
	}

	setDataItemText("ID", tr("%1 (%2h)").arg(QString::number(info.id())).arg(QString::number(info.id(), 16)));
	setDataItemText("EquipmentID", info.moduleequipmentid().c_str());
	setDataItemText("Caption", info.modulecaption().c_str());
	setDataItemNumber("DataType", info.lancontrollerinfo()[m_channel].lancontrollertype());
	setDataItemText("IP", info.lancontrollerinfo()[m_channel].tuningip().c_str());
	setDataItemNumber("Port", info.lancontrollerinfo()[m_channel].tuningport());
	setDataItemText("Channel", info.subsystemchannel().c_str());
	setDataItemNumber("SubsystemID", info.subsystemkey());
	setDataItemText("Subsystem", info.subsystemid().c_str());

	setDataItemNumber("LmNumber", info.lmnumber());
	setDataItemText("LmModuleType", tr("%1 (%2h)").arg(QString::number(info.moduletype())).arg(QString::number(info.moduletype(), 16)));
	setDataItemText("LmAdapterID", info.lancontrollerinfo()[m_channel].equipmentid().c_str());
	setDataItemNumber("LmDataEnable", info.lancontrollerinfo()[m_channel].tuningenable());
	setDataItemText("LmDataID", tr("%1 (%2h)").
					arg(QString::number(info.lancontrollerinfo()[m_channel].tuningdatauid())).
			arg(QString::number(info.lancontrollerinfo()[m_channel].tuningdatauid(), 16)));

	// state

	if (m_channel >= ts.statesChannelsCount())
	{
		// No state is received yet
		return;
	}

	item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	const ::Network::TuningSourceState& state = ts.state(m_channel);
	const ::Network::TuningSourceState& previousState = ts.previousState(m_channel);

	item->setData(0, Qt::UserRole, 0);

	setDataItemText("LanEquipmentID", state.lanequipmentid().c_str());
	setDataItemText("IsReply", state.isreply() ? "Yes" : "No");

	{
		QTreeWidgetItem* isReplyItem = dataItem("IsReply");
		if (isReplyItem == nullptr)
		{
			assert(isReplyItem);
			return;
		}

		if (state.isreply() == false)
		{
			isReplyItem->setForeground(1, QBrush(DialogSourceInfo::dataItemErrorColor));
		}
		else
		{
			isReplyItem->setForeground(1, QBrush(Qt::black));
		}
	}

	setDataItemNumber("RequestCount", state.requestcount());
	setDataItemNumber("ReplyCount", state.replycount());
	setDataItemNumber("CommandQueueSize", state.commandqueuesize());
	setDataItemText("ControlIsActive", state.controlisactive() ? "Yes" : "No");
	setDataItemText("SetSOR", state.setsor() ? "Yes" : "No");
	setDataItemText("WritingDisabled", state.writingdisabled() ? "Yes" : "No");

	setDataItemNumber("ErrUntimelyReplay", state.erruntimelyreplay());
	setDataItemNumber("ErrSent", state.errsent());
	setDataItemNumber("ErrPartialSent", state.errpartialsent());
	setDataItemNumber("ErrReplySize", state.errreplysize());
	setDataItemNumberCompare(item, "ErrNoReply", state.errnoreply(), previousState.errnoreply());
	setDataItemNumber("ErrAnalogLowBoundCheck", state.erranaloglowboundcheck());
	setDataItemNumber("ErrAnalogHighBoundCheck", state.erranaloghighboundcheck());

	updateParentItemState(item);

	// RupFrameHeader

	item = m_treeWidget->topLevelItem(2);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumberCompare(item, "ErrRupProtocolVersion", state.errrupprotocolversion(), previousState.errrupprotocolversion());
	setDataItemNumberCompare(item, "ErrRupFrameSize", state.errrupframesize(), previousState.errrupframesize());
	setDataItemNumberCompare(item, "ErrRupNoTuningData", state.errrupnontuningdata(), previousState.errrupnontuningdata());
	setDataItemNumberCompare(item, "ErrRupModuleType", state.errrupmoduletype(), previousState.errrupmoduletype());
	setDataItemNumberCompare(item, "ErrRupFramesQuantity", state.errrupframesquantity(), previousState.errrupframesquantity());
	setDataItemNumberCompare(item, "ErrRupFrameNumber", state.errrupframenumber(), previousState.errrupframenumber());
	setDataItemNumberCompare(item, "ErrRupCRC", state.errrupcrc(), previousState.errrupcrc());

	updateParentItemState(item);

	// FotipHeader

	item = m_treeWidget->topLevelItem(3);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumberCompare(item, "ErrFotipProtocolVersion", state.errfotipprotocolversion(), previousState.errfotipprotocolversion());
	setDataItemNumberCompare(item, "ErrFotipUniqueID", state.errfotipuniqueid(), previousState.errfotipuniqueid());
	setDataItemNumberCompare(item, "ErrFotipLmNumber", state.errfotiplmnumber(), previousState.errfotiplmnumber());
	setDataItemNumberCompare(item, "ErrFotipSubsystemCode", state.errfotipsubsystemcode(), previousState.errfotipsubsystemcode());

	setDataItemNumberCompare(item, "ErrFotipOperationCode", state.errfotipoperationcode(), previousState.errfotipoperationcode());
	setDataItemNumberCompare(item, "ErrFotipFrameSize", state.errfotipframesize(), previousState.errfotipframesize());
	setDataItemNumberCompare(item, "ErrFotipRomSize", state.errfotipromsize(), previousState.errfotipromsize());
	setDataItemNumberCompare(item, "ErrFotipRomFrameSize", state.errfotipromframesize(), previousState.errfotipromframesize());

	updateParentItemState(item);

	// FotipFlags

	item = m_treeWidget->topLevelItem(4);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumber("FotipFlagBoundsCheckSuccess", state.fotipflagboundschecksuccess());
	setDataItemNumber("FotipFlagWriteSuccess", state.fotipflagwritesuccess());
	setDataItemNumberCompare(item, "FotipFlagDataTypeErr", state.fotipflagdatatypeerr(), previousState.fotipflagdatatypeerr());
	setDataItemNumberCompare(item, "FotipFlagOpCodeErr", state.fotipflagopcodeerr(), previousState.fotipflagopcodeerr());

	setDataItemNumberCompare(item, "FotipFlagStartAddrErr", state.fotipflagstartaddrerr(), previousState.fotipflagstartaddrerr());
	setDataItemNumberCompare(item, "FotipFlagRomSizeErr", state.fotipflagromsizeerr(), previousState.fotipflagromsizeerr());
	setDataItemNumberCompare(item, "FotipFlagRomFrameSizeErr", state.fotipflagromframesizeerr(), previousState.fotipflagromframesizeerr());
	setDataItemNumberCompare(item, "FotipFlagFrameSizeErr", state.fotipflagframesizeerr(), previousState.fotipflagframesizeerr());

	setDataItemNumberCompare(item, "FotipFlagProtocolVersionErr", state.fotipflagprotocolversionerr(), previousState.fotipflagprotocolversionerr());
	setDataItemNumberCompare(item, "FotipFlagSubsystemKeyErr", state.fotipflagsubsystemkeyerr(), previousState.fotipflagsubsystemkeyerr());
	setDataItemNumberCompare(item, "FotipFlagUniueIDErr", state.fotipflaguniueiderr(), previousState.fotipflaguniueiderr());
	setDataItemNumberCompare(item, "FotipFlagOffsetErr", state.fotipflagoffseterr(), previousState.fotipflagoffseterr());

	setDataItemNumber("FotipFlagApplySuccess", state.fotipflagapplysuccess());
	setDataItemNumber("FotipFlagSetSOR", state.fotipflagsetsor());
	setDataItemNumber("FotipFlagWritingDisabled", state.fotipflagwritingdisabled());

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
			for (int c = 0; c < ts.controllersCount(); c++)
			{
				sourcesCount++;

				Hash hash = ::calcHash(client->tuningServiceId() + ts.controllerEquipmentId(c));

				if (m_tuningClientsLansHashes.find(hash) == m_tuningClientsLansHashes.end())
				{
					return true;	// Unknown source has been occured
				}
			}
		}
	}

	if (sourcesCount != m_tuningClientsLansHashes.size())
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

		m_tuningClientsLansHashes.clear();

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

			clientItem->setData(columnIndex_ClientHash, Qt::UserRole, ::calcHash(client->tuningServiceId()));

			// Add child items with client's sources

			std::vector<TuningSource> clientSources = client->tuningSourcesInfo();

			for (const TuningSource& ts : clientSources)
			{
				const ::Network::DataSourceInfo& info = ts.info();

				for (int i = 0; i < ts.controllersCount(); i++)
				{
					QStringList connectionStrings;

					QString lanEquipmentId = ts.controllerEquipmentId(i);

					if (ts.statesChannelsCount() == 1)
					{
						connectionStrings << info.moduleequipmentid().c_str();
					}
					else
					{
						connectionStrings << lanEquipmentId;
					}

					connectionStrings << info.lancontrollerinfo()[i].tuningip().c_str();
					connectionStrings << QString::number(info.lancontrollerinfo()[i].tuningport());

					connectionStrings << info.subsystemchannel().c_str();

					connectionStrings << info.subsystemid().c_str();
					connectionStrings << QString::number(info.lmnumber());

					QTreeWidgetItem* sourceItem = new QTreeWidgetItem(connectionStrings);
					clientItem->addChild(sourceItem);

					sourceItem->setData(columnIndex_SourceEquipmentIdHash, Qt::UserRole, ::calcHash(ts.equipmentId()));	// Source Index
					sourceItem->setData(columnIndex_SourceChannel, Qt::UserRole, i);	// LAN Controller index

					// Count full hash to control dynamic changes
					//
					Hash hash = ::calcHash(client->tuningServiceId() + lanEquipmentId);
					Q_ASSERT (m_tuningClientsLansHashes.find(hash) == m_tuningClientsLansHashes.end());
					m_tuningClientsLansHashes.insert(hash);
				}
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

		Hash clientHash = clientItem->data(columnIndex_ClientHash, Qt::UserRole).value<Hash>();

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

				Hash sourceHash = sourceItem->data(columnIndex_SourceEquipmentIdHash, Qt::UserRole).value<Hash>();

				TuningSource ts;

				if (client->tuningSourceInfo(sourceHash, &ts) == false)
				{
					sourceItem->setText(static_cast<int>(Columns::State), "???");
					sourceItem->setText(static_cast<int>(Columns::IsActive), "???");
					sourceItem->setText(static_cast<int>(Columns::HasUnappliedParams), "???");
					sourceItem->setText(static_cast<int>(Columns::RequestCount), "???");
					sourceItem->setText(static_cast<int>(Columns::ReplyCount), "???");
					continue;
				}

				int sourceChannel = sourceItem->data(columnIndex_SourceChannel, Qt::UserRole).value<int>();

				if (sourceChannel >= ts.statesChannelsCount())
				{
					// No state received for this channel
					sourceItem->setText(static_cast<int>(Columns::State), "???");
					sourceItem->setText(static_cast<int>(Columns::IsActive), "???");
					sourceItem->setText(static_cast<int>(Columns::HasUnappliedParams), "???");
					sourceItem->setText(static_cast<int>(Columns::RequestCount), "???");
					sourceItem->setText(static_cast<int>(Columns::ReplyCount), "???");
					continue;
				}

				const ::Network::TuningSourceState& state = ts.state(sourceChannel);

				if (state.controlisactive() == true)
				{
					if (state.isreply() == false)
					{
						sourceItem->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

						sourceItem->setText(static_cast<int>(Columns::State), tr("No Reply"));
					}
					else
					{
						int errorsCount = ts.getErrorsCount(sourceChannel);

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

				sourceItem->setText(static_cast<int>(Columns::IsActive), state.controlisactive() ? tr("Yes") : tr("No"));
				sourceItem->setText(static_cast<int>(Columns::HasUnappliedParams), state.hasunappliedparams() ? tr("Yes") : tr("No"));
				sourceItem->setText(static_cast<int>(Columns::RequestCount), QString::number(state.requestcount()));
				sourceItem->setText(static_cast<int>(Columns::ReplyCount), QString::number(state.replycount()));
			}
		}
	}


	if (m_hasActivationControls == true)
	{
		TuningTcpClient* client = selectedClient();
		Hash sourceHash  = selectedSourceHash();
		int sourceChannel  = selectedSourceChannel();

		bool buttonEnableEnabled = false;
		bool buttonDisableEnabled = false;

		if (client != nullptr && client->singleLmControlMode() == true && sourceHash != UNDEFINED_HASH && sourceChannel != -1)
		{
			TuningSource ts;
			if (client->tuningSourceInfo(sourceHash, &ts) == true)
			{
				const ::Network::TuningSourceState& state = ts.state(sourceChannel);

				buttonEnableEnabled = state.controlisactive() == false;
				buttonDisableEnabled = state.controlisactive() == true;
			}
		}

		m_btnEnableControl->setEnabled(buttonEnableEnabled);
		m_btnDisableControl->setEnabled(buttonDisableEnabled);
	}
}

void TuningSourcesWidget::closeClicked()
{
	emit closeButtonPressed();
}

void TuningSourcesWidget::detailsClicked()
{
	Hash sourceHash  = selectedSourceHash();
	int sourceChannel  = selectedSourceChannel();

	if (sourceHash == UNDEFINED_HASH || sourceChannel == -1)
	{
		return;
	}

	Hash uniqueHash = sourceHash + sourceChannel;

	auto it = m_sourceInfoDialogsMap.find(uniqueHash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClients, this, sourceHash, sourceChannel);
		connect(dlg, &DialogTuningSourceInfo::dialogClosed, this, &TuningSourcesWidget::detailsDialogClosed);
		dlg->show();
		dlg->activateWindow();

		m_sourceInfoDialogsMap[uniqueHash] = dlg;
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
	TuningTcpClient* client = selectedClient();
	Hash sourceHash  = selectedSourceHash();

	m_btnDetails->setEnabled(sourceHash != UNDEFINED_HASH);

	if (m_hasActivationControls == true)
	{
		int sourceChannel = selectedSourceChannel();

		bool buttonEnableEnabled = false;
		bool buttonDisableEnabled = false;

		if (client != nullptr && client->singleLmControlMode() == true && sourceHash != UNDEFINED_HASH && sourceChannel != -1)
		{
			TuningSource ts;
			if (client->tuningSourceInfo(sourceHash, &ts) == true)
			{
				const ::Network::TuningSourceState& state = ts.state(sourceChannel);

				buttonEnableEnabled = state.controlisactive() == false;
				buttonDisableEnabled = state.controlisactive() == true;
			}
		}

		m_btnEnableControl->setEnabled(buttonEnableEnabled);
		m_btnDisableControl->setEnabled(buttonDisableEnabled);
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
	TuningTcpClient* client = selectedClient();

	Hash sourceHash = selectedSourceHash();

	if (client == nullptr || sourceHash == UNDEFINED_HASH)
	{
		Q_ASSERT(false);
		return;
	}

	if (login() == false)
	{
		return;
	}

	TuningSource ts;
	if (client->tuningSourceInfo(sourceHash, &ts) == false)
	{
		Q_ASSERT(false);
		return;
	}

	QString action = enable ? tr("activate") : tr("deactivate");

	bool forceTakeControl = false;

	if (client->singleLmControlMode() == true && client->clientIsActive() == false)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!\n\nCurrent client is not selected as active now.\n\nAre you sure you want to take control and %1 the source %2?")
								 .arg(action)
								 .arg(ts.equipmentId()),
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
								 tr("Are you sure you want to %1 the source %2?").arg(action).arg(ts.equipmentId()),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	if (client->activateTuningSourceControl(ts.equipmentId(), enable, forceTakeControl) == false)
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

	Hash clientHash = item->data(columnIndex_ClientHash, Qt::UserRole).value<Hash>();

	for (TuningTcpClient* client : m_tuningTcpClients)
	{
		if (clientHash == ::calcHash(client->tuningServiceId()))
		{
			return client;
		}
	}

	return nullptr;
}

const Hash TuningSourcesWidget::selectedSourceHash() const
{
	TuningTcpClient* client = selectedClient();
	if (client == nullptr)
	{
		return UNDEFINED_HASH;
	}

	QTreeWidgetItem* item = m_treeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr)
	{
		return UNDEFINED_HASH;
	}

	Hash sourceHash = item->data(columnIndex_SourceEquipmentIdHash, Qt::UserRole).value<Hash>();
	return sourceHash;
}

const int TuningSourcesWidget::selectedSourceChannel() const
{
	TuningTcpClient* client = selectedClient();
	if (client == nullptr)
	{
		return -1;
	}

	QTreeWidgetItem* item = m_treeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr)
	{
		return -1;
	}

	int sourceChannel = item->data(columnIndex_SourceChannel, Qt::UserRole).value<int>();
	return sourceChannel;
}

