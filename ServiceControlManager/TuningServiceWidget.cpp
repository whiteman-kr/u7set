#include "TuningServiceWidget.h"
#include "Brush.h"

TuningServiceWidget::TuningServiceWidget(ServiceTableModel* srvTableModel,
	const SoftwareInfo& softwareInfo,
	const ServiceData& serviceData,
	quint32 ip, quint16 tcpPort,
	QWidget* parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, serviceData, ip, tcpPort, parent)
{
}

TuningServiceWidget::~TuningServiceWidget()
{
}

void TuningServiceWidget::initWidget()
{
	addGeneralTab();
	addClientsTab();
	addTuningSourcesTab();
}

void TuningServiceWidget::updateDerivedWidgets(const Network::ServiceInfo& srvInfo)
{
	updateModels(srvInfo);
}

void TuningServiceWidget::clearDerivedWidgets()
{
	Network::ServiceInfo clearSrvInfo;

	updateModels(clearSrvInfo);
}

int TuningServiceWidget::updateSrvStatus(int rowCount)
{
	return rowCount;
}

int TuningServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		return rowCount;
	}

	std::shared_ptr<TuningServiceSettings> st = std::dynamic_pointer_cast<TuningServiceSettings>(m_serviceData.settings);

	TEST_PTR_RETURN_VALUE(st, rowCount);

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID1);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP1);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID1.isEmpty() ? Separator::EMPTY_STR :
							 st->cfgServiceIP1.toString());
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceEquipmentID2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->cfgServiceID2);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), m_cfgServiceIP2);
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->cfgServiceID2.isEmpty() ? Separator::EMPTY_STR :
							 st->cfgServiceIP2.toString());
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ClientRequestIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 clientRequestIpInfoStr(st->securityLevel, st->clientRequestIP, st->clientRequestNetmask));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("Single LM control"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->singleLmControl ? "Yes" : "No");
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("Disable modules type checking"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 st->disableModulesTypeChecking ? "Yes" : "No");
	rowCount++;

	return rowCount;
}

void TuningServiceWidget::forgetWidget(QString dataSourceID)
{
	m_sourceWidgets.erase(dataSourceID);
}

void TuningServiceWidget::addTuningSourcesTab()
{
	m_sourcesModel = new TuningSourcesModel(this);
	m_sourcesView = createTableView(m_sourcesModel, m_sourcesModel->columns());

	connect(m_sourcesView, &QTableView::doubleClicked, this, &TuningServiceWidget::onSourceDoubleClicked);

	addTab(m_sourcesView, "Tuning sources");
}

void TuningServiceWidget::onSourceDoubleClicked(const QModelIndex& index)
{
	int row = index.row();

	QString sourceEquipmentID = m_sourcesModel->getSourceEquipmentID(row);

	TuningSourceWidget* srcWidget = getValueOrNullptr(m_sourceWidgets, sourceEquipmentID);

	if (srcWidget == nullptr)
	{
		srcWidget = new TuningSourceWidget(sourceEquipmentID, this);

		m_sourceWidgets.emplace(sourceEquipmentID, srcWidget);

		srcWidget->show();

		connect(srcWidget, &TuningSourceWidget::forgetMe, this, &TuningServiceWidget::forgetWidget);
	}
	else
	{
		srcWidget->activateWindow();
	}
}


void TuningServiceWidget::updateModels(const Network::ServiceInfo& srvInfo)
{
	if (m_sourcesModel != nullptr)
	{
		m_sourcesModel->updateData(srvInfo);
	}

	if (m_sourceWidgets.size() > 0)
	{
		int srcCount = srvInfo.tuningsourcesinfostate_size();

		for(int i = 0; i < srcCount; i++)
		{
			const Network::TuningSourceInfoState& infoState = srvInfo.tuningsourcesinfostate(i);

			TuningSourceWidget* w = getValueOrNullptr(m_sourceWidgets,
										QString::fromStdString(infoState.info().moduleequipmentid()));

			if (w != nullptr)
			{
				w->updateData(infoState);
			}
		}
	}

}
