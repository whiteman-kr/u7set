#include "MonitorSignalSnapshot.h"
#include "MonitorConfigController.h"
#include "MonitorCentralWidget.h"

#include <ClientLib/AppSignalManager.h>

MonitorDialogSignalSnapshot* MonitorDialogSignalSnapshot::createDialog(MonitorConfigController *configController,
											 ClientLib::AppSignalManager* monitorAppSignalManager,
											 MonitorCentralWidget* centralWidget)
{
	if (configController == nullptr || monitorAppSignalManager == nullptr || centralWidget == nullptr)
	{
		Q_ASSERT(configController);
		Q_ASSERT(monitorAppSignalManager);
		Q_ASSERT(centralWidget);
		return nullptr;
	}

	MonitorDialogSignalSnapshot* dss = new MonitorDialogSignalSnapshot(configController,
																	   monitorAppSignalManager,
																	   monitorAppSignalManager,
																	   configController->configInfo().project,
																	   configController->configInfo().softwareEquipmentId,
																	   centralWidget);

	connect(dss, &SchemaClientLib::DialogSignalSnapshot::signalContextMenu, centralWidget, &MonitorCentralWidget::slot_signalContextMenu);
	connect(dss, &SchemaClientLib::DialogSignalSnapshot::signalInfo, centralWidget, &MonitorCentralWidget::slot_signalInfo);

	connect(monitorAppSignalManager, &ClientLib::AppSignalManager::signalParamsUpdated, dss, &MonitorDialogSignalSnapshot::signalsUpdated);
	connect(configController, &MonitorConfigController::configurationUpdated, dss, &MonitorDialogSignalSnapshot::schemasUpdated);

	return dss;
}


MonitorDialogSignalSnapshot::MonitorDialogSignalSnapshot(MonitorConfigController* configController,
														 IAppSignalManager* appSignalManager,
														 ISignalDataServer* signalDataServer,
														 const QString& projectName,
														 const QString& equipmentId,
														 QWidget *parent) :
	SchemaClientLib::DialogSignalSnapshot(appSignalManager,
						 signalDataServer,
						 configController->configuration().appDataServices,
						 projectName,
						 equipmentId,
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

