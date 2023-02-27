#include "MonitorSignalSnapshot.h"
#include "MonitorConfigController.h"

MonitorDialogSignalSnapshot* MonitorDialogSignalSnapshot::createDialog(MonitorConfigController *configController,
											 MonitorSignalManager* monitorSignalManager,
											 MonitorCentralWidget* centralWidget)
{
	if (configController == nullptr || monitorSignalManager == nullptr || centralWidget == nullptr)
	{
		Q_ASSERT(configController);
		Q_ASSERT(monitorSignalManager);
		Q_ASSERT(centralWidget);
		return nullptr;
	}

	MonitorDialogSignalSnapshot* dss = new MonitorDialogSignalSnapshot(configController,
																	   monitorSignalManager,
																	   centralWidget);

	connect(dss, &DialogSignalSnapshot::signalContextMenu, centralWidget, &MonitorCentralWidget::slot_signalContextMenu);
	connect(dss, &DialogSignalSnapshot::signalInfo, centralWidget, &MonitorCentralWidget::slot_signalInfo);

	connect(monitorSignalManager, &MonitorSignalManager::signalParamsUpdated, dss, &MonitorDialogSignalSnapshot::signalsUpdated);
	connect(configController, &MonitorConfigController::configurationUpdate, dss, &MonitorDialogSignalSnapshot::schemasUpdated);

	return dss;
}


MonitorDialogSignalSnapshot::MonitorDialogSignalSnapshot(MonitorConfigController *configController,
														 MonitorSignalManager* monitorSignalManager,
														 QWidget *parent)
	:DialogSignalSnapshot(monitorSignalManager,
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

	return {};
}

