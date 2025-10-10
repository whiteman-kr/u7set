#include "AppDataSourceWidget.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "ScmTcpAppDataClient.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <UiLib/UiTools.h>

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
