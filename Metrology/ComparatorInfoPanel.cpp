#include "ComparatorInfoPanel.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "ProcessData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoTable::ComparatorInfoTable(QObject*)
{
	connect(&theSignalBase, &SignalBase::signalParamChanged, this, &ComparatorInfoTable::signalParamChanged, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoTable::~ComparatorInfoTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::setComparatorInfo(const ComparatorInfoOption& comparatorInfo)
{
	m_comparatorInfo = comparatorInfo;
}

// -------------------------------------------------------------------------------------------------------------------

int ComparatorInfoTable::columnCount(const QModelIndex&) const
{
	return Metrology::ComparatorCount;
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
		if (section >= 0 && section < Metrology::ComparatorCount)
		{
			result = tr("Comparator %1").arg(section + 1);
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
	if (column < 0 || column >= Metrology::ComparatorCount)
	{
		return QVariant();
	}

	const Metrology::SignalParam& inParam = signalParam(row).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
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
		return m_comparatorInfo.font();
	}

	if (role == Qt::BackgroundRole)
	{
		if (comparatorEx->outputState() == true)
		{
			return m_comparatorInfo.colorStateTrue();
		}
		else
		{
			return m_comparatorInfo.colorStateFalse();
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
	stateStr += comparatorEx->compareOnlineValueStr(Metrology::CmpValueTypeSetPoint);
	stateStr += " : ";
	stateStr += comparatorEx->outputStateStr(m_comparatorInfo.displayingStateTrue(), m_comparatorInfo.displayingStateFalse());

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::updateState()
{
	emit dataChanged(index(0, 0), index(m_signalCount - 1, Metrology::ComparatorCount - 1), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam ComparatorInfoTable::signalParam(int index) const
{
	if (index < 0 || index >= m_signalCount)
	{
		return IoSignalParam();
	}

	QMutexLocker l(&m_signalMutex);

	return m_signalList[index];
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

void ComparatorInfoTable::signalParamChanged(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QMutexLocker l(&m_signalMutex);

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
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorInfoPanel::ComparatorInfoPanel(const ComparatorInfoOption& comparatorInfo, QWidget* parent) :
	QDockWidget(parent),
	m_comparatorInfo(comparatorInfo)
{
	setWindowTitle(tr("Panel comparator information"));
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &ComparatorInfoPanel::activeSignalChanged, Qt::QueuedConnection);

	startComparatorStateTimer(m_comparatorInfo.timeForUpdate());
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

	m_comparatorTable.setComparatorInfo(m_comparatorInfo);

	m_pView = new QTableView(m_pComparatorInfoWindow);
	m_pView->setModel(&m_comparatorTable);
	QSize cellSize = QFontMetrics(m_comparatorInfo.font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pComparatorInfoWindow->setCentralWidget(m_pView);

	for(int column = 0; column < Metrology::ComparatorCount; column++)
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

//	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
//	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

//	m_pContextMenu->addSeparator();

	m_pComparatorPropertyAction = m_pContextMenu->addAction(tr("Propertу ..."));
	m_pComparatorPropertyAction->setIcon(QIcon(":/icons/Property.png"));

//	connect(m_pCopyAction, &QAction::triggered, this, &ComparatorInfoPanel::copy);
	connect(m_pComparatorPropertyAction, &QAction::triggered, this, &ComparatorInfoPanel::comparatorProperty);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &ComparatorInfoPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= Metrology::ComparatorCount)
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

void ComparatorInfoPanel::startComparatorStateTimer(int timeout)
{
	if (m_updateComparatorStateTimer == nullptr)
	{
		m_updateComparatorStateTimer = new QTimer(this);
		connect(m_updateComparatorStateTimer, &QTimer::timeout, this, &ComparatorInfoPanel::updateComparatorState);
	}

	m_updateComparatorStateTimer->start(timeout);
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

void ComparatorInfoPanel::restartComparatorStateTimer(int timeout)
{
	if (m_updateComparatorStateTimer != nullptr)
	{
		if(m_updateComparatorStateTimer->interval() == timeout)
		{
			return;
		}
	}

	stopComparatorStateTimer();
	startComparatorStateTimer(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::measureKindChanged(int kind)
{
	if (kind < 0 || kind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	m_measureKind = kind;
}


// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::signalConnectionTypeChanged(int type)
{
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	m_signalConnectionType = type;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::setComparatorInfo(const ComparatorInfoOption& comparatorInfo)
{
	m_comparatorInfo = comparatorInfo;
	m_comparatorTable.setComparatorInfo(m_comparatorInfo);
	restartComparatorStateTimer(m_comparatorInfo.timeForUpdate());
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

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
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

	if (m_pCalibratorBase == nullptr)
	{
		return;
	}

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

		Metrology::Signal* pSignal = nullptr;

		switch (activeSignal.signalConnectionType())
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:
				pSignal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).metrologySignal(c);
				break;
			default:
				pSignal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).metrologySignal(c);
				break;
		}

		if (pSignal != nullptr && pSignal->param().isValid() == true)
		{
			if (maxComparatorCount < pSignal->param().comparatorCount())
			{
				maxComparatorCount = pSignal->param().comparatorCount();
			}

			ioParam.setParam(MEASURE_IO_SIGNAL_TYPE_INPUT, pSignal->param());
			ioParam.setSignalConnectionType(activeSignal.signalConnectionType());
			ioParam.setCalibratorManager(m_pCalibratorBase->calibratorForMeasure(c));
		}

		ioParamList.append(ioParam);
	}

	m_comparatorTable.set(ioParamList);

	for(int c = 0; c < Metrology::ComparatorCount; c ++)
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
	QSize cellSize = QFontMetrics(m_comparatorInfo.font()).size(Qt::TextSingleLine,"A");

	if (activeSignal.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(static_cast<int>(cellSize.height() * 2.1));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::updateComparatorState()
{
	m_comparatorTable.updateState();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoPanel::comparatorProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_comparatorTable.signalCount())
	{
		return;
	}

	const Metrology::SignalParam& inParam = m_comparatorTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
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
