#include "Statistic.h"

#include <QClipboard>
#include <QHeaderView>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "SignalProperty.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool StatisticTable::m_showCustomID = true;
bool StatisticTable::m_showADCInHex = true;

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::StatisticTable(QObject*)
{
    connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &StatisticTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::~StatisticTable()
{
    m_signalMutex.lock();

        m_signalParamList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::columnCount(const QModelIndex&) const
{
    return STATISTIC_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::rowCount(const QModelIndex&) const
{
    return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < STATISTIC_COLUMN_COUNT)
        {
            result = StatisticColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticTable::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
    {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= signalCount())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > STATISTIC_COLUMN_COUNT)
    {
        return QVariant();
    }

    SignalParam param = signalParam(row);
    if (param.isValid() == false)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case STATISTIC_COLUMN_CASE:             result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_ID:               result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_EQUIPMENT_ID:     result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_CAPTION:          result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_SUBBLOCK:         result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_BLOCK:            result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_ENTRY:            result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_ADC:              result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_IN_PH_RANGE:      result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_IN_EL_RANGE:      result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_OUTPUT_TYPE:      result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_OUT_PH_RANGE:     result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_OUT_EL_RANGE:     result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_MEASURE_COUNT:    result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_STATE:            result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }

    if (role == Qt::FontRole)
    {
        return theOptions.measureView().font();
    }

    if (role == Qt::TextColorRole)
    {
        if (column == STATISTIC_COLUMN_STATE && param.statistic().measureCount() == 0)
        {
            return QColor( Qt::lightGray );
        }

        return QVariant();
    }

    if (role == Qt::BackgroundColorRole)
    {
        if (column == STATISTIC_COLUMN_STATE && param.statistic().measureCount() != 0)
        {
            if (param.statistic().state() == STATISTIC_STATE_INVALID)
            {
                return theOptions.measureView().colorErrorLimit();
            }
            if (param.statistic().state() == STATISTIC_STATE_SUCCESS)
            {
                return theOptions.measureView().colorNotError();
            }

        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, param);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticTable::text(int row, int column, const SignalParam& param) const
{
    if (row < 0 || row >= signalCount())
    {
        return QString();
    }

    if (column < 0 || column > STATISTIC_COLUMN_COUNT)
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
        case STATISTIC_COLUMN_CASE:             result = param.position().caseStr();                        break;
        case STATISTIC_COLUMN_ID:               result = m_showCustomID == true ? param.customAppSignalID() : param.appSignalID(); break;
        case STATISTIC_COLUMN_EQUIPMENT_ID:     result = param.position().equipmentID();                    break;
        case STATISTIC_COLUMN_CAPTION:          result = param.caption();                                   break;
        case STATISTIC_COLUMN_SUBBLOCK:         result = param.position().subblockStr();                    break;
        case STATISTIC_COLUMN_BLOCK:            result = param.position().blockStr();                       break;
        case STATISTIC_COLUMN_ENTRY:            result = param.position().entryStr();                       break;
        case STATISTIC_COLUMN_ADC:              result = param.adcRangeStr(m_showADCInHex);                 break;
        case STATISTIC_COLUMN_IN_PH_RANGE:      result = param.inputPhysicalRangeStr();                     break;
        case STATISTIC_COLUMN_IN_EL_RANGE:      result = param.inputElectricRangeStr();                     break;
        case STATISTIC_COLUMN_OUTPUT_TYPE:      result = QString();                                         break;
        case STATISTIC_COLUMN_OUT_PH_RANGE:     result = param.outputPhysicalRangeStr();                    break;
        case STATISTIC_COLUMN_OUT_EL_RANGE:     result = param.outputElectricRangeStr();                    break;
        case STATISTIC_COLUMN_MEASURE_COUNT:    result = param.statistic().measureCountStr();               break;
        case STATISTIC_COLUMN_STATE:            result = param.statistic().stateStr();                      break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalParamList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam StatisticTable::signalParam(int index) const
{
    SignalParam param;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalParamList.size())
        {
             param = m_signalParamList[index];
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::set(const QList<SignalParam> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_signalMutex.lock();

            m_signalParamList = list_add;

        m_signalMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::clear()
{
    int count = m_signalParamList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_signalMutex.lock();

            m_signalParamList.clear();

        m_signalMutex.unlock();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::updateSignalParam(const Hash& signalHash)
{
    if (signalHash == 0)
    {
        assert(signalHash != 0);
        return;
    }

    m_signalMutex.lock();

        int count = m_signalParamList.count();

        for(int i = 0; i < count; i ++)
        {
            if (m_signalParamList[i].hash() == signalHash)
            {
                m_signalParamList[i] = theSignalBase.signalParam(signalHash);

                break;
            }
        }

    m_signalMutex.unlock();
}
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int StatisticDialog::m_columnWidth[STATISTIC_COLUMN_COUNT] =
{
    100,    // STATISTIC_COLUMN_CASE
    250,    // STATISTIC_COLUMN_ID
    250,    // STATISTIC_COLUMN_EQUIPMENT_ID
    150,    // STATISTIC_COLUMN_CAPTION
     60,    // STATISTIC_COLUMN_SUBBLOCK
     60,    // STATISTIC_COLUMN_BLOCK
     60,    // STATISTIC_COLUMN_ENTRY
    100,    // STATISTIC_COLUMN_ADC
    150,    // STATISTIC_COLUMN_IN_PH_RANGE
    150,    // STATISTIC_COLUMN_IN_EL_RANGE
    100,    // STATISTIC_COLUMN_OUTPUT_TYPE
    150,    // STATISTIC_COLUMN_OUT_PH_RANGE
    150,    // STATISTIC_COLUMN_OUT_EL_RANGE
    100,    // STATISTIC_COLUMN_MEASURE_COUNT
    100,    // STATISTIC_COLUMN_MEASURE_STAT
};

// -------------------------------------------------------------------------------------------------------------------

int StatisticDialog::m_measureType = MEASURE_TYPE_LINEARITY;

// -------------------------------------------------------------------------------------------------------------------

StatisticDialog::StatisticDialog(QWidget *parent) :
    QDialog(parent)
{
    m_pMainWindow = dynamic_cast<QMainWindow*> (parent);

    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
    if (pMainWindow != nullptr && pMainWindow->m_pSignalSocket != nullptr)
    {
        m_measureType = pMainWindow->measureType();

        connect(pMainWindow->m_pSignalSocket, &SignalSocket::signalsLoaded, this, &StatisticDialog::updateList, Qt::QueuedConnection);
        connect(pMainWindow->m_pSignalSocket, &SignalSocket::socketDisconnected, this, &StatisticDialog::updateList, Qt::QueuedConnection);
        connect(&pMainWindow->m_measureThread, &MeasureThread::measureComplite, this, &StatisticDialog::updateList, Qt::QueuedConnection);
    }

    createInterface();
    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

StatisticDialog::~StatisticDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createInterface()
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/icons/Statistics.png"));
    setWindowTitle(tr("Statistics"));
    resize(QApplication::desktop()->availableGeometry().width() - 800, 800);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());
    installEventFilter(this);

    m_pMenuBar = new QMenuBar(this);
    m_pSignalMenu = new QMenu(tr("&Signal"), this);
    m_pEditMenu = new QMenu(tr("&Edit"), this);
    m_pViewMenu = new QMenu(tr("&View"), this);

    m_pPrintAction = m_pSignalMenu->addAction(tr("&Print ..."));
    m_pPrintAction->setIcon(QIcon(":/icons/Print.png"));
    m_pPrintAction->setShortcut(Qt::CTRL + Qt::Key_P);

    m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
    m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
    m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

    m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
    m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
    m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

    m_pEditMenu->addSeparator();

    m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
    m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

    m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
    m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
    m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

    m_pEditMenu->addSeparator();

    m_pViewMeasureTypeMenu = new QMenu(tr("Measure type"), this);
    m_pTypeLinearityAction = m_pViewMeasureTypeMenu->addAction(tr("Linearity"));
    m_pTypeLinearityAction->setCheckable(true);
    m_pTypeLinearityAction->setChecked(m_measureType == MEASURE_TYPE_LINEARITY);

    m_pTypeComparatorsAction = m_pViewMeasureTypeMenu->addAction(tr("Comparators"));
    m_pTypeComparatorsAction->setCheckable(true);
    m_pTypeComparatorsAction->setChecked(m_measureType == MEASURE_TYPE_COMPARATOR);

    m_pViewShowMenu = new QMenu(tr("Show"), this);
    m_pShowCustomIDAction = m_pViewShowMenu->addAction(tr("Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(m_signalParamTable.showCustomID());
    m_pShowCustomIDAction->setShortcut(Qt::CTRL + Qt::Key_Tab);

    m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
    m_pShowADCInHexAction->setCheckable(true);
    m_pShowADCInHexAction->setChecked(m_signalParamTable.showADCInHex());

    m_pViewGotoMenu = new QMenu(tr("Go to next"), this);
    m_pGotoNextNotMeasuredAction = m_pViewGotoMenu->addAction(tr("Not measured"));
    m_pGotoNextInvalidAction = m_pViewGotoMenu->addAction(tr("Invalid"));

    m_pViewMenu->addMenu(m_pViewMeasureTypeMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewShowMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewGotoMenu);

    m_pMenuBar->addMenu(m_pSignalMenu);
    m_pMenuBar->addMenu(m_pEditMenu);
    m_pMenuBar->addMenu(m_pViewMenu);

    createStatusBar();

    connect(m_pPrintAction, &QAction::triggered, this, &StatisticDialog::printSignal);
    connect(m_pExportAction, &QAction::triggered, this, &StatisticDialog::exportSignal);

    connect(m_pFindAction, &QAction::triggered, this, &StatisticDialog::find);
    connect(m_pCopyAction, &QAction::triggered, this, &StatisticDialog::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &StatisticDialog::selectAll);

    connect(m_pTypeLinearityAction, &QAction::triggered, this, &StatisticDialog::showTypeLinearity);
    connect(m_pTypeComparatorsAction, &QAction::triggered, this, &StatisticDialog::showTypeComparators);
    connect(m_pShowCustomIDAction, &QAction::triggered, this, &StatisticDialog::showCustomID);
    connect(m_pShowADCInHexAction, &QAction::triggered, this, &StatisticDialog::showADCInHex);
    connect(m_pGotoNextNotMeasuredAction, &QAction::triggered, this, &StatisticDialog::gotoNextNotMeasured);
    connect(m_pGotoNextInvalidAction, &QAction::triggered, this, &StatisticDialog::gotoNextInvalid);


    m_pView = new QTableView(this);
    m_pView->setModel(&m_signalParamTable);
    QSize cellSize = QFontMetrics( theOptions.measureView().font() ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_pView, &QTableView::doubleClicked , this, &StatisticDialog::onListDoubleClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->setMenuBar(m_pMenuBar);
    mainLayout->addWidget(m_pView);
    mainLayout->addWidget(m_pStatusBar);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);

    createHeaderContexMenu();
    createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createHeaderContexMenu()
{
    // init header context menu
    //
    m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &StatisticDialog::onHeaderContextMenu);

    m_headerContextMenu = new QMenu(m_pView);

    for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
    {
        m_pColumnAction[column] = m_headerContextMenu->addAction(StatisticColumn[column]);
        if (m_pColumnAction[column] != nullptr)
        {
            m_pColumnAction[column]->setCheckable(true);
            m_pColumnAction[column]->setChecked(true);

            connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &StatisticDialog::onColumnAction);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr(""), this);

    m_pContextMenu->addAction(m_pCopyAction);
    m_pContextMenu->addSeparator();
    m_pSelectSignalForMeasure = m_pContextMenu->addAction(tr("&Select signal for measuring"));
    m_pSelectSignalForMeasure->setIcon(QIcon(":/icons/Start.png"));

    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &StatisticDialog::onContextMenu);

    connect(m_pSelectSignalForMeasure, &QAction::triggered, this, &StatisticDialog::selectSignalForMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createStatusBar()
{
    m_pStatusBar = new QStatusBar(this);

    m_statusEmpty = new QLabel(m_pStatusBar);
    m_statusMeasureInavlid = new QLabel(m_pStatusBar);
    m_statusMeasured = new QLabel(m_pStatusBar);

    m_pStatusBar->addWidget(m_statusMeasureInavlid);
    m_pStatusBar->addWidget(m_statusMeasured);
    m_pStatusBar->addWidget(m_statusEmpty);

    m_statusMeasureInavlid->setFixedWidth(100);
    m_statusMeasured->setFixedWidth(150);

    m_pStatusBar->setLayoutDirection(Qt::RightToLeft);
    m_pStatusBar->setSizeGripEnabled(false);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateList()
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
    if (pMainWindow == nullptr)
    {
        return ;
    }

    m_MeasuredCount = 0;
    m_invalidMeasureCount = 0;

    updateVisibleColunm();

    m_signalParamTable.clear();

    QList<SignalParam> signalParamList;

    int count = theSignalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        SignalParam param = theSignalBase.signalParam(i);
        if (param.isValid() == false)
        {
            continue;
        }

        if (param.isAnalog() == false || param.isInput() == false)
        {
            continue;
        }

        if (param.position().subblock() == -1 || param.position().block() == -1 || param.position().entry() == -1)
        {
            continue;
        }

        // temporary solution // param.setStatistic( theMeasureBase.statisticItem( param.hash() ) );
        //
            MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
            if (pMeasureView != nullptr)
            {
                param.setStatistic( pMeasureView->table().m_measureBase.statistic( param.hash() ) );
            }
        //
        // temporary solution

        if (param.statistic().measureCount() != 0)
        {
            m_MeasuredCount++;
        }

        if (param.statistic().state() == STATISTIC_STATE_INVALID)
        {
            m_invalidMeasureCount ++;
        }

        signalParamList.append( param );
    }

    m_signalParamTable.set(signalParamList);

    updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateStatusBar()
{
    int signalCount = m_signalParamTable.signalCount();

    m_statusMeasured->setText(tr("Measured: %1 / %2").arg(m_MeasuredCount).arg(signalCount));
    m_statusMeasureInavlid->setText(tr("Invalid: %1").arg(m_invalidMeasureCount));

    if (m_invalidMeasureCount == 0)
    {
        m_statusMeasureInavlid->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
    }
    else
    {
        m_statusMeasureInavlid->setStyleSheet("background-color: rgb(255, 160, 160);");
    }
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateVisibleColunm()
{
    for(int c = 0; c < STATISTIC_COLUMN_COUNT; c++)
    {
        hideColumn(c, false);
    }

    hideColumn(STATISTIC_COLUMN_EQUIPMENT_ID, true);
    hideColumn(STATISTIC_COLUMN_ADC, true);
    hideColumn(STATISTIC_COLUMN_IN_PH_RANGE, true);
    hideColumn(STATISTIC_COLUMN_IN_EL_RANGE, true);
    hideColumn(STATISTIC_COLUMN_OUTPUT_TYPE, true);
    hideColumn(STATISTIC_COLUMN_OUT_PH_RANGE, true);
    hideColumn(STATISTIC_COLUMN_OUT_EL_RANGE, true);

//    if (m_measureType == MEASURE_TYPE_LINEARITY)
//    {
//        hideColumn(STATISTIC_COLUMN_OUT_PH_RANGE, true);
//        hideColumn(STATISTIC_COLUMN_OUT_EL_RANGE, true);
//
//    }

//    if (m_typeAD == MEASURE_TYPE_COMPARATOR)
//    {
//        hideColumn(STATISTIC_COLUMN_ADC, true);
//        hideColumn(STATISTIC_COLUMN_IN_PH_RANGE, true);
//        hideColumn(STATISTIC_COLUMN_IN_EL_RANGE, true);
//        hideColumn(STATISTIC_COLUMN_OUT_PH_RANGE, true);
//        hideColumn(STATISTIC_COLUMN_OUT_EL_RANGE, true);
//    }
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::hideColumn(int column, bool hide)
{
    if (column < 0 || column >= STATISTIC_COLUMN_COUNT)
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

bool StatisticDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>( event );

        if (keyEvent->key() == Qt::Key_Return)
        {
            // stat
        }
    }

    return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::printSignal()
{

}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::exportSignal()
{
    ExportData* dialog = new ExportData(m_pView, tr("Statistics"));
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::selectSignalForMeasure()
{

}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::find()
{
    FindData* dialog = new FindData(m_pView);
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::copy()
{
    QString textClipboard;

    int rowCount = m_pView->model()->rowCount();
    int columnCount = m_pView->model()->columnCount();

    for(int row = 0; row < rowCount; row++)
    {
        if (m_pView->selectionModel()->isRowSelected(row, QModelIndex() ) == false)
        {
            continue;
        }

        for(int column = 0; column < columnCount; column++)
        {
            if (m_pView->isColumnHidden(column) == true)
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

void StatisticDialog::showTypeLinearity()
{
    m_measureType = MEASURE_TYPE_LINEARITY;

    m_pTypeLinearityAction->setChecked(true);
    m_pTypeComparatorsAction->setChecked(false);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showTypeComparators()
{
    m_measureType = MEASURE_TYPE_COMPARATOR;

    m_pTypeLinearityAction->setChecked(false);
    m_pTypeComparatorsAction->setChecked(true);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showCustomID()
{
    m_signalParamTable.setShowCustomID( m_pShowCustomIDAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showADCInHex()
{
    m_signalParamTable.setShowADCInHex( m_pShowADCInHexAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::gotoNextNotMeasured()
{
    int signaCount = m_signalParamTable.signalCount();
    if (signaCount == 0)
    {
        return;
    }

    int startIndex = m_pView->currentIndex().row() ;
    int foundIndex = -1;

    for(int i = startIndex + 1; i < signaCount; i++)
    {
        SignalParam param = m_signalParamTable.signalParam(i);
        if (param.isValid() == false)
        {
            continue;
        }

        if (param.statistic().measureCount() == 0)
        {
            foundIndex = i;

            break;
        }
    }

    if (foundIndex == -1)
    {
        return;
    }

    m_pView->setCurrentIndex( m_pView->model()->index(foundIndex, 0) );
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::gotoNextInvalid()
{
    int signaCount = m_signalParamTable.signalCount();
    if (signaCount == 0)
    {
        return;
    }

    int startIndex = m_pView->currentIndex().row() ;
    int foundIndex = -1;

    for(int i = startIndex + 1; i < signaCount; i++)
    {
        SignalParam param = m_signalParamTable.signalParam(i);
        if (param.isValid() == false)
        {
            continue;
        }

        if (param.statistic().state() == STATISTIC_STATE_INVALID)
        {
            foundIndex = i;

            break;
        }
    }

    if (foundIndex == -1)
    {
        return;
    }

    m_pView->setCurrentIndex( m_pView->model()->index(foundIndex, 0) );
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onColumnAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
    {
        if (m_pColumnAction[column] == action)
        {
            hideColumn(column,  !action->isChecked());

            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------