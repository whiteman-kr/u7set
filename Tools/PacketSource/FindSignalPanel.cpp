#include "FindSignalPanel.h"

#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindItem::FindItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

FindItem::FindItem(int row, int column, const QString& text) :
	m_row (row) ,
	m_column (column) ,
	m_text (text)
{

}

// -------------------------------------------------------------------------------------------------------------------

FindItem& FindItem::operator=(const FindItem& from)
{
	m_row = from.m_row;
	m_column = from.m_column;

	m_text = from.m_text;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindSignalTable::FindSignalTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

FindSignalTable::~FindSignalTable()
{
	m_findItemList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int FindSignalTable::columnCount(const QModelIndex&) const
{
	return FIND_SIGNAL_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int FindSignalTable::rowCount(const QModelIndex&) const
{
	return m_findItemList.count();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FindSignalTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < FIND_SIGNAL_COLUMN_COUNT)
		{
			result = FindSignalTextColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant FindSignalTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > FIND_SIGNAL_COLUMN_COUNT)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::TextColorRole)
	{
		if (column == FIND_SIGNAL_COLUMN_ROW)
		{
			return QColor(Qt::lightGray);
		}

		return QVariant();
	}

	if (role == Qt::UserRole)
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

QString FindSignalTable::text(int row, int column) const
{
	if (row < 0 || row >= m_findItemList.count())
	{
		return QString();
	}

	if (column < 0 || column > FIND_SIGNAL_COLUMN_COUNT)
	{
		return QString();
	}

	FindItem item = m_findItemList.at(row);

	QString result;

	switch (column)
	{
		case FIND_SIGNAL_COLUMN_ROW:	result = QString::number(item.row() + 1);	break;
		case FIND_SIGNAL_COLUMN_TEXT:	result = item.text();						break;
		default:						assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------


FindItem FindSignalTable::at(int index) const
{
	if (index < 0 || index >= count())
	{
		return FindItem();
	}

	return m_findItemList.at(index);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalTable::set(const QVector<FindItem>& list_add)
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

void FindSignalTable::clear()
{
	int count = m_findItemList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_findItemList.clear();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindSignalPanel::FindSignalPanel(QWidget* parent) :
	QDockWidget(parent)
{
	m_pMainWindow = dynamic_cast<QMainWindow*> (parent);
	if (m_pMainWindow == nullptr)
	{
		return;
	}

	setWindowTitle("Search signal panel");
	setObjectName(windowTitle());
	loadSettings();

	createInterface();
	createContextMenu();
	updateColumnsCombo();
}

// -------------------------------------------------------------------------------------------------------------------

FindSignalPanel::~FindSignalPanel()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::createInterface()
{
	m_pFindWindow = new QMainWindow;

	QToolBar *findToolBar = new QToolBar(m_pFindWindow);

	findToolBar->setAllowedAreas(Qt::TopToolBarArea);
	findToolBar->setWindowTitle(tr("Search signal text"));
	findToolBar->setMovable(false);

	m_findColumnCombo = new QComboBox(findToolBar);
	m_findTextEdit = new QLineEdit(m_findText, findToolBar);
	m_findTextEdit->setPlaceholderText(tr("Search Text"));
	m_findTextEdit->setClearButtonEnabled(true);

	findToolBar->addWidget(m_findColumnCombo);
	findToolBar->addWidget(m_findTextEdit);
	QAction* action = findToolBar->addAction(QIcon(":/icons/Search.png"), tr("Find text"));
	connect(action, &QAction::triggered, this, &FindSignalPanel::find);

	m_pFindWindow->addToolBarBreak(Qt::TopToolBarArea);
	m_pFindWindow->addToolBar(findToolBar);
	m_pFindWindow->addToolBarBreak(Qt::TopToolBarArea);

	m_pView = new QTableView(m_pFindWindow);
	m_pView->setModel(&m_table);
	m_pView->verticalHeader()->setDefaultSectionSize(22);

	m_pFindWindow->setCentralWidget(m_pView);

	m_pView->setColumnWidth(FIND_SIGNAL_COLUMN_ROW, FIND_SIGNAL_COLUMN_ROW_WIDTH);

	connect(m_pView, &QTableView::clicked, this, &FindSignalPanel::selectItemInSignalView);
	m_pView->installEventFilter(this);

	m_pView->horizontalHeader()->hide();
	m_pView->verticalHeader()->hide();
	m_pView->setShowGrid(false);
	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

	QStatusBar* statusBar = m_pFindWindow->statusBar();
	m_statusLabel = new QLabel(tr("Found: 0"), statusBar);
	statusBar->addWidget(m_statusLabel);
	statusBar->setLayoutDirection(Qt::RightToLeft);

	setWidget(m_pFindWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::createContextMenu()
{
	if (m_pFindWindow == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Signal text"), m_pFindWindow);

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pContextMenu->addSeparator();

	m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	connect(m_pCopyAction, &QAction::triggered, this, &FindSignalPanel::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &FindSignalPanel::selectAll);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &FindSignalPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::updateColumnsCombo()
{
	if(m_findColumnCombo == nullptr)
	{
		return;
	}

	m_findColumnCombo->addItem(tr("All columns"));

	for (int c = 0; c < SIGNAL_LIST_COLUMN_COUNT; c++)
	{
		m_findColumnCombo->addItem(SignalListColumn[c]);
	}

	connect(m_findColumnCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &FindSignalPanel::find);
}

// -------------------------------------------------------------------------------------------------------------------

bool FindSignalPanel::event(QEvent* e)
{
	if (e->type() == QEvent::Hide)
	{
		saveSettings();
	}

	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

		if (keyEvent->key() == Qt::Key_Return)
		{
			find();
		}
	}

	if (e->type() == QEvent::Resize)
	{
		QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(e);

		if (m_pView != nullptr)
		{
			m_pView->setColumnWidth(FIND_SIGNAL_COLUMN_TEXT, resizeEvent->size().width() - FIND_SIGNAL_COLUMN_ROW_WIDTH - 20);
		}
	}

	return QDockWidget::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

bool FindSignalPanel::eventFilter(QObject* object, QEvent* e)
{
	if (e->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent *>(e);

		if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown)
		{
			selectItemInSignalView();
		}
	}

	return QDockWidget::eventFilter(object, e);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::find()
{
	m_table.clear();

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

	QTableView* pSignalView = pMainWindow->signalView();
	if (pSignalView == nullptr)
	{
		return;
	}

	if(m_findColumnCombo == nullptr)
	{
		return;
	}

	int selectedColumn = m_findColumnCombo->currentIndex();
	if (selectedColumn == -1)
	{
		return;
	}

	QRegExp rx(m_findText);
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	QVector<FindItem> findItemList;

	int rowCount = pSignalView->model()->rowCount();
	int columnCount = pSignalView->model()->columnCount();

	for(int row = 0; row < rowCount; row ++)
	{
		for(int column = 0; column < columnCount; column++)
		{
			if (selectedColumn != FIND_SIGNAL_ALL_COLUMNS)
			{
				if (selectedColumn - 1 != column)
				{
					continue;
				}
			}

			if (pSignalView->isColumnHidden(column) == true)
			{
				continue;
			}

			QString text = pSignalView->model()->data(pSignalView->model()->index(row, column)).toString();
			if (text.isEmpty() == true)
			{
				continue;
			}

			if(rx.exactMatch(text) == false)
			{
				continue;
			}

			findItemList.append(FindItem(row, column, text));
		}
	}

	m_statusLabel->setText(QString("Found: %1").arg(findItemList.count()));

	if (findItemList.count() == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Text \"%1\" was not found!").arg(m_findText));
		return;
	}

	m_table.set(findItemList);

	m_pView->setCurrentIndex(m_pView->model()->index(0, 0));
	m_pView->setFocus();

	selectItemInSignalView();
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::selectItemInSignalView()
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return;
	}

	QTableView* pSignalView = pMainWindow->signalView();
	if (pSignalView == nullptr)
	{
		return;
	}

	int indexFindItem = m_pView->currentIndex().row();
	if (indexFindItem < 0 || indexFindItem >= m_table.count())
	{
		return;
	}

	FindItem fi = m_table.at(indexFindItem);

	int row = fi.row();
	if (row < 0 || row >= pSignalView->model()->rowCount())
	{
		return;
	}

	int column = fi.column();
	if (column < 0 || column > pSignalView->model()->columnCount())
	{
		return;
	}

	QModelIndex selectIndex = pSignalView->model()->index(row, column);
	pSignalView->setCurrentIndex(selectIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::copy()
{
	QString textClipboard;

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::loadSettings()
{
	QSettings s;

	m_findText = s.value(QString("%1/FindText").arg(FIND_SIGNAL_OPTIONS_KEY), QString()).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void FindSignalPanel::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1/FindText").arg(FIND_SIGNAL_OPTIONS_KEY), m_findText);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
