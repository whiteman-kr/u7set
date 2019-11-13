#include "ComparatorInfoPanel.h"

#include <QApplication>
#include <QMainWindow>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QClipboard>

#include "Options.h"
#include "ObjectProperties.h"
#include "Conversion.h"
#include "CalibratorBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoTable::ComparatorInfoTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoTable::~ComparatorInfoTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorInfoTable::columnCount(const QModelIndex&) const
{
	return COMPARATOR_INFO_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorInfoTable::rowCount(const QModelIndex&) const
{
	return m_signalCount;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorInfoTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < COMPARATOR_INFO_COLUMN_COUNT)
		{
			result = QString("Comparator %1").arg(section + 1);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorInfoTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= m_signalCount)
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column >= COMPARATOR_INFO_COLUMN_COUNT)
	{
		return QVariant();
	}

	Metrology::Signal* pInputSignal = signal(row);
	if (pInputSignal == nullptr || pInputSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (column >= pInputSignal->comparatorCount())
	{
		return QVariant();
	}

	std::shared_ptr<::Builder::Comparator> pComparator = pInputSignal->comparator(column);
	if (pComparator == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignLeft;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.comparatorInfo().font();
	}

	if (role == Qt::BackgroundColorRole)
	{
		Metrology::SignalState state = theSignalBase.signalState(pComparator->output().appSignalID());
		if (state.value() != 0.0)
		{
			return theOptions.comparatorInfo().colorStateTrue();
		}
		else
		{
			return theOptions.comparatorInfo().colorStateFalse();
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(pComparator);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorInfoTable::text(std::shared_ptr<::Builder::Comparator> pComparator) const
{
	if (pComparator == nullptr)
	{
		return QString();
	}

	QString stateStr;

	switch (pComparator->cmpType())
	{
		case ::Builder::Comparator::CmpType::Equ:		stateStr = "= ";		break;
		case ::Builder::Comparator::CmpType::Greate:	stateStr = "> ";		break;
		case ::Builder::Comparator::CmpType::Less:		stateStr = "< ";		break;
		case ::Builder::Comparator::CmpType::NotEqu:	stateStr = "≠ ";		break;
	}

	int precision = pComparator->precision();
	switch (pComparator->intAnalogSignalFormat())
	{
		case E::AnalogAppSignalFormat::Float32:		precision = pComparator->precision();	break;
		case E::AnalogAppSignalFormat::SignedInt32:	precision = 0;							break;
		default:									assert(0);
	}

	if (pComparator->compare().isConst() == true)
	{
		stateStr += QString::number(pComparator->compare().constValue(), 10, precision);
	}
	else
	{
		if (pComparator->compare().appSignalID().isEmpty() == true)
		{
			return QString();
		}

		Metrology::Signal* pCompareSignal = theSignalBase.signalPtr(calcHash(pComparator->compare().appSignalID()));
		if(pCompareSignal == nullptr || pCompareSignal->param().isValid() == false)
		{
			return QString();
		}

		Metrology::SignalState compareState = theSignalBase.signalState(pCompareSignal->param().hash());
		stateStr += QString::number(compareState.value(), 10, precision);
	}

	stateStr += " : ";

	Metrology::SignalState outputState = theSignalBase.signalState(pComparator->output().appSignalID());
	if (outputState.value() == 0.0)
	{
		stateStr += theOptions.comparatorInfo().displayingStateFalse();
	}
	else
	{
		stateStr += theOptions.comparatorInfo().displayingStateTrue();
	}

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::updateState()
{
	emit dataChanged(index(0, 0), index(m_signalCount - 1, COMPARATOR_INFO_COLUMN_COUNT - 1), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* ComparatorInfoTable::signal(int index) const
{
	if (index < 0 || index >= m_signalCount)
	{
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		pSignal = m_signalList[index];

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::set(const QVector<Metrology::Signal*>& signalList)
{
	int signalCount = signalList.count();
	if (signalCount == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, signalCount - 1);

		m_signalMutex.lock();

			m_signalList = signalList;
			m_signalCount = signalCount;

		m_signalMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::clear()
{
	if (m_signalCount == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, m_signalCount - 1);

		m_signalMutex.lock();

			m_signalCount = 0;
			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoPanel::ComparatorInfoPanel(QWidget* parent) :
	QDockWidget(parent)
{
	setWindowTitle(tr("Panel comparator information"));
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &ComparatorInfoPanel::activeSignalChanged, Qt::QueuedConnection);

	startComparatorStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoPanel::~ComparatorInfoPanel()
{
	stopComparatorStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::createInterface()
{
	m_pComparatorInfoWindow = new QMainWindow;

	m_pComparatorInfoWindow->installEventFilter(this);

	m_pView = new QTableView(m_pComparatorInfoWindow);
	m_pView->setModel(&m_comparatorTable);
	QSize cellSize = QFontMetrics(theOptions.comparatorInfo().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pComparatorInfoWindow->setCentralWidget(m_pView);

	for(int column = 0; column < COMPARATOR_INFO_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, COMPARATOR_INFO_COLUMN_WIDTH);
	}

	//m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(m_pView, &QTableView::doubleClicked , this, &ComparatorInfoPanel::onListDoubleClicked);

	setWidget(m_pComparatorInfoWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pComparatorInfoWindow);

	m_pComparatorPropertyAction = m_pContextMenu->addAction(tr("Properties ..."));
	m_pComparatorPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	connect(m_pComparatorPropertyAction, &QAction::triggered, this, &ComparatorInfoPanel::comparatorProperty);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &ComparatorInfoPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= COMPARATOR_INFO_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pView->hideColumn(column);
	}
	else
	{
		m_pView->showColumn(column);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::startComparatorStateTimer()
{
	if (m_updateComparatorStateTimer == nullptr)
	{
		m_updateComparatorStateTimer = new QTimer(this);
		connect(m_updateComparatorStateTimer, &QTimer::timeout, this, &ComparatorInfoPanel::updateComparatorState);
	}

	m_updateComparatorStateTimer->start(theOptions.comparatorInfo().timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::stopComparatorStateTimer()
{
	if (m_updateComparatorStateTimer != nullptr)
	{
		m_updateComparatorStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::restartComparatorStateTimer()
{
	stopComparatorStateTimer();
	startComparatorStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool ComparatorInfoPanel::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
		{
			comparatorProperty();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::activeSignalChanged(const MeasureSignal& activeSignal)
{
	clear();

	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	int signalCount = activeSignal.channelCount();
	if (signalCount == 0)
	{
		return;
	}

	int maxComparatorCount = 0;

	QVector<Metrology::Signal*> signalList;

	for(int c = 0; c < signalCount; c ++)
	{
		Metrology::Signal* pSignal = activeSignal.multiSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).metrologySignal(c);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->comparatorCount() > maxComparatorCount)
		{
			maxComparatorCount = pSignal->comparatorCount();
		}

		signalList.append(pSignal);
	}

	m_comparatorTable.set(signalList);

	for(int c = 0; c < COMPARATOR_INFO_COLUMN_COUNT; c ++)
	{
		if (c < maxComparatorCount)
		{
			hideColumn(c, false);
		}
		else
		{
			hideColumn(c, true);
		}
	}

	//
	//
	QSize cellSize = QFontMetrics(theOptions.comparatorInfo().font()).size(Qt::TextSingleLine,"A");

	if (activeSignal.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height() * 2);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::updateComparatorState()
{
	m_comparatorTable.updateState();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::comparatorProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_comparatorTable.signalCount())
	{
		return;
	}

//	Metrology::SignalParam param;

//	if (theOptions.toolBar().outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
//	{
//		param = m_comparatorParamTable.signal(index).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
//	}
//	else
//	{
//		param = m_comparatorParamTable.signal(index).param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
//	}

//	if (param.isValid() == false)
//	{
//		return;
//	}

//	SignalPropertyDialog dialog(param);
//	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
