#include "ComparatorList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorListTable::ComparatorListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorListTable::~ComparatorListTable()
{
	m_comparatorMutex.lock();

		m_comparatorList.clear();

	m_comparatorMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorListTable::columnCount(const QModelIndex&) const
{
	return COMPARATOR_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorListTable::rowCount(const QModelIndex&) const
{
	return comparatorCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < COMPARATOR_LIST_COLUMN_COUNT)
		{
			result = ComparatorListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorListTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= comparatorCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > COMPARATOR_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = comparator(row);
	if (comparatorEx == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextColorRole)
	{
		if (comparatorEx->signalsIsValid()  == false)
		{
			return QColor(Qt::red);
		}

		return QVariant();
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorListTable::text(int row, int column, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (row < 0 || row >= comparatorCount())
	{
		return QString();
	}

	if (column < 0 || column > COMPARATOR_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (comparatorEx == nullptr)
	{
		return QString();
	}

	QString inputAppSignalID = comparatorEx->input().appSignalID();

	int prevRow = row - 1;
	if (prevRow >= 0 && prevRow < comparatorCount())
	{
		std::shared_ptr<Metrology::ComparatorEx> prevComparatorEx = comparator(prevRow);
		if (prevComparatorEx != nullptr)
		{

			if (prevComparatorEx->input().appSignalID() == inputAppSignalID)
			{
				inputAppSignalID.clear();
			}
		}
	}

	QString compareValue;

	compareValue += comparatorEx->cmpTypeStr();

	if (comparatorEx->compare().isConst() == true)
	{
		compareValue += QString::number(comparatorEx->compare().constValue(), 'f', comparatorEx->valuePrecision());
	}
	else
	{
		compareValue += comparatorEx->compare().appSignalID();
	}

	QString hysteresisValue;

	if (comparatorEx->hysteresis().isConst() == true)
	{
		hysteresisValue = QString::number(comparatorEx->hysteresis().constValue(), 'f', comparatorEx->valuePrecision());
	}
	else
	{
		hysteresisValue = comparatorEx->hysteresis().appSignalID();
	}

	QString result;

	switch (column)
	{
		case COMPARATOR_LIST_COLUMN_INPUT:				result = inputAppSignalID;						break;
		case COMPARATOR_LIST_COLUMN_VALUE:				result = compareValue;							break;
		case COMPARATOR_LIST_COLUMN_HYSTERESIS:			result = hysteresisValue;						break;
		case COMPARATOR_LIST_COLUMN_OUTPUT:				result = comparatorEx->output().appSignalID();	break;
		case COMPARATOR_LIST_COLUMN_SCHEMA:				result = comparatorEx->schemaID();				break;
		default:										assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorListTable::comparatorCount() const
{
	int count = 0;

	m_comparatorMutex.lock();

		count = m_comparatorList.count();

	m_comparatorMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<Metrology::ComparatorEx> ComparatorListTable::comparator(int index) const
{
	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = nullptr;

	m_comparatorMutex.lock();

		if (index >= 0 && index < m_comparatorList.count())
		{
			 comparatorEx = m_comparatorList[index];
		}

	m_comparatorMutex.unlock();

	return comparatorEx;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListTable::set(const QVector<std::shared_ptr<Metrology::ComparatorEx>>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_comparatorMutex.lock();

			m_comparatorList = list_add;

		m_comparatorMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListTable::clear()
{
	int count = comparatorCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_comparatorMutex.lock();

			m_comparatorList.clear();

		m_comparatorMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

E::SignalInOutType	ComparatorListDialog::m_typeIO		= E::SignalInOutType::Input;
int					ComparatorListDialog::m_currenIndex = 0;

// -------------------------------------------------------------------------------------------------------------------

ComparatorListDialog::ComparatorListDialog(QWidget *parent) :
	QDialog(parent)
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->configSocket() != nullptr)
	{
		connect(pMainWindow->configSocket(), &ConfigSocket::configurationLoaded, this, &ComparatorListDialog::updateList, Qt::QueuedConnection);
	}

	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorListDialog::~ComparatorListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Comparator.png"));
	setWindowTitle(tr("Comparators"));
	resize(QApplication::desktop()->availableGeometry().width() - 850, 500);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pComparatorMenu = new QMenu(tr("&Comparator"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);

	m_pExportAction = m_pComparatorMenu->addAction(tr("&Export ..."));
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

	m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Properties ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pViewTypeIOMenu = new QMenu(tr("Type In/Int"), this);
	m_pTypeInputAction = m_pViewTypeIOMenu->addAction(tr("Input"));
	m_pTypeInputAction->setCheckable(true);
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction = m_pViewTypeIOMenu->addAction(tr("Internal"));
	m_pTypeInternalAction->setCheckable(true);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);

	m_pViewMenu->addMenu(m_pViewTypeIOMenu);

	m_pMenuBar->addMenu(m_pComparatorMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pExportAction, &QAction::triggered, this, &ComparatorListDialog::exportComparator);

	connect(m_pFindAction, &QAction::triggered, this, &ComparatorListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &ComparatorListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &ComparatorListDialog::selectAll);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &ComparatorListDialog::comparatorProperties);

	connect(m_pTypeInputAction, &QAction::triggered, this, &ComparatorListDialog::showTypeInput);
	connect(m_pTypeInternalAction, &QAction::triggered, this, &ComparatorListDialog::showTypeInternal);

	m_pView = new QTableView(this);
	m_pView->setModel(&m_comparatorTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < COMPARATOR_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, ComparatorListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_pView, &QTableView::doubleClicked , this, &ComparatorListDialog::onListDoubleClicked);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);

	setLayout(mainLayout);

	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &ComparatorListDialog::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < COMPARATOR_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(ComparatorListColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &ComparatorListDialog::onColumnAction);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addMenu(m_pViewTypeIOMenu);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSignalPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &ComparatorListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::updateList()
{
	updateVisibleColunm();

	m_comparatorTable.clear();

	QVector<std::shared_ptr<Metrology::ComparatorEx>> comparatorList;

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.isAnalog() == false)
		{
			continue;
		}

		if (param.inOutType() != m_typeIO)
		{
			continue;
		}

		int comparatorCount = pSignal->param().comparatorCount();
		for (int c = 0; c < comparatorCount; c++)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx = pSignal->param().comparator(c);
			if (comparatorEx == nullptr)
			{
				continue;
			}

			comparatorList.append(pSignal->param().comparator(c));
		}
	}

	m_comparatorTable.set(comparatorList);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::updateVisibleColunm()
{
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);

	for(int c = 0; c < COMPARATOR_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(COMPARATOR_LIST_COLUMN_SCHEMA, true);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= COMPARATOR_LIST_COLUMN_COUNT)
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

bool ComparatorListDialog::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
		{
			comparatorProperties();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::exportComparator()
{
	ExportData* dialog = new ExportData(m_pView, tr("Comparators"));
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::copy()
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

void ComparatorListDialog::comparatorProperties()
{
	QModelIndex visibleIndex = m_pView->currentIndex();

	int index = visibleIndex .row();
	if (index < 0 || index >= m_comparatorTable.comparatorCount())
	{
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_comparatorTable.comparator(index);
	if (comparatorEx == nullptr)
	{
		return;
	}

	ComparatorPropertyDialog dialog(*comparatorEx);
	int result = dialog.exec();
	if (result != QDialog::Accepted)
	{
		return;
	}

	*comparatorEx = dialog.comparator();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::showTypeInput()
{
	m_typeIO = E::SignalInOutType::Input;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::showTypeInternal()
{
	m_typeIO = E::SignalInOutType::Internal;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < COMPARATOR_LIST_COLUMN_COUNT; column++)
	{
		if (m_pColumnAction[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::onListDoubleClicked(const QModelIndex&)
{
	comparatorProperties();
}

// -------------------------------------------------------------------------------------------------------------------
