#include "Statistic.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "SignalProperty.h"


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::stateString()
{
    QString state;

    if (m_measureCount == 0)
    {
        return "no measured";
    }

    state = QString::number(m_measureCount);

    switch(m_state)
    {
        case STATISTIC_STATE_INVALID:   state.append(" - invalid"); break;
        case STATISTIC_STATE_SUCCESS:   state.append(" - Ok");      break;
    }

    return state;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool StatisticTable::m_showCustomID = true;
bool StatisticTable::m_showADCInHex = true;

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::StatisticTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::~StatisticTable()
{
    m_signalHashList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::columnCount(const QModelIndex&) const
{
    return STATISTIC_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::rowCount(const QModelIndex&) const
{
    return m_signalHashList.count();
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
    if (row < 0 || row >= m_signalHashList.count())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > STATISTIC_COLUMN_COUNT)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case STATISTIC_COLUMN_ID:               result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_EQUIPMENT_ID:     result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_CAPTION:          result = Qt::AlignLeft;     break;
            case STATISTIC_COLUMN_CASE:             result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_SUBBLOCK:         result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_BLOCK:            result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_ENTRY:            result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_ADC:              result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_IN_PH_RANGE:      result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_IN_EL_RANGE:      result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_OUT_PH_RANGE:     result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_OUT_EL_RANGE:     result = Qt::AlignCenter;   break;
            case STATISTIC_COLUMN_MEASURE_COUNT:    result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_signalHashList.at(row));
        return var;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticTable::text(int row, int column) const
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
    if (pMainWindow == nullptr)
    {
        return "";
    }

    if (row < 0 || row >= m_signalHashList.count())
    {
        return "";
    }

    if (column < 0 || column > STATISTIC_COLUMN_COUNT)
    {
        return "";
    }

    Hash signalHash = m_signalHashList.at(row);
    if (signalHash == 0)
    {
        return "";
    }

    MeasureSignal s = theSignalBase.signal(signalHash);
    if (s.param().appSignalID().isEmpty() == true || s.param().hash() == 0)
    {
        return "";
    }

    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return "";
    }

    //StatisticItem si = theMeasureBase.statisticItem(signalHash);
    StatisticItem si =  pMainWindow->m_measureView[ m_measureType ]->table().m_measureBase.statisticItem(signalHash);

    QString result;

    switch (column)
    {
        case STATISTIC_COLUMN_ID:               result = m_showCustomID == true ? s.param().customAppSignalID() : s.param().appSignalID(); break;
        case STATISTIC_COLUMN_EQUIPMENT_ID:     result = s.param().equipmentID();               break;
        case STATISTIC_COLUMN_CAPTION:          result = s.param().caption();                   break;
        case STATISTIC_COLUMN_CASE:             result = s.position().caseString();             break;
        case STATISTIC_COLUMN_SUBBLOCK:         result = s.position().subblockString();         break;
        case STATISTIC_COLUMN_BLOCK:            result = s.position().blockString();            break;
        case STATISTIC_COLUMN_ENTRY:            result = s.position().entryString();            break;
        case STATISTIC_COLUMN_ADC:              result = s.adcRange(m_showADCInHex);            break;
        case STATISTIC_COLUMN_IN_PH_RANGE:      result = s.inputPhysicalRange();                break;
        case STATISTIC_COLUMN_IN_EL_RANGE:      result = s.inputElectricRange();                break;
        case STATISTIC_COLUMN_OUT_PH_RANGE:     result = s.outputPhysicalRange();               break;
        case STATISTIC_COLUMN_OUT_EL_RANGE:     result = s.outputElectricRange();               break;
        case STATISTIC_COLUMN_MEASURE_COUNT:    result = si.stateString();                      break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

Hash StatisticTable::at(int index)
{
    if (index < 0 || index >= count())
    {
        return 0;
    }

    return m_signalHashList.at(index);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::set(const QList<Hash> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_signalHashList = list_add;

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::clear()
{
    int count = m_signalHashList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_signalHashList.clear();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int StatisticDialog::m_columnWidth[STATISTIC_COLUMN_COUNT] =
{
    250,    // STATISTIC_COLUMN_ID
    250,    // STATISTIC_COLUMN_EQUIPMENT_ID
    150,    // STATISTIC_COLUMN_CAPTION
    100,    // STATISTIC_COLUMN_CASE
     60,    // STATISTIC_COLUMN_SUBBLOCK
     60,    // STATISTIC_COLUMN_BLOCK
     60,    // STATISTIC_COLUMN_ENTRY
    100,    // STATISTIC_COLUMN_ADC
    150,    // STATISTIC_COLUMN_IN_PH_RANGE
    150,    // STATISTIC_COLUMN_IN_EL_RANGE
    150,    // STATISTIC_COLUMN_OUT_PH_RANGE
    150,    // STATISTIC_COLUMN_OUT_EL_RANGE
    120,    // STATISTIC_COLUMN_MEASURE_COUNT
};

// -------------------------------------------------------------------------------------------------------------------

int StatisticDialog::m_measureType = MEASURE_TYPE_LINEARITY;

// -------------------------------------------------------------------------------------------------------------------

StatisticDialog::StatisticDialog(QWidget *parent) :
    QDialog(parent)
{
    m_table.m_pMainWindow = dynamic_cast<QMainWindow*> (parent);

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
    resize(QApplication::desktop()->availableGeometry().width() - 200, 500);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());
    installEventFilter(this);

    m_pMenuBar = new QMenuBar(this);
    m_pEditMenu = new QMenu(tr("&Edit"), this);
    m_pViewMenu = new QMenu(tr("&View"), this);


    m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
    m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

    m_pFindAction = m_pEditMenu->addAction(tr("&Find"));
    m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
    m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

    m_pEditMenu->addSeparator();

    m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Properties"));
    m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));


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
    m_pShowCustomIDAction->setChecked(m_table.showCustomID());
    m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
    m_pShowADCInHexAction->setCheckable(true);
    m_pShowADCInHexAction->setChecked(m_table.showADCInHex());

    m_pViewMenu->addMenu(m_pViewMeasureTypeMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewShowMenu);


    m_pMenuBar->addMenu(m_pEditMenu);
    m_pMenuBar->addMenu(m_pViewMenu);

    connect(m_pCopyAction, &QAction::triggered, this, &StatisticDialog::copy);
    connect(m_pFindAction, &QAction::triggered, this, &StatisticDialog::find);
    connect(m_pSignalPropertyAction, &QAction::triggered, this, &StatisticDialog::signalProperty);

    connect(m_pTypeLinearityAction, &QAction::triggered, this, &StatisticDialog::showTypeLinearity);
    connect(m_pTypeComparatorsAction, &QAction::triggered, this, &StatisticDialog::showTypeComparators);
    connect(m_pShowCustomIDAction, &QAction::triggered, this, &StatisticDialog::showCustomID);
    connect(m_pShowADCInHexAction, &QAction::triggered, this, &StatisticDialog::showADCInHex);


    m_pView = new QTableView(this);
    m_pView->setModel(&m_table);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
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

    setLayout(mainLayout);

    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &StatisticDialog::onContextMenu);

    createHeaderContexMenu();
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

void StatisticDialog::updateList()
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_table.m_pMainWindow);
    if (pMainWindow == nullptr)
    {
        return ;
    }

    updateVisibleColunm();

    m_table.clear();

    QList<Hash> signalHashList;

    int count = theSignalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        Signal param = theSignalBase.signalParam(i);
        if (param.appSignalID().isEmpty() == true || param.hash() == 0)
        {
            continue;
        }

        if (param.isAnalog() == false || param.isInput() == false)
        {
            continue;
        }

        switch(m_measureType)
        {
            case MEASURE_TYPE_LINEARITY:            signalHashList.append( param.hash() );  break;
            case MEASURE_TYPE_COMPARATOR:           break;
            case MEASURE_TYPE_COMPLEX_COMPARATOR:   break;
            default:                                assert(0);
        }
    }

    m_table.set(signalHashList);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateVisibleColunm()
{
    for(int c = 0; c < STATISTIC_COLUMN_COUNT; c++)
    {
        hideColumn(c, false);
    }

    hideColumn(STATISTIC_COLUMN_ADC, true);
    hideColumn(STATISTIC_COLUMN_IN_PH_RANGE, true);
    hideColumn(STATISTIC_COLUMN_IN_EL_RANGE, true);
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
            signalProperty();
        }
    }

    return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::copy()
{
    bool appendRow;
    QString textRow;
    QString textClipboard;

    int count = m_table.count();
    for(int r = 0; r < count; r++)
    {
        appendRow = false;
        textRow = "";

        for(int c = 0; c < STATISTIC_COLUMN_COUNT; c++)
        {
            if (m_pView->selectionModel()->isSelected( m_pView->model()->index(r,c) ) == true)
            {
                appendRow = true;
                textRow.append(m_table.text(r, c));
            }

            if (c != STATISTIC_COLUMN_COUNT - 1)
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

void StatisticDialog::find()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::signalProperty()
{
    QModelIndex visibleIndex = m_pView->currentIndex();

    int index = visibleIndex .row();
    if (index < 0 || index >= m_table.count())
    {
        return;
    }

    Hash signalHash = m_table.at(index);
    if (signalHash == 0)
    {
        return;
    }

    MeasureSignal signal = theSignalBase.signal(signalHash);
    if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
    {
        return;
    }

    SignalPropertyDialog dialog( signal.param().hash() );
    dialog.exec();

    updateList();

    m_pView->setCurrentIndex(visibleIndex);
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
    m_table.setShowCustomID( m_pShowCustomIDAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showADCInHex()
{
    m_table.setShowADCInHex( m_pShowADCInHexAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onContextMenu(QPoint)
{
    m_pViewMenu->exec(QCursor::pos());
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
