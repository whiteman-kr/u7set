#include "MeasureView.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QClipboard>

#include "Database.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureTable::MeasureTable(QObject*)
{
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
    m_measureBase.load(type);
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
    return m_measureBase.measurementCount();
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
    if (indexRow < 0 || indexRow >= m_measureBase.measurementCount())
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
        return backgroundColor(indexRow, indexColumn);
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(indexRow, indexColumn);
    }

    return QVariant();
}
// -------------------------------------------------------------------------------------------------------------------

QColor MeasureTable::backgroundColor(int row, int column) const
{
    if (row < 0 || row >= m_measureBase.measurementCount())
    {
        return Qt::white;
    }

    if (column < 0 || column > m_header.count())
    {
        return Qt::white;
    }

    MeasureViewColumn* pColumn = m_header.column(column);
    if (pColumn == nullptr)
    {
        return Qt::white;
    }

    QColor result = pColumn->color();

    if (column != MVC_CMN_L_ERROR)
    {
        return pColumn->color();
    }

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
            {

                LinearityMeasurement* pLinearityMeasurement = static_cast<LinearityMeasurement*> (m_measureBase.measurement(row));
                if (pLinearityMeasurement == nullptr)
                {
                    break;
                }

                int errorType = theOptions.linearity().m_errorType;
                if (errorType < 0 || errorType >= ERROR_TYPE_COUNT)
                {
                    break;
                }

                if ( pLinearityMeasurement->errorInput(errorType) > pLinearityMeasurement->errorLimit(errorType) )
                {
                    result = QColor(0xFF, 0xA0, 0xA0);
                }
            }
            break;

        case MEASURE_TYPE_COMPARATOR:

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            break;

        default:
            assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::text(int row, int column) const
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return "";
    }

    if (row < 0 || row >= m_measureBase.measurementCount())
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
    if (row < 0 || row >= m_measureBase.measurementCount())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    LinearityMeasurement* m = static_cast<LinearityMeasurement*> (m_measureBase.measurement(row));
    if (m == nullptr)
    {
        return "";
    }

    int detailValueType = VALUE_TYPE_ELECTRIC;

    if (theOptions.linearity().m_viewType == LO_VIEW_TYPE_DETAIL_PHYSICAL)
    {
        detailValueType = VALUE_TYPE_PHYSICAL;
    }

    int errorType = theOptions.linearity().m_errorType;

    QString result;

    switch(column)
    {
        case MVC_CMN_L_INDEX:					result = QString::number(m->measureID()); break;

        case MVC_CMN_L_CASE:					result = m->position().caseString(); break;
        case MVC_CMN_L_ID:                      result = theOptions.measureView().m_showExternalID ? m->customAppSignalID() : m->appSignalID(); break;
        case MVC_CMN_L_NAME:					result = m->name(); break;

        case MVC_CMN_L_SUBBLOCK:				result = m->position().subblockString(); break;
        case MVC_CMN_L_BLOCK:					result = m->position().blockString(); break;
        case MVC_CMN_L_ENTRY:					result = m->position().entryString(); break;

        case MVC_CMN_L_EL_NOMINAL:				result = m->nominalString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_NOMINAL:				result = m->nominalString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_NOMINAL:				result = m->nominalString(VALUE_TYPE_OUTPUT); break;

        case MVC_CMN_L_PERCENT:					result = QString::number(m->percent(), 10, 2); break;

        case MVC_CMN_L_EL_MEASURE:				result = m->measureString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_MEASURE:				result = m->measureString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_MEASURE:				result = m->measureString(VALUE_TYPE_OUTPUT); break;

        case MVC_CMN_L_EL_RANGE:				result = m->limitString(VALUE_TYPE_ELECTRIC); break;
        case MVC_CMN_L_PH_RANGE:				result = m->limitString(VALUE_TYPE_PHYSICAL); break;
        case MVC_CMN_L_OUT_RANGE:				result = m->limitString(VALUE_TYPE_OUTPUT); break;

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

        case MVC_CMN_L_SYSTEM_ERROR:			result = QString::number(m->additionalValue(ADDITIONAL_VALUE_SYSTEM_ERROR), 10, 2); break;
        case MVC_CMN_L_MSE:                     result = QString::number(m->additionalValue(ADDITIONAL_VALUE_MSE), 10, 2); break;
        case MVC_CMN_L_LOW_BORDER:				result = QString::number(-m->additionalValue(ADDITIONAL_VALUE_LOW_HIGH_BORDER), 10, 2); break;
        case MVC_CMN_L_HIGH_BORDER:             result = QString::number(m->additionalValue(ADDITIONAL_VALUE_LOW_HIGH_BORDER), 10, 2); break;

        case MVC_CMN_L_ERROR:                   result = QString::number(m->errorInput(errorType), 10, m->errorPrecision(errorType)); break;
        case MVC_CMN_L_OUT_ERROR:               result = QString::number(m->errorOutput(errorType), 10, m->errorPrecision(errorType)); break;
        case MVC_CMN_L_LIMIT_ERROR:             result = QString::number(m->errorLimit(errorType), 10, m->errorPrecision(errorType)); break;

        case MVC_CMN_L_MEASUREMENT_TIME:		result = m->measureTime().toString("dd-MM-yyyy hh:mm:ss"); break;

        default:                                result = QString(); break;
    }

    if (row > 0)
    {
        Measurement* prev_m = m_measureBase.measurement(row - 1);
        if (prev_m != nullptr)
        {
            if ( prev_m->signalHash() == m->signalHash() )
            {
                if (column <= MVC_CMN_L_ENTRY)
                {
                    result = "";
                }
            }
        }
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureTable::textComparator(int row, int column) const
{
    if (row < 0 || row >= m_measureBase.measurementCount())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    Measurement* m = m_measureBase.measurement(row);
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
    if (row < 0 || row >= m_measureBase.measurementCount())
    {
        return "";
    }

    if (column < 0 || column > m_header.count())
    {
        return "";
    }

    Measurement* m = m_measureBase.measurement(row);
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

bool MeasureTable::append(Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return false;
    }

    if (pMeasurement->measureType() != m_measureType)
    {
        return false;
    }

    if (thePtrDB == nullptr)
    {
        return false;
    }

    if (thePtrDB->appendMeasure(pMeasurement) == false)
    {
        QMessageBox::critical(nullptr, tr("Append measurements"), tr("Error append measurements to database"));
        return false;
    }

    int indexTable = m_measureBase.measurementCount();

    beginInsertRows(QModelIndex(), indexTable, indexTable);

        m_measureBase.append(pMeasurement);

    endInsertRows();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureTable::remove(const QList<int> removeIndexList)
{
    if (thePtrDB == nullptr)
    {
        return false;
    }

    QVector<int> keyList;

    // remove from database
    //
    int count = removeIndexList.count();
    for(int index = 0; index < count; index++)
    {
        int removeIndex = removeIndexList.at(index);

        Measurement* pMeasuremet = m_measureBase.measurement(removeIndex);
        if (pMeasuremet == nullptr)
        {
            continue;
        }

        if (pMeasuremet->measureType() != m_measureType)
        {
            continue;
        }

        keyList.append(pMeasuremet->measureID());
    }

    if (thePtrDB->removeMeasure(m_measureType, keyList) == false)
    {
        QMessageBox::critical(nullptr, tr("Delete measurements"), tr("Error remove measurements from database"));
        return false;
    }

    // remove from MeasureBase
    //
    for(int index = count-1; index >= 0; index--)
    {
        int removeIndex = removeIndexList.at(index);

        Measurement* pMeasurement = m_measureBase.measurement(removeIndex);
        if (pMeasurement == nullptr)
        {
            continue;
        }

        if (pMeasurement->measureType() != m_measureType)
        {
            continue;
        }

        beginRemoveRows(QModelIndex(), removeIndex, removeIndex);

            m_measureBase.remove(removeIndex);

        endRemoveRows();
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureView::MeasureView(int type, QWidget *parent) :
    QTableView(parent),
    m_measureType(type)
{
    m_table.setMeasureType(type);
    setModel(&m_table);

    //setSelectionBehavior(QAbstractItemView::SelectRows);

    createContextMenu();

    updateColumn();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureView::~MeasureView()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::createContextMenu()
{
    // create header context menu
    //
    m_headerContextMenu = new QMenu(this);

    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MeasureView::onHeaderContextMenu);
    connect(horizontalHeader(), &QHeaderView::sectionResized, this, &MeasureView::onColumnResized);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::updateColumn()
{
    m_headerContextMenu->clear();

    m_table.header().updateColumnState();

    int count = m_table.header().count();
    for (int index = 0; index < count; index++)
    {
        MeasureViewColumn* pColumn = m_table.header().column(index);
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
    if (index < 0 || index >= m_table.header().count())
    {
        return;
    }

    MeasureViewColumn* pColumn = m_table.header().column(index);
    if (pColumn == nullptr)
    {
        return;
    }

    setColumnHidden(index, action->isChecked() == false );
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::onColumnResized(int index, int, int width)
{
    if (index < 0 || index >= m_table.header().count())
    {
        return;
    }

    MeasureViewColumn* pColumn = m_table.header().column(index);
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

void MeasureView::appendMeasure(Measurement* pMeasurement)
{
    if (pMeasurement == nullptr)
    {
        return;
    }

    if (pMeasurement->measureType() != m_measureType)
    {
        return;
    }

    if (m_table.append(pMeasurement) == false)
    {
        return;
    }

    emit measureCountChanged(m_table.count());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::removeMeasure()
{
    int measureCount = m_table.count();
    int columnCount = m_table.header().count();

    if (measureCount == 0 || columnCount == 0)
    {
        return;
    }

    bool removeMeasure;
    QList<int> removeIndexList;

    for(int index = 0; index < measureCount; index++)
    {
        removeMeasure = false;

        for(int c = 0; c < columnCount; c++)
        {
            if (selectionModel()->isSelected( model()->index(index, c) ) == true)
            {
                removeMeasure = true;
            }
        }

        if (removeMeasure == true)
        {
            removeIndexList.append(index);
        }
    }

    if (removeIndexList.count() == 0)
    {
        return;
    }

    if (QMessageBox::question(this, tr("Confirm delete"), "Delete selected measurements?") == QMessageBox::No)
    {
        return;
    }

    if (m_table.remove(removeIndexList) == false)
    {
        return;
    }

    emit measureCountChanged(m_table.count());

    QMessageBox::information(this, tr("Delete"), tr("Deleted %1 measurement(s)").arg(removeIndexList.count()));
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureView::copy()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText("");

    int rowCount = m_table.count();
    int columnCount = m_table.header().count();

    if (rowCount == 0 || columnCount == 0)
    {
        return;
    }

    bool copyMeasure;

    QString textRow;
    QString textClipboard;

    // copy header column title
    //
    for(int c = 0; c < columnCount; c++)
    {
        MeasureViewColumn* pColumn = m_table.header().column(c);
        if (pColumn == nullptr)
        {
            continue;
        }

        if (pColumn->enableVisible() == false)
        {
            continue;
        }

        textRow.append(pColumn->title());
        textRow.append("\t");
    }

    textRow.remove(textRow.count() - 1, 1);

    textClipboard.append(textRow);
    textClipboard.append("\n");

    // copy measure
    //
    for(int r = 0; r < rowCount; r++)
    {
        copyMeasure = false;
        textRow = "";

        for(int c = 0; c < columnCount; c++)
        {
            MeasureViewColumn* pColumn = m_table.header().column(c);
            if (pColumn == nullptr)
            {
                continue;
            }

            if (pColumn->enableVisible() == false)
            {
                continue;
            }

            if (selectionModel()->isSelected( model()->index(r,c) ) == true)
            {
                copyMeasure = true;
            }

            textRow.append(m_table.text(r, c));
            textRow.append("\t");
        }

        textRow.remove(textRow.count() - 1, 1);

        if (copyMeasure == true)
        {
            textClipboard.append(textRow);
            textClipboard.append("\n");
        }
    }

    clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
