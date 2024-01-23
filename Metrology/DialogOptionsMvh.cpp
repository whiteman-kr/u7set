#include "DialogOptionsMvh.h"
#include "Delegate.h"
#include "MeasureViewHeader.h"

// -------------------------------------------------------------------------------------------------------------------

DialogOptionsMeasureViewHeader::DialogOptionsMeasureViewHeader(const MeasureViewOption& header, QWidget* parent) :
	QDialog(parent),
	m_header(header)
{
	setStyleSheet(".OptionsMeasureViewHeaderDialog { border: 1px solid grey } ");

	QHBoxLayout* measureTypeLayout = new QHBoxLayout;

	m_pMeasureTypeLabel = new QLabel(tr("Measure type: "), this);

	m_pMeasureTypeList = new QComboBox(this);

	for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType++)
	{
		m_pMeasureTypeList->addItem(qApp->translate("MeasureBase", Measure::TypeCaption(static_cast<Measure::Type>(measureType)).toUtf8()));
	}
	m_pMeasureTypeList->setCurrentIndex(m_measureType);

	m_pDefaultButton = new QPushButton(tr("Default"), this);

	measureTypeLayout->addWidget(m_pMeasureTypeLabel);
	measureTypeLayout->addWidget(m_pMeasureTypeList);
	measureTypeLayout->addStretch();
	measureTypeLayout->addWidget(m_pDefaultButton);
	m_pDefaultButton->hide();

	m_languageType = static_cast<OT::LanguageType>(theOptions.language().languageType());

	QVBoxLayout* mainLayout = new QVBoxLayout;

	m_pColumnList = new QTableWidget;
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pColumnList->verticalHeader()->setDefaultSectionSize(cellSize.height());

	mainLayout->addLayout(measureTypeLayout);
	mainLayout->addWidget(m_pColumnList);

	setLayout(mainLayout);

	connect(m_pMeasureTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogOptionsMeasureViewHeader::setMeasureType);
	connect(m_pDefaultButton, &QPushButton::clicked, this, &DialogOptionsMeasureViewHeader::onDefault);

	setHeaderList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogOptionsMeasureViewHeader::~DialogOptionsMeasureViewHeader()
{
	clearList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::setHeaderList()
{
	QStringList horizontalHeaderLabels;

	for(int c = 0; c < MVH_COLUMN_COUNT; c++)
	{
		horizontalHeaderLabels.append(qApp->translate("DialogOptionsMvh", MvhColumn[c]));
	}

	m_pColumnList->setColumnCount(static_cast<int>(horizontalHeaderLabels.count()));
	m_pColumnList->setHorizontalHeaderLabels(horizontalHeaderLabels);
	m_pColumnList->verticalHeader()->hide();

	for(int c = 0; c < MVH_COLUMN_COUNT; c++)
	{
		m_pColumnList->setColumnWidth(c, MvhColumnWidth[c]);
	}

	connect(m_pColumnList, &QTableWidget::cellDoubleClicked, this, &DialogOptionsMeasureViewHeader::onEdit);
	connect(m_pColumnList, &QTableWidget::cellChanged, this, &DialogOptionsMeasureViewHeader::cellChanged);
	connect(m_pColumnList, &QTableWidget::currentCellChanged, this, &DialogOptionsMeasureViewHeader::currentCellChanged);

	IntDelegate* delegate = new IntDelegate(this);
	m_pColumnList->setItemDelegateForColumn(MVH_COLUMN_WIDTH, delegate);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::updateList()
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

	for(int column = 0; column < Measure::MaxColumnCount; column++)
	{
		if (m_header.m_column[m_measureType][m_languageType][column].title().isEmpty() == true)
		{
			rowCount = column;
			break;
		}
	}

	m_pColumnList->setRowCount(rowCount);

	m_updatingList = true;
	QTableWidgetItem* cell = nullptr;
	QStringList verticalHeaderLabels;
	QFont boldFont = m_pColumnList->font();
	boldFont.setBold(true);

	// update list
	//
	for(int index = 0; index < rowCount; index++)
	{
		verticalHeaderLabels.append(QString("%1").arg(index + 1));

		Measure::HeaderColumn& column = m_header.m_column[m_measureType][m_languageType][index];

		bool visible = column.enableVisible();

		cell = new QTableWidgetItem(column.title());
		cell->setTextAlignment(Qt::AlignLeft);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_pColumnList->setItem(index, MVH_COLUMN_TITLE, cell);

		cell = new QTableWidgetItem(visible ? tr("True") : tr("False"));
		cell->setTextAlignment(Qt::AlignHCenter);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_pColumnList->setItem(index, MVH_COLUMN_VISIBLE, cell);

		cell = new QTableWidgetItem(QString::number(column.width()));
		cell->setTextAlignment(Qt::AlignHCenter);
		if (visible == false)
		{
			cell->setForeground(Qt::lightGray);
		}

		m_pColumnList->setItem(index, MVH_COLUMN_WIDTH, cell);
	}

	m_pColumnList->setVerticalHeaderLabels(verticalHeaderLabels);
	m_updatingList = false;

	emit dataUpdated();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::clearList()
{
	int columnCount = m_pColumnList->columnCount();
	int rowCount = m_pColumnList->rowCount();

	for(int column = 0; column < columnCount; column++)
	{
		for(int row = 0; row < rowCount; row++)
		{
			QTableWidgetItem* item = m_pColumnList->item(row, column);
			if (item != nullptr)
			{
				delete item;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::setMeasureType(int measureType)
{
	if (Measure::ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	m_measureType = static_cast<Measure::Type>(measureType);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::onDefault()
{
	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

//	O::LanguageType languageType = theOptions.language().languageType();
//	if (ERR_LANGUAGE_TYPE(languageType) == true)
//	{
//		return;
//	}

//	Measure::ViewHeader header;

//	header.setMeasureType(m_measureType);

//	int columnCount = header.count();
//	for(int column = 0; column < columnCount; column++)
//	{
//		Measure::HeaderColumn* pColumn = header.column(column);
//		if (pColumn == nullptr)
//		{
//			continue;
//		}

//		qDebug() << pColumn->title() << pColumn->enableVisible() << pColumn->width();

//		m_header.m_column[m_measureType][languageType][column] = *pColumn;
//	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::cellChanged(int row, int column)
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

	if (m_pColumnList == nullptr)
	{
		return;
	}

	if (row < 0 || row >= m_pColumnList->rowCount())
	{
		return;
	}

	QTableWidgetItem* item = m_pColumnList->item(row, column);
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
	}

	updateList();

	m_pColumnList->setFocus();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::currentCellChanged(int, int column, int, int)
{
	if (column == MVH_COLUMN_WIDTH)
	{
		m_pColumnList->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
	}
	else
	{
		m_pColumnList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::onEdit(int row, int column)
{
	if (row < 0 || row >= m_pColumnList->rowCount())
	{
		return;
	}

	if (column < 0 || column >= m_pColumnList->columnCount())
	{
		return;
	}

	if (m_pColumnList == nullptr)
	{
		return;
	}

	QTableWidgetItem* cell = m_pColumnList->item(row, column);
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

	Measure::HeaderColumn& headerColumn = m_header.m_column[m_measureType][m_languageType][row];

	switch(column)
	{
		case MVH_COLUMN_TITLE:
			{
				m_pColumnList->editItem(cell);
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
				m_pColumnList->editItem(cell);
			}
			break;

		default:
			assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
	{
		int row = m_pColumnList->currentRow();
		int column = m_pColumnList->currentColumn();
		onEdit(row, column);

		return;
	}

	QWidget::keyPressEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptionsMeasureViewHeader::showEvent(QShowEvent* e)
{
	updateList();

	QWidget::showEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------n
