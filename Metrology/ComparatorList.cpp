#include "ComparatorList.h"

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorListTable::ComparatorListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorListTable::~ComparatorListTable()
{
	QMutexLocker l(&m_comparatorMutex);

	m_comparatorList.clear();
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
			result = qApp->translate("ComparatorList.h", ComparatorListColumn[section]);
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

	Metrology::Signal* pInSignal = theSignalBase.signalPtr(comparatorEx->input().appSignalID());
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::ForegroundRole)
	{
		if (comparatorEx->signalsIsValid()  == false)
		{
			return QColor(Qt::red);
		}
		else
		{
			if (column == COMPARATOR_LIST_COLUMN_HYSTERESIS)
			{
				if (comparatorEx->deviation() != Metrology::ComparatorEx::DeviationType::Unused)
				{
					return QColor(Qt::lightGray);
				}
			}
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (pInSignal->param().isInput() == true)
		{
			if (column == COMPARATOR_LIST_COLUMN_EL_RANGE)
			{
				if (pInSignal->param().electricRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
			}

			if (column == COMPARATOR_LIST_COLUMN_EL_SENSOR)
			{
				if (pInSignal->param().electricSensorType() == E::SensorType::NoSensor)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
			}
		}

		return QVariant();
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pInSignal, comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorListTable::text(int row, int column, Metrology::Signal* pInSignal, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (row < 0 || row >= comparatorCount())
	{
		return QString();
	}

	if (column < 0 || column > COMPARATOR_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pInSignal == nullptr)
	{
		return QString();
	}

	const Metrology::SignalParam& param = pInSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	if (comparatorEx == nullptr)
	{
		return QString();
	}

	bool visible = true;

	if (row > 0)
	{
		std::shared_ptr<Metrology::ComparatorEx> prevComparatorEx = comparator(row - 1);
		if (prevComparatorEx != nullptr)
		{
			if (prevComparatorEx->input().appSignalID() == param.appSignalID())
			{
				visible = false;
			}
		}
	}

	//
	//
	QString strCompareValue;

	if (comparatorEx->compare().isConst() == true)
	{
		strCompareValue = comparatorEx->compareDefaultValueStr() + " " + pInSignal->param().unit();

		if (pInSignal->param().electricRangeIsValid() == true)
		{
			double electric = conversion(comparatorEx->compareConstValue(), CT_ENGINEER_TO_ELECTRIC, pInSignal->param());

			strCompareValue += "  [" + QString::number(electric, 'f', pInSignal->param().electricPrecision()) + " " + pInSignal->param().electricUnitStr() + "]";
		}
	}
	else
	{
		strCompareValue = comparatorEx->compareDefaultValueStr();
	}

	//
	//
	QString strHysteresisValue;

	if (comparatorEx->deviation() == Metrology::ComparatorEx::DeviationType::Unused)
	{
		switch (comparatorEx->cmpType())
		{
			case E::CmpType::Less:		strHysteresisValue = "+ " + comparatorEx->hysteresisDefaultValueStr(); break;
			case E::CmpType::Greate:	strHysteresisValue = "- " + comparatorEx->hysteresisDefaultValueStr(); break;
		}

		if (comparatorEx->hysteresis().isConst() == true)
		{
			strHysteresisValue += " " + pInSignal->param().unit();
		}
	}
	else
	{
		strHysteresisValue = comparatorEx->hysteresisDefaultValueStr();
	}

	//
	//
	QString result;

	switch (column)
	{
		case COMPARATOR_LIST_COLUMN_INPUT:				result = visible ? param.appSignalID() : QString();												break;
		case COMPARATOR_LIST_COLUMN_SETPOINT:			result = strCompareValue;																		break;
		case COMPARATOR_LIST_COLUMN_HYSTERESIS:			result = qApp->translate("MetrologySignal.cpp", strHysteresisValue.toUtf8());					break;
		case COMPARATOR_LIST_COLUMN_TYPE:				result = param.signalTypeStr();																	break;
		case COMPARATOR_LIST_COLUMN_EL_RANGE:			result = param.electricRangeStr();																break;
		case COMPARATOR_LIST_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();															break;
		case COMPARATOR_LIST_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();															break;
		case COMPARATOR_LIST_COLUMN_OUTPUT:				result = comparatorEx->output().appSignalID();													break;
		case COMPARATOR_LIST_COLUMN_SCHEMA:				result = comparatorEx->schemaID();																break;
		default:										assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorListTable::comparatorCount() const
{
	QMutexLocker l(&m_comparatorMutex);

	return m_comparatorList.count();
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<Metrology::ComparatorEx> ComparatorListTable::comparator(int index) const
{
	QMutexLocker l(&m_comparatorMutex);

	if (index < 0 || index >= m_comparatorList.count())
	{
		return nullptr;
	}

	return m_comparatorList[index];
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

int ComparatorListDialog::m_currenIndex = 0;

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
	resize(QGuiApplication::primaryScreen()->availableGeometry().width() - 750, 500);
	move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pComparatorMenu = new QMenu(tr("&Comparator"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

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

	m_pEditMenu->addSeparator();

	m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Propertу ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pMenuBar->addMenu(m_pComparatorMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pExportAction, &QAction::triggered, this, &ComparatorListDialog::exportComparator);

	connect(m_pFindAction, &QAction::triggered, this, &ComparatorListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &ComparatorListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &ComparatorListDialog::selectAll);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &ComparatorListDialog::comparatorProperties);

	m_pView = new QTableView(this);
	m_pView->setModel(&m_comparatorTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < COMPARATOR_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, ComparatorListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

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
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("ComparatorList.h", ComparatorListColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &ComparatorListDialog::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

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
	for(int c = 0; c < COMPARATOR_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(COMPARATOR_LIST_COLUMN_EL_SENSOR, true);
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

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			comparatorProperties();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorListDialog::exportComparator()
{
	ExportData* dialog = new ExportData(m_pView, false, "Comparators");
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
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	*comparatorEx = dialog.comparator();
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
