#include "AppDataSourceWidget.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "ScmTcpAppDataClient.h"
#include <QHBoxLayout>
#include <QSplitter>
#include "../UtilsLib/Ui/WidgetUtils.h"

struct staticPropertyFieldDefinition
{
	QString fieldName;
	std::function<QVariant (const OnlineLib::DataSource& source)> fieldValueGetter;
};

struct dynamicPropertyFieldDefinition
{
	QString fieldName;
	std::function<QVariant (const Network::AppDataSourceState& state)> fieldValueGetter;
};

static const std::vector<staticPropertyFieldDefinition> staticPropertiesFieldList
{
	{
		QStringLiteral("Module EquipmentID"),
		[](const OnlineLib::DataSource& source)
		{
			return source.moduleEquipmentID();
		}
	},

	{
		QStringLiteral("Module caption"),
		[](const OnlineLib::DataSource& source)
		{
			return source.moduleCaption();
		}
	},

	{
		QStringLiteral("Module type"),
		[](const OnlineLib::DataSource& source)
		{
			return "0x" + QString("%1").arg(source.moduleType(), 4, 16, Latin1Char::ZERO).toUpper();
		}
	},

	{
		QStringLiteral("Module preset name"),
		[](const OnlineLib::DataSource& source)
		{
			return source.modulePresetName();
		}
	},

	{
		QStringLiteral("Module workcycle, mcs"),
		[](const OnlineLib::DataSource& source)
		{
			return source.moduleWorkcycle_mcs();
		}
	},

	{
		QStringLiteral("RUP protocol version"),
		[](const OnlineLib::DataSource& source)
		{
			return source.rupVersion();
		}
	},

	{
		QStringLiteral("Subsystem ID"),
		[](const OnlineLib::DataSource& source)
		{
			return source.subsystemID();
		}
	},

	{
		QStringLiteral("Subsystem key"),
		[](const OnlineLib::DataSource& source)
		{
			return source.subsystemKey();
		}
	},

	{
		QStringLiteral("LM number"),
		[](const OnlineLib::DataSource& source)
		{
			return source.lmNumber();
		}
	},

	{
		QStringLiteral("Subsystem channel"),
		[](const OnlineLib::DataSource& source)
		{
			return source.subsystemChannel();
		}
	},

	{
		QStringLiteral("Lan controller ID"),
		[](const OnlineLib::DataSource& source)
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
		[](const OnlineLib::DataSource& source)
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
		[](const OnlineLib::DataSource& source)
		{
			quint32 dataID = source.rupAppDataUID();
			return "0x" + QString("%1").arg(dataID, sizeof(dataID) * 2,
											16, QChar('0')).toUpper();
		}
	},

	{
		QStringLiteral("App data frames quantity"),
		[](const OnlineLib::DataSource& source)
		{
			return source.appDataFramesQuantity();
		}
	},

	{
		QStringLiteral("App data size, bytes"),
		[](const OnlineLib::DataSource& source)
		{
			return source.appDataSizeBytes();
		}
	},
};

static const std::vector<dynamicPropertyFieldDefinition> dynamicPropertiesFieldList
{
	{
		QStringLiteral("State"),
		[](const Network::AppDataSourceState& state)
		{
			return state.receivesdata() ?  "Receive data" : "No data";
		}
	},

	{
		QStringLiteral("Uptime"),
		[](const Network::AppDataSourceState& state)
		{
			return formatUptime(state.uptime());
		}
	},

	{
		QStringLiteral("Date receiving speed, bytes/sec"),
		[](const Network::AppDataSourceState& state)
		{
			return state.datareceivingspeed();
		}
	},

	{
		QStringLiteral("Received data size, bytes"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.receiveddatasize());
		}
	},

	{
		QStringLiteral("Received frames count"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.receivedframescount());
		}
	},

	{
		QStringLiteral("Received packets count"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.receivedpacketcount());
		}
	},

	{
		QStringLiteral("Received AppDataUID"),
		[](const Network::AppDataSourceState& state)
		{
			quint32 dataID = state.receiveddataid();
			return "0x" + QString("%1").arg(dataID, sizeof(dataID) * 2,
											16, QChar('0')).toUpper();
		}
	},

	{
		QStringLiteral("RUP frame numerator"),
		[](const Network::AppDataSourceState& state)
		{
			return state.rupframenumerator();
		}
	},

	{
		QStringLiteral("RUP frame plant time"),
		[](const Network::AppDataSourceState& state)
		{
		 return OnlineLib::DataSourceOnline::getTimeStr(state.lmtime());
		}
	},

	{
		QStringLiteral("Lost packet count"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.lostpacketcount());
		}
	},

	{
		QStringLiteral("Error protocol version"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorprotocolversion());
		}
	},

	{
		QStringLiteral("Error frames quantity"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorframesquantity());
		}
	},

	{
		QStringLiteral("Error frame number"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorframeno());
		}
	},

	{
		QStringLiteral("Error frame CRC"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorframecrc());
		}
	},

	{
		QStringLiteral("Error AppDataUID"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errordataid());
		}
	},

	{
		QStringLiteral("Error plant time format"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorplanttimeformat());
		}
	},

	{
		QStringLiteral("Error duplicate plant time"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errorduplicateplanttime());
		}
	},

	{
		QStringLiteral("Error non monotonic plant time"),
		[](const Network::AppDataSourceState& state)
		{
			return static_cast<qint64>(state.errornonmonotonicplanttime());
		}
	},
};

AppDataSourceWidget::AppDataSourceWidget(const QString& lanControllerID, QWidget* parent) :
	QWidget(parent),
	m_lanControllerID(lanControllerID)
{
	setWindowFlag(Qt::Dialog, true);

	setAttribute(Qt::WA_DeleteOnClose);

	// Source info
	//
	m_infoTable = new QTableView;

	initTable(m_infoTable, &m_infoModel);

	// Source state
	//
	m_stateTable = new QTableView;

	initTable(m_stateTable, &m_stateModel);

	//

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(m_infoTable);
	hl->addWidget(m_stateTable);
	setLayout(hl);

	setWindowTitle("AppDataSource " + m_lanControllerID);

	setWindowPosition(this, APP_DATA_SRC_WIDGET_KEY + m_lanControllerID);

	QSettings settings;

	m_infoTable->setColumnWidth(0, settings.value(APP_DATA_SRC_WIDGET_KEY + m_lanControllerID + INFO_COLUMN_WIDTH_KEY, m_infoTable->columnWidth(0)).toInt());
	m_stateTable->setColumnWidth(0, settings.value(APP_DATA_SRC_WIDGET_KEY + m_lanControllerID + STATE_COLUMN_WIDTH_KEY, m_stateTable->columnWidth(0)).toInt());
}

AppDataSourceWidget::~AppDataSourceWidget()
{
	emit forgetMe(m_lanControllerID);
}

void AppDataSourceWidget::updateData(const Network::AppDataSourceState& state)
{
/*	if (m_infoModel != nullptr)
	{
		m_infoModel->updateData(state);
	}*/

	m_infoModel.updateData(state);
	m_stateModel.updateData(state);
}

void AppDataSourceWidget::closeEvent(QCloseEvent *event)
{
	saveWindowPosition(this, APP_DATA_SRC_WIDGET_KEY + m_lanControllerID);

	QSettings settings;

	settings.setValue(APP_DATA_SRC_WIDGET_KEY + m_lanControllerID + INFO_COLUMN_WIDTH_KEY, m_infoTable->columnWidth(0));
	settings.setValue(APP_DATA_SRC_WIDGET_KEY + m_lanControllerID + STATE_COLUMN_WIDTH_KEY, m_stateTable->columnWidth(0));

	QWidget::closeEvent(event);
}

void AppDataSourceWidget::initTable(QTableView* table, QAbstractTableModel* model)
{
	table->verticalHeader()->setDefaultSectionSize(static_cast<int>(table->fontMetrics().height() * 1.4));
	table->verticalHeader()->hide();

	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setHighlightSections(false);

	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);

	table->setColumnWidth(0, 300);

	table->setModel(model);
}
