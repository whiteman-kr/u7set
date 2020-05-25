#include "MonitorSignalInfo.h"
#include "MonitorCentralWidget.h"
#include "TcpSignalClient.h"
#include "../lib/Ui/UiTools.h"

bool MonitorSignalInfo::showDialog(QString appSignalId, MonitorConfigController* configController, TcpSignalClient* tcpSignalClient, MonitorCentralWidget* centralWidget)
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
		AppSignalParam signal = theSignals.signalParam(appSignalId, &ok);

		if (ok == true)
		{
			bool tuningEnabled = configController->configuration().tuningEnabled == true;

			MonitorSignalInfo* msi = new MonitorSignalInfo(signal,
														   configController,
														   &theSignals,
														   centralWidget->tuningController(),
														   tuningEnabled,
														   centralWidget);

			connect(tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, msi, &DialogSignalInfo::onSignalParamAndUnitsArrived);

			msi->show();
			msi->raise();
			msi->activateWindow();

			DialogSignalInfo::registerDialog(appSignalId, msi);
		}
		else
		{
			QMessageBox::critical(centralWidget, qAppName(), tr("Signal %1 not found.").arg(appSignalId));
			return false;
		}
	}

	return true;
}

MonitorSignalInfo::MonitorSignalInfo(const AppSignalParam& signal,
									 MonitorConfigController* configController,
									 IAppSignalManager* appSignalManager,
									 VFrame30::TuningController* tuningController, bool tuningEnabled,
									 MonitorCentralWidget* centralWidget):
	DialogSignalInfo(signal, appSignalManager, tuningController, tuningEnabled, centralWidget),
	m_configController(configController),
	m_centralWidget(centralWidget)
{
	if (m_configController == nullptr || m_centralWidget == nullptr)
	{
		Q_ASSERT(m_configController);
		Q_ASSERT(m_centralWidget);
		return;
	}

}

QStringList MonitorSignalInfo::schemasByAppSignalId(const QString& appSignalId)
{
	return m_configController->schemasByAppSignalId(appSignalId);
}

void MonitorSignalInfo::setSchema(QString schemaId, QStringList highlightIds)
{
	MonitorSchemaWidget* currentTab = m_centralWidget->currentTab();
	if (currentTab == nullptr)
	{
		Q_ASSERT(currentTab);
		return;
	}

	if (currentTab->schemaId() != schemaId)
	{
		currentTab->setSchema(schemaId, highlightIds);
	}

	return;
}


/*std::vector<VFrame30::SchemaDetails> MonitorSignalInfo::schemasDetails()
{
	return m_configController->schemasDetails();
}

std::set<QString> MonitorSignalInfo::schemaAppSignals(const QString& schemaStrId)
{
	if (schemaStrId.isEmpty() == false)
	{
		return m_configController->schemaAppSignals(schemaStrId);
	}

	return std::set<QString>();
}*/
