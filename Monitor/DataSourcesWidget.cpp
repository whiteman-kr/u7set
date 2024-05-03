#include "DataSourcesWidget.h"
#include <SchemaClientLib/AppDataSourcesWidget.h>
#include <SchemaClientLib/TuningSourcesWidget.h>

DataSourcesWidget::DataSourcesWidget(const MonitorConfigController& configController,
									 ClientLib::TuningConnection& tuningConnection,
									 ILogFile* logFile,
									 QWidget* parent) :
	QWidget{parent},
	m_configController{configController},
	m_tuningConnection{tuningConnection},
	m_logFile{logFile}
{
	if (m_logFile == nullptr)
	{
		Q_ASSERT(m_logFile);
		return;
	}

	QVBoxLayout* mainLayout = new QVBoxLayout;

	QLabel* l = new QLabel(tr("Application Data Sources"));
	mainLayout->addWidget(l);

	// AppDataSourcesWidget
	//
	m_appDataSourcesWidget = new SchemaClientLib::AppDataSourcesWidget{m_tcpSignalClientCtrl, this};
	mainLayout->addWidget(m_appDataSourcesWidget);

	// TuningSourcesWidget
	//
	m_tuningSourcesLabel = new QLabel{tr("Tuning Data Sources")};
	mainLayout->addWidget(m_tuningSourcesLabel);

	m_tuningSourcesWidget = new SchemaClientLib::TuningSourcesWidget{m_tuningConnection, false /*hasActivationControls*/, this};
	mainLayout->addWidget(m_tuningSourcesWidget);

	if (m_configController.configurationTuningEnabled() == false)
	{
		m_tuningSourcesLabel->setVisible(false);
		m_tuningSourcesWidget->setVisible(false);
	}

	connect(&m_configController, &MonitorConfigController::configurationArrived, this, &DataSourcesWidget::slot_configurationArrived);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonsLayout);

	QPushButton* detailsButton = new QPushButton(tr("Details..."));
	buttonsLayout->addWidget(detailsButton);
	detailsButton->setFocusPolicy(Qt::NoFocus);
	connect(detailsButton, &QPushButton::clicked, this, &DataSourcesWidget::detailsClicked);

	buttonsLayout->addStretch();

	setLayout(mainLayout);

	// --
	//
	m_tcpSignalClientCtrl.updateConnections(m_configController.softwareInfo(), m_configController.configuration().appDataServices);

	return;
}

void DataSourcesWidget::slot_configurationArrived(MonitorConfigSettings configuration)
{
	m_tcpSignalClientCtrl.updateConnections(m_configController.softwareInfo(), configuration.appDataServices);

	m_tuningSourcesLabel->setVisible(configuration.tuningEnabled);
	m_tuningSourcesWidget->setVisible(configuration.tuningEnabled);

	return;
}

void DataSourcesWidget::detailsClicked()
{
	if (m_tuningSourcesWidget->treeIsFocused())
	{
		m_tuningSourcesWidget->detailsClicked();
	}
	else
	{
		m_appDataSourcesWidget->detailsClicked();
	}

	return;
}
