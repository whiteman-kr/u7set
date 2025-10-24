#include "MonitorSignalInfo.h"
#include "MonitorCentralWidget.h"
#include "MonitorConfigController.h"
#include "ui_DialogSignalInfo.h"

#include <UiLib/UiTools.h>

#include <ClientLib/AppSignalManager.h>


bool MonitorSignalInfo::showDialog(QString appSignalId,
								   ClientLib::AppSignalManager& appSignalManager,
								   ITuningSignalManager& tuningSignalManager,
								   ITuningConnection& tuningConnection,
								   ITuningAuthorization& tuningAuthorization,
								   MonitorConfigController* configController,
								   MonitorCentralWidget* centralWidget)
{
	Q_ASSERT(configController);
	Q_ASSERT(centralWidget);

	if (appSignalId.startsWith('@') == true)
	{
		auto s = appSignalManager.signalParamByEquipmentId(appSignalId);

		if (s.has_value() == true)
		{
			appSignalId = s->appSignalId();
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
		auto signal = appSignalManager.signalParam(appSignalId);

		if (signal.has_value() == true)
		{
			bool tuningEnabled = configController->configurationTuningEnabled();

			MonitorSignalInfo* msi = new MonitorSignalInfo(*signal,
														   configController,
														   appSignalManager,
														   &appSignalManager,
														   tuningSignalManager,
														   tuningConnection,
														   tuningAuthorization,
														   tuningEnabled,
														   centralWidget);

			connect(&appSignalManager,
					&ClientLib::AppSignalManager::signalParamsUpdated,
					msi,
					&MonitorSignalInfo::onSignalParamAndUnitsArrived);

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
									 IAppSignalManager& appSignalManager,
									 ClientLib::ISignalDataServer* signalDataServer,
									 ITuningSignalManager& tuningSignalManager,
									 ITuningConnection& tuningConnection,
									 ITuningAuthorization& tuningAuthorization,
									 bool tuningEnabled,
									 MonitorCentralWidget* centralWidget) :
	DialogSignalInfo(signal,
					 &appSignalManager,
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
	setTuningEnabled(m_configController->configurationTuningEnabled());

	// Refresh signal param itself
	//
	auto newSignal = signalManager()->signalParam(signal().hash());

	if (newSignal.has_value() == false)
	{
		// Signal was deleted, keep its #appSignalId and Hash
		//
		AppSignalParam oldSignal = signal();

		newSignal = AppSignalParam{};
		newSignal->setAppSignalId(oldSignal.appSignalId());
		newSignal->setHash(oldSignal.hash());
	}

	updateSignal(*newSignal);

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
