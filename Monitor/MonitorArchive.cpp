#include "MonitorArchive.h"
#include "Settings.h"
#include <QMessageBox>

std::map<QString, MonitorArchiveWidget*> MonitorArchive::m_archiveList;

std::vector<QString> MonitorArchive::getArchiveList()
{
	std::vector<QString> result;
	result.reserve(m_archiveList.size());

	for (std::pair<QString, MonitorArchiveWidget*> p : m_archiveList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorArchive::activateWindow(QString archiveName)
{
	if (m_archiveList.count(archiveName) != 1)
	{
		assert(m_archiveList.count(archiveName) != 1);
		return false;
	}

	MonitorArchiveWidget* widget = m_archiveList[archiveName];
	assert(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorArchive::startNewWidget(MonitorConfigController* configController, const std::vector<AppSignalParam>& appSignals, QWidget* parent)
{
	MonitorArchiveWidget* window = new MonitorArchiveWidget(configController, parent);

	window->setSignals(appSignals);

	window->show();

	return false;
}

void MonitorArchive::registerWindow(QString name, MonitorArchiveWidget* window)
{
	assert(m_archiveList.count(name) == 0);
	m_archiveList[name] = window;

	return;
}

void MonitorArchive::unregisterWindow(QString name)
{
	assert(m_archiveList.count(name) == 1);
	m_archiveList.erase(name);

	return;
}

MonitorArchiveWidget::MonitorArchiveWidget(MonitorConfigController* configController, QWidget* parent) :
	QMainWindow(parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_archiveService1(configController->configuration().archiveService1),
	m_archiveService2(configController->configuration().archiveService2),
	m_schemasDetais(configController->schemasDetails())
{
	setAttribute(Qt::WA_DeleteOnClose);

static int no = 1;
	QString name = QString("Monitor Archive %1").arg(no++);
	MonitorArchive::registerWindow(name, this);

	setWindowTitle(name);

	setMinimumSize(QSize(750, 400));

	// ToolBar
	//
	m_toolBar = new QToolBar(tr("ToolBar"));
	m_toolBar->setMovable(false);

	m_exportButton = new QPushButton(tr("Export..."));
	m_printButton = new QPushButton(tr("Print..."));
	m_updateButton = new QPushButton(tr("Update"));
	m_signalsButton = new QPushButton(tr("Signals..."));

	m_toolBar->addWidget(m_exportButton);
	m_toolBar->addWidget(m_printButton);
	m_toolBar->addSeparator();

	// Add stretecher
	//
	QWidget* empty = new QWidget();
	empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_toolBar->addWidget(empty);

	m_toolBar->addWidget(m_updateButton);
	m_toolBar->addWidget(m_signalsButton);

	addToolBar(m_toolBar);

	// Status bar
	//
	m_statusBar = new QStatusBar;

	m_statusBarTextLabel = new QLabel(m_statusBar);
	m_statusBarStatesReceivedLabel = new QLabel(m_statusBar);
	m_statusBarNetworkRequestsLabel = new QLabel(m_statusBar);
	m_statusBarServerLabel = new QLabel(m_statusBar);
	m_statusBarConnectionStateLabel = new QLabel(m_statusBar);

	m_statusBar->addWidget(m_statusBarTextLabel, 1);
	m_statusBar->addWidget(m_statusBarStatesReceivedLabel, 0);
	m_statusBar->addWidget(m_statusBarNetworkRequestsLabel, 0);
	m_statusBar->addWidget(m_statusBarServerLabel, 0);
	m_statusBar->addWidget(m_statusBarConnectionStateLabel, 0);

	setStatusBar(m_statusBar);

	// --
	//
	m_source.timeType = static_cast<E::TimeType>(theSettings.m_archiveTimeType);

	m_source.requestEndTime = TimeStamp(QDateTime::currentDateTime());
	m_source.requestStartTime = m_source.requestEndTime.timeStamp - 1_hour;

	// --
	//
	connect(m_updateButton, &QPushButton::clicked, this, &MonitorArchiveWidget::updateButton);
	connect(m_signalsButton, &QPushButton::clicked, this, &MonitorArchiveWidget::signalsButton);

	// Central widget - model/view
	//
	setContentsMargins(2, 2, 2, 2);

	m_view->setModel(m_model);
	setCentralWidget(m_view);

	// Communication thread
	//
	m_tcpClient = new ArchiveTcpClient(configController);
	m_tcpClientThread = new SimpleThread(m_tcpClient);

	m_tcpClientThread->start();

	connect(m_tcpClient, &ArchiveTcpClient::dataReady, this, &MonitorArchiveWidget::dataReceived);
	connect(m_tcpClient, &ArchiveTcpClient::requestError, this, &MonitorArchiveWidget::tcpClientError);
	connect(m_tcpClient, &ArchiveTcpClient::statusUpdate, this, &MonitorArchiveWidget::tcpStatus);

	// --
	//
	setAcceptDrops(true);

	restoreWindowState();

	startTimer(100);

	return;
}

MonitorArchiveWidget::~MonitorArchiveWidget()
{
	MonitorArchive::unregisterWindow(this->windowTitle());

	assert(m_tcpClientThread);
	m_tcpClientThread->quitAndWait(10000);

	return;
}

void MonitorArchiveWidget::ensureVisible()
{
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
	}
}

bool MonitorArchiveWidget::setSignals(const std::vector<AppSignalParam>& appSignals)
{
	m_source.acceptedSignals = appSignals;
	return true;
}


void MonitorArchiveWidget::requestData()
{
	if (m_tcpClient == nullptr ||
		m_tcpClientThread == nullptr)
	{
		assert(m_tcpClient);
		assert(m_tcpClientThread);
		return;
	}

	if (m_tcpClientThread->isRunning() == false)
	{
		assert(m_tcpClientThread->isRunning() == true);
		return;
	}

	if (m_tcpClient->isRequestInProgress() == true)
	{
		cancelRequest();
		assert(m_tcpClient->isRequestInProgress());
	}

	m_tcpClient->requestData(m_source.requestStartTime,
							 m_source.requestEndTime,
							 m_source.timeType,
							 m_source.acceptedSignals);

	return;
}

void MonitorArchiveWidget::cancelRequest()
{
	if (m_tcpClient == nullptr ||
		m_tcpClientThread == nullptr)
	{
		assert(m_tcpClient);
		assert(m_tcpClientThread);
		return;
	}

	if (m_tcpClientThread->isRunning() == false)
	{
		assert(m_tcpClientThread->isRunning() == true);
		return;
	}

	if (m_tcpClient->isRequestInProgress() == false)
	{
		return;
	}

	m_tcpClient->cancelRequest();

	assert(m_tcpClient->isRequestInProgress());

	return;
}

void MonitorArchiveWidget::closeEvent(QCloseEvent*e)
{
	saveWindowState();
	e->accept();
	return;
}

void MonitorArchiveWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void MonitorArchiveWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		assert(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data
	//
	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			auto foundId = std::find_if(m_source.acceptedSignals.begin(), m_source.acceptedSignals.end(),
										[&appSignalParam](const AppSignalParam& sp)
										{
											return sp.appSignalId() == appSignalParam.appSignalId();
										});

			if (foundId == m_source.acceptedSignals.end())
			{
				m_source.acceptedSignals.push_back(appSignalParam);
			}
		}
	}

	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	return;

}

void MonitorArchiveWidget::saveWindowState()
{
	theSettings.m_archiveWindowPos = pos();
	theSettings.m_archiveWindowGeometry = saveGeometry();
	theSettings.m_archiveWindowState = saveState();

	theSettings.writeUserScope();
}

void MonitorArchiveWidget::restoreWindowState()
{
	move(theSettings.m_archiveWindowPos);
	restoreGeometry(theSettings.m_archiveWindowGeometry);
	restoreState(theSettings.m_archiveWindowState);

	ensureVisible();

	return;
}

void MonitorArchiveWidget::updateButton()
{
	if (m_source.acceptedSignals.empty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("To request data from archive add at least one signal."));
		return;
	}

	m_model->clear();
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	requestData();
	return;

}

void MonitorArchiveWidget::signalsButton()
{
	//std::vector<AppSignalParam> appSignals = m_appSignals;

	// --
	//
	DialogChooseArchiveSignals dialog(m_schemasDetais, m_source, this);

	int result = dialog.exec();

	if (result == QDialog::Rejected)
	{
		return;
	}

	m_source = dialog.accpetedResult();

	theSettings.m_archiveTimeType = static_cast<int>(m_source.timeType);

	// Request data from archive
	//
	m_model->clear();
	m_model->setParams(m_source.acceptedSignals, m_source.timeType);

	requestData();

	return;
}

void MonitorArchiveWidget::dataReceived(std::shared_ptr<ArchiveChunk> chunk)
{
	if (chunk == nullptr)
	{
		assert(chunk);
		return;
	}

	m_model->addData(chunk);

	return;
}

void MonitorArchiveWidget::tcpClientError(QString errorMessage)
{
	QMessageBox::critical(this, qAppName(), errorMessage);
	return;
}

void MonitorArchiveWidget::tcpStatus(QString status, int statesReceived, int requestCount, int repliesCount)
{
	assert(m_statusBar);

	m_statusBarTextLabel->setText(status);
	m_statusBarStatesReceivedLabel->setText(QString("States received: %1").arg(statesReceived));

	m_statusBarNetworkRequestsLabel->setText(QString(" Network requests/replies: %1/%2 ").arg(requestCount).arg(repliesCount));

	HostAddressPort server = m_tcpClient->currentServerAddressPort();
	m_statusBarServerLabel->setText(QString(" ArchiveServer: %1 ").arg(server.addressPortStr()));

	if (m_tcpClient->isConnected() == true)
	{
		m_statusBarConnectionStateLabel->setText(" Connected ");
	}
	else
	{
		m_statusBarConnectionStateLabel->setText(" NoConnection ");
	}

	return;
}
