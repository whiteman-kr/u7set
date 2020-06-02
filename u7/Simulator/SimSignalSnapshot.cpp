#include "SimSignalSnapshot.h"
#include "SimWidget.h"
#include "SimIdeSimulator.h"

bool SimDialogSignalSnapshot::showDialog(SimIdeSimulator* simuator,
										 VFrame30::AppSignalController* appSignalController,
										 SimWidget* simWidget)
{

	SimDialogSignalSnapshot* dss = new SimDialogSignalSnapshot(simuator,
															   appSignalController->appSignalManager(),
															   simuator->projectName(),
															   tr("Simulator"),
															   simWidget);

	connect(simuator, &SimIdeSimulator::projectUpdated, dss, &SimDialogSignalSnapshot::projectUpdated);
	connect(simuator, &SimIdeSimulator::schemaDetailsUpdated, dss, &SimDialogSignalSnapshot::schemasUpdated);

	connect(dss, &DialogSignalSnapshot::signalContextMenu, simWidget, &SimWidget::signalContextMenu);
	connect(dss, &DialogSignalSnapshot::signalInfo, simWidget, &SimWidget::signalInfo);

	connect(simWidget, &SimWidget::needCloseChildWindows, dss, &QDialog::accept);

	dss->show();

	return true;
}


SimDialogSignalSnapshot::SimDialogSignalSnapshot(SimIdeSimulator* simuator,
												 IAppSignalManager* appSignalManager,
												 QString projectName,
												 QString softwareEquipmentId,
												 QWidget *parent)
	:DialogSignalSnapshot(appSignalManager, projectName, softwareEquipmentId, parent),
	  m_simuator(simuator)
{
	if (m_simuator == nullptr)
	{
		Q_ASSERT(m_simuator);
		return;
	}
}

void SimDialogSignalSnapshot::projectUpdated()
{
	setProjectName(m_simuator->projectName());

	signalsUpdated();

	if (m_simuator->isLoaded() == false)
	{
		schemasUpdated();
	}

	return;
}

std::vector<VFrame30::SchemaDetails> SimDialogSignalSnapshot::schemasDetails()
{
	return m_simuator->schemasDetails();
}

std::set<QString> SimDialogSignalSnapshot::schemaAppSignals(const QString& schemaStrId)
{
	if (schemaStrId.isEmpty() == false)
	{
		return m_simuator->schemaAppSignals(schemaStrId);
	}

	return std::set<QString>();
}

