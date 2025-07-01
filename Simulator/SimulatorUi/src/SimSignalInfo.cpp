#include "SimSignalInfo.h"
#include "SimWidgetPrivate.h"
#include "ui_DialogSignalInfo.h"

#include "../UtilsLib/Ui/UiTools.h"

#include <SimulatorLib/SimAppSignalManager.h>
#include <SimulatorUi/SimIdeSimulator.h>


namespace SimUi
{
	bool SimSignalInfo::showDialog(QString appSignalId, SimIdeSimulator* simulator, SimWidgetPrivate* simWidget)
	{
		DialogSignalInfo* dsi = DialogSignalInfo::dialogRegistered(appSignalId);

		if (dsi != nullptr)
		{
			dsi->raise();
			dsi->activateWindow();

			UiTools::adjustDialogPlacement(dsi);
		}
		else
		{
			bool ok = false;

			AppSignalParam signal = simulator->appSignalManager().signalParam(appSignalId, &ok);

			if (ok == true)
			{
				SimSignalInfo* msi = new SimSignalInfo(signal, simulator, simWidget);

				connect(simulator, &SimIdeSimulator::projectUpdated, msi, &SimSignalInfo::onSignalParamAndUnitsArrived);

				connect(simWidget, &SimWidgetPrivate::needCloseChildWindows, msi, &QDialog::accept);

				connect(msi, &SimSignalInfo::openSchema, simWidget, &SimWidgetPrivate::openSchemaTabPage);

				msi->show();
				msi->raise();
				msi->activateWindow();

				DialogSignalInfo::registerDialog(appSignalId, msi);
			}
			else
			{
				QMessageBox::critical(simWidget, qAppName(), tr("Signal %1 not found.").arg(appSignalId));
				return false;
			}
		}

		return true;
	}

	SimSignalInfo::SimSignalInfo(const AppSignalParam& signal, SimIdeSimulator* simulator, SimWidgetPrivate* simWidget) :
		DialogSignalInfo(signal,
						 &simulator->appSignalManager(),
						 nullptr /*signalDataServer*/,
						 {} /*appDataServices*/,
						 simulator->tuningSignalManagerInterface(),
						 m_tuningConnection,
						 m_tuningAuthorization,
						 false /*tuningEnabled*/,
						 DialogType::Simulator,

						 simWidget),
		m_simulator(simulator)
	{
		if (m_simulator == nullptr)
		{
			Q_ASSERT(m_simulator);
			return;
		}

		return;
	}

	void SimSignalInfo::onSignalParamAndUnitsArrived()
	{
		// Refresh signal param inself

		bool ok = false;

		AppSignalParam newSignal = m_simulator->appSignalManager().signalParam(signal().hash(), &ok);

		if (ok == false)
		{
			// Signal was deleted, keep its #appSignalId and Hash
			//
			AppSignalParam oldSignal = signal();

			newSignal = AppSignalParam();
			newSignal.setAppSignalId(oldSignal.appSignalId());
			newSignal.setHash(oldSignal.hash());
		}

		updateSignal(newSignal);

		return;
	}

	QStringList SimSignalInfo::schemasByAppSignalId(const QString& appSignalId)
	{
		if (m_simulator == nullptr)
		{
			Q_ASSERT(m_simulator);
			return {};
		}

		return m_simulator->schemasByAppSignalId(appSignalId);
	}

	void SimSignalInfo::setSchema(QString schemaId, QStringList highlightIds)
	{
		emit openSchema(schemaId, highlightIds);
		return;
	}

	std::optional<AppSignal> SimSignalInfo::getSignalExt(const AppSignalParam& appSignalParam)
	{
		return m_simulator->appSignalManager().signalParamExt(appSignalParam.hash());
	}
} // namespace SimUi