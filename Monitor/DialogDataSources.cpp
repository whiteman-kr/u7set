#include "DialogDataSources.h"

DialogDataSources::DialogDataSources(MonitorConfigController* configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	  m_configController(configController),
	  m_logFile(logFile)
{
	if (m_configController == nullptr || m_logFile == nullptr)
	{
		Q_ASSERT(m_configController);
		Q_ASSERT(m_logFile);
		return;
	}

	setWindowTitle(tr("Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	m_mainLayout = new QVBoxLayout();

	QLabel* l = new QLabel(tr("Application Data Sources"));
	m_mainLayout->addWidget(l);

	createAppSourceStateClients();

	// AppDataSourcesWidget
	//
	m_appDataSourcesWidget = new AppDataSourcesWidget(m_tcpSourcesStateClients, m_configController->configuration().tuningEnabled == false /*closeButton*/, this);

	connect(m_appDataSourcesWidget, &AppDataSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

	m_mainLayout->addWidget(m_appDataSourcesWidget);

	// Tuning
	//
	setTuningTcpClients(std::move(tcpTuningClients));

	//
	connect(m_configController, &MonitorConfigController::configurationArrived,
			this, &DialogDataSources::slot_configurationArrived,
			Qt::QueuedConnection);

	// --
	//
	setLayout(m_mainLayout);

	return;
}

DialogDataSources::~DialogDataSources()
{
	deleteAppSourceStateClients();
}

void DialogDataSources::setTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients)
{
	bool showTuningWidget = m_configController->configuration().tuningEnabled;

	if (showTuningWidget == true)
	{
		// Show tuning widget

		if (m_tuningSourcesLabel == nullptr)
		{
			m_tuningSourcesLabel = new QLabel(tr("Tuning Data Sources"));
			m_mainLayout->addWidget(m_tuningSourcesLabel);
		}

		if (m_tuningSourcesWidget == nullptr)
		{
			m_tuningSourcesWidget = new TuningSourcesWidget(std::move(tcpTuningClients), false/*hasActivationControls*/, true/*hasCloseButton*/, this);

			connect(m_tuningSourcesWidget, &TuningSourcesWidget::closeButtonPressed, this, &DialogDataSources::reject);

			m_mainLayout->addWidget(m_tuningSourcesWidget);
		}
		else
		{
			m_tuningSourcesWidget->setTuningTcpClients(std::move(tcpTuningClients));
		}
	}
	else
	{
		// Delete tuning widget

		if (m_tuningSourcesLabel != nullptr)
		{
			delete m_tuningSourcesLabel;
			m_tuningSourcesLabel = nullptr;
		}

		if (m_tuningSourcesWidget != nullptr)
		{
			delete m_tuningSourcesWidget;
			m_tuningSourcesWidget = nullptr;
		}
	}

	m_appDataSourcesWidget->showCloseButton(showTuningWidget == false);

	if (showTuningWidget == true)
	{
		setMinimumSize(1024, 500);
	}
	else
	{
		setMinimumSize(1024, 300);
	}
}

void DialogDataSources::reject()
{
	emit dialogClosed();
	QDialog::reject();
}

void DialogDataSources::slot_configurationArrived(ConfigSettings configuration)
{
	m_appDataSourcesWidget->showCloseButton(configuration.tuningEnabled == false);

	size_t appDataServicesCount = configuration.appDataServices.size();
	if (appDataServicesCount == m_tcpSourcesStateClients.size())
	{
		// AppDataServices count was not changed, just update their addresses
		//
		for (int i = 0; i < appDataServicesCount; i++)
		{
			const MonitorSettings::AppDataService& ads = configuration.appDataServices[i];

			if (m_tcpSourcesStateClients[i]->serverAddressPort(0) != ads.address ||
					m_tcpSourcesStateClients[i]->serverAddressPort(1) != ads.address)
			{
				m_tcpSourcesStateClients[i]->setServers(ads.address, ads.address, true);
			}
		}
	}
	else
	{
		// Re-create TcpAppSourcesState clients because number of AppDataServices was changed
		//

		deleteAppSourceStateClients();
		createAppSourceStateClients();

		m_appDataSourcesWidget->setAppSourceTcpClients(m_tcpSourcesStateClients);
	}

	return;
}

void DialogDataSources::createAppSourceStateClients()
{
	if (m_sourcesStateClientThreads.empty() == false)
	{
		Q_ASSERT(m_sourcesStateClientThreads.empty() == true);
		return;
	}

	for (const MonitorSettings::AppDataService& ads : m_configController->configuration().appDataServices)
	{
		TcpAppSourcesState* client = new TcpAppSourcesState(m_configController->softwareInfo(), ads.address, m_logFile);
		m_tcpSourcesStateClients.push_back(client);

		SimpleThread* thread = new SimpleThread(client);
		m_sourcesStateClientThreads.push_back(thread);

		thread->start();
	}
}

void DialogDataSources::deleteAppSourceStateClients()
{
	for (SimpleThread* thread : m_sourcesStateClientThreads)
	{
		thread->quitAndWait(10000);
	}
	for (SimpleThread* thread : m_sourcesStateClientThreads)
	{
		delete thread;
	}
	m_sourcesStateClientThreads.clear();
	m_tcpSourcesStateClients.clear();
}
