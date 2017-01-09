#include "SignalInfoPanel.h"

#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QClipboard>

#include "MainWindow.h"
#include "MeasureView.h"
#include "Options.h"
#include "Delegate.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::SignalInfoTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::~SignalInfoTable()
{
    m_activeSignal.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::columnCount(const QModelIndex&) const
{
    return SIGNAL_INFO_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::rowCount(const QModelIndex&) const
{
    return MEASURE_MULTI_SIGNAL_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalInfoTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < SIGNAL_INFO_COLUMN_COUNT)
        {
            result = SignalInfoColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalInfoTable::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
    {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case SIGNAL_INFO_COLUMN_CASE:           result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_ID:             result = Qt::AlignLeft;     break;
            case SIGNAL_INFO_COLUMN_STATE:          result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_SUBBLOCK:       result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_BLOCK:          result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_ENTRY:          result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_CAPTION:        result = Qt::AlignLeft;     break;
            case SIGNAL_INFO_COLUMN_ADJUSTMENT:     result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_IN_PH_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_IN_EL_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = Qt::AlignLeft;     break;
            case SIGNAL_INFO_COLUMN_OUT_PH_RANGE:   result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_OUT_EL_RANGE:   result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_activeSignal.signal(row));
        return var;
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (column == SIGNAL_INFO_COLUMN_STATE)
        {
            MeasureSignal* pSignal = m_activeSignal.signal(row);
            if (pSignal != nullptr)
            {
                if (pSignal->state().flags.valid == 0)
                {
                    return QColor(0xFF, 0x00, 0x00);
                }
            }
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::text(int row, int column) const
{
    if (row < 0 || row >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return "";
    }

    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return "";
    }

    MeasureSignal* pSignal = m_activeSignal.signal(row);
    if (pSignal == nullptr)
    {
        return "";
    }

    QString result;

    switch (column)
    {
        case SIGNAL_INFO_COLUMN_CASE:           result = pSignal->position().caseString();      break;
        case SIGNAL_INFO_COLUMN_ID:             result = pSignal->param().appSignalID();        break;
        case SIGNAL_INFO_COLUMN_STATE:          result = pSignal->stateString();                break;
        case SIGNAL_INFO_COLUMN_SUBBLOCK:       result = pSignal->position().subblockString();  break;
        case SIGNAL_INFO_COLUMN_BLOCK:          result = pSignal->position().blockString();     break;
        case SIGNAL_INFO_COLUMN_ENTRY:          result = pSignal->position().entryString();     break;
        case SIGNAL_INFO_COLUMN_CAPTION:        result = pSignal->param().caption();            break;
        case SIGNAL_INFO_COLUMN_ADJUSTMENT:     result = pSignal->adjustmentString();           break;
        case SIGNAL_INFO_COLUMN_IN_PH_RANGE:    result = pSignal->inputPhysicalRange();         break;
        case SIGNAL_INFO_COLUMN_IN_EL_RANGE:    result = pSignal->inputElectricRange();         break;
        case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = "";                                    break;
        case SIGNAL_INFO_COLUMN_OUT_PH_RANGE:   result = pSignal->outputPhysicalRange();        break;
        case SIGNAL_INFO_COLUMN_OUT_EL_RANGE:   result = pSignal->outputElectricRange();        break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::updateState()
{
    for (int row = 0; row < MEASURE_MULTI_SIGNAL_COUNT; row ++)
    {
        QModelIndex cellIndex = index(row, SIGNAL_INFO_COLUMN_STATE);

        emit dataChanged( cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
    }
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal* SignalInfoTable::at(int index)
{
    if (index < 0 || index >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return nullptr;
    }

    return m_activeSignal.signal(index);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::set(MeasureMultiSignal& signal)
{
    if (signal.isEmpty() == true)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, MEASURE_MULTI_SIGNAL_COUNT - 1);

        m_activeSignal = signal;

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::clear()
{
    beginRemoveRows(QModelIndex(), 0, MEASURE_MULTI_SIGNAL_COUNT - 1 );

        m_activeSignal.clear();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int SignalInfoPanel::m_columnWidth[SIGNAL_INFO_COLUMN_COUNT] =
{
    100,    // SIGNAL_INFO_COLUMN_CASE
    270,    // SIGNAL_INFO_COLUMN_ID
    100,    // SIGNAL_INFO_COLUMN_STATE
     60,    // SIGNAL_INFO_COLUMN_SUBBLOCK
     60,    // SIGNAL_INFO_COLUMN_BLOCK
     60,    // SIGNAL_INFO_COLUMN_ENTRY
    150,    // SIGNAL_INFO_COLUMN_CAPTION
    100,    // SIGNAL_INFO_COLUMN_ADJUSTMENT
    150,    // SIGNAL_INFO_COLUMN_IN_PH_RANGE
    150,    // SIGNAL_INFO_COLUMN_IN_EL_RANGE
    100,    // SIGNAL_INFO_COLUMN_CALIBRATOR
    150,    // SIGNAL_INFO_COLUMN_OUT_PH_RANGE
    150,    // SIGNAL_INFO_COLUMN_OUT_EL_RANGE
};

// -------------------------------------------------------------------------------------------------------------------

SignalInfoPanel::SignalInfoPanel(QWidget* parent) :
    QDockWidget(parent)
{
    m_pMainWindow = dynamic_cast<QMainWindow*> (parent);
    if (m_pMainWindow == nullptr)
    {
        return;
    }

    connect((const MainWindow*) m_pMainWindow, &MainWindow::setActiveSignal, this, &SignalInfoPanel::onSetActiveSignal, Qt::QueuedConnection);

    setWindowTitle("Panel signal information");
    setObjectName(windowTitle());

    createInterface();
    createHeaderContexMenu();
    createContextMenu();

    hideColumn(SIGNAL_INFO_COLUMN_SUBBLOCK, true);
    hideColumn(SIGNAL_INFO_COLUMN_BLOCK, true);
    hideColumn(SIGNAL_INFO_COLUMN_ENTRY, true);
    hideColumn(SIGNAL_INFO_COLUMN_ADJUSTMENT, true);
    hideColumn(SIGNAL_INFO_COLUMN_CALIBRATOR, true);
    hideColumn(SIGNAL_INFO_COLUMN_OUT_PH_RANGE, true);
    hideColumn(SIGNAL_INFO_COLUMN_OUT_EL_RANGE, true);

    startSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoPanel::~SignalInfoPanel()
{
    stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createInterface()
{
    m_pSignalInfoWindow = new QMainWindow;

    m_pView = new QTableView(m_pSignalInfoWindow);
    m_pView->setModel(&m_table);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    m_pSignalInfoWindow->setCentralWidget(m_pView);

    for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    setWidget(m_pSignalInfoWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createHeaderContexMenu()
{
    // init header context menu
    //
    m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &SignalInfoPanel::onHeaderContextMenu);

    m_headerContextMenu = new QMenu(m_pView);

    for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
    {
        m_pColumnAction[column] = m_headerContextMenu->addAction(SignalInfoColumn[column]);
        if (m_pColumnAction[column] != nullptr)
        {
            m_pColumnAction[column]->setCheckable(true);
            m_pColumnAction[column]->setChecked(true);

            connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalInfoPanel::onColumnAction);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr("&Measurements"), m_pSignalInfoWindow);


    m_pShowCustomIDAction = m_pContextMenu->addAction(tr("Show Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(false);

    m_pChangeRangeAction = m_pContextMenu->addAction(tr("Change signal range"));

    m_pContextMenu->addSeparator();

    m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

    connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalInfoPanel::showCustomID);
    connect(m_pChangeRangeAction, &QAction::triggered, this, &SignalInfoPanel::changeRange);
    connect(m_pCopyAction, &QAction::triggered, this, &SignalInfoPanel::copy);

    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalInfoPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::hideColumn(int column, bool hide)
{
    if (column < 0 || column >= SIGNAL_INFO_COLUMN_COUNT)
    {
        return;
    }

    if (hide == true)
    {
        m_pView->hideColumn(column);
        m_pColumnAction[column]->setChecked(false);
    }
    else
    {
        m_pView->showColumn(column);
        m_pColumnAction[column]->setChecked(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::startSignalStateTimer()
{
    if (m_updateSignalStateTimer == nullptr)
    {
        m_updateSignalStateTimer = new QTimer(this);
        connect(m_updateSignalStateTimer, &QTimer::timeout, this, &SignalInfoPanel::updateStateActiveSignal);
    }

    m_updateSignalStateTimer->start(100); //   100 ms
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::stopSignalStateTimer()
{
    if (m_updateSignalStateTimer != nullptr)
    {
        m_updateSignalStateTimer->stop();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onSetActiveSignal()
{
    m_table.clear();
    m_table.set( theSignalBase.activeSignal() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateStateActiveSignal()
{
    m_table.updateState();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showCustomID()
{

}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::changeRange()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::copy()
{
    bool appendRow;
    QString textRow;
    QString textClipboard;

    int count = m_table.count();
    for(int r = 0; r < count; r++)
    {
        appendRow = false;
        textRow = "";

        for(int c = 0; c < SIGNAL_INFO_COLUMN_COUNT; c++)
        {
            if (m_pView->selectionModel()->isSelected( m_pView->model()->index(r,c) ) == true)
            {
                appendRow = true;
                textRow.append(m_table.text(r, c));
            }

            if (c != FM_COLUMN_COUNT - 1)
            {
                textRow.append("\t");
            }
        }

        if (appendRow == true)
        {
            textClipboard.append(textRow);
            textClipboard.append("\n");
        }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onColumnAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
    {
        if (m_pColumnAction[column] == action)
        {
            hideColumn(column,  !action->isChecked());

            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
