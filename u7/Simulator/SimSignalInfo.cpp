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

		std::optional<Signal> signalExt = simuator->appSignalManager().signalParamExt(appSignalId);

		if (ok == true)
		{
			SimSignalInfo* msi = new SimSignalInfo(signal,
												   signalExt,
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
							 std::optional<Signal> signalExt,
							 SimIdeSimulator* simuator,
							 SimWidget* simWidget):
	DialogSignalInfo(signal,
					 signalExt,
					 &simuator->appSignalManager(),
					 nullptr,
					 false,
					 simWidget),
	m_simuator(simuator)
{
	if (m_simuator == nullptr)
	{
		Q_ASSERT(m_simuator);
		return;
	}

	// Modify UI to simulator requirements
	//
	ui->labelPlantTimeHeader->hide();
	ui->labelPlantTime->hide();

	ui->labelServerTimeHeader->setText(tr("Time"));

	hideTabPage("Action");

	return;
}

void SimSignalInfo::onSignalParamAndUnitsArrived()
{
	// Refresh signal param inself

	bool ok = false;

	AppSignalParam newSignal = m_simuator->appSignalManager().signalParam(signal().hash(), &ok);

	std::optional<Signal> newSignalExt = m_simuator->appSignalManager().signalParamExt(signal().hash());

	if (ok == false)
	{
		//Signal was deleted, keep its #appSignalId and Hash
		//
		AppSignalParam oldSignal = signal();

		newSignal = AppSignalParam();
		newSignal.setAppSignalId(oldSignal.appSignalId());
		newSignal.setHash(oldSignal.hash());

		newSignalExt = {};
	}

	setSignal(newSignal);
	setSignalExt(newSignalExt);

	updateStaticData();

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

void SimSignalInfo::setSchema(QString schemaId, QStringList /*highlightIds*/)
{
	emit openSchema(schemaId);

	return;
}


