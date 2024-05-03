#include "DialogDataSources.h"

#include "../UtilsLib/Ui/UiTools.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"

#include <SchemaClientLib/TuningSourcesWidget.h>
#include <SchemaClientLib/AppDataSourcesWidget.h>


void DialogDataSources::create(const TestSuite::TestSuiteConfigController& configController,
							   ILogFile* logFile,
							   QWidget* parent)
{
	if (s_dialogDataSources == nullptr)
	{
		s_dialogDataSources = new DialogDataSources(configController, logFile, parent);
		s_dialogDataSources->show();
	}
	else
	{
		s_dialogDataSources->activateWindow();
	}

	UiTools::adjustDialogPlacement(s_dialogDataSources);

	return;
}

DialogDataSources::DialogDataSources(const TestSuite::TestSuiteConfigController& configController,
									 ILogFile* logFile,
									 QWidget* parent) :
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
	m_appDataSourcesWidget = new SchemaClientLib::AppDataSourcesWidget(m_tcpSignalClientCtrl, this);
	m_mainLayout->addWidget(m_appDataSourcesWidget);

	// TuningSourcesWidget
	//
	m_tuningSourcesLabel = new QLabel(tr("Tuning Data Sources"));
	m_mainLayout->addWidget(m_tuningSourcesLabel);

	m_tuningSourcesWidget = new SchemaClientLib::TuningSourcesWidget(m_tcpTuningCtrl, false/*hasActivationControls*/, this);
	m_mainLayout->addWidget(m_tuningSourcesWidget);

	if (m_configController.configurationTuningEnabled() == false)
	{
		m_tuningSourcesLabel->setVisible(false);
		m_tuningSourcesWidget->setVisible(false);
	}

	connect(&m_configController, &TestSuite::TestSuiteConfigController::configurationArrived, this, &DialogDataSources::slot_configurationArrived);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	m_mainLayout->addLayout(buttonsLayout);

	QPushButton* detailsButton = new QPushButton(tr("Details..."));
	buttonsLayout->addWidget(detailsButton);
	detailsButton->setFocusPolicy(Qt::NoFocus);
	connect(detailsButton, &QPushButton::clicked, this, &DialogDataSources::detailsClicked);

	buttonsLayout->addStretch();

	QPushButton* closeButton = new QPushButton(tr("Close"));
	buttonsLayout->addWidget(closeButton);
	closeButton->setFocusPolicy(Qt::NoFocus);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

	// --
	//
	setLayout(m_mainLayout);

	if (m_configController.configurationTuningEnabled() == true)
	{
		setMinimumSize(1150, 500);
	}
	else
	{
		setMinimumSize(1150, 300);
	}

	// --
	//
	m_tcpSignalClientCtrl.updateConnections(m_configController.softwareInfo(),
											m_configController.configuration().appDataServices);

	m_tcpTuningCtrl.updateConnections(m_configController.softwareInfo(),
									  m_configController.configuration().tuningServices,
									  true,
									  TuningClientSettings::LmStatusFlagMode::None);
	return;
}

DialogDataSources::~DialogDataSources()
{
	Q_ASSERT(s_dialogDataSources);
	s_dialogDataSources = nullptr;

}

void DialogDataSources::slot_configurationArrived()
{
	m_tcpSignalClientCtrl.updateConnections(m_configController.softwareInfo(),
											m_configController.configuration().appDataServices);
	
	m_tcpTuningCtrl.updateConnections(m_configController.softwareInfo(),
									  m_configController.configuration().tuningServices,
									  true,
									  TuningClientSettings::LmStatusFlagMode::None);

	m_tuningSourcesLabel->setVisible(m_configController.configurationTuningEnabled());
	m_tuningSourcesWidget->setVisible(m_configController.configurationTuningEnabled());

	if (m_configController.configurationTuningEnabled() == true)
	{
		setMinimumSize(1100, 500);
	}
	{
		setMinimumSize(1100, 300);
	}

	return;
}

void DialogDataSources::detailsClicked()
{
	if (m_tuningSourcesWidget->treeIsFocused())
	{
		m_tuningSourcesWidget->detailsClicked();
	}
	else
	{
		m_appDataSourcesWidget->detailsClicked();
	}
}
