#include "TuningSignalList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "SignalProperty.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSourceListTable::TuningSourceListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceListTable::~TuningSourceListTable()
{
    m_sourceMutex.lock();

        m_sourceIdList.clear();

    m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceListTable::columnCount(const QModelIndex&) const
{
    return TUN_SOURCE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceListTable::rowCount(const QModelIndex&) const
{
    return sourceCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < TUN_SOURCE_LIST_COLUMN_COUNT)
        {
            result = TuningSourceListColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceListTable::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
    {
        return QVariant();
    }

    int row = index.row();
    if (row < 0 || row >= sourceCount())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
    {
        return QVariant();
    }

    TuningSource src = source(row);

    TuningSourceState sourceState = src.state();

    if (column == TUN_SOURCE_LIST_COLUMN_IS_REPLY || column == TUN_SOURCE_LIST_COLUMN_REQUESTS || column == TUN_SOURCE_LIST_COLUMN_REPLIES  || column == TUN_SOURCE_LIST_COLUMN_COMMANDS )
    {
        // get fresh state from base
        //
         sourceState = theTuningSignalBase.sourceState( src.sourceID() );
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:   result = Qt::AlignLeft;     break;
            case TUN_SOURCE_LIST_COLUMN_CAPTION:        result = Qt::AlignLeft;     break;
            case TUN_SOURCE_LIST_COLUMN_IP:             result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_CHANNEL:        result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:      result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:      result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_IS_REPLY:       result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_REQUESTS:       result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_REPLIES:        result = Qt::AlignCenter;   break;
            case TUN_SOURCE_LIST_COLUMN_COMMANDS:       result = Qt::AlignCenter;   break;
            default:                                    assert(0);
        }

        return result;
    }

    if (role == Qt::FontRole)
    {
        return theOptions.measureView().font();
    }

    if (role == Qt::TextColorRole)
    {
        if (column == TUN_SOURCE_LIST_COLUMN_REQUESTS || column == TUN_SOURCE_LIST_COLUMN_REPLIES  || column == TUN_SOURCE_LIST_COLUMN_COMMANDS )
        {
            if (sourceState.isReply() == false)
            {
                return QColor( Qt::darkGray );
            }
        }

        return QVariant();
    }


    if (role == Qt::BackgroundColorRole)
    {
        if (column == TUN_SOURCE_LIST_COLUMN_IS_REPLY)
        {
            if (sourceState.isReply() == false)
            {
                return QColor(0xFF, 0xA0, 0xA0) ;
            }
            else
            {
                return QColor(0xA0, 0xFF, 0xA0) ;
            }
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, src, sourceState);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSourceListTable::text(int  row, int column, const TuningSource& source, const TuningSourceState& state) const
{
    if (row < 0 || row >= sourceCount())
    {
        return QString();
    }

    if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
    {
        return QString();
    }

    QString result;

    switch (column)
    {
        case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:   result = source.equipmentID();                          break;
        case TUN_SOURCE_LIST_COLUMN_CAPTION:        result = source.caption();                              break;
        case TUN_SOURCE_LIST_COLUMN_IP:             result = source.serverIP() + " ( " + QString::number( source.serverPort() ) + " )";    break;
        case TUN_SOURCE_LIST_COLUMN_CHANNEL:        result = QString::number( source.channel() + 1);        break;
        case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:      result = source.subSystem();                            break;
        case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:      result = QString::number( source.lmNumber() );          break;
        case TUN_SOURCE_LIST_COLUMN_IS_REPLY:       result = state.isReply() == false ? "No" : "Yes";       break;
        case TUN_SOURCE_LIST_COLUMN_REQUESTS:       result = QString::number( state.requestCount() );       break;
        case TUN_SOURCE_LIST_COLUMN_REPLIES:        result = QString::number( state.replyCount() );         break;
        case TUN_SOURCE_LIST_COLUMN_COMMANDS:       result = QString::number( state.commandQueueSize() );   break;
        default:                                    assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceListTable::updateColumn(int column)
{
    if (column < 0 || column >= TUN_SOURCE_LIST_COLUMN_COUNT)
    {
        return;
    }

    int count = rowCount();

    for (int row = 0; row < count; row ++)
    {
        QModelIndex cellIndex = index(row, column);

        emit dataChanged( cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
    }
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceListTable::sourceCount() const
{
    int count = 0;

    m_sourceMutex.lock();

        count = m_sourceIdList.size();

    m_sourceMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource TuningSourceListTable::source(int index) const
{
    TuningSource param;

    m_sourceMutex.lock();

        if (index >= 0 && index < m_sourceIdList.size())
        {
             param = m_sourceIdList[index];
        }

    m_sourceMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceListTable::set(const QList<TuningSource> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_sourceMutex.lock();

            m_sourceIdList = list_add;

        m_sourceMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceListTable::clear()
{
    int count = m_sourceIdList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_sourceMutex.lock();

            m_sourceIdList.clear();

        m_sourceMutex.unlock();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool TuningSignalListTable::m_showCustomID = true;

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListTable::TuningSignalListTable(QObject*)
{
    connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &TuningSignalListTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListTable::~TuningSignalListTable()
{
    m_signalMutex.lock();

        m_signallList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalListTable::columnCount(const QModelIndex&) const
{
    return TUN_SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalListTable::rowCount(const QModelIndex&) const
{
    return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSignalListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < TUN_SIGNAL_LIST_COLUMN_COUNT)
        {
            result = TuningSignalListColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSignalListTable::data(const QModelIndex &index, int role) const
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
    if (column < 0 || column > TUN_SIGNAL_LIST_COLUMN_COUNT)
    {
        return QVariant();
    }

    TuningSignal signal = at(row);
    if (signal.hash() == 0)
    {
        return QVariant();
    }

    if (column == TUN_SIGNAL_LIST_COLUMN_STATE || column == TUN_SIGNAL_LIST_COLUMN_RANGE)
    {
        // get fresh state from base
        //
        TuningSignalState signalState = theTuningSignalBase.signalState( signal.hash() );
        signal.setState( signalState );
    }

    if (role == Qt::TextAlignmentRole)
    {
        int result = Qt::AlignLeft;

        switch (column)
        {
            case TUN_SIGNAL_LIST_COLUMN_CASE:           result = Qt::AlignLeft;     break;
            case TUN_SIGNAL_LIST_COLUMN_ID:             result = Qt::AlignLeft;     break;
            case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:   result = Qt::AlignLeft;     break;
            case TUN_SIGNAL_LIST_COLUMN_CAPTION:        result = Qt::AlignLeft;     break;
            case TUN_SIGNAL_LIST_COLUMN_STATE:          result = Qt::AlignCenter;   break;
            case TUN_SIGNAL_LIST_COLUMN_DEFAULT:        result = Qt::AlignCenter;   break;
            case TUN_SIGNAL_LIST_COLUMN_RANGE:          result = Qt::AlignCenter;   break;
            default:                                    assert(0);
        }

        return result;
    }

    if (role == Qt::FontRole)
    {
        return theOptions.measureView().font();
    }

    if (role == Qt::TextColorRole)
    {
        if (column == TUN_SIGNAL_LIST_COLUMN_DEFAULT)
        {
            return QColor( Qt::darkGray );
        }

        return QVariant();
    }


    if (role == Qt::BackgroundColorRole)
    {
        if (column == TUN_SIGNAL_LIST_COLUMN_STATE)
        {
            if (signal.state().valid() == false)
            {
                return theOptions.signalInfo().colorFlagValid();
            }
        }

        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column, signal);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignalListTable::text(int  row, int column, const TuningSignal& signal) const
{
    if (row < 0 || row >= signalCount())
    {
        return QString();
    }

    if (column < 0 || column > TUN_SIGNAL_LIST_COLUMN_COUNT)
    {
        return QString();
    }

    if (signal.hash() == 0)
    {
        return QString();
    }



    QString result;

    switch (column)
    {
        case TUN_SIGNAL_LIST_COLUMN_CASE:           result = signal.caseStr();          break;
        case TUN_SIGNAL_LIST_COLUMN_ID:             result = m_showCustomID == true ? signal.customAppSignalID() : signal.appSignalID(); break;
        case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:   result = signal.equipmentID();      break;
        case TUN_SIGNAL_LIST_COLUMN_CAPTION:        result = signal.caption();          break;
        case TUN_SIGNAL_LIST_COLUMN_STATE:          result = signal.valueStr();         break;
        case TUN_SIGNAL_LIST_COLUMN_DEFAULT:        result = signal.defaultValueStr();  break;
        case TUN_SIGNAL_LIST_COLUMN_RANGE:          result = signal.rangeStr();         break;
        default:                                    assert(0);
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListTable::updateColumn(int column)
{
    if (column < 0 || column >= TUN_SIGNAL_LIST_COLUMN_COUNT)
    {
        return;
    }

    int count = rowCount();

    for (int row = 0; row < count; row ++)
    {
        QModelIndex cellIndex = index(row, column);

        emit dataChanged( cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
    }
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalListTable::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signallList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal TuningSignalListTable::at(int index) const
{
    TuningSignal signal;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signallList.size())
        {
             signal = m_signallList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListTable::set(const QList<TuningSignal> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_signalMutex.lock();

            m_signallList = list_add;

        m_signalMutex.unlock();

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListTable::clear()
{
    int count = m_signallList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_signalMutex.lock();

            m_signallList.clear();

        m_signalMutex.unlock();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListTable::updateSignalParam(const Hash& signalHash)
{
    if (signalHash == 0)
    {
        assert(signalHash != 0);
        return;
    }

    m_signalMutex.lock();

        int count = m_signallList.count();

        for(int i = 0; i < count; i ++)
        {
            if (m_signallList[i].hash() == signalHash)
            {
				Metrology::SignalParam param = theSignalBase.signalParam(signalHash);
                if (param.isValid() == false)
                {
                    continue;
                }

                TuningSignal signal = theTuningSignalBase.signalForRead( m_signallList[i].hash() );
                if (signal.hash() == 0)
                {
                    continue;
                }

                signal.setCustomAppSignalID( param.customAppSignalID() );
                signal.setCaption( param.caption() );
                signal.setDefaultValue( param.tuningDefaultValue() );
                signal.setPrecision( param.inputPhysicalPrecision() );

                theTuningSignalBase.setSignalForRead(signal);

                break;
            }
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int TuningSignalListDialog::m_sourceColumnWidth[TUN_SOURCE_LIST_COLUMN_COUNT] =
{
    250,    // TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID
    150,    // TUN_SOURCE_LIST_COLUMN_CAPTION
    150,    // TUN_SOURCE_LIST_COLUMN_IP
    100,    // TUN_SOURCE_LIST_COLUMN_CHANNEL
    100,    // TUN_SOURCE_LIST_COLUMN_SUBSYSTEM
    100,    // TUN_SOURCE_LIST_COLUMN_LM_NUMBER
    100,    // TUN_SOURCE_LIST_COLUMN_IS_REPLY
    100,    // TUN_SOURCE_LIST_COLUMN_REQUESTS
    100,    // TUN_SOURCE_LIST_COLUMN_REPLIES
    100,    // TUN_SOURCE_LIST_COLUMN_COMMANDS
};

int TuningSignalListDialog::m_signalColumnWidth[TUN_SIGNAL_LIST_COLUMN_COUNT] =
{
    100,    // TUN_SIGNAL_LIST_COLUMN_CASE
    250,    // TUN_SIGNAL_LIST_COLUMN_ID
    250,    // TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID
    150,    // TUN_SIGNAL_LIST_COLUMN_CAPTION
    100,    // TUN_SIGNAL_LIST_COLUMN_VALUE
    100,    // TUN_SIGNAL_LIST_COLUMN_DEFAULT
    150,    // TUN_SIGNAL_LIST_COLUMN_RANGE
};

// -------------------------------------------------------------------------------------------------------------------

E::SignalType       TuningSignalListDialog::m_typeAD = E::SignalType::Analog;
bool                TuningSignalListDialog::m_showSource = false;

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListDialog::TuningSignalListDialog(bool hasButtons, QWidget *parent) :
    QDialog(parent)
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->m_pConfigSocket != nullptr)
    {
		if (pMainWindow->m_pConfigSocket != nullptr)
        {
			connect(pMainWindow->m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &TuningSignalListDialog::updateSignalList, Qt::QueuedConnection);
        }

        if (pMainWindow->m_pTuningSocket != nullptr)
        {
            connect(pMainWindow->m_pTuningSocket, &TuningSocket::sourcesLoaded, this, &TuningSignalListDialog::updateSourceList, Qt::QueuedConnection);
            connect(pMainWindow->m_pTuningSocket, &TuningSocket::socketDisconnected, this, &TuningSignalListDialog::updateSourceList, Qt::QueuedConnection);
        }
    }

    createInterface(hasButtons);
    updateSourceList();
    updateSignalList();

    startSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListDialog::~TuningSignalListDialog()
{
    stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createInterface(bool hasButtons)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/icons/InOut.png"));
    setWindowTitle(tr("Tuning signals"));
    resize(QApplication::desktop()->availableGeometry().width() - 200, 500);
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

    m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
    m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
    m_pTypeAnalogAction->setCheckable(true);
    m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
    m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
    m_pTypeDiscreteAction->setCheckable(true);
    m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);


    m_pViewShowMenu = new QMenu(tr("Show"), this);
    m_pShowSoucreAction = m_pViewShowMenu->addAction(tr("Sources"));
    m_pShowSoucreAction->setCheckable(true);
    m_pShowSoucreAction->setChecked(m_showSource);

    m_pShowCustomIDAction = m_pViewShowMenu->addAction(tr("Custom ID"));
    m_pShowCustomIDAction->setCheckable(true);
    m_pShowCustomIDAction->setChecked(m_signalTable.showCustomID());
    m_pShowCustomIDAction->setShortcut(Qt::CTRL + Qt::Key_Tab);


    m_pViewMenu->addMenu(m_pViewTypeADMenu);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addMenu(m_pViewShowMenu);

    m_pMenuBar->addMenu(m_pSignalMenu);
    m_pMenuBar->addMenu(m_pEditMenu);
    m_pMenuBar->addMenu(m_pViewMenu);

    connect(m_pPrintAction, &QAction::triggered, this, &TuningSignalListDialog::printSignal);
    connect(m_pExportAction, &QAction::triggered, this, &TuningSignalListDialog::exportSignal);

    connect(m_pFindAction, &QAction::triggered, this, &TuningSignalListDialog::find);
    connect(m_pCopyAction, &QAction::triggered, this, &TuningSignalListDialog::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &TuningSignalListDialog::selectAll);

    connect(m_pTypeAnalogAction, &QAction::triggered, this, &TuningSignalListDialog::showTypeAnalog);
    connect(m_pTypeDiscreteAction, &QAction::triggered, this, &TuningSignalListDialog::showTypeDiscrete);
    connect(m_pShowSoucreAction, &QAction::triggered, this, &TuningSignalListDialog::showSources);
    connect(m_pShowCustomIDAction, &QAction::triggered, this, &TuningSignalListDialog::showCustomID);


    m_pSourceView = new QTableView(this);
    m_pSourceView->setModel(&m_sourceTable);
    QSize sourceCellSize = QFontMetrics( theOptions.measureView().font() ).size(Qt::TextSingleLine,"A");
    m_pSourceView->verticalHeader()->setDefaultSectionSize(sourceCellSize.height());

    for(int column = 0; column < TUN_SOURCE_LIST_COLUMN_COUNT; column++)
    {
        m_pSourceView->setColumnWidth(column, m_sourceColumnWidth[column]);
    }

    m_pSourceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pSourceView->setFixedHeight(120);

    if (m_showSource == false)
    {
        m_pSourceView->hide();
    }

    m_pSignalView = new QTableView(this);
    m_pSignalView->setModel(&m_signalTable);
    QSize signalCellSize = QFontMetrics( theOptions.measureView().font() ).size(Qt::TextSingleLine,"A");
    m_pSignalView->verticalHeader()->setDefaultSectionSize(signalCellSize.height());

    for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        m_pSignalView->setColumnWidth(column, m_signalColumnWidth[column]);
    }

    m_pSignalView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_pSignalView, &QTableView::doubleClicked , this, &TuningSignalListDialog::onListDoubleClicked);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->setMenuBar(m_pMenuBar);
    mainLayout->addWidget(m_pSourceView);
    mainLayout->addWidget(m_pSignalView);

    if (hasButtons == true)
    {
        m_pSignalView->setSelectionMode(QAbstractItemView::SingleSelection);

        m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &TuningSignalListDialog::onOk);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &TuningSignalListDialog::reject);

        mainLayout->addWidget(m_buttonBox);
    }

    setLayout(mainLayout);

    createHeaderContexMenu();
    createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createHeaderContexMenu()
{
    // init header context menu
    //
    m_pSignalView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pSignalView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &TuningSignalListDialog::onHeaderContextMenu);

    m_headerContextMenu = new QMenu(m_pSignalView);

    for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        m_pColumnAction[column] = m_headerContextMenu->addAction(TuningSignalListColumn[column]);
        if (m_pColumnAction[column] != nullptr)
        {
            m_pColumnAction[column]->setCheckable(true);
            m_pColumnAction[column]->setChecked(true);

            connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &TuningSignalListDialog::onColumnAction);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr(""), this);

    m_pContextMenu->addAction(m_pCopyAction);
    m_pContextMenu->addSeparator();
    m_pContextMenu->addMenu(m_pViewTypeADMenu);

    // init context menu
    //
    m_pSignalView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pSignalView, &QTableWidget::customContextMenuRequested, this, &TuningSignalListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateSourceList()
{
    // update source list

    m_sourceTable.clear();

    QList<TuningSource> sourceList;

    int souceCount = theTuningSignalBase.sourceCount();
    for(int i = 0; i < souceCount; i++)
    {
        TuningSource src = theTuningSignalBase.source(i);
        if (src.sourceID() == 0)
        {
            continue;
        }

        sourceList.append( src );
    }

    m_sourceTable.set(sourceList);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateSignalList()
{
    updateVisibleColunm();

    // update signal list

    m_signalTable.clear();

    QList<TuningSignal> signalList;

    int sigbalCount = theTuningSignalBase.signalCount();
    for(int i = 0; i < sigbalCount; i++)
    {
        TuningSignal signal = theTuningSignalBase.signalForRead(i);
        if (signal.hash() == 0)
        {
            continue;
        }

        if (signal.signalType() != m_typeAD)
        {
            continue;
        }

        signalList.append( signal );
    }

    m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateState()
{
    m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_IS_REPLY);
    m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REQUESTS);
    m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REPLIES);
    m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_COMMANDS);

    m_signalTable.updateColumn(TUN_SIGNAL_LIST_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateVisibleColunm()
{
    m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
    m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);

    m_pShowSoucreAction->setChecked(m_showSource);

    for(int c = 0; c < TUN_SIGNAL_LIST_COLUMN_COUNT; c++)
    {
        hideColumn(c, false);
    }

    hideColumn(TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID, true);

    if (m_typeAD == E::SignalType::Discrete)
    {
        hideColumn(TUN_SIGNAL_LIST_COLUMN_RANGE, true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::hideColumn(int column, bool hide)
{
    if (column < 0 || column >= TUN_SIGNAL_LIST_COLUMN_COUNT)
    {
        return;
    }

    if (hide == true)
    {
        m_pSignalView->hideColumn(column);
        m_pColumnAction[column]->setChecked(false);
    }
    else
    {
        m_pSignalView->showColumn(column);
        m_pColumnAction[column]->setChecked(true);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::startSignalStateTimer()
{
    if (m_updateSignalStateTimer == nullptr)
    {
        m_updateSignalStateTimer = new QTimer(this);
        connect(m_updateSignalStateTimer, &QTimer::timeout, this, &TuningSignalListDialog::updateState);
    }

    m_updateSignalStateTimer->start(250); // 250 ms
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::stopSignalStateTimer()
{
    if (m_updateSignalStateTimer != nullptr)
    {
        m_updateSignalStateTimer->stop();
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool TuningSignalListDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>( event );

        if (keyEvent->key() == Qt::Key_Return)
        {
            if (m_buttonBox != nullptr)
            {
                emit onOk();
            }
        }
    }

    return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::printSignal()
{

}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::exportSignal()
{
    ExportData* dialog = new ExportData(m_pSignalView, tr("Signals"));
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::find()
{
    FindData* dialog = new FindData(m_pSignalView);
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::copy()
{
    QString textClipboard;

    int rowCount = m_pSignalView->model()->rowCount();
    int columnCount = m_pSignalView->model()->columnCount();

    for(int row = 0; row < rowCount; row++)
    {
        if (m_pSignalView->selectionModel()->isRowSelected(row, QModelIndex() ) == false)
        {
            continue;
        }

        for(int column = 0; column < columnCount; column++)
        {
            if (m_pSignalView->isColumnHidden(column) == true)
            {
                continue;
            }

            textClipboard.append(m_pSignalView->model()->data( m_pSignalView->model()->index(row, column)).toString() + "\t");
        }

        textClipboard.replace(textClipboard.length() - 1, 1, "\n");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showTypeAnalog()
{
    m_typeAD = E::SignalType::Analog;

    updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showTypeDiscrete()
{
    m_typeAD = E::SignalType::Discrete;

    updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showSources()
{
    m_showSource = m_pShowSoucreAction->isChecked();

    if (m_showSource == true)
    {
        m_pSourceView->show();
    }
    else
    {
        m_pSourceView->hide();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showCustomID()
{
    m_signalTable.setShowCustomID( m_pShowCustomIDAction->isChecked() );

    updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onHeaderContextMenu(QPoint)
{
    if (m_headerContextMenu == nullptr)
    {
        return;
    }

    m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onColumnAction(QAction* action)
{
    if (action == nullptr)
    {
        return;
    }

    for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
    {
        if (m_pColumnAction[column] == action)
        {
            hideColumn(column,  !action->isChecked());

            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onListDoubleClicked(const QModelIndex&)
{
    if (m_buttonBox == nullptr)
    {
        return;
    }

    emit onOk();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onOk()
{
    int index = m_pSignalView->currentIndex().row();
    if (index < 0 || index >= m_signalTable.signalCount())
    {
        return;
    }

    TuningSignal signal = m_signalTable.at(index);
    if (signal.hash() == 0)
    {
        return;
    }

    m_selectedSignalHash = signal.hash();

    accept();
}

// -------------------------------------------------------------------------------------------------------------------
