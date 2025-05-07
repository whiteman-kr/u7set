#include "AppDataServiceWidget.h"
#include "ScmTcpAppDataClient.h"
#include "AppDataSourceWidget.h"
#include "../UtilsLib/Ui/WidgetUtils.h"

#include <QTableView>
#include <QMenu>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

AppDataServiceWidget::AppDataServiceWidget(ServiceTableModel* srvTableModel,
	const SoftwareInfo& softwareInfo,
	const ServiceData& serviceData,
	quint32 ip, quint16 tcpPort,
	QWidget* parent) :
	BaseServiceWidget(srvTableModel, softwareInfo, serviceData, ip, tcpPort, parent)
{
}

AppDataServiceWidget::~AppDataServiceWidget()
{
	for(const auto& [id, widget] : m_sourceWidgets)
	{
		TEST_PTR_CONTINUE(widget);
		delete widget;
	}
}

void AppDataServiceWidget::initWidget()
{
	addGeneralTab();
	addClientsTab();
	addAppDataSourcesTab();
	addArchiveSignalsTab();
}

void AppDataServiceWidget::updateDerivedWidgets(const Network::ServiceInfo& srvInfo)
{
	updateModels(srvInfo);
}

void AppDataServiceWidget::clearDerivedWidgets()
{
	Network::ServiceInfo clearSrvInfo;

	clearSrvInfo.set_archsignalsupdated(true);

	updateModels(clearSrvInfo);
}

void AppDataServiceWidget::forgetWidget(QString dataSourceID)
{
	m_sourceWidgets.erase(dataSourceID);
}

void AppDataServiceWidget::updateModels(const Network::ServiceInfo& srvInfo)
{
	if (m_sourcesModel != nullptr)
	{
		m_sourcesModel->updateData(srvInfo);
	}

	if (m_sourceWidgets.size() > 0)
	{
		int srcCount = srvInfo.appdatasourcesstates_size();

		for(int i = 0; i < srcCount; i++)
		{
			const Network::AppDataSourceState& dsState = srvInfo.appdatasourcesstates(i);

			AppDataSourceWidget* w = getValueOrNullptr(m_sourceWidgets, QString::fromStdString(dsState.lancontrollerid()));

			if (w != nullptr)
			{
				w->updateData(dsState);
			}
		}
	}

	if (m_archSignalsModel != nullptr )
	{
		if (srvInfo.archsignalsupdated() == true)
		{
			m_archSignalsModel->updateData(srvInfo);
		}

		m_archSignalsProgressBar->setValue(srvInfo.archsignalsupdateprogress());
	}
}

void AppDataServiceWidget::addAppDataSourcesTab()
{
	m_sourcesModel = new AppDataSourcesModel(this);
	m_sourcesView = createTableView(m_sourcesModel, m_sourcesModel->columns());

	connect(m_sourcesView, &QTableView::doubleClicked, this, &AppDataServiceWidget::onSourceDoubleClicked);

	addTab(m_sourcesView, "AppData sources");
}

void AppDataServiceWidget::addArchiveSignalsTab()
{
	QWidget* archSignalsWidget = new QWidget;

	//

	m_archSignalsModel = new ArchiveSignalsModel(this);
	m_archSignalsView = createTableView(m_archSignalsModel, m_archSignalsModel->columns());

	m_archSignalsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_archSignalsView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_archSignalsView, &QTableView::customContextMenuRequested,
			this, &AppDataServiceWidget::onCustomContextMenuRequested);

	//

	m_archSignalsProgressBar = new QProgressBar;

	m_archSignalsProgressBar->setRange(0, 100);
	m_archSignalsProgressBar->setStyleSheet(R"(
									QProgressBar {
										border: 2px solid grey;
										border-radius: 5px;
										text-align: center;
										background-color: #eee;
									}

									QProgressBar::chunk {
										background-color: #3498db;
										width: 10px;  /* ширина одного блока */
										margin: 1px;  /* расстояние между блоками */
									})");

	m_archSignalsProgressBar->setFixedHeight(15);

	//

	QVBoxLayout* vBoxLayout = new QVBoxLayout;

	vBoxLayout->addWidget(m_archSignalsView);
	vBoxLayout->addWidget(m_archSignalsProgressBar);

	archSignalsWidget->setLayout(vBoxLayout);

	addTab(archSignalsWidget, "TOP-500 archive signals");
}

int AppDataServiceWidget::updateSettings(int rowCount)
{
	if (m_serviceData.settings == nullptr)
	{
		return rowCount;
	}

	std::shared_ptr<AppDataServiceSettings> st = std::dynamic_pointer_cast<AppDataServiceSettings>(m_serviceData.settings);

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

	for(const RqCtrlSettings& rcs : st->rcSettings)
	{
		m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QString("Request Controller %1").arg(rcs.ID()));
		m_settingsModel->setData(m_settingsModel->index(rowCount, 1), rqCtrlInfoStr(rcs));
		rowCount++;
	}

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("AppDataReceivingIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1),
							 QString("%1, Netmask = %2").arg(st->appDataReceivingIP.toString()).arg(st->appDataReceivingNetmask.toString()));
	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ArchiveServiceEquipmentID"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->archServiceID);

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("ArchiveServiceIP"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->archServiceIP.toString());

	rowCount++;

	m_settingsModel->setData(m_settingsModel->index(rowCount, 0), QStringLiteral("AutoArchiveInterval (min)"));
	m_settingsModel->setData(m_settingsModel->index(rowCount, 1), st->autoArchiveInterval);

	rowCount++;

	return rowCount;
}

void AppDataServiceWidget::onSourceDoubleClicked(const QModelIndex& index)
{
	int row = index.row();

	QString lanControllerID = m_sourcesModel->getSourceLanControllerID(row);

	AppDataSourceWidget* srcWidget = getValueOrNullptr(m_sourceWidgets, lanControllerID);

	if (srcWidget == nullptr)
	{
		srcWidget = new AppDataSourceWidget(lanControllerID, this);

		m_sourceWidgets.emplace(lanControllerID, srcWidget);

		srcWidget->show();

		connect(srcWidget, &AppDataSourceWidget::forgetMe, this, &AppDataServiceWidget::forgetWidget);
	}
	else
	{
		srcWidget->activateWindow();
	}
}

void AppDataServiceWidget::onCustomContextMenuRequested(const QPoint& pos)
{
	m_selectedRows.clear();

	QModelIndex index = m_archSignalsView->indexAt(pos);

	if (index.isValid() == false)
	{
		return;
	}

	QModelIndexList selectedIndexes = m_archSignalsView->selectionModel()->selectedRows();

	m_selectedRows.reserve(selectedIndexes.size());

	for(const QModelIndex& indx : selectedIndexes)
	{
		m_selectedRows.push_back(indx.row());
	}

	QMenu menu;

	QAction* changeAperturesAction = new QAction("Change aperture(s)",&menu);

	menu.addAction(changeAperturesAction);

	connect(changeAperturesAction, &QAction::triggered,
			this, &AppDataServiceWidget::onChangeApertures);

	menu.exec(m_archSignalsView->viewport()->mapToGlobal(pos));
}

void AppDataServiceWidget::onChangeApertures()
{
	// init dialog parameters

	QListWidget* signalsList = new QListWidget;

	QStringList appSignalIDs;
	QStringList discreteAppSignalIDs;
	std::optional<int> apertureType;
	std::optional<double> coarseAperture;
	std::optional<double> fineAperture;

	int index = 0;

	for(int row : m_selectedRows)
	{
		const Network::ArchSignalInfo& asi = m_archSignalsModel->at(row);

		QString appSignalID = QString::fromStdString(asi.appsignalid());

		if (asi.signaltype() == TO_INT(E::SignalType::Discrete))
		{
			discreteAppSignalIDs.append(appSignalID);
			continue;
		}

		appSignalIDs.append(appSignalID);

		signalsList->addItem(appSignalID);

		if (index == 0)
		{
			apertureType = asi.aperturetype();
			coarseAperture = asi.coarseaperture();
			fineAperture = asi.fineaperture();
		}
		else
		{
			if (apertureType.has_value() && apertureType.value() != asi.aperturetype())
			{
				apertureType.reset();
			}

			if (coarseAperture.has_value() && coarseAperture.value() != asi.coarseaperture())
			{
				coarseAperture.reset();
			}

			if (fineAperture.has_value() && fineAperture.value() != asi.fineaperture())
			{
				fineAperture.reset();
			}
		}

		index++;
	}

	if (discreteAppSignalIDs.isEmpty() == false)
	{
		QString msg;

		msg.append("It is not possible to change the aperture of discrete signal(s):\n\n");

		int rest = 0;

		if (discreteAppSignalIDs.size() > 5)
		{
			rest = discreteAppSignalIDs.size() - 5;
			discreteAppSignalIDs.remove(5, rest);
		}

		msg.append(discreteAppSignalIDs.join(Separator::NEW_LINE));

		if (rest > 0)
		{
			msg.append(QString("\n\nand %1 more signal(s)").arg(rest));
		}

		if (QMessageBox::warning(this, "Warning", msg, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
		{
			return;
		}
	}

	if (appSignalIDs.isEmpty())
	{
		return;
	}

	QDialog dlg;

	QGridLayout* gridLayout = new QGridLayout(&dlg);

	// create dialog layout

	gridLayout->addWidget(signalsList, 0, 0, 1, 2);

	//

	gridLayout->addWidget(new QLabel("Aperture type"), 1, 0);

	QComboBox* apertureTypeList = new QComboBox;

	std::vector<std::pair<int, QString>> values = E::enumValues<E::ApertureType>();

	if (apertureType.has_value() == false)
	{
		apertureTypeList->addItem(QString(), -1);
		apertureTypeList->setCurrentText(QString());
	}

	for(const auto& [value, text] : values)
	{
		apertureTypeList->addItem(text, value);

		if (apertureType.has_value() && apertureType.value() == value)
		{
			apertureTypeList->setCurrentText(text);
		}
	}

	gridLayout->addWidget(apertureTypeList, 1, 1);

	//

	gridLayout->addWidget(new QLabel("Coarse aperture"), 2, 0);

	QLineEdit* coarseApertureEdit = new QLineEdit;

	if (coarseAperture.has_value())
	{
		coarseApertureEdit->setText(QString::number(coarseAperture.value()));
	}

	gridLayout->addWidget(coarseApertureEdit, 2, 1);

	//

	gridLayout->addWidget(new QLabel("Fine aperture"), 3, 0);

	QLineEdit* fineApertureEdit = new QLineEdit;

	if (fineAperture.has_value())
	{
		fineApertureEdit->setText(QString::number(fineAperture.value()));
	}

	gridLayout->addWidget(fineApertureEdit, 3, 1);

	// create dialog

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	gridLayout->addWidget(buttonBox, 4, 0, 1, 2);

	connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

	E::ApertureType resultApertureType = E::ApertureType::RangePercent;
	double resultCoarseAperture = 0;
	double resultFineAperture = 0;

	connect(buttonBox, &QDialogButtonBox::accepted, &dlg, [&]()
	{
		QString errStr;
		bool ok = false;

		// check aperture type

		QString valueStr = apertureTypeList->currentText();

		resultApertureType = E::stringToValue<E::ApertureType>(valueStr, &ok);

		if (ok == false)
		{
			errStr.append("The ApertureType don't set.\n\n");
		}

		// check coarse aperture

		valueStr = coarseApertureEdit->text();

		resultCoarseAperture = valueStr.toDouble(&ok);

		if (ok == false)
		{
			errStr.append("The CoarseAperture don't set.\n\n");
		}
		else
		{
			resultCoarseAperture = abs(resultCoarseAperture);
			coarseApertureEdit->setText(QString::number(resultCoarseAperture));

			if (resultCoarseAperture == 0)
			{
				errStr.append("The CoarseAperture can't be 0.\n\n");
			}
		}

		// check fine aperture

		valueStr = fineApertureEdit->text();

		resultFineAperture = valueStr.toDouble(&ok);

		if (ok == false)
		{
			errStr.append("The FineAperture don't set.\n\n");
		}
		else
		{
			resultFineAperture = abs(resultFineAperture);
			fineApertureEdit->setText(QString::number(resultFineAperture));

			if (resultFineAperture == 0)
			{
				errStr.append("The FineAperture can't be 0.\n\n");
			}
		}

		//

		if (errStr.isEmpty() && resultCoarseAperture <= resultFineAperture)
		{
			errStr.append("The CoarseAperture should be greate than the FineAperture.\n\n");
		}

		//

		if (errStr.isEmpty() == false)
		{
			QMessageBox::critical(&dlg, "Error", errStr);
			return;
		}

		dlg.accept();
	});

	//

	dlg.setLayout(gridLayout);
	dlg.resize(500, 400);

	int result = dlg.exec();

	if (result == QDialog::Rejected)
	{
		return;
	}

	//

	std::vector<ApertureRecord> apertures;

	apertures.reserve(appSignalIDs.size());

	for(const QString& appSignalID : appSignalIDs)
	{
		ApertureRecord ar;

		ar.signalID = appSignalID;
		ar.apertureType = resultApertureType;
		ar.coarseAperture = resultCoarseAperture;
		ar.fineAperture = resultFineAperture;

		apertures.push_back(ar);

	}

	overrideApertures(apertures);
}
