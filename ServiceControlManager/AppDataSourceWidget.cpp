#include "AppDataSourceWidget.h"
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
	std::function<QVariant (const DataSource& source)> fieldValueGetter;
};

struct dynamicPropertyFieldDefinition
{
	QString fieldName;
	std::function<QVariant (const DataSourceOnline& source)> fieldValueGetter;
};

static const QList<staticPropertyFieldDefinition> staticPropertiesFieldList
{
	{
		QStringLiteral("Module EquipmentID"),
		[](const DataSource& source)
		{
			return source.moduleEquipmentID();
		}
	},

	{
		QStringLiteral("Module caption"),
		[](const DataSource& source)
		{
			return source.moduleCaption();
		}
	},

	{
		QStringLiteral("Module type"),
		[](const DataSource& source)
		{
			return "0x" + QString("%1").arg(source.moduleType(), 4, 16, Latin1Char::ZERO).toUpper();
		}
	},

	{
		QStringLiteral("Module preset name"),
		[](const DataSource& source)
		{
			return source.modulePresetName();
		}
	},

	{
		QStringLiteral("Module workcycle, mcs"),
		[](const DataSource& source)
		{
			return source.moduleWorkcycle_mcs();
		}
	},

	{
		QStringLiteral("RUP protocol version"),
		[](const DataSource& source)
		{
			return source.rupVersion();
		}
	},

	{
		QStringLiteral("Subsystem ID"),
		[](const DataSource& source)
		{
			return source.subsystemID();
		}
	},

	{
		QStringLiteral("Subsystem key"),
		[](const DataSource& source)
		{
			return source.subsystemKey();
		}
	},

	{
		QStringLiteral("LM number"),
		[](const DataSource& source)
		{
			return source.lmNumber();
		}
	},

	{
		QStringLiteral("Subsystem channel"),
		[](const DataSource& source)
		{
			return source.subsystemChannel();
		}
	},

	{
		QStringLiteral("Lan controller ID"),
		[](const DataSource& source)
		{
			const auto& lanControllersInfo = source.lanControllersInfo()();
			QString str;

			for(const auto& lci : lanControllersInfo)
			{
				if (lci.isAppDataEnabled() == false)
				{
					continue;
				}

				if (str.isEmpty() == false)
				{
					str += ", ";
				}

				str += lci.equipmentID;
			}

			return str;
		}
	},

	{
		QStringLiteral("IP"),
		[](const DataSource& source)
		{
			const auto& lanControllersInfo = source.lanControllersInfo()();
			QString str;

			for(const auto& lci : lanControllersInfo)
			{
				if (lci.isAppDataEnabled() == false)
				{
					continue;
				}

				if (str.isEmpty() == false)
				{
					str += ", ";
				}

				str += QString("%1:%2").arg(lci.appDataIP).arg(lci.appDataPort);
			}

			return str;
		}
	},

	{
		QStringLiteral("Expected AppDataUID"),
		[](const DataSource& source)
		{
			quint32 dataID = source.rupAppDataUID();
			return "0x" + QString("%1").arg(dataID, sizeof(dataID) * 2,
											16, QChar('0')).toUpper();
		}
	},

	{
		QStringLiteral("App data frames quantity"),
		[](const DataSource& source)
		{
			return source.appDataFramesQuantity();
		}
	},

	{
		QStringLiteral("App data size, bytes"),
		[](const DataSource& source)
		{
			return source.appDataSizeBytes();
		}
	},
};

static const QList<dynamicPropertyFieldDefinition> dynamicPropertiesFieldList
{
	{
		QStringLiteral("State"),
		[](const DataSourceOnline& source)
		{
			return source.stateStr();
		}
	},

	{
		QStringLiteral("Uptime"),
		[](const DataSourceOnline& source)
		{
			return formatUptime(source.uptime());
		}
	},

	{
		QStringLiteral("Date receiving speed, bytes/sec"),
		[](const DataSourceOnline& source)
		{
			return source.dataReceivingSpeed();
		}
	},

	{
		QStringLiteral("Received data size, bytes"),
		[](const DataSourceOnline& source)
		{
			return source.receivedDataSize();
		}
	},

	{
		QStringLiteral("Received frames count"),
		[](const DataSourceOnline& source)
		{
			return source.receivedFramesCount();
		}
	},

	{
		QStringLiteral("Received packets count"),
		[](const DataSourceOnline& source)
		{
			return source.receivedPacketCount();
		}
	},

	{
		QStringLiteral("Received AppDataUID"),
		[](const DataSourceOnline& source)
		{
			quint32 dataID = source.receivedDataID();
			return "0x" + QString("%1").arg(dataID, sizeof(dataID) * 2,
											16, QChar('0')).toUpper();
		}
	},

	{
		QStringLiteral("RUP frame numerator"),
		[](const DataSourceOnline& source)
		{
			return source.rupFrameNumerator();
		}
	},

	{
		QStringLiteral("RUP frame plant time"),
		[](const DataSourceOnline& source)
		{
			return DataSourceOnline::getTimeStr(source.rupFramePlantTime());
		}
	},

	{
		QStringLiteral("Lost packet count"),
		[](const DataSourceOnline& source)
		{
			return source.lostPacketCount();
		}
	},

	{
		QStringLiteral("Error protocol version"),
		[](const DataSourceOnline& source)
		{
			return source.errorProtocolVersion();
		}
	},

	{
		QStringLiteral("Error frames quantity"),
		[](const DataSourceOnline& source)
		{
			return source.errorFramesQuantity();
		}
	},

	{
		QStringLiteral("Error frame number"),
		[](const DataSourceOnline& source)
		{
			return source.errorFrameNo();
		}
	},

	{
		QStringLiteral("Error frame CRC"),
		[](const DataSourceOnline& source)
		{
			return source.errorFrameCRC();
		}
	},

	{
		QStringLiteral("Error AppDataUID"),
		[](const DataSourceOnline& source)
		{
			return source.errorDataID();
		}
	},

	{
		QStringLiteral("Error plant time format"),
		[](const DataSourceOnline& source)
		{
			return source.errorPlantTimeFormat();
		}
	},

	{
		QStringLiteral("Error duplicate plant time"),
		[](const DataSourceOnline& source)
		{
			return source.errorDuplicatePlantTime();
		}
	},

	{
		QStringLiteral("Error non monotonic plant time"),
		[](const DataSourceOnline& source)
		{
			return source.errorNonmonotonicPlantTime();
		}
	},
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
	m_infoModel = new QStandardItemModel(static_cast<int>(staticPropertiesFieldList.count()), 2, this);

	for (int i = 0; i < staticPropertiesFieldList.count(); i++)
	{
		auto& field = staticPropertiesFieldList[i];

		m_infoModel->setData(m_infoModel->index(i, 0), field.fieldName);
	}

	initTable(m_infoTable, m_infoModel);

	// Source state
	//
	m_stateTable = new QTableView(this);
	m_stateModel = new QStandardItemModel(static_cast<int>(dynamicPropertiesFieldList.count()), 2, this);

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
		if (source->moduleUniqueID() == m_id && source->moduleEquipmentID() == m_equipmentId)
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
		if (source->moduleUniqueID() == m_id && source->moduleEquipmentID() == m_equipmentId)
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
