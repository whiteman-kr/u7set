#include "MonitorSignalSnapshot.h"
#include "MonitorConfigController.h"

bool MonitorDialogSignalSnapshot::showDialog(MonitorConfigController *configController,
					   TcpSignalClient* tcpSignalClient,
					   AppSignalManager* appSignalManager,
					   QString projectName,
					   QString softwareEquipmentId,
					   MonitorCentralWidget* centralWidget)
{

	MonitorDialogSignalSnapshot* dss = new MonitorDialogSignalSnapshot(configController,
														 appSignalManager,
														 projectName,
														 softwareEquipmentId,
														 centralWidget);

	connect(dss, &DialogSignalSnapshot::signalContextMenu, centralWidget, &MonitorCentralWidget::slot_signalContextMenu);
	connect(dss, &DialogSignalSnapshot::signalInfo, centralWidget, &MonitorCentralWidget::slot_signalInfo);

	connect(tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, dss, &MonitorDialogSignalSnapshot::signalsUpdated);
	connect(configController, &MonitorConfigController::configurationUpdate, dss, &MonitorDialogSignalSnapshot::schemasUpdated);

	dss->show();

	return true;
}


MonitorDialogSignalSnapshot::MonitorDialogSignalSnapshot(MonitorConfigController *configController,
							  AppSignalManager* appSignalManager,
							  QString projectName,
							  QString softwareEquipmentId,
							  QWidget *parent)
	:DialogSignalSnapshot(appSignalManager, projectName, softwareEquipmentId, parent),
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

