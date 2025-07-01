#include "SimSignalSnapshot.h"
#include "SimWidgetPrivate.h"

#include <SimulatorUi/SimIdeSimulator.h>
#include <VFrame30/AppSignalController.h>


namespace SimUi
{
	bool SimDialogSignalSnapshot::showDialog(SimIdeSimulator* simulator,
											 VFrame30::AppSignalController* appSignalController,
											 QString lmEquipmentId,
											 SimWidgetPrivate* simWidget)
	{
		SimDialogSignalSnapshot* dss = new SimDialogSignalSnapshot(simulator,
																   &appSignalController->appSignalManager(),
																   simulator->projectName(),
																   tr("Simulator"),
																   lmEquipmentId,
																   simWidget);

		connect(simulator, &SimIdeSimulator::projectUpdated, dss, &SimDialogSignalSnapshot::projectUpdated);

		connect(dss, &DialogSignalSnapshot::signalContextMenu, simWidget, &SimWidgetPrivate::signalContextMenu);
		connect(dss, &DialogSignalSnapshot::signalInfo, simWidget, &SimWidgetPrivate::signalInfo);

		connect(simWidget, &SimWidgetPrivate::needCloseChildWindows, dss, &QDialog::accept);

		dss->show();

		return true;
	}


	SimDialogSignalSnapshot::SimDialogSignalSnapshot(SimIdeSimulator* simulator,
													 IAppSignalManager* appSignalManager,
													 QString projectName,
													 QString softwareEquipmentId,
													 QString lmEquipmentId,
													 QWidget* parent) :
		DialogSignalSnapshot(appSignalManager, nullptr, &simulator->appSignalListSet(), {}, projectName, softwareEquipmentId, parent),
		m_simulator(simulator)
	{
		if (m_simulator == nullptr)
		{
			Q_ASSERT(m_simulator);
			return;
		}

		resetSignalsType();

		setSignalsTags({});

		if (lmEquipmentId.isEmpty() == true)
		{
			setSignalsMask({});
		}
		else
		{
			setLmEquipmentId(lmEquipmentId);
		}

		return;
	}

	void SimDialogSignalSnapshot::projectUpdated()
	{
		setProjectName(m_simulator->projectName());

		signalsUpdated();

		return;
	}

	std::vector<VFrame30::SchemaDetails> SimDialogSignalSnapshot::schemasDetails()
	{
		return m_simulator->schemasDetails();
	}

	std::set<QString> SimDialogSignalSnapshot::schemaAppSignals(const QString& schemaStrId)
	{
		if (schemaStrId.isEmpty() == false)
		{
			return m_simulator->schemaAppSignals(schemaStrId);
		}

		return std::set<QString>();
	}
} // namespace SimUi