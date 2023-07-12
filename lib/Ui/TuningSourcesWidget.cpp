#include "TuningSourcesWidget.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../UtilsLib/Ui/UiTools.h"

#include <QTreeWidget>

DialogTuningSourceInfo::DialogTuningSourceInfo(ClientLib::TuningConnection& connection, QWidget* parent, quint64 sourceId, Hash lanEquipmentHash) :
	DialogSourceInfo(parent, lanEquipmentHash /*this is unique identifier, NOT sourceHash!*/),
	m_tuningConnection(connection),
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

	createDataItem(stateItem, "LmTime");
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

void DialogTuningSourceInfo::updateData()
{
	std::vector<ClientLib::TuningSource> tss = m_tuningConnection.tuningSourceInfo(m_sourceHash);

	if (tss.empty() == true)
	{
		static const QString noTuningSourceString = tr("Tuning Source - ") + "?";

		if (windowTitle() != noTuningSourceString)
		{
			setWindowTitle(noTuningSourceString);
		}
		return;
	}

	for (const ClientLib::TuningSource& ts : tss)
	{
		for (int i = 0; i < ts.controllersCount(); i++)
		{
			if (::calcHash(ts.controllerEquipmentId(i)) == m_lanEquipmentHash)
			{
				// Update Window title

				QString title = tr("Tuning Source - %1").arg(ts.equipmentId());

				if (windowTitle() != title)
				{
					setWindowTitle(title);
				}

				// Update information

				updateInfo(ts);

				updateState(ts);
			}
		}
	}
}

void DialogTuningSourceInfo::updateInfo(const ClientLib::TuningSource& ts)
{
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

void DialogTuningSourceInfo::updateState(const ClientLib::TuningSource& ts)
{
	QTreeWidgetItem* item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	const ::Network::TuningSourceState& state = ts.state(m_lanEquipmentHash);
	const ::Network::TuningSourceState& previousState = ts.previousState(m_lanEquipmentHash);

	item->setData(0, Qt::UserRole, 0);

	QDateTime tm;

	tm.setTimeSpec(Qt::UTC);

	tm.setMSecsSinceEpoch(state.lmtime());
	setDataItemText("LmTime", tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

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


TuningSourcesWidget::TuningSourcesWidget(ClientLib::TuningConnection& tuningConnection, bool hasActivationControls, QWidget* parent) :
	QWidget(parent),
	m_hasActivationControls(hasActivationControls),
	m_tuningConnection(tuningConnection),
	m_parent(parent)
{
	setWindowTitle(tr("Tuning Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &TuningSourcesWidget::treeWidgetItemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &TuningSourcesWidget::treeWidgetItemSelectionChanged);

	m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &TuningSourcesWidget::contextMenuRequested);

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
	headerLabels << tr("LmTime");
	headerLabels << tr("IsActive");
	headerLabels << tr("HasUnapplied");
	headerLabels << tr("RequestCount");
	headerLabels << tr("ReplyCount");

	m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
	m_treeWidget->setHeaderLabels(headerLabels);

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);// sort by EquipmentID

	m_updateStateTimerId = startTimer(250);
}

TuningSourcesWidget::~TuningSourcesWidget()
{
}

bool TuningSourcesWidget::treeIsFocused() const
{
	return m_treeWidget->hasFocus();
}

void TuningSourcesWidget::detailsClicked()
{
	auto sel = m_treeWidget->selectedItems();
	if (sel.size() != 1)
	{
		return;
	}

	Hash sourceHash = sel[0]->data(columnIndex_SourceHash, Qt::UserRole).toULongLong();

	Hash lanControllerHash = sel[0]->data(columnIndex_ControllerHash, Qt::UserRole).toULongLong();

	auto it = m_sourceInfoDialogsMap.find(lanControllerHash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		DialogTuningSourceInfo* dlg = new DialogTuningSourceInfo(m_tuningConnection, this, sourceHash, lanControllerHash);
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

void TuningSourcesWidget::enableControlClicked()
{
	activateControl(true);
}

void TuningSourcesWidget::disableControlClicked()
{
	activateControl(false);
}

void TuningSourcesWidget::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateData();
	}
}

bool TuningSourcesWidget::login()
{
	return true;
}

void TuningSourcesWidget::contextMenuRequested()
{
	if (m_treeWidget->selectedItems().empty() == true)
	{
		return;
	}

	QMenu menu(this);

	QAction action(tr("Details..."));
	connect(&action, &QAction::triggered, this, [this](){
		detailsClicked();
	});

	QAction actionActivate(tr("Activate..."));
	actionActivate.setEnabled(m_buttonActivateEnabled);
	connect(&actionActivate, &QAction::triggered, this, [this](){
		enableControlClicked();
	});

	QAction actionDeactivate(tr("Deactivate..."));
	actionDeactivate.setEnabled(m_buttonDeactivateEnabled);
	connect(&actionDeactivate, &QAction::triggered, this, [this](){
		disableControlClicked();
	});

	menu.addAction(&action);
	if (m_hasActivationControls == true)
	{
		menu.addSeparator();
		menu.addAction(&actionActivate);
		menu.addAction(&actionDeactivate);

	}

	menu.exec(this->cursor().pos());

	return;
}

void TuningSourcesWidget::updateData()
{
	updateTuningSourcesStates();

	if (m_hasActivationControls == true)
	{
		enableActivationControls();
	}
}

void TuningSourcesWidget::treeWidgetItemSelectionChanged()
{
	if (m_hasActivationControls == true)
	{
		enableActivationControls();
	}
}

void TuningSourcesWidget::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	QTimer::singleShot(10, this, &TuningSourcesWidget::detailsClicked);
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

void TuningSourcesWidget::updateTuningSourcesStates()
{
	bool newItemsCreated = false;

	int controllersCount = 0;

	std::vector<ClientLib::TuningSource> sources = m_tuningConnection.tuningSourcesInfo();

	for (const ClientLib::TuningSource& ts: sources)
	{
		const ::Network::DataSourceInfo& info = ts.info();

		controllersCount += info.lancontrollerinfo_size();

		// Find items which contain every LAN controller. If such item does not exist - create it
		//
		for (int i = 0; i < info.lancontrollerinfo_size(); i++)
		{
			QString lanEquipmentId = QString::fromStdString(info.lancontrollerinfo()[i].equipmentid());
			Hash controllerHash = ::calcHash(lanEquipmentId);

			QTreeWidgetItem* controllerItem = nullptr;
			for (int h = 0; h < m_treeWidget->topLevelItemCount(); h++)
			{
				QTreeWidgetItem* item = m_treeWidget->topLevelItem((h));
				if (item->data(columnIndex_ControllerHash, Qt::UserRole).toULongLong() == controllerHash)
				{
					controllerItem = item;
					break;
				}
			}

			if (controllerItem != nullptr)
			{
				continue;	// Item for controller already exists
			}

			// Controller item does not exist - create and fill
			//
			QStringList connectionStrings;
			connectionStrings << lanEquipmentId;
			connectionStrings << info.lancontrollerinfo()[i].tuningip().c_str();
			connectionStrings << QString::number(info.lancontrollerinfo()[i].tuningport());
			connectionStrings << info.subsystemchannel().c_str();
			connectionStrings << info.subsystemid().c_str();
			connectionStrings << QString::number(info.lmnumber());

			controllerItem = new QTreeWidgetItem(connectionStrings);
			controllerItem->setData(columnIndex_SourceHash, Qt::UserRole, ::calcHash(ts.equipmentId()));
			controllerItem->setData(columnIndex_SourceEquipmentId, Qt::UserRole, ts.equipmentId());
			controllerItem->setData(columnIndex_ControllerHash, Qt::UserRole, ::calcHash(lanEquipmentId));
			m_treeWidget->addTopLevelItem(controllerItem);

			newItemsCreated = true;
		}

		// Find an item for each state and update it

		for (int i = 0; i < ts.statesCount(); i++)
		{
			const Network::TuningSourceState& state = ts.state(i);

			Hash controllerHash = ::calcHash(QString::fromStdString(state.lanequipmentid()));

			QTreeWidgetItem* controllerItem = nullptr;
			for (int h = 0; h < m_treeWidget->topLevelItemCount(); h++)
			{
				QTreeWidgetItem* item = m_treeWidget->topLevelItem((h));
				if (item->data(columnIndex_ControllerHash, Qt::UserRole).toULongLong() == controllerHash)
				{
					controllerItem = item;
					break;
				}
			}

			if (controllerItem == nullptr)
			{
				Q_ASSERT(controllerItem);
				continue;
			}

			if (ts.valid() == false)
			{
				controllerItem->setText(static_cast<int>(Columns::State), QString());
				controllerItem->setText(static_cast<int>(Columns::LmTime), QString());
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

			QDateTime tm;
			tm.setTimeSpec(Qt::UTC);

			tm.setMSecsSinceEpoch(state.lmtime());
			controllerItem->setText(static_cast<int>(Columns::LmTime), tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

			controllerItem->setText(static_cast<int>(Columns::IsActive), state.controlisactive() ? tr("Yes") : tr("No"));
			controllerItem->setText(static_cast<int>(Columns::HasUnappliedParams), state.hasunappliedparams() ? tr("Yes") : tr("No"));
			controllerItem->setText(static_cast<int>(Columns::RequestCount), QString::number(state.requestcount()));
			controllerItem->setText(static_cast<int>(Columns::ReplyCount), QString::number(state.replycount()));
		}
	}

	// If items were created - adjust their width
	//
	if (newItemsCreated == true)
	{
		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		m_treeWidget->sortByColumn(static_cast<int>(Columns::EquipmentId), Qt::AscendingOrder);

		m_treeWidget->setColumnWidth(static_cast<int>(Columns::State), 120);
	}

	// If items count is more than controllers count - delete all and re-create them on next step
	//
	if (m_treeWidget->topLevelItemCount() > controllersCount)
	{
		m_treeWidget->clear();
	}

	return;
}

void TuningSourcesWidget::enableActivationControls()
{
	if (m_hasActivationControls == false)
	{
		Q_ASSERT(false);
		return;
	}

	bool buttonActivateEnabled = false;
	bool buttonDeactivateEnabled = false;

	auto sel = m_treeWidget->selectedItems();
	if (sel.size() == 1)
	{
		Hash sourceHash =  sel[0]->data(columnIndex_SourceHash, Qt::UserRole).toULongLong();

		int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(sourceHash);
		int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(sourceHash);
		buttonActivateEnabled = activeStatesCount < sourceStatesCount;
		buttonDeactivateEnabled = activeStatesCount != 0 && activeStatesCount == sourceStatesCount;
	}

	if (buttonActivateEnabled != m_buttonActivateEnabled ||	buttonDeactivateEnabled != m_buttonDeactivateEnabled)
	{
		m_buttonActivateEnabled = buttonActivateEnabled;
		m_buttonDeactivateEnabled = buttonDeactivateEnabled;
		emit activationControlsAccessChanged(m_buttonActivateEnabled, m_buttonDeactivateEnabled);
	}

	return;
}

void TuningSourcesWidget::activateControl(bool enable)
{
	if (m_hasActivationControls == false)
	{
		Q_ASSERT(false);
		return;
	}

	auto sel = m_treeWidget->selectedItems();
	if (sel.size() == 1)
	{
		QString sourceEquipmentId = sel[0]->data(columnIndex_SourceEquipmentId, Qt::UserRole).toString();

		if (login() == false)
		{
			return;
		}

		emit activateSourceControl(sourceEquipmentId, enable);
	}

	return;
}

