#include "SignalInfoPanel.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QClipboard>

#include "Options.h"
#include "SignalProperty.h"
#include "Conversion.h"
#include "CalibratorBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
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

    MeasureParam measureParam = signalParam(row);
    if (measureParam.isValid() == false)
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
            case SIGNAL_INFO_COLUMN_PH_RANGE:       result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_EL_RANGE:       result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_EL_SENSOR:      result = Qt::AlignCenter;   break;
            case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }

    if (role == Qt::FontRole)
    {
        return theOptions.signalInfo().font();
    }

    if (role == Qt::TextColorRole)
    {
        if (column == SIGNAL_INFO_COLUMN_PH_RANGE && measureParam.equalPhysicalRange() == false)
        {
            return QColor( Qt::red );
        }

        if (column == SIGNAL_INFO_COLUMN_CALIBRATOR)
        {
            if (measureParam.calibratorManager() == nullptr || measureParam.calibratorManager()->calibratorIsConnected() == false)
            {
                return QColor( Qt::lightGray );
            }
        }

        return QVariant();
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (column == SIGNAL_INFO_COLUMN_STATE)
        {
            AppSignalState state;

            if (measureParam.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
            {
                state = theSignalBase.signalState(measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT).hash());
            }
            else
            {
                state = theSignalBase.signalState(measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT).hash());
            }

            if (state.flags.valid == false)
            {
                return theOptions.signalInfo().colorFlagValid();
            }

            if (state.flags.overflow == true)
            {
                return theOptions.signalInfo().colorFlagOverflow();
            }

            if (state.flags.underflow == true)
            {
                return theOptions.signalInfo().colorFlagUnderflow();
            }
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, measureParam);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::text(int row, int column, const MeasureParam& measureParam) const
{
    if (row < 0 || row >= MAX_CHANNEL_COUNT)
    {
        return QString();
    }

    if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
    {
        return QString();
    }

    if (measureParam.isValid() == false)
    {
        return QString();
    }

    bool showCustomID = theOptions.signalInfo().showCustomID();

    const QString divider = "\n";

    QString stateStr;

    if (column == SIGNAL_INFO_COLUMN_STATE)
    {
		Metrology::SignalParam inParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
        if (inParam.isValid() == true)
        {
            AppSignalState inState = theSignalBase.signalState(inParam.hash());
            stateStr = signalStateStr(inParam, inState);
        }

        if (measureParam.outputSignalType() != OUTPUT_SIGNAL_TYPE_UNUSED)
        {
			Metrology::SignalParam outParam = measureParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
            if (outParam.isValid() == true)
            {
                AppSignalState outState = theSignalBase.signalState(outParam.hash());
                stateStr += divider + signalStateStr(outParam, outState);
            }
        }
    }

    QString result;

    switch (column)
    {
        case SIGNAL_INFO_COLUMN_CASE:           result = measureParam.rackCaption();                        break;
        case SIGNAL_INFO_COLUMN_ID:             result = measureParam.signalID(showCustomID, divider);  break;
        case SIGNAL_INFO_COLUMN_STATE:          result = stateStr;                                      break;
        case SIGNAL_INFO_COLUMN_SUBBLOCK:       result = measureParam.chassisStr();                    break;
        case SIGNAL_INFO_COLUMN_BLOCK:          result = measureParam.moduleStr();                       break;
        case SIGNAL_INFO_COLUMN_ENTRY:          result = measureParam.placeStr();                       break;
        case SIGNAL_INFO_COLUMN_CAPTION:        result = measureParam.caption(divider);                 break;
        case SIGNAL_INFO_COLUMN_PH_RANGE:       result = measureParam.physicalRangeStr(divider);        break;
        case SIGNAL_INFO_COLUMN_EL_RANGE:       result = measureParam.electricRangeStr(divider);        break;
        case SIGNAL_INFO_COLUMN_EL_SENSOR:      result = measureParam.electricSensorStr(divider);       break;
        case SIGNAL_INFO_COLUMN_CALIBRATOR:     result = measureParam.calibratorStr();                  break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::signalStateStr(const Metrology::SignalParam& param, const AppSignalState& state) const
{
    if (param.isValid() == false)
    {
        return QString();
    }

    if (state.flags.valid == false)
    {
        return tr("No valid");
    }

    QString stateStr, formatStr;

    formatStr.sprintf( ("%%.%df"), param.inputPhysicalPrecision() );

    stateStr.sprintf( formatStr.toAscii(), state.value );

    if ( theUnitBase.hasUnit( param.inputPhysicalUnitID() ) == true)
    {
        stateStr.append( " " + theUnitBase.unit( param.inputPhysicalUnitID() ) );
    }

    // append electrical equivalent
    //
    if (theOptions.signalInfo().showElectricState() == true)
    {
        if (param.isInput() == true)
        {
            double electric = conversion( state.value, CT_PHYSICAL_TO_ELECTRIC, param);
            stateStr.append( " = " + QString::number( electric, 10, param.inputElectricPrecision() ) );

            if ( theUnitBase.hasUnit( param.inputElectricUnitID() ) == true)
            {
                stateStr.append( " " + theUnitBase.unit( param.inputElectricUnitID() ) );
            }
        }

        if (param.isOutput() == true)
        {
            double electric = (state.value - param.inputPhysicalLowLimit()) * (param.outputElectricHighLimit() - param.outputElectricLowLimit()) / (param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) + param.outputElectricLowLimit();
            stateStr.append( " = " + QString::number( electric, 10, param.outputElectricPrecision() ) );

            if ( theUnitBase.hasUnit( param.outputElectricUnitID() ) == true)
            {
                stateStr.append( " " + theUnitBase.unit( param.outputElectricUnitID() ) );
            }
        }
    }

    // append adc equivalent in Dec
    //
    if (theOptions.signalInfo().showAdcState() == true)
    {
        int adc = (state.value - param.inputPhysicalLowLimit())*(param.highADC() - param.lowADC())/( param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) + param.lowADC();
        stateStr.append( " = " + QString::number( adc, 10 ) );
    }

    // append adc equivalent in Hex
    //
    if (theOptions.signalInfo().showAdcHexState() == true)
    {
        int adc = (state.value - param.inputPhysicalLowLimit())* (param.highADC() - param.lowADC())/ ( param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) + param.lowADC();
        stateStr.append( " = 0x" + QString::number( adc, 16 ));
    }

    // check flags
    //
    if (state.flags.underflow == true)
    {
        stateStr.append(" - Underflow");
    }

    if (state.flags.overflow == true)
    {
        stateStr.append(" - Overflow");
    }

    return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::updateColumn(int column)
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

MeasureParam SignalInfoTable::signalParam(int index) const
{
    if (index < 0 || index >= MAX_CHANNEL_COUNT)
    {
        return MeasureParam();
    }

    MeasureParam param;

    m_signalMutex.lock();

        param = m_activeSignalParam[index];

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::set(const MeasureSignal &activeSignal)
{
    if (activeSignal.isEmpty() == true)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, MAX_CHANNEL_COUNT - 1);

        m_signalMutex.lock();

            for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
            {
                m_activeSignalParam[c].clear();

                for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
                {
                    Hash signalHash = activeSignal.signal(type).hash(c);
                    if (signalHash == 0)
                    {
                        continue;
                    }

					Metrology::SignalParam param =  theSignalBase.signalParam(signalHash);
                    if (param.isValid() == false)
                    {
                        continue;
                    }

                    m_activeSignalParam[c].setParam(type, param);
                    m_activeSignalParam[c].setOutputSignalType( activeSignal.outputSignalType() );
                    m_activeSignalParam[c].setCalibratorManager( theCalibratorBase.calibratorForMeasure(c) );
                }
            }

        m_signalMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::clear()
{
    beginRemoveRows(QModelIndex(), 0, MAX_CHANNEL_COUNT - 1 );

        m_signalMutex.lock();

            for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
            {
                m_activeSignalParam[c].clear();
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

        for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
        {
            for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
            {
                if (m_activeSignalParam[c].param(type).hash() == signalHash)
                {
                    m_activeSignalParam[c].setParam(type, theSignalBase.signalParam(signalHash));
                }
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
    150,    // SIGNAL_INFO_COLUMN_PH_RANGE
    150,    // SIGNAL_INFO_COLUMN_EL_RANGE
    100,    // SIGNAL_INFO_COLUMN_EL_SENSOR
    150,    // SIGNAL_INFO_COLUMN_CALIBRATOR
};

// -------------------------------------------------------------------------------------------------------------------

SignalInfoPanel::SignalInfoPanel(QWidget* parent) :
    QDockWidget(parent)
{
    setWindowTitle("Panel signal information");
    setObjectName(windowTitle());

    createInterface();
    createHeaderContexMenu();
    createContextMenu();

    connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &SignalInfoPanel::activeSignalChanged, Qt::QueuedConnection);

    hideColumn(SIGNAL_INFO_COLUMN_SUBBLOCK, true);
    hideColumn(SIGNAL_INFO_COLUMN_BLOCK, true);
    hideColumn(SIGNAL_INFO_COLUMN_ENTRY, true);
    hideColumn(SIGNAL_INFO_COLUMN_EL_SENSOR, true);

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
    QSize cellSize = QFontMetrics( theOptions.signalInfo().font() ).size(Qt::TextSingleLine,"A");
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
    m_pShowCustomIDAction->setChecked(theOptions.signalInfo().showCustomID());

    m_pShowMenu->addSeparator();

    m_pShowElectricValueAction = m_pShowMenu->addAction(tr("Electrical"));
    m_pShowElectricValueAction->setCheckable(true);
    m_pShowElectricValueAction->setChecked(theOptions.signalInfo().showElectricState());


    m_pShowAdcValueAction = m_pShowMenu->addAction(tr("ADC"));
    m_pShowAdcValueAction->setCheckable(true);
    m_pShowAdcValueAction->setChecked(theOptions.signalInfo().showAdcState());

    m_pShowAdcHexValueAction = m_pShowMenu->addAction(tr("ADC (hex)"));
    m_pShowAdcHexValueAction->setCheckable(true);
    m_pShowAdcHexValueAction->setChecked(theOptions.signalInfo().showAdcHexState());

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
    m_pShowCustomIDAction->setChecked(theOptions.signalInfo().showCustomID());
    m_pShowElectricValueAction->setChecked(theOptions.signalInfo().showElectricState());
    m_pShowAdcValueAction->setChecked(theOptions.signalInfo().showAdcState());
    m_pShowAdcHexValueAction->setChecked(theOptions.signalInfo().showAdcHexState());

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

void SignalInfoPanel::activeSignalChanged(const MeasureSignal& signal)
{
    clear();

    if (signal.isEmpty() == true)
    {
        return;
    }

    m_signalParamTable.set( signal );

    QSize cellSize = QFontMetrics( theOptions.signalInfo().font() ).size(Qt::TextSingleLine,"A");

    if (signal.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
    {
        m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
    }
    else
    {
        m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height() * 2);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateSignalState()
{
    m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showCustomID()
{
    theOptions.signalInfo().setShowCustomID( m_pShowCustomIDAction->isChecked() );
    m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_ID);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showElectricValue()
{
    theOptions.signalInfo().setShowElectricState( m_pShowElectricValueAction->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showAdcValue()
{
    theOptions.signalInfo().setShowAdcState( m_pShowAdcValueAction->isChecked() );
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showAdcHexValue()
{
    theOptions.signalInfo().setShowAdcHexState( m_pShowAdcHexValueAction->isChecked() );
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

	Metrology::SignalParam param;

    if ( theOptions.toolBar().outputSignalType() ==  OUTPUT_SIGNAL_TYPE_UNUSED)
    {
        param = m_signalParamTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
    }
    else
    {
        param = m_signalParamTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
    }

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
