#include "SignalInfoPanel.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "SignalProperty.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalInfoTable::m_showCustomID = true;

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
            case SIGNAL_INFO_COLUMN_IN_PH_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_IN_EL_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_OUT_PH_RANGE:   result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_OUT_EL_RANGE:   result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_activeSignal.hash(row));
        return var;
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (column == SIGNAL_INFO_COLUMN_STATE)
        {
            Hash signaHash = m_activeSignal.hash(row);
            if (signaHash != 0)
            {
                MeasureSignal signal = theSignalBase.signal(signaHash);
                if (signal.param().appSignalID().isEmpty() == false || signal.param().hash() != 0)
                {
                    if (signal.state().flags.valid == 0)
                    {
                        return QColor(0xFF, 0xA0, 0xA0);
                    }
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

QString SignalInfoTable::text(const int &row, const int &column) const
{
    if (row < 0 || row >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return "";
    }

    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return "";
    }

    Hash signaHash = m_activeSignal.hash(row);
    if (signaHash == 0)
    {
        return "";
    }

    MeasureSignal signal = theSignalBase.signal(signaHash);
    if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
    {
        return "";
    }

    QString result;

    switch (column)
    {
        case SIGNAL_INFO_COLUMN_CASE:           result = signal.position().caseString();      break;
        case SIGNAL_INFO_COLUMN_ID:             result = m_showCustomID == true ? signal.param().customAppSignalID() : signal.param().appSignalID();  break;
        case SIGNAL_INFO_COLUMN_STATE:          result = signal.stateString();                break;
        case SIGNAL_INFO_COLUMN_SUBBLOCK:       result = signal.position().subblockString();  break;
        case SIGNAL_INFO_COLUMN_BLOCK:          result = signal.position().blockString();     break;
        case SIGNAL_INFO_COLUMN_ENTRY:          result = signal.position().entryString();     break;
        case SIGNAL_INFO_COLUMN_CAPTION:        result = signal.param().caption();            break;
        case SIGNAL_INFO_COLUMN_IN_PH_RANGE:    result = signal.inputPhysicalRange();         break;
        case SIGNAL_INFO_COLUMN_IN_EL_RANGE:    result = signal.inputElectricRange();         break;
        case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = signal.calibratorIndexString(row);   break;
        case SIGNAL_INFO_COLUMN_OUT_PH_RANGE:   result = signal.outputPhysicalRange();        break;
        case SIGNAL_INFO_COLUMN_OUT_EL_RANGE:   result = signal.outputElectricRange();        break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::updateColumn(const int& column)
{
    if (column < 0 || column >= SIGNAL_INFO_COLUMN_COUNT)
    {
        return;
    }

    for (int row = 0; row < MEASURE_MULTI_SIGNAL_COUNT; row ++)
    {
        QModelIndex cellIndex = index(row, column);

        emit dataChanged( cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
    }
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalInfoTable::at(int index)
{
    if (index < 0 || index >= count())
    {
        return 0;
    }

    return m_activeSignal.hash(index);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::set(const MeasureMultiSignal& multiSignal)
{
    if (multiSignal.isEmpty() == true)
    {
        return;
    }

    clear();

    beginInsertRows(QModelIndex(), 0, MEASURE_MULTI_SIGNAL_COUNT - 1);

        m_activeSignal = multiSignal;

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
    150,    // SIGNAL_INFO_COLUMN_IN_PH_RANGE
    150,    // SIGNAL_INFO_COLUMN_IN_EL_RANGE
    150,    // SIGNAL_INFO_COLUMN_CALIBRATOR
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

    m_pSignalInfoWindow->installEventFilter(this);

    m_pView = new QTableView(m_pSignalInfoWindow);
    m_pView->setModel(&m_table);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    m_pSignalInfoWindow->setCentralWidget(m_pView);

    for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    connect(m_pView, &QTableView::doubleClicked , this, &SignalInfoPanel::onListDoubleClicked);

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

    m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

    m_pShowCustomIDAction = m_pContextMenu->addAction(tr("Show Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(true);

    m_pContextMenu->addSeparator();

    m_pSignalPropertyAction = m_pContextMenu->addAction(tr("Properties"));
    m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

    connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalInfoPanel::showCustomID);
    connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalInfoPanel::signalProperty);
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
    m_pShowCustomIDAction->setChecked(m_table.showCustomID());

    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalInfoPanel::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>( event );

        if (keyEvent->key() == Qt::Key_Return)
        {
            signalProperty();
        }
    }

    return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onSetActiveSignal()
{
    m_table.set( theSignalBase.activeSignal() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateStateActiveSignal()
{
    m_table.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showCustomID()
{
    m_table.setShowCustomID( m_pShowCustomIDAction->isChecked() );
    m_table.updateColumn(SIGNAL_INFO_COLUMN_ID);
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

                if (c != SIGNAL_INFO_COLUMN_COUNT - 1)
                {
                    textRow.append("\t");
                }
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

void SignalInfoPanel::signalProperty()
{
    int index = m_pView->currentIndex().row();
    if (index < 0 || index >= m_table.count())
    {
        return;
    }

    Hash hash = m_table.at(index);
    if (hash == 0)
    {
        return;
    }

    MeasureSignal signal = theSignalBase.signal(hash);
    if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
    {
        return;
    }

    SignalPropertyDialog dialog( signal.param().hash() );
    dialog.exec();
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
