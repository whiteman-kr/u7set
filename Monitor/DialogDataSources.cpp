#include "DialogDataSources.h"
#include "../UtilsLib/Ui/UiTools.h"


void DialogDataSources::create(const MonitorConfigController& configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent)
{
	if (s_dialogDataSources == nullptr)
	{
		s_dialogDataSources = new DialogDataSources(configController, std::move(tcpTuningClients), logFile, parent);
		s_dialogDataSources->show();
	}
	else
	{
		s_dialogDataSources->activateWindow();
	}

	UiTools::adjustDialogPlacement(s_dialogDataSources);

	return;
}

void DialogDataSources::updateTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients)
{
	if (s_dialogDataSources != nullptr)
	{
		s_dialogDataSources->setTuningTcpClients(std::move(tcpTuningClients));
	}

	return;
}

DialogDataSources::DialogDataSources(const MonitorConfigController& configController, std::vector<TuningTcpClient*> tcpTuningClients, ILogFile* logFile, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	  m_configController(configController),
	  m_logFile(logFile)
{
	if (m_logFile == nullptr)
	{
		Q_ASSERT(m_logFile);
		return;
	}

	setWindowTitle(tr("Data Sources"));

	setAttribute(Qt::WA_DeleteOnClose);

	m_mainLayout = new QVBoxLayout();

	QLabel* l = new QLabel(tr("Application Data Sources"));
	m_mainLayout->addWidget(l);

	// AppDataSourcesWidget
	//
	m_appDataSourcesWidget = new AppDataSourcesWidget(m_tcpSignalClientCtrl, this);
	m_mainLayout->addWidget(m_appDataSourcesWidget);

	// TuningSourcesWidget
	//
	m_tuningSourcesLabel = new QLabel(tr("Tuning Data Sources"));
	m_mainLayout->addWidget(m_tuningSourcesLabel);

	m_tuningSourcesWidget = new TuningSourcesWidget(std::move(tcpTuningClients), false/*hasActivationControls*/, this);
	m_mainLayout->addWidget(m_tuningSourcesWidget);

	if (m_configController.configuration().tuningEnabled == false)
	{
		m_tuningSourcesLabel->setVisible(false);
		m_tuningSourcesWidget->setVisible(false);
	}

	connect(&m_configController, &MonitorConfigController::configurationArrived, this, &DialogDataSources::slot_configurationArrived);

	// --
	//
	setLayout(m_mainLayout);

	if (m_configController.configuration().tuningEnabled == true)
	{
		setMinimumSize(1024, 500);
	}
	else
	{
		setMinimumSize(1024, 300);
	}

	return;
}

DialogDataSources::~DialogDataSources()
{
	Q_ASSERT(s_dialogDataSources);
	s_dialogDataSources = nullptr;

}

void DialogDataSources::setTuningTcpClients(std::vector<TuningTcpClient*> tcpTuningClients)
{
	m_tuningSourcesWidget->setTuningTcpClients(std::move(tcpTuningClients));
}

void DialogDataSources::slot_configurationArrived(ConfigSettings configuration)
{
	m_tuningSourcesLabel->setVisible(configuration.tuningEnabled);
	m_tuningSourcesWidget->setVisible(configuration.tuningEnabled);

	if (configuration.tuningEnabled == true)
	{
		setMinimumSize(1024, 500);
	}
	{
		setMinimumSize(1024, 300);
	}

	return;
}
