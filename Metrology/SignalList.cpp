#include "SignalList.h"

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalListTable::SignalListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::~SignalListTable()
{
    m_signalList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::columnCount(const QModelIndex&) const
{
    return SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::rowCount(const QModelIndex&) const
{
    return m_signalList.count();
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
    if (row < 0 || row >= m_signalList.count())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
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

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_signalList.at(row));
        return var;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalListTable::text(int row, int column) const
{
    if (row < 0 || row >= m_signalList.count())
    {
        return "";
    }

    if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
    {
        return "";
    }

    MeasureSignal s = m_signalList.at(row);
    if (s.param().appSignalID().isEmpty() == true || s.param().hash() == 0)
    {
        return "";
    }

    QString result;

    switch (column)
    {
        case SIGNAL_LIST_COLUMN_ID:             result = m_showAppSignalID == true ? s.param().appSignalID() : s.param().customAppSignalID(); break;
        case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:   result = s.param().equipmentID();       break;
        case SIGNAL_LIST_COLUMN_CAPTION:        result = s.param().caption();           break;
        case SIGNAL_LIST_COLUMN_CASE:           result = s.position().caseString();     break;
        case SIGNAL_LIST_COLUMN_SUBBLOCK:       result = s.position().subblockString(); break;
        case SIGNAL_LIST_COLUMN_BLOCK:          result = s.position().blockString();    break;
        case SIGNAL_LIST_COLUMN_ENTRY:          result = s.position().entryString();    break;
        case SIGNAL_LIST_COLUMN_ADC:            result = s.adcRange(m_showADCInHex);    break;
        case SIGNAL_LIST_COLUMN_IN_PH_RANGE:    result = s.inputPhysicalRange();        break;
        case SIGNAL_LIST_COLUMN_IN_EL_RANGE:    result = s.inputElectricRange();        break;
        case SIGNAL_LIST_COLUMN_OUT_PH_RANGE:   result = s.outputPhysicalRange();       break;
        case SIGNAL_LIST_COLUMN_OUT_EL_RANGE:   result = s.outputElectricRange();       break;
        default:                                assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------


MeasureSignal SignalListTable::at(int index)
{
    if (index < 0 || index >= count())
    {
        return MeasureSignal();
    }

    return m_signalList.at(index);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::set(const QList<MeasureSignal> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_signalList = list_add;

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::clear()
{
    int count = m_signalList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_signalList.clear();

    endRemoveRows();
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

SignalListDialog::SignalListDialog(QWidget *parent) :
    QDialog(parent)
{
    createInterface();

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalListDialog::~SignalListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createInterface()
{
    setWindowFlags(Qt::Window);
    setWindowIcon(QIcon(":/icons/Signals.png"));
    setWindowTitle(tr("Signals"));

    m_pMenuBar = new QMenuBar(this);
    m_pViewMenu = new QMenu(tr("&View"), this);

    m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
    m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
    m_pTypeAnalogAction->setCheckable(true);
    m_pTypeAnalogAction->setChecked(true);
    m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
    m_pTypeDiscreteAction->setCheckable(true);
    m_pTypeDiscreteAction->setChecked(false);

    m_pViewTypeIOMenu = new QMenu(tr("Type I/O"), this);
    m_pTypeInputAction = m_pViewTypeIOMenu->addAction(tr("Input"));
    m_pTypeInputAction->setCheckable(true);
    m_pTypeInputAction->setChecked(true);
    m_pTypeOutputAction = m_pViewTypeIOMenu->addAction(tr("Output"));
    m_pTypeOutputAction->setCheckable(true);
    m_pTypeOutputAction->setChecked(false);
    m_pTypeInternalAction = m_pViewTypeIOMenu->addAction(tr("Internal"));
    m_pTypeInternalAction->setCheckable(true);
    m_pTypeInternalAction->setChecked(false);

    m_pViewShowMenu = new QMenu(tr("Show"), this);
    m_pShowAppSignalIDAction = m_pViewShowMenu->addAction(tr("internal ID"));
    m_pShowAppSignalIDAction->setCheckable(true);
    m_pShowAppSignalIDAction->setChecked(false);
    m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
    m_pShowADCInHexAction->setCheckable(true);
    m_pShowADCInHexAction->setChecked(true);

    m_pViewMenu->addMenu(m_pViewTypeADMenu);
    m_pViewMenu->addMenu(m_pViewTypeIOMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewShowMenu);


    m_pMenuBar->addMenu(m_pViewMenu);

    connect(m_pTypeAnalogAction, &QAction::triggered, this, &SignalListDialog::showTypeAnalog);
    connect(m_pTypeDiscreteAction, &QAction::triggered, this, &SignalListDialog::showTypeDiscrete);
    connect(m_pTypeInputAction, &QAction::triggered, this, &SignalListDialog::showTypeInput);
    connect(m_pTypeOutputAction, &QAction::triggered, this, &SignalListDialog::showTypeOutput);
    connect(m_pTypeInternalAction, &QAction::triggered, this, &SignalListDialog::showTypeInternal);
    connect(m_pShowAppSignalIDAction, &QAction::triggered, this, &SignalListDialog::showAppSignalID);
    connect(m_pShowADCInHexAction, &QAction::triggered, this, &SignalListDialog::showADCInHex);


    m_pView = new QTableView(this);
    m_pView->setModel(&m_table);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        m_pView->setColumnWidth(column, m_columnWidth[column]);
    }

    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(m_pMenuBar);

    mainLayout->addWidget(m_pView);

    setLayout(mainLayout);

    createHeaderContexMenu();
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

void SignalListDialog::updateList()
{
    updateVisibleColunm();

    m_table.clear();

    QList<MeasureSignal> signalList;

    int count = theSignalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        Signal param = theSignalBase.signalParam(i);
        if (param.appSignalID().isEmpty() == true || param.hash() == 0)
        {
            continue;
        }

        if (param.signalType() != m_typeAD)
        {
            continue;
        }

        if (param.inOutType() != m_typeIO)
        {
            continue;
        }

        signalList.append( theSignalBase.signal(i) );
    }

    m_table.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::updateVisibleColunm()
{
    for(int c = 0; c < SIGNAL_LIST_COLUMN_COUNT; c++)
    {
        hideColumn(c, false);
    }

    if (m_typeAD == E::SignalType::Analog && m_typeIO == E::SignalInOutType::Input)
    {
        hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
        hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);

        m_pColumnAction[SIGNAL_LIST_COLUMN_OUT_PH_RANGE]->setChecked(false);
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

void SignalListDialog::hideColumn(int column, bool hide)
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

void SignalListDialog::showTypeAnalog()
{
    m_typeAD = E::SignalType::Analog;

    m_pTypeAnalogAction->setChecked(true);
    m_pTypeDiscreteAction->setChecked(false);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeDiscrete()
{
    m_typeAD = E::SignalType::Discrete;

    m_pTypeAnalogAction->setChecked(false);
    m_pTypeDiscreteAction->setChecked(true);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInput()
{
    m_typeIO = E::SignalInOutType::Input;

    m_pTypeInputAction->setChecked(true);
    m_pTypeOutputAction->setChecked(false);
    m_pTypeInternalAction->setChecked(false);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeOutput()
{
    m_typeIO = E::SignalInOutType::Output;

    m_pTypeInputAction->setChecked(false);
    m_pTypeOutputAction->setChecked(true);
    m_pTypeInternalAction->setChecked(false);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInternal()
{
    m_typeIO = E::SignalInOutType::Internal;

    m_pTypeInputAction->setChecked(false);
    m_pTypeOutputAction->setChecked(false);
    m_pTypeInternalAction->setChecked(true);

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showAppSignalID()
{
    m_table.setShowAppSignalID( m_pShowAppSignalIDAction->isChecked() );

    updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showADCInHex()
{
    m_table.setShowADCInHex( m_pShowADCInHexAction->isChecked() );

    updateList();
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




