#include "TcpClientsStatistics.h"

//
// TcpClientInstance
//

TcpClientInstance::TcpClientInstance(Tcp::Client* client):
	m_client(client)
{
	QMutexLocker l(&TcpClientInstances::m_mutex);

	TcpClientInstances::addConnection(m_client);
}

TcpClientInstance::~TcpClientInstance()
{
	QMutexLocker l(&TcpClientInstances::m_mutex);

	TcpClientInstances::removeConnection(m_client);
}

//
// TcpClientInstances
//

QMutex TcpClientInstances::m_mutex;

std::vector<Tcp::Client*> TcpClientInstances::m_clients;

void TcpClientInstances::addConnection(Tcp::Client* client)
{
	if (std::find(m_clients.begin(), m_clients.end(), client) != m_clients.end())
	{
		Q_ASSERT(false);
		return;
	}

	m_clients.push_back(client);

	return;
}

void TcpClientInstances::removeConnection(Tcp::Client* client)
{
	auto it = std::find(m_clients.begin(), m_clients.end(), client);
	if (it == m_clients.end())
	{
		Q_ASSERT(false);
		return;
	}

	m_clients.erase(it);

	return;
}

std::vector<Tcp::Client*> TcpClientInstances::clients()
{
	return m_clients;
}

//
// DialogStatistics
//

DialogStatistics::DialogStatistics(QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	setWindowTitle(tr("Connections Statistics"));

	setAttribute(Qt::WA_DeleteOnClose);

	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_treeWidget = new QTreeWidget();
	mainLayout->addWidget(m_treeWidget);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	mainLayout->addLayout(bottomLayout);

	QPushButton* reconnectButton = new QPushButton(tr("Reconnect"));
	connect(reconnectButton, &QPushButton::clicked, this, &DialogStatistics::onReconnect);
	bottomLayout->addWidget(reconnectButton);

	bottomLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	connect(closeButton, &QPushButton::clicked, this, &DialogStatistics::reject);
	bottomLayout->addWidget(closeButton);

	setLayout(mainLayout);

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

	m_updateStateTimerId = startTimer(250);
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

			auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to reconnect the connection '%1'?\r\n\r\nData will not be available at the time of reconnection.").arg(item->text(0)), QMessageBox::Yes, QMessageBox::No);
			if (mbResult == QMessageBox::No)
			{
				return;
			}

			QMutexLocker l(&TcpClientInstances::m_mutex);	// Mutex should be locked for all processing time

			std::vector<Tcp::Client*> clients = TcpClientInstances::clients();

			int index = m_treeWidget->currentIndex().row();
			if (index < 0 || index >= static_cast<int>(clients.size()))
			{
					return;
			}

			std::vector<Tcp::Client*> reconnectClients;
			reconnectClients.push_back(clients[index]);

			reconnect(reconnectClients);
	};

	connect(actionReconnect, &QAction::triggered, this, f);

	menu.addAction(actionReconnect);

	menu.exec(QCursor::pos());
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
}

void DialogStatistics::onReconnect()
{
	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to reconnect all connections?\r\n\r\nData will not be available at the time of reconnection."), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	QMutexLocker l(&TcpClientInstances::m_mutex);	// Mutex should be locked for all processing time

	std::vector<Tcp::Client*> clients = TcpClientInstances::clients();

	reconnect(clients);
}

void DialogStatistics::update()
{
	QMutexLocker l(&TcpClientInstances::m_mutex);	// Mutex should be locked for all processing time

	std::vector<Tcp::Client*> displayClients = TcpClientInstances::clients();

	int count = static_cast<int>(displayClients.size());

	bool refreshOnly = true;

	if (m_treeWidget->topLevelItemCount() != count)
	{
		refreshOnly = false;
	}

	if (refreshOnly == false)
	{
		m_treeWidget->clear();

		for (auto client : displayClients)
		{
			if (client == nullptr)
			{
				Q_ASSERT(client);
				return;
			}

			QTreeWidgetItem* item = new QTreeWidgetItem();
			m_treeWidget->addTopLevelItem(item);
		}
	}

	for (int i = 0; i < count; i++)
	{
		Tcp::Client* client = displayClients[i];
		if (client == nullptr)
		{
			Q_ASSERT(client);
			return;
		}

		QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(false);
			continue;
		}

		Tcp::ConnectionState state = client->getConnectionState();

		item->setText(Caption, client->metaObject()->className());
		item->setText(IsConnected, state.isConnected ? tr("Yes") : tr("No"));

		if (state.isConnected == true)
		{
			item->setText(AddressPort, tr("%1 (Server %2)")
						  .arg(state.peerAddr.addressPortStr())
						  .arg(client->selectedServerIndex()));

			QDateTime startTime = QDateTime::fromMSecsSinceEpoch(state.startTime);
			item->setText(StartTime, startTime.toString("dd/MM/yyyy HH:mm:ss"));

			int upTime = (QDateTime::currentMSecsSinceEpoch() - state.startTime) / 1000.0;
			int s = upTime % 60; upTime /= 60;
			int m = upTime % 60; upTime /= 60;
			int h = upTime % 24; upTime /= 24;
			item->setText(UpTime, QString("%1d %2:%3:%4").arg(upTime).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

			item->setText(SentKbytes, QString::number(state.sentBytes / 1024.0, 'f', 2));
			item->setText(ReceivedKbytes, QString::number(state.receivedBytes / 1024.0, 'f', 2));
			item->setText(RequestCount, QString::number(state.requestCount));
			item->setText(ReplyCount, QString::number(state.replyCount));
		}
		else
		{
			item->setText(AddressPort, "");
			item->setText(StartTime, "");
			item->setText(UpTime, "");
			item->setText(SentKbytes, "");
			item->setText(ReceivedKbytes, "");
			item->setText(RequestCount, "");
			item->setText(ReplyCount, "");
		}
	}

	if (refreshOnly == false)
	{
		for (int i = 0; i < m_treeWidget->columnCount(); i++)
		{
			m_treeWidget->resizeColumnToContents(i);
		}
	}
}

void DialogStatistics::reconnect(std::vector<Tcp::Client*> clients)
{
	/*
	for (auto client : clients)
	{
		if (client->selectedServerIndex() == 1)
		{
			client->selectServer1(true);
		}
		else
		{
			client->selectServer2(true);
		}
	}*/
}
