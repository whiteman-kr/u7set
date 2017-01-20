#include "FindMeasurePanel.h"

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

FindItem::FindItem(const int& row, const int& column, const int& beginPos, const int& endPos, const QString& text)
{
    m_row = row;
    m_column = column;

    m_beginPos = beginPos;
    m_endPos = endPos;

    m_text = text;
}

// -------------------------------------------------------------------------------------------------------------------

FindItem& FindItem::operator=(const FindItem& from)
{
    m_row = from.m_row;
    m_column = from.m_column;

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
    return FIND_MEASURE_COLUMN_COUNT;
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
        if (section >= 0 && section < FIND_MEASURE_COLUMN_COUNT)
        {
            result = FindMeasureColumn[section];
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

    int row = index.row();
    if (row < 0 || row >= m_findItemList.count())
    {
        return QVariant();
    }

    int column = index.column();
    if (column < 0 || column > FIND_MEASURE_COLUMN_COUNT)
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignLeft;
    }

    if (role == Qt::TextColorRole)
    {
        if (column == FIND_MEASURE_COLUMN_ROW)
        {
            return QColor(0xA0, 0xA0, 0xA0);
        }

        return QVariant();
    }

    if (role == Qt::UserRole )
    {
        QVariant var;
        var.setValue(m_findItemList.at(row));
        return var;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return text(row, column);
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

    if (column < 0 || column > FIND_MEASURE_COLUMN_COUNT)
    {
        return "";
    }

    FindItem item = m_findItemList.at(row);

    QString result;

    switch (column)
    {
        case FIND_MEASURE_COLUMN_ROW:       result = QString::number(item.row() + 1);   break;
        case FIND_MEASURE_COLUMN_TEXT:      result = item.text();                       break;
        default:                            assert(0);
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

FindMeasurePanel::FindMeasurePanel(QWidget* parent) :
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

void FindMeasurePanel::createInterface()
{
    m_pFindWindow = new QMainWindow;

    QToolBar *toolBar = new QToolBar(m_pFindWindow);

    QLabel* label = new QLabel(tr("Input text: "), toolBar);
    m_findTextEdit = new QLineEdit(m_findText, toolBar);

    toolBar->addWidget(label);
    toolBar->addWidget(m_findTextEdit);
    QAction* action =  toolBar->addAction(QIcon(":/icons/Search.png"), tr("Find text"));
    connect(action, &QAction::triggered, this, &FindMeasurePanel::find);

    toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    toolBar->setWindowTitle(tr("Search measurements ToolBar"));
    m_pFindWindow->addToolBarBreak(Qt::TopToolBarArea);
    m_pFindWindow->addToolBar(toolBar);

    m_pFindItemView = new QTableView(m_pFindWindow);
    m_pFindItemView->setModel(&m_table);
    QSize cellSize = QFontMetrics( theOptions.measureView().m_font ).size(Qt::TextSingleLine,"A");
    m_pFindItemView->verticalHeader()->setDefaultSectionSize(cellSize.height());

    m_pFindWindow->setCentralWidget(m_pFindItemView);

    m_pFindItemView->setColumnWidth(FIND_MEASURE_COLUMN_ROW, FIND_MEASURE_COLUMN_ROW_WIDTH);

    connect(m_pFindItemView, &QTableView::clicked, this, &FindMeasurePanel::selectItemInMeasureView);
    m_pFindItemView->installEventFilter(this);

    FindTextDelegate* textDelegate = new FindTextDelegate(this);
    m_pFindItemView->setItemDelegateForColumn(FIND_MEASURE_COLUMN_TEXT, textDelegate);

    m_pFindItemView->horizontalHeader()->hide();
    m_pFindItemView->verticalHeader()->hide();
    m_pFindItemView->setShowGrid(false);
    m_pFindItemView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStatusBar* statusBar = m_pFindWindow->statusBar();
    m_statusLabel = new QLabel(tr("Found: 0"), statusBar);
    statusBar->addWidget(m_statusLabel);
    statusBar->setLayoutDirection(Qt::RightToLeft);

    setWidget(m_pFindWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::createContextMenu()
{
    // create context menu
    //
    m_pContextMenu = new QMenu(tr("&Measurements"), m_pFindWindow);

    m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
    m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
    m_pContextMenu->addSeparator();
    m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));
    m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

    connect(m_pCopyAction, &QAction::triggered, this, &FindMeasurePanel::copy);
    connect(m_pSelectAllAction, &QAction::triggered, this, &FindMeasurePanel::selectAll);

    // init context menu
    //
    m_pFindItemView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_pFindItemView, &QTableWidget::customContextMenuRequested, this, &FindMeasurePanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

bool FindMeasurePanel::event(QEvent* e)
{
    if (e->type() == QEvent::Hide)
    {
        saveSettings();
    }

    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( e );

        if (keyEvent->key() == Qt::Key_Return)
        {
            find();
        }
    }

    if (e->type() == QEvent::Resize)
    {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>( e );

        m_pFindItemView->setColumnWidth(FIND_MEASURE_COLUMN_TEXT, resizeEvent->size().width() - FIND_MEASURE_COLUMN_ROW_WIDTH - 20 );

    }

    return QDockWidget::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

bool FindMeasurePanel::eventFilter(QObject* object, QEvent* e)
{
    if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* ke =  static_cast<QKeyEvent *>( e );

        if(ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown)
        {
            selectItemInMeasureView(QModelIndex());
        }
    }

    return QDockWidget::eventFilter(object, e);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::find()
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

    int measureType = pMainWindow->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    m_measureType = measureType;

    MeasureView* pMeasureView = pMainWindow->m_measureView[measureType];
    if (pMeasureView == nullptr)
    {
        return;
    }

    QList<FindItem> findItemList;

    m_table.clear();

    int rowCount = pMeasureView->table().count();
    int columnCount = pMeasureView->table().header().count();

    for(int row = 0; row < rowCount; row ++)
    {
        for(int column = 0; column < columnCount; column++)
        {
            if (pMeasureView->table().columnIsVisible(column) == false)
            {
                continue;
            }

            QString text = pMeasureView->table().text(row, column);

            int pos = text.indexOf(m_findText);
            if (pos == -1)
            {
                continue;
            }

            findItemList.append( FindItem(row, column, pos, pos + m_findText.count(), text ) );
        }
    }

    m_statusLabel->setText(QString("Found: %1").arg(findItemList.count()) );

    if (findItemList.count() == 0 )
    {
        QMessageBox::information(this, windowTitle(), tr("Text \"%1\" was not found!").arg(m_findText));
        return;
    }

    m_table.set(findItemList);

    m_pFindItemView->setCurrentIndex(m_pFindItemView->model()->index(0, FIND_MEASURE_COLUMN_ROW));
    m_pFindItemView->setFocus();

    selectItemInMeasureView(QModelIndex());
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::selectItemInMeasureView(QModelIndex)
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

    int indexFindItem = m_pFindItemView->currentIndex().row();
    if (indexFindItem < 0 || indexFindItem >= m_table.count())
    {
        return;
    }

    FindItem fi = m_table.at(indexFindItem);

    int row = fi.row();
    if (row < 0 || row >= pMeasureView->table().count())
    {
        return;
    }

    int column = fi.column();
    if (column < 0 || column > pMeasureView->table().header().count())
    {
        return;
    }

    QModelIndex selectIndex = pMeasureView->model()->index(row, column);
    pMeasureView->setCurrentIndex(selectIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::onContextMenu(QPoint)
{
    m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::copy()
{
    bool appendRow;
    QString textRow;
    QString textClipboard;

    int count = m_table.count();
    for(int r = 0; r < count; r++)
    {
        appendRow = false;
        textRow = "";

        for(int c = 0; c < FIND_MEASURE_COLUMN_COUNT; c++)
        {
            if (m_pFindItemView->selectionModel()->isSelected( m_pFindItemView->model()->index(r,c) ) == true)
            {
                appendRow = true;
                textRow.append(m_table.text(r, c));
            }

            if (c != FIND_MEASURE_COLUMN_COUNT - 1)
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

void FindMeasurePanel::selectAll()
{
    m_pFindItemView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::loadSettings()
{
    QSettings s;

    m_findText = s.value( QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), "").toString();
}

// -------------------------------------------------------------------------------------------------------------------

void FindMeasurePanel::saveSettings()
{
    QSettings s;

    s.setValue( QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), m_findText);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
