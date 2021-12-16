#include "MonitorSignalSnapshot.h"
#include "MonitorConfigController.h"

MonitorDialogSignalSnapshot* MonitorDialogSignalSnapshot::createDialog(MonitorConfigController *configController,
											 TcpSignalClient* tcpSignalClient,
											 AppSignalManager* appSignalManager,
											 MonitorCentralWidget* centralWidget)
{
	if (configController == nullptr || tcpSignalClient == nullptr || appSignalManager == nullptr || centralWidget == nullptr)
	{
		Q_ASSERT(configController);
		Q_ASSERT(tcpSignalClient);
		Q_ASSERT(appSignalManager);
		Q_ASSERT(centralWidget);
		return nullptr;
	}

	MonitorDialogSignalSnapshot* dss = new MonitorDialogSignalSnapshot(configController,
																	   appSignalManager,
																	   centralWidget);

	connect(dss, &DialogSignalSnapshot::signalContextMenu, centralWidget, &MonitorCentralWidget::slot_signalContextMenu);
	connect(dss, &DialogSignalSnapshot::signalInfo, centralWidget, &MonitorCentralWidget::slot_signalInfo);

	connect(tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, dss, &MonitorDialogSignalSnapshot::signalsUpdated);
	connect(configController, &MonitorConfigController::configurationUpdate, dss, &MonitorDialogSignalSnapshot::schemasUpdated);

	return dss;
}


MonitorDialogSignalSnapshot::MonitorDialogSignalSnapshot(MonitorConfigController *configController,
														 AppSignalManager* appSignalManager,
														 QWidget *parent)
	:DialogSignalSnapshot(appSignalManager,
						  configController->configuration().project,
						  configController->configuration().softwareEquipmentId,
						  parent),
	  m_configController(configController)
{
	if (m_configController == nullptr)
	{
		Q_ASSERT(m_configController);
		return;
	}
}

std::vector<VFrame30::SchemaDetails> MonitorDialogSignalSnapshot::schemasDetails()
{
	return m_configController->schemasDetails();
}

std::set<QString> MonitorDialogSignalSnapshot::schemaAppSignals(const QString& schemaStrId)
{
	if (schemaStrId.isEmpty() == false)
	{
		return m_configController->schemaAppSignals(schemaStrId);
	}

	return std::set<QString>();
}

