#include "DialogList.h"

#include "ProcessData.h"
#include "DialogObjectProperties.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogList::DialogList(double width, double height, bool hasButtons, QWidget* parent) :
	QDialog(parent)
{
	createInterface(width, height, hasButtons);
}

// -------------------------------------------------------------------------------------------------------------------

DialogList::~DialogList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::createInterface(double width, double height, bool hasButtons)
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Signal.png"));
	setWindowTitle(tr("List"));

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * width), static_cast<int>(screen.height() * height));
	move(screen.center() - rect().center());

	installEventFilter(this);

	//
	//
	m_pMenuBar = new QMenuBar(this);

	//
	//
	m_pExportAction = new QAction(tr("&Export ..."), this);
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pFindAction = new QAction(tr("&Find ..."), this);
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
	m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

	m_pCopyAction = new QAction(tr("&Copy"), this);
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = new QAction(tr("Select &All"), this);
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pPropertyAction = new QAction(tr("Propertу ..."), this);
	m_pPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	//
	//
	connect(m_pExportAction, &QAction::triggered, this, &DialogList::onExport);
	connect(m_pFindAction, &QAction::triggered, this, &DialogList::onFind);
	connect(m_pCopyAction, &QAction::triggered, this, &DialogList::onCopy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &DialogList::onSelectAll);
	connect(m_pPropertyAction, &QAction::triggered, this, &DialogList::onProperties);

	//
	//
	m_pView = new QTableView(this);

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &DialogList::onListDoubleClicked);

	//
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);

	if (hasButtons == true)
	{
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

		connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogList::onOk);
		connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogList::reject);

		mainLayout->addWidget(m_buttonBox);
	}

	setLayout(mainLayout);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::setModel(QAbstractItemModel *model)
{
	if (m_pView == nullptr)
	{
		return;
	}

	if (model == nullptr)
	{
		return;
	}

	m_pView->setModel(model);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::createHeaderContexMenu(int columnCount, const char* const* columnCaption, const int* columnWidth)
{
	if (m_pView == nullptr)
	{
		return;
	}

	if (columnCount == 0)
	{
		return;
	}

	if (columnCaption == nullptr)
	{
		return;
	}

	if (columnWidth == nullptr)
	{
		return;
	}

	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &DialogList::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	for(int column = 0; column < columnCount; column++)
	{
		m_pView->setColumnWidth(column, columnWidth[column]);

		QAction* pAction = m_headerContextMenu->addAction(qApp->translate(metaObject()->className(), columnCaption[column]));
		if (pAction == nullptr)
		{
			continue;
		}

		pAction->setCheckable(true);
		pAction->setChecked(true);

		m_pColumnActionList.append(pAction);
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),	this, &DialogList::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::createContextMenu()
{
	if (m_pView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &DialogList::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::addMenu(QMenu *menu)
{
	if (m_pMenuBar == nullptr)
	{
		return;
	}

	if (menu == nullptr)
	{
		return;
	}

	m_pMenuBar->addMenu(menu);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::addContextMenu(QMenu *menu)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	if (menu == nullptr)
	{
		return;
	}

	m_pContextMenu->addMenu(menu);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::addContextSeparator()
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->addSeparator();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::addContextAction(QAction *action)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	if (action == nullptr)
	{
		return;
	}

	m_pContextMenu->addAction(action);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::updateList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::updateVisibleColunm()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::hideColumn(int column, bool hide)
{
	int columnCount = m_pColumnActionList.count();
	if (column < 0 || column >= columnCount)
	{
		return;
	}

	if (hide == true)
	{
		m_pView->hideColumn(column);
		m_pColumnActionList[column]->setChecked(false);
	}
	else
	{
		m_pView->showColumn(column);
		m_pColumnActionList[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogList::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent* >(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			onProperties();
			return true;
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onExport()
{
	ExportData* dialog = new ExportData(m_pView, false, windowTitle().remove(' '));
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onFind()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onCopy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onProperties()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	int columnCount = m_pColumnActionList.count();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pColumnActionList[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onListDoubleClicked(const QModelIndex&)
{
	onProperties();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogList::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
