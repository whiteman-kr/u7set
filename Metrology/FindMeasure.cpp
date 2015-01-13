#include "FindMeasure.h"

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

FindItem::FindItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

FindItem::FindItem(int row, int column, QString columnTitle, int beginPos, int endPos, QString text)
{
    m_row = row;
    m_column = column;
    m_columnTitle = columnTitle;

    m_beginPos = beginPos;
    m_endPos = endPos;

    m_text = text;
}

// -------------------------------------------------------------------------------------------------------------------

FindItem& FindItem::operator=(const FindItem& from)
{
    m_row = from.m_row;
    m_column = from.m_column;
    m_columnTitle = from.m_columnTitle;

    m_beginPos = from.m_beginPos;
    m_endPos = from.m_endPos;

    m_text = from.m_text;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindMeasureTable::FindMeasureTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

FindMeasureTable::~FindMeasureTable()
{
    m_findItemList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int FindMeasureTable::columnCount(const QModelIndex&) const
{
    return FM_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int FindMeasureTable::rowCount(const QModelIndex&) const
{
    return m_findItemList.count();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FindMeasureTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    QVariant result = QVariant();

    if(orientation == Qt::Horizontal )
    {
        if (section >= 0 && section < FM_COLUMN_COUNT)
        {
            result = FmColumn[section];
        }
    }

    if(orientation == Qt::Vertical )
    {
        result = QString("%1").arg( section + 1 );
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FindMeasureTable::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
    {
        return QVariant();
    }

    int indexRow = index.row();
    if (indexRow < 0 || indexRow >= m_findItemList.count())
    {
        return QVariant();
    }

    int indexColumn = index.column();
    if (indexColumn < 0 || indexColumn > FM_COLUMN_COUNT)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {

        int result = Qt::AlignCenter;

        switch (indexColumn)
        {
            case FM_COLUMN_TEXT:    result = Qt::AlignLeft;     break;
            case FM_COLUMN_INDEX:   result = Qt::AlignCenter;   break;
            case FM_COLUMN_COLUMN:  result = Qt::AlignLeft;     break;
            default:                assert(0);                  break;
        }

        return result;
    }

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_findItemList.at(indexRow));
        return var;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(indexRow, indexColumn);
    }

    return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString FindMeasureTable::text(int row, int column) const
{
    if (row < 0 || row >= m_findItemList.count())
    {
        return "";
    }

    if (column < 0 || column > FM_COLUMN_COUNT)
    {
        return "";
    }

    FindItem item = m_findItemList.at(row);

    QString result;

    switch (column)
    {
        case FM_COLUMN_TEXT:    result = item.text();                       break;
        case FM_COLUMN_INDEX:   result = QString::number(item.row() + 1);   break;
        case FM_COLUMN_COLUMN:  result = item.columnTitle();                break;
        default:                assert(0);                                  break;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------


FindItem FindMeasureTable::at(int index)
{
    if (index < 0 || index >= count())
    {
        return FindItem();
    }

    return m_findItemList.at(index);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasureTable::set(const QList<FindItem> list_add)
{
    int count = list_add.count();
    if (count == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, count - 1);

        m_findItemList = list_add;

    endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasureTable::clear()
{
    int count = m_findItemList.count();
    if (count == 0)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, count - 1 );

        m_findItemList.clear();

    endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int FindMeasure::m_columnWidth[FM_COLUMN_COUNT] = { 200, 50, 100, };

// -------------------------------------------------------------------------------------------------------------------

FindMeasure::FindMeasure(QWidget* parent) :
    QDockWidget(parent)
{
    m_pMainWindow = dynamic_cast<QMainWindow*> (parent);
    if (m_pMainWindow == nullptr)
    {
        return;
    }

    setWindowTitle("Search measurements panel");
    setObjectName(windowTitle());

    loadSettings();

    createInterface();
    createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::createInterface()
{
    m_pFindWindow = new QMainWindow;

    QToolBar *toolBar = new QToolBar(m_pFindWindow);

    QLabel* label = new QLabel(tr("Input text: "), toolBar);
    m_findTextEdit = new QLineEdit(m_findText, toolBar);

    toolBar->addWidget(label);
    toolBar->addWidget(m_findTextEdit);
    QAction* action =  toolBar->addAction(QIcon(":/icons/Search.png"), tr("Find text"));
    connect(action, &QAction::triggered, this, &FindMeasure::find);

    toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    toolBar->setWindowTitle(tr("Search measurements ToolBar"));
    m_pFindWindow->addToolBarBreak(Qt::TopToolBarArea);
    m_pFindWindow->addToolBar(toolBar);

    m_pView = new QTableView(m_pFindWindow);
    m_pView->setModel(&m_table);
    m_pView->verticalHeader()->setDefaultSectionSize(20);
    m_pFindWindow->setCentralWidget(m_pView);

    for(int c = 0; c < FM_COLUMN_COUNT; c++)
    {
        m_pView->setColumnWidth(c, m_columnWidth[c]);
    }

    connect(m_pView, &QTableView::clicked, this, &FindMeasure::selectMeasureCell);
    m_pView->installEventFilter(this);

    FindTextDelegate* textDelegate = new FindTextDelegate(this);
    m_pView->setItemDelegateForColumn(FM_COLUMN_TEXT, textDelegate);

    m_pView->verticalHeader()->hide();
    m_pView->setShowGrid(false);
    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_pView->horizontalHeader(), &QHeaderView::sectionResized, this, &FindMeasure::onColumnResized);

    QStatusBar* statusBar = m_pFindWindow->statusBar();
    m_statusLabel = new QLabel(tr("Found: 0"), statusBar);
    statusBar->addWidget(m_statusLabel);
    statusBar->setLayoutDirection(Qt::RightToLeft);

    setWidget(m_pFindWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr("&Measurements"), m_pFindWindow);

    m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
    m_pContextMenu->addSeparator();
    m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));

    connect(m_pCopyAction, &QAction::triggered, this, &FindMeasure::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &FindMeasure::selectAll);

    // init context menu
    //
    m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pView, &QTableWidget::customContextMenuRequested, this, &FindMeasure::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::find()
{
    m_findText = m_findTextEdit->text();
    if (m_findText.isEmpty() == true)
    {
        return;
    }

    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
    if (pMainWindow == nullptr)
    {
        return;
    }

    int type = pMainWindow->measureType();
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_measureType = type;

    MeasureView* pMeasureView = pMainWindow->m_measureView[type];
    if (pMeasureView == nullptr)
    {
        return;
    }

    QList<FindItem> findItemList;

    m_table.clear();

    int columnCount = pMeasureView->table().header().count();
    int measureCount = pMeasureView->table().count();

    for(int m = 0; m < measureCount; m ++)
    {
        for(int c = 0; c < columnCount; c++)
        {
            if (pMeasureView->table().columnIsVisible(c) == false)
            {
                continue;
            }

            QString title = pMeasureView->table().header().column(c)->title();
            QString text = pMeasureView->table().text(m, c);
            int pos = text.indexOf(m_findText);
            if (pos != -1)
            {
                findItemList.append( FindItem(m, c, title, pos, pos + m_findText.count(), text ) );
            }
        }
    }

    m_statusLabel->setText(QString("Found: %1").arg(findItemList.count()) );

    if (findItemList.count() == 0 )
    {
        QMessageBox::information(this, windowTitle(), tr("Text not found!"));
        return;
    }

    m_table.set(findItemList);

    m_pView->setCurrentIndex(m_pView->model()->index(0, FM_COLUMN_TEXT));
    m_pView->setFocus();

    selectMeasureCell(QModelIndex());
}

// -------------------------------------------------------------------------------------------------------------------

bool FindMeasure::event(QEvent* e)
{
    if (e->type() == QEvent::Hide)
    {
        saveSettings();
    }

    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent =  static_cast<QKeyEvent *>( e );

        if (keyEvent->key() == Qt::Key_Return)
        {
            find();
        }
    }

    return QDockWidget::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

bool FindMeasure::eventFilter(QObject* object, QEvent* e)
{
    if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* ke =  static_cast<QKeyEvent *>( e );

        if(ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown)
        {
            selectMeasureCell(QModelIndex());
        }
    }

    return QDockWidget::eventFilter(object, e);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::selectMeasureCell(QModelIndex)
{
    MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
    if (pMainWindow == nullptr)
    {
        return;
    }

    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    MeasureView* pMeasureView = pMainWindow->m_measureView[m_measureType];
    if (pMeasureView == nullptr)
    {
        return;
    }

    int indexTable = m_pView->currentIndex().row();
    if (indexTable < 0 || indexTable >= m_table.count())
    {
        return;
    }

    FindItem fi = m_table.at(indexTable);

    int indexRow = fi.row();
    if (indexRow < 0 || indexRow >= pMeasureView->table().count())
    {
        return;
    }

    int indexColumn = fi.column();
    if (indexColumn < 0 || indexColumn > pMeasureView->table().header().count())
    {
        return;
    }

    QModelIndex selectIndex = pMeasureView->model()->index(indexRow, indexColumn);
    pMeasureView->setCurrentIndex(selectIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::copy()
{
    bool appendRow;
    QString textRow;
    QString textClipboard;

    int count = m_table.count();
    for(int r = 0; r < count; r++)
    {
        appendRow = false;
        textRow = "";

        for(int c = 0; c < FM_COLUMN_COUNT; c++)
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

void FindMeasure::selectAll()
{
    m_pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::onColumnResized(int index, int, int width)
{
    if (index < 0 || index >= FM_COLUMN_COUNT)
    {
        return;
    }

    m_columnWidth[index] = width;
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::loadSettings()
{
    QSettings s;

    m_findText = s.value( QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), "").toString();

    for(int c = 0; c < FM_COLUMN_COUNT; c++)
    {
        m_columnWidth[c] = s.value(QString("%1/Header/%2/Width").arg(FIND_MEASURE_OPTIONS_KEY).arg(FmColumn[c]), m_columnWidth[c]).toInt();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasure::saveSettings()
{
    QSettings s;

    s.setValue( QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), m_findText);

    for(int c = 0; c < FM_COLUMN_COUNT; c++)
    {
        s.setValue(QString("%1/Header/%2/Width").arg(FIND_MEASURE_OPTIONS_KEY).arg(FmColumn[c]), m_columnWidth[c]);
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
