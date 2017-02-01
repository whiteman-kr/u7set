#include "SignalInfoPanel.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "SignalProperty.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalInfoTable::m_showCustomID = true;
bool SignalInfoTable::m_showElectricValue = false;
bool SignalInfoTable::m_showAdcValue = false;
bool SignalInfoTable::m_showAdcHexValue = false;

// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::SignalInfoTable(QObject*)
{
    connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &SignalInfoTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::~SignalInfoTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::columnCount(const QModelIndex&) const
{
    return SIGNAL_INFO_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::rowCount(const QModelIndex&) const
{
    return MAX_CHANNEL_COUNT;
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
    if (row < 0 || row >= MAX_CHANNEL_COUNT)
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return QVariant();
    }

    MeasureSignalParam param = signalParam(row);
    if (param.isValid() == false)
    {
        return QVariant();
    }

    AppSignalState state = theSignalBase.signalState(param.hash());

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

    if (role == Qt::BackgroundColorRole)
    {
        if (column == SIGNAL_INFO_COLUMN_STATE)
        {
            if (state.flags.underflow == true || state.flags.overflow == true || state.flags.valid == false)
            {
                return QColor(0xFF, 0xA0, 0xA0);
            }
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, param, state);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::text(const int row, const int column, const MeasureSignalParam& param, const AppSignalState& state) const
{
    if (row < 0 || row >= MAX_CHANNEL_COUNT)
    {
        return QString();
    }

    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return QString();
    }

    if (param.isValid() == false)
    {
        return QString();
    }

    QString result;

    switch (column)
    {
        case SIGNAL_INFO_COLUMN_CASE:           result = param.position().caseStr();        break;
        case SIGNAL_INFO_COLUMN_ID:             result = m_showCustomID == true ? param.customAppSignalID() : param.appSignalID();          break;
        case SIGNAL_INFO_COLUMN_STATE:          result = signalStateStr(param, state);      break;
        case SIGNAL_INFO_COLUMN_SUBBLOCK:       result = param.position().subblockStr();    break;
        case SIGNAL_INFO_COLUMN_BLOCK:          result = param.position().blockStr();       break;
        case SIGNAL_INFO_COLUMN_ENTRY:          result = param.position().entryStr();       break;
        case SIGNAL_INFO_COLUMN_CAPTION:        result = param.caption();                   break;
        case SIGNAL_INFO_COLUMN_IN_PH_RANGE:    result = param.inputPhysicalRangeStr();     break;
        case SIGNAL_INFO_COLUMN_IN_EL_RANGE:    result = param.inputElectricRangeStr();     break;
        case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = param.calibratorIndexStr(row);     break;
        case SIGNAL_INFO_COLUMN_OUT_PH_RANGE:   result = param.outputPhysicalRangeStr();    break;
        case SIGNAL_INFO_COLUMN_OUT_EL_RANGE:   result = param.outputElectricRangeStr();    break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::signalStateStr(const MeasureSignalParam& param, const AppSignalState& state) const
{
    if (param.isValid() == false)
    {
        return QString();
    }

    QString stateStr, formatStr;

    formatStr.sprintf( ("%%.%df"), param.inputPhysicalPrecise() );

    stateStr.sprintf( formatStr.toAscii(), state.value );

    int physicalUnit = param.inputPhysicalUnitID();
    if ( physicalUnit >= 0 && physicalUnit < theUnitBase.unitCount())
    {
        stateStr.append( " " + theUnitBase.unit( physicalUnit ) );
    }

    // append electrical equivalent
    //
    if (m_showElectricValue == true)
    {
        double electric = conversion( state.value, CT_PHYSICAL_TO_ELECTRIC, param);
        stateStr.append( " = " + QString::number( electric, 10, param.inputElectricPrecise() ) );

        int electricUnit = param.inputElectricUnitID();
        if ( electricUnit >= 0 && electricUnit < theUnitBase.unitCount())
        {
            stateStr.append( " " + theUnitBase.unit( electricUnit ) );
        }
    }

    // append adc equivalent in Dec
    //
    if (m_showAdcValue == true)
    {
        int adc = (state.value - param.inputPhysicalLowLimit())*(param.adcHighLimit() - param.adcLowLimit())/( param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) + param.adcLowLimit();
        stateStr.append( " = " + QString::number( adc, 10 ) );
    }

    // append adc equivalent in Hex
    //
    if (m_showAdcHexValue == true)
    {
        int adc = (state.value - param.inputPhysicalLowLimit())* (param.adcHighLimit() - param.adcLowLimit())/ ( param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) + param.adcLowLimit();
        stateStr.append( " = " + QString::number( adc, 16 ) + " h");
    }

    // check flags
    //
    if (state.flags.underflow != 0)
    {
        stateStr.append(" - Underflow");
    }

    if (state.flags.overflow != 0)
    {
        stateStr.append(" - Overflow");
    }

    if (state.flags.valid == 0)
    {
        stateStr = "No valid";
    }

    return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::updateColumn(const int column)
{
    if (column < 0 || column >= SIGNAL_INFO_COLUMN_COUNT)
    {
        return;
    }

    for (int row = 0; row < MAX_CHANNEL_COUNT; row ++)
    {
        QModelIndex cellIndex = index(row, column);

        emit dataChanged( cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
    }
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam SignalInfoTable::signalParam(const int index) const
{
    MeasureSignalParam param;

    m_signalMutex.lock();

        if (index >= 0 || index < MAX_CHANNEL_COUNT)
        {
            param = m_activeSignalParam[index];
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::set(const MeasureMultiSignal& activeSignal)
{
    clear();

    if (activeSignal.isEmpty() == true)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, MAX_CHANNEL_COUNT - 1);

        m_signalMutex.lock();

            for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
            {
                Hash signalHash = activeSignal.hash(i);
                if (signalHash == 0)
                {
                    continue;
                }

                m_activeSignalParam[i] = theSignalBase.signalParam(signalHash);
            }

        m_signalMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::clear()
{
    beginRemoveRows(QModelIndex(), 0, MAX_CHANNEL_COUNT - 1 );

        m_signalMutex.lock();

            for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
            {
                m_activeSignalParam[i].setAppSignalID(QString());
            }

        m_signalMutex.unlock();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::updateSignalParam(const Hash& signalHash)
{
    if (signalHash == 0)
    {
        assert(signalHash != 0);
        return;
    }

    m_signalMutex.lock();

        for(int i = 0; i < MAX_CHANNEL_COUNT; i ++)
        {
            if (m_activeSignalParam[i].hash() == signalHash)
            {
                m_activeSignalParam[i] = theSignalBase.signalParam(signalHash);

                break;
            }
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int SignalInfoPanel::m_columnWidth[SIGNAL_INFO_COLUMN_COUNT] =
{
    100,    // SIGNAL_INFO_COLUMN_CASE
    270,    // SIGNAL_INFO_COLUMN_ID
    150,    // SIGNAL_INFO_COLUMN_STATE
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

    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
    if (pMainWindow != nullptr)
    {
        connect(pMainWindow, &MainWindow::setActiveSignal, this, &SignalInfoPanel::onSetActiveSignal, Qt::QueuedConnection);

        if (pMainWindow->m_pSignalSocket != nullptr)
        {
            connect(pMainWindow->m_pSignalSocket, &SignalSocket::socketDisconnected, this, &SignalInfoPanel::onSetActiveSignal, Qt::QueuedConnection);
        }
    }

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
    m_pView->setModel(&m_signalParamTable);
    QSize cellSize = QFontMetrics( theOptions.measureView().font() ).size(Qt::TextSingleLine,"A");
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
    m_pContextMenu = new QMenu(tr(""), m_pSignalInfoWindow);

    m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

    m_pContextMenu->addSeparator();

    m_pShowMenu = new QMenu(tr("Show"), m_pSignalInfoWindow);

    m_pShowCustomIDAction = m_pShowMenu->addAction(tr("Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(true);

    m_pShowMenu->addSeparator();

    m_pShowElectricValueAction = m_pShowMenu->addAction(tr("Electrical"));
    m_pShowElectricValueAction->setCheckable(true);
    m_pShowElectricValueAction->setChecked(false);


    m_pShowAdcValueAction = m_pShowMenu->addAction(tr("ADC"));
    m_pShowAdcValueAction->setCheckable(true);
    m_pShowAdcValueAction->setChecked(false);

    m_pShowAdcHexValueAction = m_pShowMenu->addAction(tr("ADC (hex)"));
    m_pShowAdcHexValueAction->setCheckable(true);
    m_pShowAdcHexValueAction->setChecked(false);

    m_pContextMenu->addMenu(m_pShowMenu);

    m_pContextMenu->addSeparator();

    m_pSignalPropertyAction = m_pContextMenu->addAction(tr("Properties ..."));
    m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

    connect(m_pCopyAction, &QAction::triggered, this, &SignalInfoPanel::copy);
    connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalInfoPanel::showCustomID);
    connect(m_pShowElectricValueAction, &QAction::triggered, this, &SignalInfoPanel::showElectricValue);
    connect(m_pShowAdcValueAction, &QAction::triggered, this, &SignalInfoPanel::showAdcValue);
    connect(m_pShowAdcHexValueAction, &QAction::triggered, this, &SignalInfoPanel::showAdcHexValue);
    connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalInfoPanel::signalProperty);


    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalInfoPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::hideColumn(const int column, const bool hide)
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
        connect(m_updateSignalStateTimer, &QTimer::timeout, this, &SignalInfoPanel::updateSignalState);
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
    m_signalParamTable.set( theSignalBase.activeSignal() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateSignalState()
{
    m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showCustomID()
{
    m_signalParamTable.setShowCustomID( m_pShowCustomIDAction->isChecked() );
    m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_ID);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showElectricValue()
{
    m_signalParamTable.setShowElectricValue( m_pShowElectricValueAction->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showAdcValue()
{
    m_signalParamTable.setShowAdcValue( m_pShowAdcValueAction->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showAdcHexValue()
{
    m_signalParamTable.setShowAdcHexValue( m_pShowAdcHexValueAction->isChecked() );
}


// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::copy()
{
    QString textClipboard;

    int rowCount = m_pView->model()->rowCount();
    int columnCount = m_pView->model()->columnCount();

    for(int row = 0; row < rowCount; row++)
    {
        for(int column = 0; column < columnCount; column++)
        {
            if (m_pView->selectionModel()->isSelected( m_pView->model()->index(row, column) ) == false)
            {
                continue;
            }

            textClipboard.append(m_pView->model()->data( m_pView->model()->index(row, column)).toString() + "\t");
        }

        textClipboard.replace(textClipboard.length() - 1, 1, "\n");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::signalProperty()
{
    int index = m_pView->currentIndex().row();
    if (index < 0 || index >= m_signalParamTable.signalCount())
    {
        return;
    }

    MeasureSignalParam param = m_signalParamTable.signalParam(index);
    if (param.isValid() == false)
    {
        return;
    }

    SignalPropertyDialog dialog( param );
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
