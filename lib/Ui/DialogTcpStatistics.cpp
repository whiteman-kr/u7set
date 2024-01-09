#include "DialogTcpStatistics.h"
#include "../../OnlineLib/TcpClientStatistics.h"

//
// DialogStatistics
//
DialogTcpStatistics::DialogTcpStatistics(QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Connections Statistics"));

	setAttribute(Qt::WA_DeleteOnClose);

	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	QPushButton* reconnectButton = new QPushButton(tr("Reconnect"));
	connect(reconnectButton, &QPushButton::clicked, this, &DialogTcpStatistics::reconnectAll);
	bottomLayout->addWidget(reconnectButton);

	bottomLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &DialogTcpStatistics::reject);
	bottomLayout->addWidget(closeButton);

	setLayout(mainLayout);

	//--
	//
	QStringList headerLabels;
	headerLabels << tr("Caption");
	headerLabels << tr("ServerID");
	headerLabels << tr("Connected");

	headerLabels << tr("Address");
	headerLabels << tr("Start Time");
	headerLabels << tr("UpTime");
	headerLabels << tr("Sent KBytes");
	headerLabels << tr("Received KBytes");
	headerLabels << tr("Request Count");
	headerLabels << tr("Reply Count");

	m_treeWidget->setColumnCount(static_cast<int>(headerLabels.size()));
	m_treeWidget->setHeaderLabels(headerLabels);

	m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,this, &DialogTcpStatistics::prepareContextMenu);

	setMinimumSize(960, 250);

	update();

	m_treeWidget->setSortingEnabled(true);
	m_treeWidget->sortByColumn(0, Qt::AscendingOrder);

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_updateStateTimerId = startTimer(250);

	return;
}

void DialogTcpStatistics::prepareContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (m_treeWidget == nullptr)
	{
		assert(m_treeWidget);
		return;
	}

	QMenu menu(this);

	QAction* actionReconnect = new QAction(tr("Reconnect"), &menu);

	auto f = [this]() -> void
		{
			QTreeWidgetItem* item = m_treeWidget->currentItem();
			if (item == nullptr)
			{
				return;
			}

			auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to reconnect the connection '%1'?\n\nData will not be available at the time of reconnection.").arg(item->text(0)), QMessageBox::Yes, QMessageBox::No);
			if (mbResult == QMessageBox::No)
			{
				return;
			}

			 size_t id = item->data(static_cast<int>(ColumnUserData::StatId), Qt::UserRole).value<uintptr_t>();
			 Q_ASSERT(id);

			TcpClientStatistics::reconnect(id);
			return;
		};

	connect(actionReconnect, &QAction::triggered, this, f);

	menu.addAction(actionReconnect);

	menu.exec(QCursor::pos());

	return;
}


void DialogTcpStatistics::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

void DialogTcpStatistics::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update();
	}

	return;
}

void DialogTcpStatistics::reconnectAll()
{
	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to reconnect all connections?\n\nData will not be available at the time of reconnection."), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	std::vector<TcpClientStatistics::Statistics> stats = TcpClientStatistics::statistics();

	for (const auto& stat : stats)
	{
		TcpClientStatistics::reconnect(stat.id);
	}

	return;
}

void DialogTcpStatistics::update()
{
	std::vector<TcpClientStatistics::Statistics> stats = TcpClientStatistics::statistics();

	int count = static_cast<int>(stats.size());

	if (m_treeWidget->topLevelItemCount() != count)
	{
		m_treeWidget->clear();

		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setData(static_cast<int>(ColumnUserData::Index), Qt::UserRole, i);
			m_treeWidget->addTopLevelItem(item);
		}
	}

	for (int i = 0; i < count; i++)
	{
		TcpClientStatistics::Statistics& stat = stats[i];

		QTreeWidgetItem* item = nullptr;

		for (int j = 0; j < m_treeWidget->topLevelItemCount(); j++)
		{
			QTreeWidgetItem* topItem = m_treeWidget->topLevelItem(j);
			if (topItem == nullptr)
			{
				Q_ASSERT(topItem);
				continue;
			}

			if (topItem->data(static_cast<int>(ColumnUserData::Index), Qt::UserRole).toInt() == i)
			{
				item = topItem;
				break;
			}
		}

		if (item == nullptr)
		{
			Q_ASSERT(item);
			continue;
		}

		item->setData(static_cast<int>(ColumnUserData::StatId), Qt::UserRole, QVariant::fromValue<uintptr_t>(stat.id));
		item->setText(static_cast<int>(Columns::Caption), stat.objectName);
		item->setText(static_cast<int>(Columns::ServerID), stat.serverId);
		item->setText(static_cast<int>(Columns::IsConnected), stat.state.isConnected ? tr("Yes") : tr("No"));

		if (stat.state.isConnected == true)
		{
			item->setText(static_cast<int>(Columns::AddressPort), tr("%1").arg(stat.state.peerAddr.addressPortStr()));

			QDateTime startTime = QDateTime::fromMSecsSinceEpoch(stat.state.startTime);
			item->setText(static_cast<int>(Columns::StartTime), startTime.toString("dd/MM/yyyy HH:mm:ss"));

			qint64 upTime = (QDateTime::currentMSecsSinceEpoch() - stat.state.startTime) / 1000;
			qint64 s = upTime % 60; upTime /= 60;
			qint64 m = upTime % 60; upTime /= 60;
			qint64 h = upTime % 24; upTime /= 24;
			item->setText(static_cast<int>(Columns::UpTime), QString("%1d %2:%3:%4").arg(upTime).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

			item->setText(static_cast<int>(Columns::SentKbytes), QString::number(stat.state.sentBytes / 1024.0, 'f', 2));
			item->setText(static_cast<int>(Columns::ReceivedKbytes), QString::number(stat.state.receivedBytes / 1024.0, 'f', 2));
			item->setText(static_cast<int>(Columns::RequestCount), QString::number(stat.state.requestCount));
			item->setText(static_cast<int>(Columns::ReplyCount), QString::number(stat.state.replyCount));
		}
		else
		{
			item->setText(static_cast<int>(Columns::AddressPort), "");
			item->setText(static_cast<int>(Columns::StartTime), "");
			item->setText(static_cast<int>(Columns::UpTime), "");
			item->setText(static_cast<int>(Columns::SentKbytes), "");
			item->setText(static_cast<int>(Columns::ReceivedKbytes), "");
			item->setText(static_cast<int>(Columns::RequestCount), "");
			item->setText(static_cast<int>(Columns::ReplyCount), "");
		}
	}

	return;
}

