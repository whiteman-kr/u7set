#include "MonitorSignalInfo.h"
#include "MonitorCentralWidget.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "ui_DialogSignalInfo.h"

class MonitorSignalManager;

bool MonitorSignalInfo::showDialog(QString appSignalId,
								   MonitorSignalManager* appSignalManager,
								   ITuningSignalManager& tuningSignalManager,
								   ITuningConnection& tuningConnection,
								   ITuningAuthorization& tuningAuthorization,
								   MonitorConfigController* configController,
								   MonitorCentralWidget* centralWidget)
{
	Q_ASSERT(appSignalManager);
	Q_ASSERT(configController);
	Q_ASSERT(centralWidget);

	if (appSignalId.startsWith('@') == true)
	{
		bool ok = true;
		AppSignalParam s = appSignalManager->signalParamByEquipemntId(appSignalId, &ok);

		if (ok == true)
		{
			appSignalId = s.appSignalId();
		}
	}

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
			bool tuningEnabled = configController->configuration().tuningEnabled == true;

			MonitorSignalInfo* msi = new MonitorSignalInfo(signal,
														   configController,
														   appSignalManager,
														   appSignalManager,
														   tuningSignalManager,
														   tuningConnection,
														   tuningAuthorization,
														   tuningEnabled,
														   centralWidget);

			connect(appSignalManager, &MonitorSignalManager::signalParamsUpdated, msi, &MonitorSignalInfo::onSignalParamAndUnitsArrived);

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
									 ISignalDataServer* signalDataServer,
									 ITuningSignalManager& tuningSignalManager,
									 ITuningConnection& tuningConnection,
									 ITuningAuthorization& tuningAuthorization,
									 bool tuningEnabled,
									 MonitorCentralWidget* centralWidget):
	DialogSignalInfo(signal,
					 appSignalManager,
					 signalDataServer,
					 configController->configuration().appDataServices,
					 tuningSignalManager,
					 tuningConnection,
					 tuningAuthorization,
					 tuningEnabled,
					 DialogType::Monitor,
					 centralWidget),
	m_configController(configController),
	m_centralWidget(centralWidget)
{
	if (m_configController == nullptr || m_centralWidget == nullptr || signalDataServer == nullptr)
	{
		Q_ASSERT(m_configController);
		Q_ASSERT(m_centralWidget);
		Q_ASSERT(signalDataServer);
		return;
	}

	return;
}

void MonitorSignalInfo::onSignalParamAndUnitsArrived()
{
	setTuningEnabled(m_configController->configuration().tuningEnabled);

	// Refresh signal param inself

	bool ok = false;

	AppSignalParam newSignal = signalManager()->signalParam(signal().hash(), &ok);

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

	currentTab->setSchema(schemaId, highlightIds, false);

	return;
}

std::optional<AppSignal> MonitorSignalInfo::getSignalExt(const AppSignalParam& /*appSignalParam*/)
{
	return {};
}
