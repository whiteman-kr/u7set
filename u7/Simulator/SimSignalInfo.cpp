#include "SimSignalInfo.h"
#include "SimWidget.h"
#include "SimIdeSimulator.h"
#include "../lib/Ui/UiTools.h"
#include "ui_DialogSignalInfo.h"

bool SimSignalInfo::showDialog(QString appSignalId,
							   SimIdeSimulator* simuator,
							   SimWidget* simWidget)
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

		AppSignalParam signal = simuator->appSignalManager().signalParam(appSignalId, &ok);

		if (ok == true)
		{
			SimSignalInfo* msi = new SimSignalInfo(signal,
												   simuator,
												   simWidget);

			connect(simuator, &SimIdeSimulator::projectUpdated, msi, &SimSignalInfo::onSignalParamAndUnitsArrived);

			connect(simWidget, &SimWidget::needCloseChildWindows, msi, &QDialog::accept);

			connect(msi, &SimSignalInfo::openSchema, simWidget, &SimWidget::openSchemaTabPage);

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

SimSignalInfo::SimSignalInfo(const AppSignalParam& signal,
							 SimIdeSimulator* simuator,
							 SimWidget* simWidget):
	DialogSignalInfo(signal,
					 &simuator->appSignalManager(),
					 nullptr,
					 false/*tuningEnabled*/,
					 DialogType::Simulator,
					 simWidget),
	m_simuator(simuator)
{
	if (m_simuator == nullptr)
	{
		Q_ASSERT(m_simuator);
		return;
	}

	return;
}

void SimSignalInfo::onSignalParamAndUnitsArrived()
{
	// Refresh signal param inself

	bool ok = false;

	AppSignalParam newSignal = m_simuator->appSignalManager().signalParam(signal().hash(), &ok);

	if (ok == false)
	{
		//Signal was deleted, keep its #appSignalId and Hash
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
	if (m_simuator == nullptr)
	{
		Q_ASSERT(m_simuator);
		return {};
	}

	return m_simuator->schemasByAppSignalId(appSignalId);
}

void SimSignalInfo::setSchema(QString schemaId, QStringList highlightIds)
{
	emit openSchema(schemaId, highlightIds);
	return;
}

std::optional<AppSignal> SimSignalInfo::getSignalExt(const AppSignalParam& appSignalParam)
{
	return m_simuator->appSignalManager().signalParamExt(appSignalParam.hash());
}
