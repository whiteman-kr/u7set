#include "PanelFindMeasure.h"

#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QTableWidget>
#include <QMessageBox>
#include <QCompleter>

#include "ProcessData.h"
#include "Delegate.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindItem::FindItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

FindItem::FindItem(int row, int column, const QString& text, int beginPos, int endPos) :
	m_row (row) ,
	m_column (column) ,
	m_text (text) ,
	m_beginPos (beginPos) ,
	m_endPos (endPos)
{
}

// -------------------------------------------------------------------------------------------------------------------

FindItem& FindItem::operator=(const FindItem& from)
{
	m_row = from.m_row;
	m_column = from.m_column;

	m_text = from.m_text;

	m_beginPos = from.m_beginPos;
	m_endPos = from.m_endPos;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant FindMeasureTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == FIND_MEASURE_COLUMN_ROW)
		{
			return QColor(Qt::lightGray);
		}

		return QVariant();
	}

	if (role == Qt::UserRole)
	{
		QVariant var;
		var.setValue(at(row));
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
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	FindItem item = at(row);

	QString result;

	switch (column)
	{
		case FIND_MEASURE_COLUMN_ROW:	result = QString::number(item.row() + 1);	break;
		case FIND_MEASURE_COLUMN_TEXT:	result = item.text();						break;
		default:						assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelFindMeasure::PanelFindMeasure(QWidget* parent) :
	QDockWidget(parent)
{
	setWindowTitle(tr("Search measurements panel"));
	setObjectName(windowTitle());
	loadSettings();

	createInterface();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

PanelFindMeasure::~PanelFindMeasure()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::clear()
{
	m_table.clear();
	m_statusLabel->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::createInterface()
{
	m_pFindWindow = new QMainWindow;

	QToolBar* toolBar = new QToolBar(m_pFindWindow);

	m_findCompleter.create(this);
	m_findCompleter.setFilterCount(20);

	m_findTextEdit = new QLineEdit(m_findText, toolBar);
	m_findTextEdit->setPlaceholderText(tr("Search Text"));
	m_findTextEdit->setCompleter(m_findCompleter.completer());
	//m_findTextEdit->setClearButtonEnabled(true);

	toolBar->addWidget(m_findTextEdit);
	QAction* action = toolBar->addAction(QIcon(":/icons/Search.png"), tr("Find text"));
	connect(action, &QAction::triggered, this, &PanelFindMeasure::find);

	toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	toolBar->setWindowTitle(tr("Search measurements ToolBar"));
	m_pFindWindow->addToolBarBreak(Qt::TopToolBarArea);
	m_pFindWindow->addToolBar(toolBar);

	m_table.setColumnCaption(metaObject()->className(), FIND_MEASURE_COLUMN_COUNT, FindMeasureColumn);

	m_pView = new QTableView(m_pFindWindow);
	m_pView->setModel(&m_table);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pFindWindow->setCentralWidget(m_pView);

	m_pView->setColumnWidth(FIND_MEASURE_COLUMN_ROW, FIND_MEASURE_COLUMN_ROW_WIDTH);

	connect(m_pView, &QTableView::clicked, this, &PanelFindMeasure::selectItemInMeasureView);
	m_pView->installEventFilter(this);

	FindTextDelegate* textDelegate = new FindTextDelegate(this);
	m_pView->setItemDelegateForColumn(FIND_MEASURE_COLUMN_TEXT, textDelegate);

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

void PanelFindMeasure::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr("&Measurements"), m_pFindWindow);

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pContextMenu->addSeparator();

	m_pSelectAllAction = m_pContextMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

	connect(m_pCopyAction, &QAction::triggered, this, &PanelFindMeasure::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &PanelFindMeasure::selectAll);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &PanelFindMeasure::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

bool PanelFindMeasure::event(QEvent* e)
{
	if (e->type() == QEvent::Hide)
	{
		saveSettings();
	}

	if (e->type() == QEvent::Show)
	{
		find();
	}

	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			find();
		}
	}

	if (e->type() == QEvent::Resize)
	{
		QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(e);

		m_pView->setColumnWidth(FIND_MEASURE_COLUMN_TEXT, resizeEvent->size().width() - FIND_MEASURE_COLUMN_ROW_WIDTH - 20);

	}

	return QDockWidget::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

bool PanelFindMeasure::eventFilter(QObject* object, QEvent* e)
{
	if (e->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent* >(e);

		if (	ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down ||
				ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown)
		{
			selectItemInMeasureView();
		}
	}

	return QDockWidget::eventFilter(object, e);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::setViewFont(const QFont& font)
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->setFont(font);
	QSize cellSize = QFontMetrics(font).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::measureViewChanged(Measure::View* pView)
{
	if (pView == nullptr)
	{
		return;
	}

	clear();

	m_pMeasureView = pView;

	if (isVisible() == false)
	{
		return;
	}

	find();
}


// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::setFindText(const QString& findText)
{
	m_findTextEdit->setText(findText);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::find()
{
	m_findText = m_findTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		return;
	}

	if (m_pMeasureView == nullptr)
	{
		return;
	}

	m_findCompleter.appendFilter(m_findText);

	saveSettings();

	QVector<FindItem> findItemList;

	m_table.clear();

	int rowCount = m_pMeasureView->table().count();
	int columnCount = m_pMeasureView->table().header().count();

	for(int row = 0; row < rowCount; row ++)
	{
		for(int column = 0; column < columnCount; column++)
		{
			if (m_pMeasureView->table().columnIsVisible(column) == false)
			{
				continue;
			}

			Measure::Item* pMeasurement = m_pMeasureView->table().at(row);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			QString text = m_pMeasureView->table().text(row, column, pMeasurement);
			if (text.isEmpty() == true)
			{
				continue;
			}

			int pos = text.indexOf(m_findText, 0, Qt::CaseInsensitive);
			if (pos == -1)
			{
				continue;
			}

			findItemList.append(FindItem(row, column, text, pos, pos + m_findText.count()));
		}
	}

	m_statusLabel->setText(QString("Found: %1").arg(findItemList.count()));

	if (findItemList.count() == 0)
	{
		//QMessageBox::information(this, windowTitle(), tr("Text \"%1\" was not found!").arg(m_findText));
		return;
	}

	m_table.set(findItemList);

	m_pView->setCurrentIndex(m_pView->model()->index(0, 0));
	m_pView->setFocus();

	selectItemInMeasureView();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::selectItemInMeasureView()
{
	if (m_pMeasureView == nullptr)
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
	if (row < 0 || row >= m_pMeasureView->table().count())
	{
		return;
	}

	int column = fi.column();
	if (column < 0 || column > m_pMeasureView->table().header().count())
	{
		return;
	}

	QModelIndex selectIndex = m_pMeasureView->model()->index(row, column);
	m_pMeasureView->setCurrentIndex(selectIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::loadSettings()
{
	QSettings s;

	m_findText = s.value(QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), QString()).toString();

	m_findCompleter.load(FIND_MEASURE_OPTIONS_KEY);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelFindMeasure::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1/FindText").arg(FIND_MEASURE_OPTIONS_KEY), m_findText);

	m_findCompleter.save(FIND_MEASURE_OPTIONS_KEY);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
