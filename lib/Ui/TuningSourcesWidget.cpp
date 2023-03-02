#include "TuningSourcesWidget.h"
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "../lib/Tuning/TuningSourcesHelper.h"

#include <QTreeWidget>

DialogTuningSourceInfo::DialogTuningSourceInfo(std::vector<TuningTcpClient*> tcpClients, QWidget* parent, quint64 sourceId, Hash lanEquipmentHash) :
	DialogSourceInfo(parent, lanEquipmentHash /*this is unique identifier, NOT sourceHash!*/),
	m_tcpClients(tcpClients),
	m_sourceHash(sourceId),
	m_lanEquipmentHash(lanEquipmentHash)
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

	m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
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
		const std::vector<TuningSource> tuningSourcesInfo = client->tuningSourcesInfo();

		for (const TuningSource& ts : tuningSourcesInfo)
		{
			for (int i = 0; i < ts.statesCount(); i++)
			{
				Hash hash = ::calcHash(QString::fromStdString(ts.state(i).lanequipmentid()));

				if (hash == m_lanEquipmentHash)
				{
					m_activeTcpClient = client;
					//m_stateIndex = i;
					return true;
				}
			}
		}
	}

	return false;
}

void DialogTuningSourceInfo::updateData()
{
	if (m_activeTcpClient == nullptr)
	{
		// Try to find active client
		//
		if (findActiveTuningTcpClient() == false)
		{
			static const QString noTuningSourceString = tr("Tuning Source - ") + "?";

			if (windowTitle() != noTuningSourceString)
			{
				setWindowTitle(noTuningSourceString);
			}

			return;
		}

		updateInfo();
	}

	if (m_activeTcpClient == nullptr)
	{
		Q_ASSERT(m_activeTcpClient);
		return;
	}

	updateState();
}

void DialogTuningSourceInfo::updateInfo()
{
	TuningSource ts;

	if (m_activeTcpClient->tuningSourceInfo(m_sourceHash, &ts) == false)
	{
		static const QString noTuningSourceString = tr("Tuning Source - ") + "?";

		if (windowTitle() != noTuningSourceString)
		{
			setWindowTitle(noTuningSourceString);
		}
		return;
	}

	// Update Window title

	QString title = tr("Tuning Source - %1").arg(ts.equipmentId());

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

	for (int i = 0; i < ts.controllersCount(); i++)
	{
		if (::calcHash(ts.controllerEquipmentId(i)) == m_lanEquipmentHash)
		{
			setDataItemText("ID", tr("%1 (%2h)").arg(QString::number(info.id())).arg(QString::number(info.id(), 16)));
			setDataItemText("EquipmentID", info.moduleequipmentid().c_str());
			setDataItemText("Caption", info.modulecaption().c_str());
			setDataItemNumber("DataType", info.lancontrollerinfo()[i].lancontrollertype());
			setDataItemText("IP", info.lancontrollerinfo()[i].tuningip().c_str());
			setDataItemNumber("Port", info.lancontrollerinfo()[i].tuningport());
			setDataItemText("Channel", info.subsystemchannel().c_str());
			setDataItemNumber("SubsystemID", info.subsystemkey());
			setDataItemText("Subsystem", info.subsystemid().c_str());

			setDataItemNumber("LmNumber", info.lmnumber());
			setDataItemText("LmModuleType", tr("%1 (%2h)").arg(QString::number(info.moduletype())).arg(QString::number(info.moduletype(), 16)));
			setDataItemText("LmAdapterID", info.lancontrollerinfo()[i].equipmentid().c_str());
			setDataItemNumber("LmDataEnable", info.lancontrollerinfo()[i].tuningenable());
			setDataItemText("LmDataID", tr("%1 (%2h)").
							arg(QString::number(info.lancontrollerinfo()[i].tuningdatauid())).
							arg(QString::number(info.lancontrollerinfo()[i].tuningdatauid(), 16)));

			break;
		}
	}
}

void DialogTuningSourceInfo::updateState()
{
	TuningSource ts;

	if (m_activeTcpClient->tuningSourceInfo(m_sourceHash, &ts) == false)
	{
		return;
	}

	QTreeWidgetItem* item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	const ::Network::TuningSourceState& state = ts.state(m_lanEquipmentHash);
	const ::Network::TuningSourceState& previousState = ts.previousState(m_lanEquipmentHash);

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


TuningSourcesWidget::TuningSourcesWidget(std::vector<TuningTcpClient*> tcpClients, bool hasActivationControls, QWidget* parent) :
	QWidget(parent),
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

	m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
	m_treeWidget->setHeaderLabels(headerLabels);

	setTuningTcpClients(tcpClients);

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

	for (TuningTcpClient* client : m_tuningTcpClients)
	{
		connect(client, &TuningTcpClient::tuningSourcesInfoArrived, this, &TuningSourcesWidget::tuningSourcesInfoArrived);
	}

	fillTuningSourcesInfo();

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

void TuningSourcesWidget::detailsClicked()
{
	quint64 sourceId  = selectedSourceId();
	if (sourceId == UNDEFINED_HASH)
	{
		return;
	}

	Hash lanControllerHash  = selectedLanControllerHash();
	if (lanControllerHash == UNDEFINED_HASH)
	{
		return;
	}

	auto it = m_sourceInfoDialogsMap.find(lanControllerHash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningTcpClients, this, sourceId, lanControllerHash);
		connect(dlg, &DialogTuningSourceInfo::dialogClosed, this, &TuningSourcesWidget::detailsDialogClosed);
		dlg->show();
		dlg->activateWindow();

		m_sourceInfoDialogsMap[lanControllerHash] = dlg;
	}
	else
	{
		DialogTuningSourceInfo* dlg = it->second;
		dlg->activateWindow();

		UiTools::adjustDialogPlacement(dlg);
	}
}

void TuningSourcesWidget::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateAll();
	}
}

bool TuningSourcesWidget::login()
{
	return true;
}

void TuningSourcesWidget::tuningSourcesInfoArrived()
{
	m_tuningSourcesInfoArrived = true;
}

void TuningSourcesWidget::updateAll()
{
	if (m_tuningSourcesInfoArrived == true)
	{
		m_tuningSourcesInfoArrived = false;
		fillTuningSourcesInfo();
	}

	updateTuningSourcesStates();

	enableActivationControls();
}

void TuningSourcesWidget::treeWidget_itemSelectionChanged()
{
	enableActivationControls();
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

void TuningSourcesWidget::detailsDialogClosed(Hash lanControllerHash)
{
	auto it = m_sourceInfoDialogsMap.find(lanControllerHash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		assert(false);
		return;
	}

	m_sourceInfoDialogsMap.erase(it);
}

void TuningSourcesWidget::fillTuningSourcesInfo()
{
	m_treeWidget->clear();
	m_sourceIdToSourceItemMap.clear();
	m_controllerHashToControllerItemMap.clear();

	for (const TuningTcpClient* client : m_tuningTcpClients)
	{
		std::vector<TuningSource> sources = client->tuningSourcesInfo();

		for (const TuningSource& ts: sources)
		{
			// Create and fill source item
			//

			quint64 sourceId = ::calcHash(ts.equipmentId());

			QTreeWidgetItem* sourceItem = nullptr;

			auto sit = m_sourceIdToSourceItemMap.find(sourceId);
			if (sit == m_sourceIdToSourceItemMap.end())
			{
				sourceItem = new QTreeWidgetItem();
				sourceItem->setText(0, ts.equipmentId());
				sourceItem->setData(columnIndex_SourceId, Qt::UserRole, sourceId);
				m_sourceIdToSourceItemMap[sourceId] = sourceItem;
				m_treeWidget->addTopLevelItem(sourceItem);
			}
			else
			{
				sourceItem = sit->second;
			}

			if (sourceItem == nullptr)
			{
				Q_ASSERT(false);
				return;
			}

			// Create and fill controllers info items
			//

			const ::Network::DataSourceInfo& info = ts.info();

			for (int c = 0; c < ts.controllersCount(); c++)
			{
				Hash controllerHash = ::calcHash(ts.controllerEquipmentId(c));

				QTreeWidgetItem* controllerItem = nullptr;

				auto cit = m_controllerHashToControllerItemMap.find(controllerHash);
				if (cit == m_controllerHashToControllerItemMap.end())
				{
					QStringList connectionStrings;

					QString lanEquipmentId = ts.controllerEquipmentId(c);

					connectionStrings << lanEquipmentId;
					connectionStrings << info.lancontrollerinfo()[c].tuningip().c_str();
					connectionStrings << QString::number(info.lancontrollerinfo()[c].tuningport());
					connectionStrings << info.subsystemchannel().c_str();
					connectionStrings << info.subsystemid().c_str();
					connectionStrings << QString::number(info.lmnumber());

					controllerItem = new QTreeWidgetItem(connectionStrings);
					controllerItem->setData(columnIndex_ControllerHash, Qt::UserRole, controllerHash);
					m_controllerHashToControllerItemMap[controllerHash] = controllerItem;
					sourceItem->addChild(controllerItem);
				}
			}

			sourceItem->setExpanded(true);
		}
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

void TuningSourcesWidget::updateTuningSourcesStates()
{
	for (const TuningTcpClient* client : m_tuningTcpClients)
	{
		std::vector<TuningSource> sources = client->tuningSourcesInfo();

		for (const TuningSource& ts: sources)
		{
			for (int i = 0; i < ts.statesCount(); i++)
			{
				const Network::TuningSourceState& state = ts.state(i);

				Hash controllerHash = ::calcHash(QString::fromStdString(state.lanequipmentid()));

				int here_asserts = 1;
				auto it = m_controllerHashToControllerItemMap.find(controllerHash);
				if (it == m_controllerHashToControllerItemMap.end())
				{
					Q_ASSERT(false);
					continue;
				}

				QTreeWidgetItem* controllerItem = it->second;

				if (ts.valid() == false)
				{
					controllerItem->setText(static_cast<int>(Columns::State), QString());
					controllerItem->setText(static_cast<int>(Columns::IsActive), QString());
					controllerItem->setText(static_cast<int>(Columns::HasUnappliedParams), QString());
					controllerItem->setText(static_cast<int>(Columns::RequestCount), QString());
					controllerItem->setText(static_cast<int>(Columns::ReplyCount), QString());
					continue;
				}

				if (state.controlisactive() == true)
				{
					if (state.isreply() == false)
					{
						controllerItem->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

						controllerItem->setText(static_cast<int>(Columns::State), tr("No Reply"));
					}
					else
					{
						int errorsCount = ts.getErrorsCount(i);

						if (errorsCount == 0)
						{
							controllerItem->setForeground(static_cast<int>(Columns::State), QBrush(Qt::black));

							controllerItem->setText(static_cast<int>(Columns::State), tr("Active"));
						}
						else
						{
							controllerItem->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

							controllerItem->setText(static_cast<int>(Columns::State), tr("E: %1").arg(errorsCount));
						}
					}
				}
				else
				{
					controllerItem->setText(static_cast<int>(Columns::State), tr("Inactive"));

					controllerItem->setForeground(static_cast<int>(Columns::State), QBrush(Qt::black));
				}

				controllerItem->setText(static_cast<int>(Columns::IsActive), state.controlisactive() ? tr("Yes") : tr("No"));
				controllerItem->setText(static_cast<int>(Columns::HasUnappliedParams), state.hasunappliedparams() ? tr("Yes") : tr("No"));
				controllerItem->setText(static_cast<int>(Columns::RequestCount), QString::number(state.requestcount()));
				controllerItem->setText(static_cast<int>(Columns::ReplyCount), QString::number(state.replycount()));
			}
		}
	}

	return;
}

void TuningSourcesWidget::enableActivationControls()
{
	if (m_hasActivationControls == false)
	{
		return;
	}

	quint64 sourceId  = selectedSourceId();
	if (sourceId == UNDEFINED_HASH)
	{
		return;
	}

	QTreeWidgetItem* item = m_sourceIdToSourceItemMap.at(sourceId);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	QString sourceEquipmentId = item->text(static_cast<int>(Columns::EquipmentId));

	bool buttonEnableEnabled = false;
	bool buttonDisableEnabled = false;

	TuningSourcesHelper::isActivationActionsAvailable(m_tuningTcpClients, sourceEquipmentId, &buttonEnableEnabled, &buttonDisableEnabled);

	m_btnEnableControl->setEnabled(buttonEnableEnabled);
	m_btnDisableControl->setEnabled(buttonDisableEnabled);

	return;
}

void TuningSourcesWidget::activateControl(bool enable)
{
	quint64 sourceId  = selectedSourceId();
	if (sourceId == UNDEFINED_HASH)
	{
		return;
	}

	QTreeWidgetItem* item = m_sourceIdToSourceItemMap.at(sourceId);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	QString sourceEquipmentId = item->text(static_cast<int>(Columns::EquipmentId));

	if (login() == false)
	{
		return;
	}

	TuningSourcesHelper::activateTuningSourceControl(m_tuningTcpClients, sourceEquipmentId, enable, this);
}

quint64 TuningSourcesWidget::selectedSourceId() const
{
	auto sel = m_treeWidget->selectedItems();
	if (sel.size() != 1)
	{
		return UNDEFINED_HASH;
	}

	QTreeWidgetItem* item = sel[0];

	if (item != nullptr)
	{
		if (item->parent() != nullptr)
		{
			// LAN controller is selected, parent item is source item
			//
			item = item->parent();
		}

		auto findResult = std::find_if(std::begin(m_sourceIdToSourceItemMap),
									   std::end(m_sourceIdToSourceItemMap),
									   [item](const std::pair<quint64, QTreeWidgetItem*> &pair)
		{
			return pair.second == item;
		});

		if (findResult != std::end(m_sourceIdToSourceItemMap))
		{
			return findResult->first;
		}
	}

	return UNDEFINED_HASH;
}

Hash TuningSourcesWidget::selectedLanControllerHash() const
{
	auto sel = m_treeWidget->selectedItems();
	if (sel.size() != 1)
	{
		return UNDEFINED_HASH;
	}

	QTreeWidgetItem* item = sel[0];

	if (item != nullptr && item->parent() != nullptr)
	{
		auto findResult = std::find_if(std::begin(m_controllerHashToControllerItemMap),
									   std::end(m_controllerHashToControllerItemMap),
									   [item](const std::pair<Hash, QTreeWidgetItem*> &pair)
		{
			return pair.second == item;
		});

		if (findResult != std::end(m_controllerHashToControllerItemMap))
		{
			return findResult->first;
		}
	}

	return UNDEFINED_HASH;
}
