#include <SchemaClientLib/AppDataSourcesWidget.h>
#include "DialogSourceInfo.h"

#include "../UtilsLib/Ui/UiTools.h"
#include <ClientLib/AdsSourceStateConnection.h>

namespace SchemaClientLib
{
	//
	// DialogAppDataSourceInfo
	//
	class DialogAppDataSourceInfo : public DialogSourceInfo
	{
	public:
		explicit DialogAppDataSourceInfo(const ClientLib::AdsSourceStateConnection& adsSourceStateConnection, QWidget* parent, quint64 id);

	private:
		void updateData() override;

	private:
		const ClientLib::AdsSourceStateConnection& m_adsSourceStateConnection;
		int m_noStateInfoTimeout = 0;
	};

	//
	// DialogAppDataSourceInfo implementation
	//
	DialogAppDataSourceInfo::DialogAppDataSourceInfo(const ClientLib::AdsSourceStateConnection& adsSourceStateConnection,
													 QWidget* parent,
													 quint64 id) :
		DialogSourceInfo(parent, id),
		m_adsSourceStateConnection(adsSourceStateConnection)
	{
		std::vector<ClientLib::AppDataSourceState> adsStates = m_adsSourceStateConnection.appDataSourceStates();

		auto foundState = std::find_if(adsStates.begin(),
									   adsStates.end(),
									   [&id](const auto& state)
									   {
										   return state.id() == id;
									   });

		if (foundState == adsStates.end())
		{
			setWindowTitle("Application Data Source - unknown");
		}
		else
		{
			setWindowTitle(tr("Application Data Source - ") + foundState->state.lmequipmentid().c_str());
		}

		//
		QHBoxLayout* l = new QHBoxLayout();

		m_treeWidget = new QTreeWidget();

		m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogSourceInfo::prepareContextMenu);

		l->addWidget(m_treeWidget);

		setLayout(l);

		setMinimumSize(700, 600);

		QStringList headerLabels;
		headerLabels << tr("Parameter");
		headerLabels << tr("Value");
		headerLabels << QString();

		m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
		m_treeWidget->setHeaderLabels(headerLabels);

		{
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
			createDataItem(infoItem, "RupAppDataUID");
			createDataItem(infoItem, "ReceivesData");

			m_treeWidget->addTopLevelItem(infoItem);

			infoItem->setExpanded(true);
		}

		{
			QTreeWidgetItem* stateItem = new QTreeWidgetItem(QStringList() << tr("2-Source State"));

			createDataItem(stateItem, "Uptime");
			createDataItem(stateItem, "LmTime");
			createDataItem(stateItem, "DataReceivingRate");
			createDataItem(stateItem, "LostPacketCount");
			createDataItem(stateItem, "DataProcessingEnabled");
			createDataItem(stateItem, "ReceivedDataID");
			createDataItem(stateItem, "ReceivedDataSize");
			createDataItem(stateItem, "ReceivedFramesCount");
			createDataItem(stateItem, "ReceivedPacketCount");
			createDataItem(stateItem, "RupFrameNumerator");
			createDataItem(stateItem, "SignalStatesQueueCurSize");
			createDataItem(stateItem, "SignalStatesQueueCurMaxSize");
			createDataItem(stateItem, "AcquiredSignalsCount");

			m_treeWidget->addTopLevelItem(stateItem);

			stateItem->setExpanded(true);
		}

		{
			QTreeWidgetItem* errorItem = new QTreeWidgetItem(QStringList() << tr("3-Errors"));

			createDataItem(errorItem, "ErrorProtocolVersion");
			createDataItem(errorItem, "ErrorFramesQuantity");
			createDataItem(errorItem, "ErrorFrameNo");
			createDataItem(errorItem, "ErrorFrameCRC");
			createDataItem(errorItem, "ErrorDataID");
			createDataItem(errorItem, "ErrorDuplicatePlantTime");
			createDataItem(errorItem, "ErrorNonmonotonicPlantTime");

			m_treeWidget->addTopLevelItem(errorItem);

			errorItem->setExpanded(true);
		}

		updateData();

		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}

		m_treeWidget->setSortingEnabled(true);
		m_treeWidget->sortByColumn(0, Qt::AscendingOrder);
	}

	void DialogAppDataSourceInfo::updateData()
	{
		auto adsStates = m_adsSourceStateConnection.appDataSourceStates();

		auto adsState = std::find_if(adsStates.begin(),
									 adsStates.end(),
									 [this](const auto& state)
									 {
										 return state.id() == m_dialogId;
									 });

		// If state was not found in <closeInterval> seconds - close the dialog
		//
		if (adsState == adsStates.end())
		{
			const int closeInterval = 10 * (1000 / 250); // 10 seconds
			if (m_noStateInfoTimeout++ > closeInterval)
			{
				// Close dialog if no information is received
				//
				reject();
			}
			return;
		}
		m_noStateInfoTimeout = 0;

		// info
		//
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(0);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		item->setData(0, Qt::UserRole, 0);

		setDataItemText("ID", tr("%1 (%2h)").arg(QString::number(adsState->info.id())).arg(QString::number(adsState->info.id(), 16)));
		setDataItemText("EquipmentID", QString::fromStdString(adsState->info.moduleequipmentid()));
		setDataItemText("Caption", QString::fromStdString(adsState->info.modulecaption()));
		setDataItemNumber("DataType", adsState->info.lancontrollerinfo()[0].lancontrollertype());
		setDataItemText("IP", QString::fromStdString(adsState->info.lancontrollerinfo()[0].appdataip()));
		setDataItemNumber("Port", adsState->info.lancontrollerinfo()[0].appdataport());
		setDataItemText("Channel", QString::fromStdString(adsState->info.subsystemchannel()));
		setDataItemNumber("SubsystemID", adsState->info.subsystemkey());
		setDataItemText("Subsystem", QString::fromStdString(adsState->info.subsystemid()));

		setDataItemNumber("LmNumber", adsState->info.lmnumber());
		setDataItemText(
			"LmModuleType",
			tr("%1 (%2h)").arg(QString::number(adsState->info.moduletype())).arg(QString::number(adsState->info.moduletype(), 16)));
		setDataItemText("LmAdapterID", QString::fromStdString(adsState->info.lancontrollerinfo()[0].equipmentid()));
		setDataItemNumber("LmDataEnable", adsState->info.lancontrollerinfo()[0].appdataenable());
		setDataItemText("RupAppDataUID",
						tr("%1 (%2h)")
							.arg(QString::number(adsState->info.lancontrollerinfo()[0].rupappdatauid()))
							.arg(QString::number(adsState->info.lancontrollerinfo()[0].rupappdatauid(), 16)));
		setDataItemNumber("AcquiredSignalsCount", adsState->info.acquiredsignalscount());

		{
			QTreeWidgetItem* dataReceivesItem = dataItem("ReceivesData");

			if (dataReceivesItem == nullptr)
			{
				Q_ASSERT(dataReceivesItem);
				return;
			}

			if (adsState->state.receivesdata() == false)
			{
				dataReceivesItem->setForeground(1, QBrush(DialogSourceInfo::dataItemErrorColor));
				dataReceivesItem->setText(1, "No");
			}
			else
			{
				dataReceivesItem->setForeground(1, QBrush(Qt::black));
				dataReceivesItem->setText(1, "Yes");
			}
		}

		QDateTime tm;
		tm.setTimeSpec(Qt::UTC);

		tm.setMSecsSinceEpoch(adsState->state.uptime());
		setDataItemText("Uptime", tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

		setDataItemNumber("ReceivedDataID", adsState->state.receiveddataid());
		double datareceivingrate = adsState->state.datareceivingspeed();
		setDataItemText("DataReceivingRate", QString::number(datareceivingrate / 1024.0, 'f', 1));
		setDataItemNumber("ReceivedDataSize", adsState->state.receiveddatasize());
		setDataItemNumber("ReceivedFramesCount", adsState->state.receivedframescount());
		setDataItemNumber("ReceivedPacketCount", adsState->state.receivedpacketcount());
		setDataItemNumber("LostPacketCount", adsState->state.lostpacketcount());
		setDataItemText("DataProcessingEnabled", adsState->state.dataprocessingenabled() ? "Yes" : "No");

		tm.setMSecsSinceEpoch(adsState->state.lmtime());
		setDataItemText("LmTime", tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

		setDataItemNumber("RupFrameNumerator", adsState->state.rupframenumerator());
		setDataItemNumber("SignalStatesQueueCurSize", adsState->state.signalstatesqueuecursize());
		setDataItemNumber("SignalStatesQueueCurMaxSize", adsState->state.signalstatesqueuecurmaxsize());

		// errors

		item = m_treeWidget->topLevelItem(1);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		item->setData(0, Qt::UserRole, 0);

		setDataItemNumberCompare(item,
								 "ErrorProtocolVersion",
								 adsState->state.errorprotocolversion(),
								 adsState->previousState().errorprotocolversion());
		setDataItemNumberCompare(item,
								 "ErrorFramesQuantity",
								 adsState->state.errorframesquantity(),
								 adsState->previousState().errorframesquantity());
		setDataItemNumberCompare(item, "ErrorFrameNo", adsState->state.errorframeno(), adsState->previousState().errorframeno());
		setDataItemNumberCompare(item, "ErrorFrameCRC", adsState->state.errorframecrc(), adsState->previousState().errorframecrc());
		setDataItemNumberCompare(item, "ErrorDataID", adsState->state.errordataid(), adsState->previousState().errordataid());
		setDataItemNumberCompare(item,
								 "ErrorDuplicatePlantTime",
								 adsState->state.errorduplicateplanttime(),
								 adsState->previousState().errorduplicateplanttime());
		setDataItemNumberCompare(item,
								 "ErrorNonmonotonicPlantTime",
								 adsState->state.errornonmonotonicplanttime(),
								 adsState->previousState().errornonmonotonicplanttime());
	}

	//
	// DialogAppDataSources
	//
	AppDataSourcesWidget::AppDataSourcesWidget(const ClientLib::AdsSourceStateConnection& connection, QWidget* parent) :
		QWidget(parent),
		m_adsSourceStateConnection(connection),
		m_parent(parent)
	{
		//

		QVBoxLayout* mainLayout = new QVBoxLayout();
		mainLayout->setContentsMargins(0, 0, 0, 0);

		m_treeWidget = new QTreeWidget();
		mainLayout->addWidget(m_treeWidget);
		connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &AppDataSourcesWidget::treeWidgetItemDoubleClicked);

		m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &AppDataSourcesWidget::contextMenuRequested);

		QHBoxLayout* bottomLayout = new QHBoxLayout();
		mainLayout->addLayout(bottomLayout);

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
		headerLabels << tr("LmTime");
		headerLabels << tr("ReceivedCount");
		headerLabels << tr("Receiving Rate, KB/sec");

		m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
		m_treeWidget->setHeaderLabels(headerLabels);

		update(false);

		m_treeWidget->setSortingEnabled(true);
		m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

		m_updateStateTimerId = startTimer(m_updateIntervalMs);

		return;
	}

	AppDataSourcesWidget::~AppDataSourcesWidget() = default;

	bool AppDataSourcesWidget::treeIsFocused() const
	{
		return m_treeWidget->hasFocus();
	}

	void AppDataSourcesWidget::detailsClicked()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();

		if (item == nullptr)
		{
			return;
		}

		quint64 id = item->data(columnIndex_Id, Qt::UserRole).value<quint64>();

		auto it = m_sourceInfoDialogsMap.find(id);
		if (it == m_sourceInfoDialogsMap.end())
		{
			DialogAppDataSourceInfo* dlg = new DialogAppDataSourceInfo(m_adsSourceStateConnection, this, id);
			connect(dlg, &DialogAppDataSourceInfo::dialogClosed, this, &AppDataSourcesWidget::detailsDialogClosed);
			dlg->show();
			dlg->activateWindow();

			m_sourceInfoDialogsMap[id] = dlg;
		}
		else
		{
			DialogAppDataSourceInfo* dlg = it->second;
			if (dlg == nullptr)
			{
				Q_ASSERT(dlg);
				return;
			}

			dlg->activateWindow();

			UiTools::adjustDialogPlacement(dlg);
		}
	}

	void AppDataSourcesWidget::timerEvent(QTimerEvent* event)
	{
		Q_ASSERT(event);

		if (event->timerId() == m_updateStateTimerId)
		{
			update(true);
		}
	}

	void AppDataSourcesWidget::tuningSourcesArrived()
	{
		update(false);
	}

	void AppDataSourcesWidget::update(bool refreshOnly)
	{
		auto adsStates = m_adsSourceStateConnection.appDataSourceStates();

		int count = static_cast<int>(adsStates.size());

		if (m_treeWidget->topLevelItemCount() != count)
		{
			refreshOnly = false;
		}

		if (refreshOnly == false)
		{
			m_treeWidget->clear();

			for (const auto& adsState : adsStates)
			{
				QStringList connectionStrings;

				connectionStrings << adsState.info.lancontrollerinfo()[0].equipmentid().c_str();
				connectionStrings << adsState.info.lancontrollerinfo()[0].appdataip().c_str();
				connectionStrings << QString::number(adsState.info.lancontrollerinfo()[0].appdataport());

				connectionStrings << adsState.info.subsystemchannel().c_str();

				connectionStrings << adsState.info.subsystemid().c_str();
				connectionStrings << QString::number(adsState.info.lmnumber());

				QTreeWidgetItem* item = new QTreeWidgetItem(connectionStrings);

				item->setData(columnIndex_Id, Qt::UserRole, adsState.id());

				m_treeWidget->addTopLevelItem(item);
			}
		}

		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
			if (item == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			quint64 id = item->data(columnIndex_Id, Qt::UserRole).toULongLong();

			auto adsState = std::find_if(adsStates.begin(),
										 adsStates.end(),
										 [&id](const auto& state)
										 {
											 return state.id() == id;
										 });

			if (adsState == adsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			QDateTime tm;
			tm.setTimeSpec(Qt::UTC);

			tm.setMSecsSinceEpoch(adsState->state.lmtime());
			item->setText(static_cast<int>(Columns::LmTime), tm.toString("dd/MM/yyyy HH:mm:ss.zzz"));

			item->setText(static_cast<int>(Columns::ReceivedPacketCount), QString::number(adsState->state.receivedpacketcount()));
			double datareceivingrate = adsState->state.datareceivingspeed();
			item->setText(static_cast<int>(Columns::DataReceivingRate), QString::number(datareceivingrate, 'f', 1));

			if (adsState->valid() == false)
			{
				item->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

				item->setText(static_cast<int>(Columns::State), tr("Unknown"));
			}
			else
			{
				if (adsState->state.receivesdata() == false)
				{
					item->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

					item->setText(static_cast<int>(Columns::State), tr("No Data Received"));
				}
				else
				{
					int errorsCount = adsState->getErrorsCount();

					if (errorsCount == 0)
					{
						item->setForeground(static_cast<int>(Columns::State), QBrush(Qt::black));

						item->setText(static_cast<int>(Columns::State), tr("Active"));
					}
					else
					{
						item->setForeground(static_cast<int>(Columns::State), QBrush(DialogSourceInfo::dataItemErrorColor));

						item->setText(static_cast<int>(Columns::State), tr("E: %1").arg(errorsCount));
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

			m_treeWidget->setColumnWidth(static_cast<int>(Columns::State), 120);
		}
	}

	void AppDataSourcesWidget::treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column)
	{
		Q_UNUSED(item);
		Q_UNUSED(column);

		QTimer::singleShot(10, this, &AppDataSourcesWidget::detailsClicked);
	}

	void AppDataSourcesWidget::detailsDialogClosed(quint64 id)
	{
		auto it = m_sourceInfoDialogsMap.find(id);
		if (it == m_sourceInfoDialogsMap.end())
		{
			Q_ASSERT(false);
			return;
		}

		m_sourceInfoDialogsMap.erase(it);
	}

	void AppDataSourcesWidget::contextMenuRequested()
	{
		if (m_treeWidget->selectedItems().empty() == true)
		{
			return;
		}

		QAction action(tr("Details..."));
		connect(&action,
				&QAction::triggered,
				this,
				[this]()
				{
					detailsClicked();
				});

		QMenu menu(this);
		menu.addAction(&action);
		menu.exec(this->cursor().pos());

		return;
	}
} // namespace SchemaClientLib