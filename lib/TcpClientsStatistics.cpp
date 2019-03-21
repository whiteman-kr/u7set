#include "TcpClientsStatistics.h"

//
// TcpClientInstance
//

QMutex TcpClientStatistics::s_mutex;
std::set<Tcp::Client*> TcpClientStatistics::s_clients;


TcpClientStatistics::TcpClientStatistics(Tcp::Client* client) :
	m_client(client)
{
	Q_ASSERT(m_client);

	QMutexLocker l(&s_mutex);
	s_clients.insert(m_client);

	return;
}

TcpClientStatistics::~TcpClientStatistics()
{
	QMutexLocker l(&s_mutex);

	Q_ASSERT(s_clients.count(m_client) == 1);
	s_clients.erase(m_client);

	return;
}

std::vector<TcpClientStatistics::Statisctics> TcpClientStatistics::statistics()
{
	std::vector<Statisctics> result;

	QMutexLocker l(&s_mutex);
	result.reserve(s_clients.size());

	for (Tcp::Client* tcpClient : s_clients)
	{
		if (tcpClient == nullptr)
		{
			Q_ASSERT(tcpClient);
			return {};
		}

		result.emplace_back(reinterpret_cast<size_t>(tcpClient),
							tcpClient->objectName(),
							tcpClient->getConnectionState());
	}

	return result;
}

void TcpClientStatistics::reconnect(size_t id)
{
	QMutexLocker l(&s_mutex);

	Tcp::Client* ptr = reinterpret_cast<Tcp::Client*>(id);
	if (s_clients.count(ptr) == 0)
	{
		return;
	}

	Tcp::Client* tcpClient = dynamic_cast<Tcp::Client*>(ptr);
	if (tcpClient == nullptr)
	{
		Q_ASSERT(tcpClient);
		return;
	}

	tcpClient->setServers(tcpClient->serverAddressPort1(), tcpClient->serverAddressPort2(), true);		// this will reconnect

	return;
}



//
// DialogStatistics
//

DialogStatistics::DialogStatistics(QWidget* parent) :
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
	connect(reconnectButton, &QPushButton::clicked, this, &DialogStatistics::reconnectAll);
	bottomLayout->addWidget(reconnectButton);

	bottomLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &DialogStatistics::reject);
	bottomLayout->addWidget(closeButton);

	setLayout(mainLayout);

	//--
	//
	QStringList headerLabels;
	headerLabels << tr("Caption");
	headerLabels << tr("Connected");

	headerLabels << tr("Address");
	headerLabels << tr("Start Time");
	headerLabels << tr("UpTime");
	headerLabels << tr("Sent KBytes");
	headerLabels << tr("Received KBytes");
	headerLabels << tr("Request Count");
	headerLabels << tr("Reply Count");

	m_treeWidget->setColumnCount(headerLabels.size());
	m_treeWidget->setHeaderLabels(headerLabels);

	m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,this, &DialogStatistics::prepareContextMenu);

	setMinimumSize(960, 250);

	update();

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	m_updateStateTimerId = startTimer(250);

	return;
}

DialogStatistics::~DialogStatistics()
{
}

void DialogStatistics::prepareContextMenu(const QPoint& pos)
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

			 size_t id = item->data(0, Qt::UserRole).value<size_t>();
			 Q_ASSERT(id);

			TcpClientStatistics::reconnect(id);
			return;
		};

	connect(actionReconnect, &QAction::triggered, this, f);

	menu.addAction(actionReconnect);

	menu.exec(QCursor::pos());

	return;
}


void DialogStatistics::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

void DialogStatistics::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		update();
	}

	return;
}

void DialogStatistics::reconnectAll()
{
	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to reconnect all connections?\n\nData will not be available at the time of reconnection."), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	std::vector<TcpClientStatistics::Statisctics> stats = TcpClientStatistics::statistics();

	for (const auto& stat : stats)
	{
		TcpClientStatistics::reconnect(stat.id);
	}

	return;
}

void DialogStatistics::update()
{
	std::vector<TcpClientStatistics::Statisctics> stats = TcpClientStatistics::statistics();

	int count = static_cast<int>(stats.size());

	bool refreshOnly = true;

	if (m_treeWidget->topLevelItemCount() != count)
	{
		refreshOnly = false;
	}

	if (refreshOnly == false)
	{
		m_treeWidget->clear();

		for (TcpClientStatistics::Statisctics& s : stats)
		{
			Q_UNUSED(s);

			QTreeWidgetItem* item = new QTreeWidgetItem();
			m_treeWidget->addTopLevelItem(item);
		}
	}

	for (int i = 0; i < count; i++)
	{
		TcpClientStatistics::Statisctics& stat = stats[i];

		QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		item->setData(0, Qt::UserRole, QVariant::fromValue<size_t>(stat.id));

		item->setText(static_cast<int>(Columns::Caption), stat.objectName);
		item->setText(static_cast<int>(Columns::IsConnected), stat.state.isConnected ? tr("Yes") : tr("No"));

		if (stat.state.isConnected == true)
		{
			item->setText(static_cast<int>(Columns::AddressPort), tr("%1").arg(stat.state.peerAddr.addressPortStr()));

			QDateTime startTime = QDateTime::fromMSecsSinceEpoch(stat.state.startTime);
			item->setText(static_cast<int>(Columns::StartTime), startTime.toString("dd/MM/yyyy HH:mm:ss"));

			int upTime = (QDateTime::currentMSecsSinceEpoch() - stat.state.startTime) / 1000.0;
			int s = upTime % 60; upTime /= 60;
			int m = upTime % 60; upTime /= 60;
			int h = upTime % 24; upTime /= 24;
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

