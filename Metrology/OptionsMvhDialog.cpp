#include "OptionsMvhDialog.h"

#include <QColorDialog>
#include <QHeaderView>

#include "Delegate.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsMeasureViewHeaderDialog::OptionsMeasureViewHeaderDialog(const MeasureViewOption& header, QWidget *parent) :
    QDialog(parent),
    m_header (header)
{
    setStyleSheet(".OptionsMeasureViewHeaderDialog { border: 1px solid grey } ");

    QHBoxLayout* measureTypeLayout = new QHBoxLayout;

    m_measureTypeLabel = new QLabel;
    m_measureTypeLabel->setText(tr("Measure type: "));

    m_measureTypeList = new QComboBox;

    for(int t = 0; t < MEASURE_TYPE_COUNT; t++)
    {
        m_measureTypeList->addItem(MeasureType[t]);
    }
    m_measureTypeList->setCurrentIndex(m_measureType);

    measureTypeLayout->addWidget(m_measureTypeLabel);
    measureTypeLayout->addWidget(m_measureTypeList);
    measureTypeLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout;

    m_columnList = new QTableWidget;

    mainLayout->addLayout(measureTypeLayout);
    mainLayout->addWidget(m_columnList);

    setLayout(mainLayout);

    connect(m_measureTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &OptionsMeasureViewHeaderDialog::onMeasureType);

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
        horizontalHeaderLabels.append( MvhColumn[c] );
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

    ColorDelegate* colorDelegate = new ColorDelegate(this);
    m_columnList->setItemDelegateForColumn(MVH_COLUMN_COLOR, colorDelegate);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::updateList()
{
    clearList();

    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    int rowCount = 0;

    for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
    {
        if (m_header.m_column[m_measureType][column].title().isEmpty() == true)
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
    for(int index = 0; index < rowCount; index++ )
    {
        verticalHeaderLabels.append(QString("%1").arg(index + 1));
        m_columnList->setRowHeight(index, 20);

        MeasureViewColumn& column = m_header.m_column[m_measureType][index];

        bool visible = column.enableVisible();
        bool bold = column.boldFont();

        cell = new QTableWidgetItem( column.title());
        cell->setTextAlignment(Qt::AlignLeft);
        if (visible == false)
        {
            cell->setTextColor(Qt::lightGray);
        }
        if (bold == true)
        {
            cell->setFont(boldFont);
        }
        m_columnList->setItem(index, MVH_COLUMN_TITLE, cell);

        cell = new QTableWidgetItem( visible ? tr("True") : tr("False"));
        cell->setTextAlignment(Qt::AlignHCenter);
        if (visible == false)
        {
            cell->setTextColor(Qt::lightGray);
        }
        if (bold == true)
        {
            cell->setFont(boldFont);
        }
        m_columnList->setItem(index, MVH_COLUMN_VISIBLE, cell);

        cell = new QTableWidgetItem( QString::number( column.width(), 10, 0) );
        cell->setTextAlignment(Qt::AlignHCenter);
        if (visible == false)
        {
            cell->setTextColor(Qt::lightGray);
        }
        if (bold == true)
        {
            cell->setFont(boldFont);
        }
        m_columnList->setItem(index, MVH_COLUMN_WIDTH, cell);

        cell = new QTableWidgetItem("");
        cell->setData(Qt::UserRole, column.color() );
        m_columnList->setItem(index, MVH_COLUMN_COLOR, cell);

        cell = new QTableWidgetItem( column.boldFont() ? tr("True") : tr("False"));
        cell->setTextAlignment(Qt::AlignHCenter);
        if (visible == false)
        {
            cell->setTextColor(Qt::lightGray);
        }
        if (bold == true)
        {
            cell->setFont(boldFont);
        }
        m_columnList->setItem(index, MVH_COLUMN_BOLD, cell);

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

    for(int column = 0; column < columnCount; column++ )
    {
        for(int row = 0; row < rowCount; row++ )
        {
            QTableWidgetItem *item = m_columnList->item(row, column);
            if (item != nullptr)
            {
                delete item;
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::onMeasureType(int type)
{
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_measureType = type;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::cellChanged(int row, int column)
{
    if (m_updatingList == true)
    {
        return;
    }

    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    if (row < 0 || row >= m_columnList->rowCount())
    {
        return;
    }

    QTableWidgetItem* item = m_columnList->item( row, column);
    if(item == nullptr)
    {
        return;
    }

    if (column == MVH_COLUMN_WIDTH)
    {
       m_header.m_column[m_measureType][row].setWidth(item->text().toInt());
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
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    if (row < 0 || row >= m_columnList->rowCount())
    {
        return;
    }

    if (column < 0 || column >= m_columnList->columnCount())
    {
        return;
    }

    QTableWidgetItem* cell = m_columnList->item( row, column);
    if(cell == nullptr)
    {
        return;
    }

    MeasureViewColumn& headerColumn = m_header.m_column[m_measureType][row];

    switch(column)
    {
        case MVH_COLUMN_TITLE:
            break;

        case MVH_COLUMN_VISIBLE:
            {
                bool visible = !headerColumn.enableVisible();
                headerColumn.setVisible(visible);
                cell->setText( visible ? tr("True") : tr("False"));
            }
            break;

        case MVH_COLUMN_WIDTH:
            {
                m_columnList->editItem(cell);
            }
            break;

        case MVH_COLUMN_COLOR:
            {
                QColor color = QColorDialog::getColor(headerColumn.color(), this, tr("Select column color"));
                if (color.rgb() == 0xFF000000)  // pressed button cancel
                {
                    break;
                }

                headerColumn.setColor( color );
                cell->setText( QString("[%1,%2,%3]").arg(color.red()).arg(color.green()).arg(color.blue()) );
            }
            break;

        case MVH_COLUMN_BOLD:
            {
                bool bold = !headerColumn.boldFont();
                headerColumn.setBoldFont(bold);
                cell->setText( bold ? tr("True") : tr("False"));
            }
            break;

        default:
            assert(0);
            break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsMeasureViewHeaderDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Return)
    {
        int row = m_columnList->currentRow();
        int column = m_columnList->currentColumn();
        onEdit( row, column);

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
