#include "MainWindow.h"
#include "TuningSchemaWidget.h"
#include "../VFrame30/MonitorSchema.h"

//
//TuningClientTuningController
//

TuningClientTuningController::TuningClientTuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QWidget* parent):
	VFrame30::TuningController(signalManager, tcpClient, parent)
{

}


bool TuningClientTuningController::checkTuningAccess() const
{
	if (theMainWindow->userManager()->login(theMainWindow) == false)
	{
		return false;
	}

	return true;
}

//
//TuningSchemaWidget
//

TuningSchemaWidget::TuningSchemaWidget(TuningSignalManager* tuningSignalManager,
									   TuningClientTuningController* tuningController,
									   VFrame30::LogController* logController,
									   std::shared_ptr<VFrame30::Schema> schema,
                                       TuningSchemaManager* schemaManager,
                                       QWidget* parent) :
    VFrame30::ClientSchemaWidget(new TuningSchemaView{schemaManager}, schema, schemaManager, parent)
{
	assert(tuningSignalManager);
	assert(tuningController);
	assert(schemaManager);

	Q_UNUSED(tuningSignalManager);

	clientSchemaView()->setTuningController(tuningController);
	clientSchemaView()->setLogController(logController);
	clientSchemaView()->setZoom(100, false);

	return;
}

TuningSchemaWidget::~TuningSchemaWidget()
{
}

