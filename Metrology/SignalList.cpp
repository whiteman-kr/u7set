#include "SignalList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "SignalProperty.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalListTable::m_showCustomID = true;
bool SignalListTable::m_showADCInHex = true;

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::SignalListTable(QObject*)
{
    connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &SignalListTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::~SignalListTable()
{
    m_signalMutex.lock();

        m_signalParamList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::columnCount(const QModelIndex&) const
{
    return SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::rowCount(const QModelIndex&) const
{
    return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < SIGNAL_LIST_COLUMN_COUNT)
        {
            result = SignalListColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalListTable::data(const QModelIndex &index, int role) const
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
    if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
    {
        return QVariant();
    }

    MeasureSignalParam param = signalParam(row);
    if (param.isValid() == false)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case SIGNAL_LIST_COLUMN_ID:             result = Qt::AlignLeft;     break;
            case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:   result = Qt::AlignLeft;     break;
            case SIGNAL_LIST_COLUMN_CAPTION:        result = Qt::AlignLeft;     break;
            case SIGNAL_LIST_COLUMN_CASE:           result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_SUBBLOCK:       result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_BLOCK:          result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_ENTRY:          result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_ADC:            result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_IN_PH_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_IN_EL_RANGE:    result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_OUT_PH_RANGE:   result = Qt::AlignCenter;   break;
            case SIGNAL_LIST_COLUMN_OUT_EL_RANGE:   result = Qt::AlignCenter;   break;
            default:                                assert(0);
        }

        return result;
    }



    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, param);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalListTable::text(const int  row, const int column, const MeasureSignalParam& param) const
{
    if (row < 0 || row >= signalCount())
    {
        return QString();
    }

    if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
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
        case SIGNAL_LIST_COLUMN_ID:             result = m_showCustomID == true ? param.customAppSignalID() : param.appSignalID(); break;
        case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:   result = param.position().equipmentID();    break;
        case SIGNAL_LIST_COLUMN_CAPTION:        result = param.caption();                   break;
        case SIGNAL_LIST_COLUMN_CASE:           result = param.position().caseStr();        break;
        case SIGNAL_LIST_COLUMN_SUBBLOCK:       result = param.position().subblockStr();    break;
        case SIGNAL_LIST_COLUMN_BLOCK:          result = param.position().blockStr();       break;
        case SIGNAL_LIST_COLUMN_ENTRY:          result = param.position().entryStr();       break;
        case SIGNAL_LIST_COLUMN_ADC:            result = param.adcRangeStr(m_showADCInHex); break;
        case SIGNAL_LIST_COLUMN_IN_PH_RANGE:    result = param.inputPhysicalRangeStr();     break;
        case SIGNAL_LIST_COLUMN_IN_EL_RANGE:    result = param.inputElectricRangeStr();     break;
        case SIGNAL_LIST_COLUMN_OUT_PH_RANGE:   result = param.outputPhysicalRangeStr();    break;
        case SIGNAL_LIST_COLUMN_OUT_EL_RANGE:   result = param.outputElectricRangeStr();    break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalParamList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam SignalListTable::signalParam(const int index) const
{
    MeasureSignalParam param;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalParamList.size())
        {
             param = m_signalParamList[index];
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::set(const QList<MeasureSignalParam> list_add)
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

void SignalListTable::clear()
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

void SignalListTable::updateSignalParam(const Hash& signalHash)
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

int SignalListDialog::m_columnWidth[SIGNAL_LIST_COLUMN_COUNT] =
{
    250,    // LIST_COLUMN_ID
    250,    // LIST_COLUMN_EQUIPMENT_ID
    150,    // LIST_COLUMN_CAPTION
    100,    // LIST_COLUMN_CASE
     60,    // LIST_COLUMN_SUBBLOCK
     60,    // LIST_COLUMN_BLOCK
     60,    // LIST_COLUMN_ENTRY
    100,    // LIST_COLUMN_ADC
    150,    // LIST_COLUMN_IN_PH_RANGE
    150,    // LIST_COLUMN_IN_EL_RANGE
    150,    // LIST_COLUMN_OUT_PH_RANGE
    150,    // LIST_COLUMN_OUT_EL_RANGE
};

// -------------------------------------------------------------------------------------------------------------------

E::SignalType       SignalListDialog::m_typeAD = E::SignalType::Analog;
E::SignalInOutType  SignalListDialog::m_typeIO = E::SignalInOutType::Input;
int                 SignalListDialog::m_currenIndex = 0;

// -------------------------------------------------------------------------------------------------------------------

SignalListDialog::SignalListDialog(const bool hasButtons, QWidget *parent) :
    QDialog(parent)
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
    if (pMainWindow != nullptr && pMainWindow->m_pSignalSocket != nullptr)
    {
        connect(pMainWindow->m_pSignalSocket, &SignalSocket::signalsLoaded, this, &SignalListDialog::updateList, Qt::QueuedConnection);
        connect(pMainWindow->m_pSignalSocket, &SignalSocket::socketDisconnected, this, &SignalListDialog::updateList, Qt::QueuedConnection);
    }

    createInterface(hasButtons);
    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalListDialog::~SignalListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createInterface(const bool hasButtons)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/icons/Signals.png"));
    setWindowTitle(tr("Signals"));
    resize(QApplication::desktop()->availableGeometry().width() - 200, 500);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());
    installEventFilter(this);

    m_pMenuBar = new QMenuBar(this);
    m_pEditMenu = new QMenu(tr("&Edit"), this);
    m_pViewMenu = new QMenu(tr("&View"), this);


    m_pFindAction = m_pEditMenu->addAction(tr("&Find"));
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

    m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Properties"));
    m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

    m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
    m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
    m_pTypeAnalogAction->setCheckable(true);
    m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
    m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
    m_pTypeDiscreteAction->setCheckable(true);
    m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);

    m_pViewTypeIOMenu = new QMenu(tr("Type I/O"), this);
    m_pTypeInputAction = m_pViewTypeIOMenu->addAction(tr("Input"));
    m_pTypeInputAction->setCheckable(true);
    m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
    m_pTypeInternalAction = m_pViewTypeIOMenu->addAction(tr("Internal"));
    m_pTypeInternalAction->setCheckable(true);
    m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
    m_pTypeOutputAction = m_pViewTypeIOMenu->addAction(tr("Output"));
    m_pTypeOutputAction->setCheckable(true);
    m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

    m_pViewShowMenu = new QMenu(tr("Show"), this);
    m_pShowCustomIDAction = m_pViewShowMenu->addAction(tr("Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(m_signalParamTable.showCustomID());
    m_pShowCustomIDAction->setShortcut(Qt::Key_Tab);
    m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
    m_pShowADCInHexAction->setCheckable(true);
    m_pShowADCInHexAction->setChecked(m_signalParamTable.showADCInHex());

    m_pViewMenu->addMenu(m_pViewTypeADMenu);
    m_pViewMenu->addMenu(m_pViewTypeIOMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewShowMenu);


    m_pMenuBar->addMenu(m_pEditMenu);
    m_pMenuBar->addMenu(m_pViewMenu);

    connect(m_pFindAction, &QAction::triggered, this, &SignalListDialog::find);
    connect(m_pCopyAction, &QAction::triggered, this, &SignalListDialog::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &SignalListDialog::selectAll);
    connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalListDialog::signalProperty);

    connect(m_pTypeAnalogAction, &QAction::triggered, this, &SignalListDialog::showTypeAnalog);
    connect(m_pTypeDiscreteAction, &QAction::triggered, this, &SignalListDialog::showTypeDiscrete);
    connect(m_pTypeInputAction, &QAction::triggered, this, &SignalListDialog::showTypeInput);
    connect(m_pTypeInternalAction, &QAction::triggered, this, &SignalListDialog::showTypeInternal);
    connect(m_pTypeOutputAction, &QAction::triggered, this, &SignalListDialog::showTypeOutput);
    connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalListDialog::showCustomID);
    connect(m_pShowADCInHexAction, &QAction::triggered, this, &SignalListDialog::showADCInHex);


    m_pView = new QTableView(this);
    m_pView->setModel(&m_signalParamTable);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_pView, &QTableView::doubleClicked , this, &SignalListDialog::onListDoubleClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->setMenuBar(m_pMenuBar);
    mainLayout->addWidget(m_pView);

    if (hasButtons == true)
    {
        m_pView->setSelectionMode(QAbstractItemView::SingleSelection);

        m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalListDialog::onOk);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalListDialog::reject);

        mainLayout->addWidget(m_buttonBox);
    }

    setLayout(mainLayout);

    createHeaderContexMenu();
    createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createHeaderContexMenu()
{
    // init header context menu
    //
    m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &SignalListDialog::onHeaderContextMenu);

    m_headerContextMenu = new QMenu(m_pView);

    for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        m_pColumnAction[column] = m_headerContextMenu->addAction(SignalListColumn[column]);
        if (m_pColumnAction[column] != nullptr)
        {
            m_pColumnAction[column]->setCheckable(true);
            m_pColumnAction[column]->setChecked(true);

            connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalListDialog::onColumnAction);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr(""), this);


    m_pContextMenu->addAction(m_pCopyAction);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addMenu(m_pViewTypeADMenu);
    m_pContextMenu->addMenu(m_pViewTypeIOMenu);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addAction(m_pSignalPropertyAction);

    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::updateList()
{
    updateVisibleColunm();

    m_signalParamTable.clear();

    QList<MeasureSignalParam> signalParamList;

    int count = theSignalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        MeasureSignalParam param = theSignalBase.signalParam(i);
        if (param.isValid() == false)
        {
            continue;
        }

        if (param.signalType() != m_typeAD || param.inOutType() != m_typeIO)
        {
            continue;
        }

        signalParamList.append( param );
    }

    m_signalParamTable.set(signalParamList);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::updateVisibleColunm()
{
    m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
    m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
    m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
    m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
    m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

    for(int c = 0; c < SIGNAL_LIST_COLUMN_COUNT; c++)
    {
        hideColumn(c, false);
    }

    if (m_typeAD == E::SignalType::Analog && m_typeIO == E::SignalInOutType::Input)
    {
        hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
        hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);
    }

    if (m_typeAD == E::SignalType::Discrete)
    {
        hideColumn(SIGNAL_LIST_COLUMN_ADC, true);
        hideColumn(SIGNAL_LIST_COLUMN_IN_PH_RANGE, true);
        hideColumn(SIGNAL_LIST_COLUMN_IN_EL_RANGE, true);
        hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
        hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::hideColumn(const int column, const bool hide)
{
    if (column < 0 || column >= SIGNAL_LIST_COLUMN_COUNT)
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

bool SignalListDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>( event );

        if (keyEvent->key() == Qt::Key_Return)
        {
            if (m_buttonBox == nullptr)
            {
                signalProperty();
            }
            else
            {
                emit onOk();
            }
        }
    }

    return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::find()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::copy()
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

void SignalListDialog::signalProperty()
{
    QModelIndex visibleIndex = m_pView->currentIndex();

    int index = visibleIndex .row();
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

void SignalListDialog::showTypeAnalog()
{
    m_typeAD = E::SignalType::Analog;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeDiscrete()
{
    m_typeAD = E::SignalType::Discrete;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInput()
{
    m_typeIO = E::SignalInOutType::Input;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInternal()
{
    m_typeIO = E::SignalInOutType::Internal;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeOutput()
{
    m_typeIO = E::SignalInOutType::Output;

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showCustomID()
{
    m_signalParamTable.setShowCustomID( m_pShowCustomIDAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showADCInHex()
{
    m_signalParamTable.setShowADCInHex( m_pShowADCInHexAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onColumnAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        if (m_pColumnAction[column] == action)
        {
            hideColumn(column,  !action->isChecked());

            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onListDoubleClicked(const QModelIndex&)
{
    if (m_buttonBox == nullptr)
    {
        signalProperty();
    }
    else
    {
        emit onOk();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onOk()
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

    m_selectedSignalHash = param.hash();

    accept();
}

// -------------------------------------------------------------------------------------------------------------------
