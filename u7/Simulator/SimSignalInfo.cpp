#include "SimSignalInfo.h"
#include "SimWidget.h"
#include "SimIdeSimulator.h"
#include "../lib/Ui/UiTools.h"

bool SimSignalInfo::showDialog(QString appSignalId,
							   SimIdeSimulator* simuator,
							   IAppSignalManager* appSignalManager,
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
		AppSignalParam signal = appSignalManager->signalParam(appSignalId, &ok);

		if (ok == true)
		{
			SimSignalInfo* msi = new SimSignalInfo(signal,
												   simuator,
												   appSignalManager,
												   simWidget);

			connect(simuator, &SimIdeSimulator::projectUpdated, msi, &DialogSignalInfo::onSignalParamAndUnitsArrived);

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
							 IAppSignalManager* appSignalManager,
							 SimWidget* simWidget):
	DialogSignalInfo(signal, appSignalManager, nullptr, false, simWidget),
	m_simuator(simuator)
{
	if (m_simuator == nullptr)
	{
		Q_ASSERT(m_simuator);
		return;
	}

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

void SimSignalInfo::setSchema(QString schemaId, QStringList /*highlightIds*/)
{
	emit openSchema(schemaId);

	return;
}


