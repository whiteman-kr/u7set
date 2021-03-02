#include "OptionsMvhDialog.h"

#include <QColorDialog>
#include <QHeaderView>

#include "Delegate.h"
#include "MeasureViewHeader.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsMeasureViewHeaderDialog::OptionsMeasureViewHeaderDialog(const MeasureViewOption& header, QWidget* parent) :
	QDialog(parent),
	m_header(header)
{
	setStyleSheet(".OptionsMeasureViewHeaderDialog { border: 1px solid grey } ");

	QHBoxLayout* measureTypeLayout = new QHBoxLayout;

	m_measureTypeLabel = new QLabel(tr("Measure type: "), this);

	m_measureTypeList = new QComboBox(this);

	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		m_measureTypeList->addItem(qApp->translate("MeasureBase", Measure::TypeCaption(measureType).toUtf8()));
	}
	m_measureTypeList->setCurrentIndex(m_measureType);

	measureTypeLayout->addWidget(m_measureTypeLabel);
	measureTypeLayout->addWidget(m_measureTypeList);
	measureTypeLayout->addStretch();

	m_languageType = static_cast<LanguageType>(theOptions.language().languageType());

	QVBoxLayout* mainLayout = new QVBoxLayout;

	m_columnList = new QTableWidget;
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_columnList->verticalHeader()->setDefaultSectionSize(cellSize.height());

	mainLayout->addLayout(measureTypeLayout);
	mainLayout->addWidget(m_columnList);

	setLayout(mainLayout);

	connect(m_measureTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptionsMeasureViewHeaderDialog::setMeasureType);

	setHeaderList();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsMeasureViewHeaderDialog::~OptionsMeasureViewHeaderDialog()
{
	clearList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::setHeaderList()
{
	QStringList horizontalHeaderLabels;

	for(int c = 0; c < MVH_COLUMN_COUNT; c++)
	{
		horizontalHeaderLabels.append(qApp->translate("OptionsMvhDialog.h", MvhColumn[c]));
	}

	m_columnList->setColumnCount(horizontalHeaderLabels.count());
	m_columnList->setHorizontalHeaderLabels(horizontalHeaderLabels);
	m_columnList->verticalHeader()->hide();

	for(int c = 0; c < MVH_COLUMN_COUNT; c++)
	{
		m_columnList->setColumnWidth(c, MvhColumnWidth[c]);
	}

	connect(m_columnList, &QTableWidget::cellDoubleClicked, this, &OptionsMeasureViewHeaderDialog::onEdit);
	connect(m_columnList, &QTableWidget::cellChanged, this, &OptionsMeasureViewHeaderDialog::cellChanged);
	connect(m_columnList, &QTableWidget::currentCellChanged, this, &OptionsMeasureViewHeaderDialog::currentCellChanged);

	IntDelegate* delegate = new IntDelegate(this);
	m_columnList->setItemDelegateForColumn(MVH_COLUMN_WIDTH, delegate);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::updateList()
{
	clearList();

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	if (ERR_LANGUAGE_TYPE(m_languageType) == true)
	{
		return;
	}

	int rowCount = 0;

	for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
	{
		if (m_header.m_column[m_measureType][m_languageType][column].title().isEmpty() == true)
		{
			rowCount = column;
			break;
		}
	}

	m_columnList->setRowCount(rowCount);

	m_updatingList = true;
	QTableWidgetItem* cell = nullptr;
	QStringList verticalHeaderLabels;
	QFont boldFont = m_columnList->font();
	boldFont.setBold(true);

	// update list
	//
	for(int index = 0; index < rowCount; index++)
	{
		verticalHeaderLabels.append(QString("%1").arg(index + 1));

		MeasureViewColumn& column = m_header.m_column[m_measureType][m_languageType][index];

		bool visible = column.enableVisible();

		cell = new QTableWidgetItem(column.title());
		cell->setTextAlignment(Qt::AlignLeft);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_columnList->setItem(index, MVH_COLUMN_TITLE, cell);

		cell = new QTableWidgetItem(visible ? tr("True") : tr("False"));
		cell->setTextAlignment(Qt::AlignHCenter);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_columnList->setItem(index, MVH_COLUMN_VISIBLE, cell);

		cell = new QTableWidgetItem(QString::number(column.width()));
		cell->setTextAlignment(Qt::AlignHCenter);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_columnList->setItem(index, MVH_COLUMN_WIDTH, cell);
	}

	m_columnList->setVerticalHeaderLabels(verticalHeaderLabels);
	m_updatingList = false;

	emit updateMeasureViewPage(true);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::clearList()
{
	int columnCount = m_columnList->columnCount();
	int rowCount = m_columnList->rowCount();

	for(int column = 0; column < columnCount; column++)
	{
		for(int row = 0; row < rowCount; row++)
		{
			QTableWidgetItem* item = m_columnList->item(row, column);
			if (item != nullptr)
			{
				delete item;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::setMeasureType(int measureType)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	m_measureType = static_cast<Measure::Type>(measureType);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::cellChanged(int row, int column)
{
	if (m_updatingList == true)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	if (ERR_LANGUAGE_TYPE(m_languageType) == true)
	{
		return;
	}

	if (m_columnList == nullptr)
	{
		return;
	}

	if (row < 0 || row >= m_columnList->rowCount())
	{
		return;
	}

	QTableWidgetItem* item = m_columnList->item(row, column);
	if (item == nullptr)
	{
		return;
	}

	switch(column)
	{
		case MVH_COLUMN_TITLE:
			m_header.m_column[m_measureType][m_languageType][row].setTitle(item->text());
			break;
		case MVH_COLUMN_VISIBLE:
			break;
		case MVH_COLUMN_WIDTH:
			m_header.m_column[m_measureType][m_languageType][row].setWidth(item->text().toInt());
			break;
		default:
			assert(0);
			break;
	}

	updateList();

	m_columnList->setFocus();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::currentCellChanged(int, int column, int, int)
{
	if (column == MVH_COLUMN_WIDTH)
	{
		m_columnList->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
	}
	else
	{
		m_columnList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::onEdit(int row, int column)
{
	if (row < 0 || row >= m_columnList->rowCount())
	{
		return;
	}

	if (column < 0 || column >= m_columnList->columnCount())
	{
		return;
	}

	if (m_columnList == nullptr)
	{
		return;
	}

	QTableWidgetItem* cell = m_columnList->item(row, column);
	if (cell == nullptr)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	if (ERR_LANGUAGE_TYPE(m_languageType) == true)
	{
		return;
	}

	MeasureViewColumn& headerColumn = m_header.m_column[m_measureType][m_languageType][row];

	switch(column)
	{
		case MVH_COLUMN_TITLE:
			{
				m_columnList->editItem(cell);
			}
			break;

		case MVH_COLUMN_VISIBLE:
			{
				bool visible = !headerColumn.enableVisible();
				headerColumn.setVisible(visible);
				cell->setText(visible ? tr("True") : tr("False"));
			}
			break;

		case MVH_COLUMN_WIDTH:
			{
				m_columnList->editItem(cell);
			}
			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		int row = m_columnList->currentRow();
		int column = m_columnList->currentColumn();
		onEdit(row, column);

		return;
	}

	QWidget::keyPressEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::showEvent(QShowEvent* e)
{
	updateList();

	QWidget::showEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------n
