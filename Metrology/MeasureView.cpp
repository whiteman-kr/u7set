#include "MeasureView.h"

#include <QHeaderView>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureTable::MeasureTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------


MeasureTable::MeasureTable(int type, QObject*) :
     m_measureType(type)
{
    m_header.init(type);
}

// -------------------------------------------------------------------------------------------------------------------

MeasureTable::~MeasureTable()
{
    m_measureBase.clear();
}


// -------------------------------------------------------------------------------------------------------------------

void MeasureTable::setMeasureType(int type)
{
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_measureType = type;
    m_header.init(type);
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureTable::columnIsVisible(int column)
{
    if (column < 0 || column >= m_header.count())
    {
        return false;
    }

    MeasureViewColumn* pColumn = m_header.column(column);
    if (pColumn == nullptr)
    {
        return false;
    }

    if (pColumn->title().isEmpty() == true)
    {
        return false;
    }

    return pColumn->enableVisible();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureTable::columnCount(const QModelIndex&) const
{
    return m_header.count();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureTable::rowCount(const QModelIndex&) const
{
    return m_measureBase.count();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MeasureTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        MeasureViewColumn* column = m_header.column(section);
        if (column != nullptr)
        {
            result = column->title();
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant MeasureTable::data(const QModelIndex &index, int role) const
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return QVariant();
    }

    if (index.isValid() == false)
    {
        return QVariant();
    }

    int indexRow = index.row();
    if (indexRow < 0 || indexRow >= m_measureBase.count())
    {
        return QVariant();
    }

    int indexColumn = index.column();
    if (indexColumn < 0 || indexColumn > m_header.count())
    {
        return QVariant();
    }

    MeasureViewColumn* pColumn = m_header.column(indexColumn);
    if (pColumn == nullptr)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return pColumn->alignment();
    }

    if (role == Qt::FontRole)
    {
        return pColumn->boldFont() ? theOptions.measureView().m_fontBold : theOptions.measureView().m_font;
    }

    if (role == Qt::BackgroundColorRole)
    {
        return pColumn->color();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(index.row(), index.column());
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::text(int row, int column) const
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return "";
    }

    if (row < 0 || row >= m_measureBase.count())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    QString result;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            result = textLinearity(row, column);         break;
        case MEASURE_TYPE_COMPARATOR:           result = textComparator(row, column);        break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   result = textComplexComparator(row, column); break;
        default:                                result = "";                                 break;
    }

    return result;

}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textLinearity(int row, int column) const
{
    if (row < 0 || row >= m_measureBase.count())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    LinearetyMeasureItem* m = static_cast<LinearetyMeasureItem*> (m_measureBase.at(row));
    if (m == nullptr)
    {
        return "";
    }

    int detailValueType = VALUE_TYPE_UNKNOWN;

    switch (theOptions.linearity().m_viewType)
    {
        case LO_VIEW_TYPE_DETAIL_PHYSICAL:  detailValueType = VALUE_TYPE_PHYSICAL;  break;
        case LO_VIEW_TYPE_DETAIL_OUTPUT:    detailValueType = VALUE_TYPE_OUTPUT;    break;
        default:                            detailValueType = VALUE_TYPE_ELECTRIC;  break;
    }

    int errorType = theOptions.linearity().m_errorType;

    QString result;

    switch(column)
    {
        case MVC_CMN_L_INDEX:					result = QString::number(m->recordID()); break;
        case MVC_CMN_L_CASE:					result = m->position().caseString(); break;
        case MVC_CMN_L_ID:                      result = theOptions.measureView().m_showExternalID ? m->extStrID() : m->strID(); break;
        case MVC_CMN_L_NAME:					result = m->name(); break;
        case MVC_CMN_L_CASE_NO:					result = m->position().channelString(); break;
        case MVC_CMN_L_SUBBLOCK:				result = m->position().subblockString(); break;
        case MVC_CMN_L_BLOCK:					result = m->position().blockString(); break;
        case MVC_CMN_L_ENTRY:					result = m->position().entryString(); break;
        case MVC_CMN_L_CORRECTION:				result = QString::number(m->adjustment(), 10, m->valuePrecision(VALUE_TYPE_PHYSICAL)); break;
        case MVC_CMN_L_EL_RANGE:				result = m->limitString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_RANGE:				result = m->limitString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_RANGE:				result = m->limitString(VALUE_TYPE_OUTPUT); break;
        case MVC_CMN_L_EL_NOMINAL:				result = m->nominalString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_NOMINAL:				result = m->nominalString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_NOMINAL:				result = m->nominalString(VALUE_TYPE_OUTPUT); break;
        case MVC_CMN_L_PERCENT:					result = QString::number(m->percent(), 10, 2); break;
        case MVC_CMN_L_EL_MEASURE:				result = m->measureString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_MEASURE:				result = m->measureString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_MEASURE:				result = m->measureString(VALUE_TYPE_OUTPUT); break;
        case MVC_CMN_L_SYSTEM_ERROR:			result = QString::number(m->errorAddional(ADDITIONAL_ERROR_SYSTEM), 10, 2); break;
        case MVC_CMN_L_MSE:                     result = QString::number(m->errorAddional(ADDITIONAL_ERROR_MSE), 10, 2); break;
        case MVC_CMN_L_LOW_BORDER:				result = QString::number(m->errorAddional(ADDITIONAL_ERROR_LOW_BORDER), 10, 2); break;
        case MVC_CMN_L_HIGH_BORDER:             result = QString::number(m->errorAddional(ADDITIONAL_ERROR_HIGH_BORDER), 10, 2); break;
        case MVC_CMN_L_VALUE_COUNT:             result = QString::number(m->measureArrayCount()); break;
        case MVC_CMN_L_VALUE_0:                 result = m->measureItemString(detailValueType, 0); break;
        case MVC_CMN_L_VALUE_1:                 result = m->measureItemString(detailValueType, 1); break;
        case MVC_CMN_L_VALUE_2:                 result = m->measureItemString(detailValueType, 2); break;
        case MVC_CMN_L_VALUE_3:                 result = m->measureItemString(detailValueType, 3); break;
        case MVC_CMN_L_VALUE_4:                 result = m->measureItemString(detailValueType, 4); break;
        case MVC_CMN_L_VALUE_5:                 result = m->measureItemString(detailValueType, 5); break;
        case MVC_CMN_L_VALUE_6:                 result = m->measureItemString(detailValueType, 6); break;
        case MVC_CMN_L_VALUE_7:                 result = m->measureItemString(detailValueType, 7); break;
        case MVC_CMN_L_VALUE_8:                 result = m->measureItemString(detailValueType, 8); break;
        case MVC_CMN_L_VALUE_9:                 result = m->measureItemString(detailValueType, 9); break;
        case MVC_CMN_L_VALUE_10:                result = m->measureItemString(detailValueType, 10); break;
        case MVC_CMN_L_VALUE_11:                result = m->measureItemString(detailValueType, 11); break;
        case MVC_CMN_L_VALUE_12:                result = m->measureItemString(detailValueType, 12); break;
        case MVC_CMN_L_VALUE_13:                result = m->measureItemString(detailValueType, 13); break;
        case MVC_CMN_L_VALUE_14:                result = m->measureItemString(detailValueType, 14); break;
        case MVC_CMN_L_VALUE_15:                result = m->measureItemString(detailValueType, 15); break;
        case MVC_CMN_L_VALUE_16:                result = m->measureItemString(detailValueType, 16); break;
        case MVC_CMN_L_VALUE_17:                result = m->measureItemString(detailValueType, 17); break;
        case MVC_CMN_L_VALUE_18:                result = m->measureItemString(detailValueType, 18); break;
        case MVC_CMN_L_VALUE_19:                result = m->measureItemString(detailValueType, 19); break;
        case MVC_CMN_L_ERROR:                   result = QString::number(m->errorInput(errorType), 10, m->errorPrecision(errorType)); break;
        case MVC_CMN_L_OUT_ERROR:               result = QString::number(m->errorOutput(errorType), 10, m->errorPrecision(errorType)); break;
        case MVC_CMN_L_LIMIT_ERROR:             result = QString::number(m->errorLimit(errorType), 10, m->errorPrecision(errorType)); break;
        case MVC_CMN_L_MEASUREMENT_TIME:		result = m->measureTimeString();
        default:                                result = ""; break;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textComparator(int row, int column) const
{
    if (row < 0 || row >= m_measureBase.count())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    MeasureItem* m = m_measureBase.at(row);
    if (m == nullptr)
    {
        return "";
    }

    QString result;

    switch(column)
    {
        case 0:

            break;

        default:
            result = "";
            break;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textComplexComparator(int row, int column) const
{
    if (row < 0 || row >= m_measureBase.count())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    MeasureItem* m = m_measureBase.at(row);
    if (m == nullptr)
    {
        return "";
    }

    QString result;

    switch(column)
    {
        case 0:

            break;

        default:
            result = "";
            break;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureTable::append(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return -1;
    }

    if (pMeasure->measureType() != m_measureType)
    {
        return -1;
    }

    int indexBase = -1;
    int indexTable = m_measureBase.count();

    beginInsertRows(QModelIndex(), indexTable, indexTable);

        indexBase = m_measureBase.append(pMeasure);

    endInsertRows();

    return indexBase;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureView::MeasureView(int type, QWidget *parent) :
    QTableView(parent),
    m_measureType(type)
{
    m_Table.setMeasureType(type);

    setModel(&m_Table);

    m_headerContextMenu = new QMenu(this);

    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MeasureView::onHeaderContextMenu);
    connect(horizontalHeader(), &QHeaderView::sectionResized, this, &MeasureView::onColumnResized);

    updateColumn();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureView::~MeasureView()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::updateColumn()
{
    m_headerContextMenu->clear();

    m_Table.header().updateColumnState();

    int count = m_Table.header().count();
    for (int index = 0; index < count; index++)
    {
        MeasureViewColumn* pColumn = m_Table.header().column(index);
        if (pColumn == nullptr)
        {
            continue;
        }

        setColumnWidth(index, pColumn->width());
        setColumnHidden(index, pColumn->enableVisible() == false );

        if(pColumn->enableVisible() == true)
        {
            QAction* pAction = m_headerContextMenu->addAction(pColumn->title());
            if (pAction != nullptr)
            {
                pAction->setCheckable(true);
                pAction->setChecked(true);
                pAction->setData(index);

                connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MeasureView::onHeaderContextAction);
            }
        }
    }

    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    verticalHeader()->setDefaultSectionSize(cellSize.height());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onHeaderContextAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    int index = action->data().toInt();
    if (index < 0 || index >= m_Table.header().count())
    {
        return;
    }

    MeasureViewColumn* pColumn = m_Table.header().column(index);
    if (pColumn == nullptr)
    {
        return;
    }

    setColumnHidden(index, action->isChecked() == false );
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onColumnResized(int index, int, int width)
{
    if (index < 0 || index >= m_Table.header().count())
    {
        return;
    }

    MeasureViewColumn* pColumn = m_Table.header().column(index);
    if (pColumn == nullptr)
    {
        return;
    }

    if (pColumn->enableVisible() == false || width == 0)
    {
        return;
    }

    pColumn->setWidth(width);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::appendMeasure(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }

    if (pMeasure->measureType() != m_measureType)
    {
        return;
    }

    int index = m_Table.append(pMeasure);
    if (index == -1)
    {
        return;
    }

    emit measureCountChanged(m_Table.count());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
