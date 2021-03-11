#include "PanelComparatorInfo.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "ProcessData.h"
#include "DialogObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant ComparatorInfoTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column >= Metrology::ComparatorCount)
	{
		return QVariant();
	}

	const Metrology::SignalParam& inParam = at(row).param(Metrology::ConnectionIoType::Source);
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
	stateStr += comparatorEx->compareOnlineValueStr(Metrology::CmpValueType::SetPoint);
	stateStr += " : ";
	stateStr += comparatorEx->outputStateStr(m_comparatorInfo.displayingStateTrue(), m_comparatorInfo.displayingStateFalse());

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::updateState()
{
	emit dataChanged(index(0, 0), index(count() - 1, Metrology::ComparatorCount - 1), QVector<int>() << Qt::DisplayRole);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorInfoTable::signalParamChanged(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QMutexLocker l(&m_mutex);

	int signalCount = m_list.count();
	for(int c = 0; c < signalCount; c ++)
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			if (m_list[c].param(ioType).appSignalID() == appSignalID)
			{
				m_list[c].setParam(ioType, theSignalBase.signalParam(appSignalID));
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelComparatorInfo::PanelComparatorInfo(const ComparatorInfoOption& comparatorInfo, QWidget* parent) :
	QDockWidget(parent),
	m_comparatorInfo(comparatorInfo)
{
	setWindowTitle(tr("Panel comparator information"));
	setObjectName(windowTitle());

	createInterface();
	createContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &PanelComparatorInfo::activeSignalChanged, Qt::QueuedConnection);

	startComparatorStateTimer(m_comparatorInfo.timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

PanelComparatorInfo::~PanelComparatorInfo()
{
	stopComparatorStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::createInterface()
{
	m_pComparatorInfoWindow = new QMainWindow;

	m_pComparatorInfoWindow->installEventFilter(this);

	for(int column = 0; column < Metrology::ComparatorCount; column++)
	{
		qstrcpy(m_comparatorInfoColumn[column], tr("Comparator %1").arg(column + 1).toUtf8());
		m_ptrComparatorInfoColumn[column] = m_comparatorInfoColumn[column];
	}

	m_comparatorTable.setColumnCaption(metaObject()->className(), Metrology::ComparatorCount, m_ptrComparatorInfoColumn);
	m_comparatorTable.setComparatorInfo(m_comparatorInfo);
	connect(&theSignalBase, &SignalBase::signalParamChanged, &m_comparatorTable, &ComparatorInfoTable::signalParamChanged, Qt::QueuedConnection);

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

	connect(m_pView, &QTableView::doubleClicked , this, &PanelComparatorInfo::onListDoubleClicked);

	setWidget(m_pComparatorInfoWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pComparatorInfoWindow);

//	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
//	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

//	m_pContextMenu->addSeparator();

	m_pComparatorPropertyAction = m_pContextMenu->addAction(tr("Propertу ..."));
	m_pComparatorPropertyAction->setIcon(QIcon(":/icons/Property.png"));

//	connect(m_pCopyAction, &QAction::triggered, this, &PanelComparatorInfo::copy);
	connect(m_pComparatorPropertyAction, &QAction::triggered, this, &PanelComparatorInfo::comparatorProperty);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &PanelComparatorInfo::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::hideColumn(int column, bool hide)
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

void PanelComparatorInfo::startComparatorStateTimer(int timeout)
{
	if (m_updateComparatorStateTimer == nullptr)
	{
		m_updateComparatorStateTimer = new QTimer(this);
		connect(m_updateComparatorStateTimer, &QTimer::timeout, this, &PanelComparatorInfo::updateComparatorState);
	}

	m_updateComparatorStateTimer->start(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::stopComparatorStateTimer()
{
	if (m_updateComparatorStateTimer != nullptr)
	{
		m_updateComparatorStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::restartComparatorStateTimer(int timeout)
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

void PanelComparatorInfo::measureKindChanged(int measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = static_cast<Measure::Kind>(measureKind);
}


// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::connectionTypeChanged(int connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	m_connectionType = static_cast<Metrology::ConnectionType>(connectionType);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::setComparatorInfo(const ComparatorInfoOption& comparatorInfo)
{
	m_comparatorInfo = comparatorInfo;
	m_comparatorTable.setComparatorInfo(m_comparatorInfo);
	restartComparatorStateTimer(m_comparatorInfo.timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool PanelComparatorInfo::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent* >(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			comparatorProperty();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::activeSignalChanged(const MeasureSignal& activeSignal)
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

		switch (activeSignal.connectionType())
		{
			case Metrology::ConnectionType::Unused:
				pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).metrologySignal(c);
				break;
			default:
				pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination).metrologySignal(c);
				break;
		}

		if (pSignal != nullptr && pSignal->param().isValid() == true)
		{
			if (maxComparatorCount < pSignal->param().comparatorCount())
			{
				maxComparatorCount = pSignal->param().comparatorCount();
			}

			ioParam.setParam(Metrology::ConnectionIoType::Source, pSignal->param());
			ioParam.setConnectionType(activeSignal.connectionType());
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

	if (activeSignal.connectionType() == Metrology::ConnectionType::Unused)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(static_cast<int>(cellSize.height() * 2.1));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::updateComparatorState()
{
	m_comparatorTable.updateState();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::comparatorProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_comparatorTable.count())
	{
		return;
	}

	const Metrology::SignalParam& inParam = m_comparatorTable.at(index).param(Metrology::ConnectionIoType::Source);
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

	DialogComparatorProperty dialog(*comparatorEx, this);
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
