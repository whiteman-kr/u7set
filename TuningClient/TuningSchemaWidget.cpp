#include "MainWindow.h"
#include "TuningSchemaWidget.h"
#include "../VFrame30/MonitorSchema.h"
#include "../lib/Tuning/TuningUserManager.h"


//
// TuningClientTuningController
//
TuningClientTuningController::TuningClientTuningController(ITuningSignalManager* signalManager, TuningUserManager& userManager, std::vector<ITuningTcpClient*> tcpClients, QWidget* parent):
	VFrame30::TuningController(signalManager, tcpClients, parent),
	m_userManager(userManager)
{
}


bool TuningClientTuningController::checkTuningAccess() const
{
	if (m_userManager.login(theMainWindow) == false)
	{
		return false;
	}

	return true;
}

//
// TuningSchemaWidget
//
TuningSchemaWidget::TuningSchemaWidget(TuningConfigController& configController,
									   TuningSignalManager& tuningSignalManager,
									   TuningClientTuningController* tuningController,
									   VFrame30::LogController* logController,
									   std::shared_ptr<VFrame30::Schema> schema,
									   TuningSchemaManager& schemaManager,
									   QWidget* parent) :
	VFrame30::ClientSchemaWidget(new TuningSchemaView{configController, schemaManager}, schema, &schemaManager, parent)
{
	assert(tuningController);

	Q_UNUSED(tuningSignalManager);

	clientSchemaView()->setTuningController(tuningController);
	clientSchemaView()->setLogController(logController);
	clientSchemaView()->setZoom(100, false);

	// Run onShowScript
	//
	Q_ASSERT(schema);
	schema->setContext(VFrame30::Context::create(clientSchemaView()));

	schema->onShowEvent(clientSchemaView()->jsEngine(), clientSchemaView()->logFile());

	return;
}


