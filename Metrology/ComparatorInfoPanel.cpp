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
	connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &ComparatorInfoTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoTable::~ComparatorInfoTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorInfoTable::columnCount(const QModelIndex&) const
{
	return theOptions.module().maxComparatorCount();
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
		if (section >= 0 && section < theOptions.module().maxComparatorCount())
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
	if (column < 0 || column >= theOptions.module().maxComparatorCount())
	{
		return QVariant();
	}

	Metrology::SignalParam inParam = signalParam(row).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		return QVariant();
	}

	if (column >= inParam.comparatorCount())
	{
		return QVariant();
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = inParam.comparator(column);
	if (comparatorEx == nullptr)
	{
		return QVariant();
	}

	if (comparatorEx->signalsIsValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::FontRole)
	{
		return theOptions.comparatorInfo().font();
	}

	if (role == Qt::BackgroundRole)
	{
		if (comparatorEx->outputState() == true)
		{
			return theOptions.comparatorInfo().colorStateTrue();
		}
		else
		{
			return theOptions.comparatorInfo().colorStateFalse();
		}
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorInfoTable::text(std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (comparatorEx == nullptr)
	{
		return QString();
	}

	if (comparatorEx->signalsIsValid() == false)
	{
		return QString();
	}

	QString stateStr;

	stateStr += comparatorEx->cmpTypeStr();
	stateStr += " ";
	stateStr += comparatorEx->compareOnlineValueStr();
	stateStr += " : ";
	stateStr += comparatorEx->outputStateStr(theOptions.comparatorInfo().displayingStateTrue(), theOptions.comparatorInfo().displayingStateFalse());

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::updateState()
{
	emit dataChanged(index(0, 0), index(m_signalCount - 1, theOptions.module().maxComparatorCount() - 1), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam ComparatorInfoTable::signalParam(int index) const
{
	if (index < 0 || index >= m_signalCount)
	{
		return IoSignalParam();
	}

	IoSignalParam param;

	m_signalMutex.lock();

		param = m_signalList[index];

	m_signalMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::set(const QVector<IoSignalParam>& signalList)
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

void ComparatorInfoTable::updateSignalParam(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	m_signalMutex.lock();

		int signalCount = m_signalList.count();
		for(int c = 0; c < signalCount; c ++)
		{
			for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
			{
				if (m_signalList[c].param(type).appSignalID() == appSignalID)
				{
					m_signalList[c].setParam(type, theSignalBase.signalParam(appSignalID));
				}
			}
		}

	m_signalMutex.unlock();
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

	for(int column = 0; column < theOptions.module().maxComparatorCount(); column++)
	{
		m_pView->setColumnWidth(column, COMPARATOR_INFO_COLUMN_WIDTH);
	}

	m_pView->setWordWrap(false);

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
	if (column < 0 || column >= theOptions.module().maxComparatorCount())
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

	QVector<IoSignalParam> ioParamList;

	for(int c = 0; c < signalCount; c ++)
	{
		IoSignalParam ioParam;

		Metrology::Signal* pSignal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).metrologySignal(c);
		if (pSignal != nullptr && pSignal->param().isValid() == true)
		{
			if (maxComparatorCount < pSignal->param().comparatorCount())
			{
				maxComparatorCount = pSignal->param().comparatorCount();
			}

			ioParam.setParam(MEASURE_IO_SIGNAL_TYPE_INPUT, pSignal->param());
			ioParam.setSignalConnectionType(activeSignal.signalConnectionType());
			ioParam.setCalibratorManager(theCalibratorBase.calibratorForMeasure(c));
		}

		ioParamList.append(ioParam);
	}

	m_comparatorTable.set(ioParamList);

	for(int c = 0; c < theOptions.module().maxComparatorCount(); c ++)
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

	if (activeSignal.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
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

	Metrology::SignalParam inParam = m_comparatorTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (inParam.isValid() == false)
	{
		return;
	}

	int indexComparator = m_pView->currentIndex().column();
	if (indexComparator < 0 || indexComparator >= inParam.comparatorCount())
	{
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = inParam.comparator(indexComparator);
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
