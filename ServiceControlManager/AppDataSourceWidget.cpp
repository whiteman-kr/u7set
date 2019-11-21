#include "AppDataSourceWidget.h"
#include "../Proto/network.pb.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "ScmTcpAppDataClient.h"
#include <QHBoxLayout>
#include <QSplitter>
#include "../lib/WidgetUtils.h"

struct staticPropertyFieldDefinition
{
	QString fieldName;
	std::function<QVariant(const DataSource& source)> fieldValueGetter;
};

struct dynamicPropertyFieldDefinition
{
	QString fieldName;
	std::function<QVariant(const DataSourceOnline& source)> fieldValueGetter;
};

static const QList<staticPropertyFieldDefinition> staticPropertiesFieldList {
	{ QStringLiteral("Equipment ID"), [](const DataSource& source) { return source.lmEquipmentID(); } },
	{ QStringLiteral("Data ID"), [](const DataSource& source) { return "0x" + QString("%1").arg(source.lmDataID(), sizeof(source.lmDataID()) * 2, 16, QChar('0')).toUpper(); } },
	{ QStringLiteral("IP"), [](const DataSource& source) { return source.lmAddressStr(); } },
	{ QStringLiteral("Enable Data"), [](const DataSource& source) { return source.lmDataEnable() ? "Yes" : "No"; } },

	{ QStringLiteral("Caption"), [](const DataSource& source) { return source.lmCaption(); } },
	{ QStringLiteral("Port"), [](const DataSource& source) { return source.lmPort(); } },
	{ QStringLiteral("Part count"), [](const DataSource& source) { return source.lmRupFramesQuantity(); } },
	{ QStringLiteral("Data type"), [](const DataSource& source) { return source.lmDataTypeStr(); } },
	{ QStringLiteral("Module number"), [](const DataSource& source) { return source.lmNumber(); } },
	{ QStringLiteral("Module type"), [](const DataSource& source) { return source.lmNumber(); } },
	{ QStringLiteral("Subsystem ID"), [](const DataSource& source) { return source.lmSubsystemKey(); } },
	{ QStringLiteral("Subsystem caption"), [](const DataSource& source) { return source.lmSubsystem(); } },
	{ QStringLiteral("Adapter ID"), [](const DataSource& source) { return source.lmAdapterID(); } },
};

static const QList<dynamicPropertyFieldDefinition> dynamicPropertiesFieldList {
	{ QStringLiteral("State"), [](const DataSourceOnline& source) { return E::valueToString<E::DataSourceState>(TO_INT(source.state())); } },

	{ QStringLiteral("Uptime"), [](const DataSourceOnline& source) {
			auto time = source.uptime();
			int s = time % 60; time /= 60;
			int m = time % 60; time /= 60;
			int h = time % 24; time /= 24;
			return QString("%1d %2:%3:%4").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
		} },

	{ QStringLiteral("Received"), [](const DataSourceOnline& source) { return source.receivedDataSize(); } },
	{ QStringLiteral("Speed"), [](const DataSourceOnline& source) { return source.dataReceivingRate(); } },

	{ QStringLiteral("Error Protocol version"), [](const DataSourceOnline& source) { return source.errorProtocolVersion(); } },
	{ QStringLiteral("Error Frames quantity"), [](const DataSourceOnline& source) { return source.errorFramesQuantity(); } },
	{ QStringLiteral("Error Frame nomber"), [](const DataSourceOnline& source) { return source.errorFrameNo(); } },
	{ QStringLiteral("Lost packet count"), [](const DataSourceOnline& source) { return source.lostPacketCount(); } },
	{ QStringLiteral("Error Data ID"), [](const DataSourceOnline& source) { return source.errorDataID(); } },
	{ QStringLiteral("Error Bad frame size"), [](const DataSourceOnline& source) { return source.errorFrameSize(); } },

	{ QStringLiteral("Error Duplicate plant time"), [](const DataSourceOnline& source) { return source.errorDuplicatePlantTime(); } },
	{ QStringLiteral("Error Non monotonic plant time"), [](const DataSourceOnline& source) { return source.errorNonmonotonicPlantTime(); } },
};

AppDataSourceWidget::AppDataSourceWidget(quint64 id, QString equipmentId, QWidget *parent) :
	QWidget(parent),
	m_id(id),
	m_equipmentId(equipmentId)
{
	setWindowFlag(Qt::Dialog, true);

	setAttribute(Qt::WA_DeleteOnClose);
	QHBoxLayout* hl = new QHBoxLayout();
	m_splitter = new QSplitter(this);
	hl->addWidget(m_splitter);
	setLayout(hl);

	// Source info
	//
	m_infoTable = new QTableView(this);
	m_infoModel = new QStandardItemModel(staticPropertiesFieldList.count(), 2, this);

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 0), field.fieldName);
	}

	initTable(m_infoTable, m_infoModel);

	// Source state
	//
	m_stateTable = new QTableView(this);
	m_stateModel = new QStandardItemModel(dynamicPropertiesFieldList.count(), 2, this);

	for (int i = 0; i < dynamicPropertiesFieldList.count(); i++)
	{
		auto& field = dynamicPropertiesFieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 0), field.fieldName);
	}

	initTable(m_stateTable, m_stateModel);

	setWindowTitle(equipmentId);

	setWindowPosition(this, "AppDataSourceWidget/" + equipmentId);

	QSettings settings;
	m_splitter->restoreState(settings.value("AppDataSourceWidget/" + equipmentId + "/splitterState", m_splitter->saveState()).toByteArray());

	m_infoTable->setColumnWidth(0, settings.value("AppDataSourceWidget/" + equipmentId + "/infoColumnWidth", m_infoTable->columnWidth(0)).toInt());
	m_stateTable->setColumnWidth(0, settings.value("AppDataSourceWidget/" + equipmentId + "/stateColumnWidth", m_stateTable->columnWidth(0)).toInt());
}

AppDataSourceWidget::~AppDataSourceWidget()
{
	emit forgetMe();
}

void AppDataSourceWidget::updateStateFields()
{
	TEST_PTR_RETURN(m_tcpClientSocket);

	const DataSourceOnline* pSource = nullptr;

	for (AppDataSource* source : m_tcpClientSocket->dataSources())
	{
		if (source->lmUniqueID() == m_id && source->lmEquipmentID() == m_equipmentId)
		{
			pSource = dynamic_cast<DataSourceOnline*>(source);
		}
	}

	if (pSource == nullptr)
	{
		// Lost widgets AppDataSource ?
		close();
		deleteLater();
		return;
	}

	for (int i = 0; i < dynamicPropertiesFieldList.count(); i++)
	{
		auto& field = dynamicPropertiesFieldList[i];

		m_stateModel->setData(m_stateModel->index(i, 1), field.fieldValueGetter(*pSource));
	}
}

void AppDataSourceWidget::setClientSocket(TcpAppDataClient *tcpClientSocket)
{
	TEST_PTR_RETURN(tcpClientSocket);

	m_tcpClientSocket = tcpClientSocket;

	connect(tcpClientSocket, &TcpAppDataClient::dataSoursesStateUpdated, this, &AppDataSourceWidget::updateStateFields);

	const DataSource* pSource = nullptr;

	for (AppDataSource* source : m_tcpClientSocket->dataSources())
	{
		if (source->lmUniqueID() == m_id && source->lmEquipmentID() == m_equipmentId)
		{
			pSource = dynamic_cast<DataSource*>(source);
		}
	}

	if (pSource == nullptr)
	{
		// Lost widgets AppDataSource ?
		close();
		deleteLater();
		return;
	}

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 1), field.fieldValueGetter(*pSource));
	}
}

void AppDataSourceWidget::unsetClientSocket()
{
	m_tcpClientSocket = nullptr;
}

void AppDataSourceWidget::closeEvent(QCloseEvent *event)
{
	saveWindowPosition(this, "AppDataSourceWidget/" + m_equipmentId);

	QSettings settings;
	settings.setValue("AppDataSourceWidget/" + m_equipmentId + "/splitterState", m_splitter->saveState());

	settings.setValue("AppDataSourceWidget/" + m_equipmentId + "/infoColumnWidth", m_infoTable->columnWidth(0));
	settings.setValue("AppDataSourceWidget/" + m_equipmentId + "/stateColumnWidth", m_stateTable->columnWidth(0));

	QWidget::closeEvent(event);
}

void AppDataSourceWidget::initTable(QTableView *table, QStandardItemModel *model)
{
	table->verticalHeader()->setDefaultSectionSize(static_cast<int>(table->fontMetrics().height() * 1.4));
	table->verticalHeader()->hide();

	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setHighlightSections(false);

	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setAlternatingRowColors(true);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);

	model->setHeaderData(0, Qt::Horizontal, "Property");
	model->setHeaderData(1, Qt::Horizontal, "Value");

	table->setColumnWidth(0, 200);

	table->setModel(model);

	m_splitter->addWidget(table);
}
