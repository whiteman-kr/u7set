#include "AppDataSourcesWidget.h"
#include "TcpAppSourcesState.h"
#include "UiTools.h"

#include <QTreeWidget>

//
// DialogAppDataSourceInfo
//

DialogAppDataSourceInfo::DialogAppDataSourceInfo(TcpAppSourcesState* tcpClient, QWidget* parent,  Hash sourceHash) :
	DialogSourceInfo(parent, sourceHash),
	m_tcpClient(tcpClient)
{
	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	bool ok = false;

	AppDataSourceState adsState = m_tcpClient->appDataSourceState(sourceHash, &ok);

	if (ok == true)
	{
		setWindowTitle(tr("Application Data Source - ") + adsState.state.lmequipmentid().c_str());
	}
	else
	{
		setWindowTitle("???");
	}

	//

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

	createDataItem(infoItem, "Uptime");
	createDataItem(infoItem, "DataReceives");
	createDataItem(stateItem, "DataReceivingRate");
	createDataItem(stateItem, "ProcessedPacketCount");
	createDataItem(stateItem, "LostPacketCount");
	createDataItem(stateItem, "DataProcessingEnabled");
	createDataItem(stateItem, "ReceivedDataID");
	createDataItem(stateItem, "ReceivedDataSize");
	createDataItem(stateItem, "ReceivedFramesCount");
	createDataItem(stateItem, "ReceivedPacketCount");
	createDataItem(stateItem, "LastPacketSystemTime");
	createDataItem(stateItem, "RupFramePlantTime");
	createDataItem(stateItem, "RupFrameNumerator");
	createDataItem(stateItem, "RupFramesQueueSize");
	createDataItem(stateItem, "RupFramesQueueMaxSize");
	createDataItem(stateItem, "SignalStatesQueueSize");
	createDataItem(stateItem, "SignalStatesQueueMaxSize");
	createDataItem(stateItem, "AcquiredSignalsCount");

	m_treeWidget->addTopLevelItem(stateItem);

	stateItem->setExpanded(true);

	QTreeWidgetItem* errorItem = new QTreeWidgetItem(QStringList() << tr("3-Errors"));

	createDataItem(errorItem, "ErrorProtocolVersion");
	createDataItem(errorItem, "ErrorFramesQuantity");
	createDataItem(errorItem, "ErrorFrameNo");
	createDataItem(errorItem, "ErrorDataID");
	createDataItem(errorItem, "ErrorFrameSize");
	createDataItem(errorItem, "ErrorDuplicatePlantTime");
	createDataItem(errorItem, "ErrorNonmonotonicPlantTime");

	m_treeWidget->addTopLevelItem(errorItem);

	errorItem->setExpanded(true);

	updateData();

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

}

DialogAppDataSourceInfo::~DialogAppDataSourceInfo()
{

}

void DialogAppDataSourceInfo::updateData()
{
	if (m_tcpClient == nullptr)
	{
		assert(m_tcpClient);
		return;
	}

	bool ok = false;

	AppDataSourceState ds = m_tcpClient->appDataSourceState(m_sourceHash, &ok);
	if (ok == false)
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

	item->setData(0, Qt::UserRole, 0);

	setDataItemText("ID", tr("%1 (%2h)").arg(QString::number(ds.info.id())).arg(QString::number(ds.info.id(), 16)));
	setDataItemText("EquipmentID", ds.info.lmequipmentid().c_str());
	setDataItemText("Caption", ds.info.lmcaption().c_str());
	setDataItemNumber("DataType", ds.info.lmdatatype());
	setDataItemText("IP", ds.info.lmip().c_str());
	setDataItemNumber("Port", ds.info.lmport());
	setDataItemText("Channel", ds.info.lmsubsystemchannel().c_str());
	setDataItemNumber("SubsystemID", ds.info.lmsubsystemid());
	setDataItemText("Subsystem", ds.info.lmsubsystem().c_str());

	setDataItemNumber("LmNumber", ds.info.lmnumber());
	setDataItemText("LmModuleType", tr("%1 (%2h)").arg(QString::number(ds.info.lmmoduletype())).arg(QString::number(ds.info.lmmoduletype(), 16)));
	setDataItemText("LmAdapterID", ds.info.lmadapterid().c_str());
	setDataItemNumber("LmDataEnable", ds.info.lmdataenable());
	setDataItemText("LmDataID", tr("%1 (%2h)").arg(QString::number(ds.info.lmdataid())).arg(QString::number(ds.info.lmdataid(), 16)));

	setDataItemText("DataReceives", ds.state.datareceives() ? "Yes" : "No");

	{
		QTreeWidgetItem*  item = dataItem("DataReceives");
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		if (ds.state.datareceives() == false)
		{
			item->setForeground(1, QBrush(DialogSourceInfo::dataItemErrorColor));
		}
		else
		{
			item->setForeground(1, QBrush(Qt::black));
		}
	}

	auto time = ds.state.uptime();
	int s = time % 60; time /= 60;
	int m = time % 60; time /= 60;
	int h = time % 24; time /= 24;
	setDataItemText("Uptime", QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

	setDataItemNumber("ReceivedDataID", ds.state.receiveddataid());
	setDataItemNumber("RupFramesQueueSize", ds.state.rupframesqueuesize());
	setDataItemNumber("RupFramesQueueMaxSize", ds.state.rupframesqueuemaxsize());
	setDataItemNumber("DataReceivingRate", ds.state.datareceivingrate() / 1024.0);
	setDataItemNumber("ReceivedDataSize", ds.state.receiveddatasize());
	setDataItemNumber("ReceivedFramesCount", ds.state.receivedframescount());
	setDataItemNumber("ReceivedPacketCount", ds.state.receivedpacketcount());
	setDataItemNumber("LostPacketCount", ds.state.lostedpacketcount());
	setDataItemText("DataProcessingEnabled", ds.state.dataprocessingenabled() ? "Yes" : "No");
	setDataItemNumber("ProcessedPacketCount", ds.state.processedpacketcount());

	QDateTime tm;
	tm.setMSecsSinceEpoch(ds.state.lastpacketsystemtime());
	setDataItemText("LastPacketSystemTime", tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

	tm.setMSecsSinceEpoch(ds.state.rupframeplanttime());
	setDataItemText("RupFramePlantTime", tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

	setDataItemNumber("RupFrameNumerator", ds.state.rupframenumerator());
	setDataItemNumber("SignalStatesQueueSize", ds.state.signalstatesqueuesize());
	setDataItemNumber("SignalStatesQueueMaxSize", ds.state.signalstatesqueuemaxsize());
	setDataItemNumber("AcquiredSignalsCount", ds.state.acquiredsignalscount());

	// errors

	item = m_treeWidget->topLevelItem(1);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(0, Qt::UserRole, 0);

	setDataItemNumberCompare(item, "ErrorProtocolVersion", ds.state.errorprotocolversion(), ds.previousState().errorprotocolversion());
	setDataItemNumberCompare(item, "ErrorFramesQuantity", ds.state.errorframesquantity(), ds.previousState().errorframesquantity());
	setDataItemNumberCompare(item, "ErrorFrameNo", ds.state.errorframeno(), ds.previousState().errorframeno());
	setDataItemNumberCompare(item, "ErrorDataID", ds.state.errordataid(), ds.previousState().errordataid());
	setDataItemNumberCompare(item, "ErrorFrameSize", ds.state.errorframesize(), ds.previousState().errorframesize());
	setDataItemNumberCompare(item, "ErrorDuplicatePlantTime", ds.state.errorduplicateplanttime(), ds.previousState().errorduplicateplanttime());
	setDataItemNumberCompare(item, "ErrorNonmonotonicPlantTime", ds.state.errornonmonotonicplanttime(), ds.previousState().errornonmonotonicplanttime());
}

//
// DialogAppDataSources
//

AppDataSourcesWidget::AppDataSourcesWidget(TcpAppSourcesState* tcpClient,  bool hasCloseButton, QWidget* parent) :
	QWidget(parent),
	m_stateTcpClient(tcpClient),
	m_parent(parent)
{
	if (m_stateTcpClient == nullptr)
	{
		assert(m_stateTcpClient);
		return;
	}

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &AppDataSourcesWidget::on_treeWidget_itemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &AppDataSourcesWidget::on_treeWidget_itemSelectionChanged);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	m_btnDetails = new QPushButton(tr("Details..."));
	m_btnDetails->setEnabled(false);
	connect(m_btnDetails, &QPushButton::clicked, this, &AppDataSourcesWidget::on_btnDetails_clicked);
	bottomLayout->addWidget(m_btnDetails);

	bottomLayout->addStretch();

	m_closeButton = new QPushButton(tr("Close"));
	connect(m_closeButton, &QPushButton::clicked, this, &AppDataSourcesWidget::on_btnClose_clicked);
	bottomLayout->addWidget(m_closeButton);

	showCloseButton(hasCloseButton);

	setLayout(mainLayout);

	//


	QStringList headerLabels;
	headerLabels << tr("EquipmentID");

	headerLabels << tr("IP");
	headerLabels << tr("Port");
	headerLabels << tr("Channel");
	headerLabels << tr("SubsystemID");
	headerLabels << tr("LmNumber");

	headerLabels << tr("State");
	headerLabels << tr("Uptime");
	headerLabels << tr("ReceivedCount");
	headerLabels << tr("Receiving Rate, KB/sec");

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

	update(false);

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

	m_updateStateTimerId = startTimer(250);
}

AppDataSourcesWidget::~AppDataSourcesWidget()
{
}

void AppDataSourcesWidget::showCloseButton(bool show)
{
	m_closeButton->setVisible(show);
}

void AppDataSourcesWidget::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update(true);
	}
}

void AppDataSourcesWidget::slot_tuningSourcesArrived()
{
	update(false);
}

void AppDataSourcesWidget::update(bool refreshOnly)
{
	if (m_stateTcpClient == nullptr)
	{
		assert(m_stateTcpClient);
		return;
	}

	std::vector<Hash> appDataSourceHashes = m_stateTcpClient->appDataSourceHashes();

	int count = static_cast<int>(appDataSourceHashes.size());

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

			bool ok = false;

			Hash hash = appDataSourceHashes[i];

			AppDataSourceState adsState = m_stateTcpClient->appDataSourceState(hash, &ok);
			if (ok == false)
			{
				assert(false);
				continue;
			}

			connectionStrings << adsState.info.lmequipmentid().c_str();
			connectionStrings << adsState.info.lmip().c_str();
			connectionStrings << QString::number(adsState.info.lmport());

			connectionStrings << adsState.info.lmsubsystemchannel().c_str();

			connectionStrings << adsState.info.lmsubsystem().c_str();
			connectionStrings << QString::number(adsState.info.lmnumber());

			QTreeWidgetItem* item = new QTreeWidgetItem(connectionStrings);

			item->setData(columnIndex_Hash, Qt::UserRole, hash);

			m_treeWidget->addTopLevelItem(item);
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

		Hash hash = item->data(columnIndex_Hash, Qt::UserRole).toULongLong();

		bool ok = false;

		AppDataSourceState adsState = m_stateTcpClient->appDataSourceState(hash, &ok);
		if (ok == false)
		{
			assert(false);
			continue;
		}

		auto time = adsState.state.uptime();
		int s = time % 60; time /= 60;
		int m = time % 60; time /= 60;
		int h = time % 24; time /= 24;

		item->setText(Uptime, QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
		item->setText(DataReceivingRate, QString::number(adsState.state.datareceivingrate() / 1024.0));
		item->setText(ReceivedPacketCount, QString::number(adsState.state.receivedpacketcount()));
		item->setText(ProcessedPacketCount, QString::number(adsState.state.processedpacketcount()));

		if (adsState.valid() == false)
		{
			item->setForeground(State, QBrush(DialogSourceInfo::dataItemErrorColor));

			item->setText(State, tr("Unknown"));
		}
		else
		{
			if (adsState.state.datareceives() == false)
			{
				item->setForeground(State, QBrush(DialogSourceInfo::dataItemErrorColor));

				item->setText(State, tr("No Data Received"));
			}
			else
			{
				int errorsCount = adsState.getErrorsCount();

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
	}

	if (refreshOnly == false)
	{
		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		m_treeWidget->setColumnWidth(State, 120);
		m_treeWidget->setColumnWidth(Uptime, 120);
	}
}

void AppDataSourcesWidget::on_btnClose_clicked()
{
	emit closeButtonPressed();
}

void AppDataSourcesWidget::on_btnDetails_clicked()
{
	if (m_stateTcpClient == nullptr)
	{
		assert(m_stateTcpClient);
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
		DialogAppDataSourceInfo* dlg = new DialogAppDataSourceInfo(m_stateTcpClient, this, hash);
		connect(dlg, &DialogAppDataSourceInfo::dialogClosed, this, &AppDataSourcesWidget::onDetailsDialogClosed);
		dlg->show();
		dlg->activateWindow();

		m_sourceInfoDialogsMap[hash] = dlg;
	}
	else
	{
		DialogAppDataSourceInfo* dlg = it->second;
		if (dlg == nullptr)
		{
			assert(dlg);
			return;
		}

		dlg->activateWindow();

		UiTools::adjustDialogPlacement(dlg);
	}
}

void AppDataSourcesWidget::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* item = m_treeWidget->currentItem();

	m_btnDetails->setEnabled(item != nullptr);
}

void AppDataSourcesWidget::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	QTimer::singleShot(10, this, &AppDataSourcesWidget::on_btnDetails_clicked);
}

void AppDataSourcesWidget::onDetailsDialogClosed(Hash hash)
{
	auto it = m_sourceInfoDialogsMap.find(hash);
	if (it == m_sourceInfoDialogsMap.end())
	{
		//assert(false);
		return;
	}

	m_sourceInfoDialogsMap.erase(it);

}
