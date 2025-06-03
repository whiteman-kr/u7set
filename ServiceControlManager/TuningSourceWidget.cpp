#include "TuningSourceWidget.h"
#include <functional>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include "ScmTcpAppDataClient.h"
#include <QHBoxLayout>
#include <QSplitter>
#include "../UtilsLib/Ui/WidgetUtils.h"

TuningSourceWidget::TuningSourceWidget(const QString& equipmentID, QWidget* parent) :
	QWidget(parent),
	m_equipmentID(equipmentID)
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

	setWindowTitle("AppDataSource " + m_equipmentID);

	setWindowPosition(this, TUNING_SRC_WIDGET_KEY + m_equipmentID);

	QSettings settings;

	m_infoTable->setColumnWidth(0, settings.value(TUNING_SRC_WIDGET_KEY + m_equipmentID + INFO_COLUMN_WIDTH_KEY, m_infoTable->columnWidth(0)).toInt());
	m_stateTable->setColumnWidth(0, settings.value(TUNING_SRC_WIDGET_KEY + m_equipmentID + STATE_COLUMN_WIDTH_KEY, m_stateTable->columnWidth(0)).toInt());
}

TuningSourceWidget::~TuningSourceWidget()
{
	emit forgetMe(m_equipmentID);
}

void TuningSourceWidget::updateData(const Network::TuningSourceInfoState& state)
{
	m_infoModel.updateData(state);
	m_stateModel.updateData(state);
}

void TuningSourceWidget::closeEvent(QCloseEvent *event)
{
	saveWindowPosition(this, TUNING_SRC_WIDGET_KEY + m_equipmentID);

	QSettings settings;

	settings.setValue(TUNING_SRC_WIDGET_KEY + m_equipmentID + INFO_COLUMN_WIDTH_KEY, m_infoTable->columnWidth(0));
	settings.setValue(TUNING_SRC_WIDGET_KEY + m_equipmentID + STATE_COLUMN_WIDTH_KEY, m_stateTable->columnWidth(0));

	QWidget::closeEvent(event);
}

void TuningSourceWidget::initTable(QTableView* table, QAbstractTableModel* model)
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
