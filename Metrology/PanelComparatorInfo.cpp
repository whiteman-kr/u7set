#include "PanelComparatorInfo.h"

#include "ProcessData.h"
#include "DialogObjectProperties.h"

#include <QApplication>

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

	const Metrology::SignalState& outState = comparatorEx->outputSignal()->state();

	if (role == Qt::FontRole)
	{
		return m_comparatorInfo.font();
	}

	if (role == Qt::ForegroundRole)
	{
		if (outState.flags().simulated == true || outState.flags().blocked == true)
		{
			return QVariant();
		}

		if (comparatorEx->enableMeasure() == false)
		{
			return QColor(Qt::lightGray);
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (outState.flags().blocked == true)
		{
			return m_comparatorInfo.colorFlagLock();
		}

		if (outState.flags().simulated == true)
		{
			return m_comparatorInfo.colorFlagSim();
		}

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

	return comparatorEx->compareOnlineValueStr(Metrology::CmpValueType::SetPoint, true);
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

	quint64 signalCount = m_list.size();
	for(quint64 c = 0; c < signalCount; c ++)
	{
		for(int ioType = 0; ioType < Metrology::CONNECTION_IO_TYPE_COUNT; ioType ++)
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

	m_comparatorTable.setColumnCaption(PanelComparatorInfo::metaObject()->className(), Metrology::ComparatorCount, m_ptrComparatorInfoColumn);
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
	if (m_pView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pComparatorInfoWindow);


	m_pMeasureMenu = new QMenu(tr("Measure"), m_pComparatorInfoWindow);

	m_pEnableMeasureAction = m_pMeasureMenu->addAction(tr("Enable"));
	m_pDisableMeasureAction = m_pMeasureMenu->addAction(tr("Disable"));

	m_pContextMenu->addMenu(m_pMeasureMenu);

	m_pContextMenu->addSeparator();


//	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
//	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pCopyCellAction = m_pContextMenu->addAction(tr("Copy cell"));
	m_pCopyCellAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pContextMenu->addSeparator();

	m_pComparatorPropertyAction = m_pContextMenu->addAction(tr("Propertу ..."));
	m_pComparatorPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	connect(m_pEnableMeasureAction, &QAction::triggered, this, &PanelComparatorInfo::onEnableMeasure);
	connect(m_pDisableMeasureAction, &QAction::triggered, this, &PanelComparatorInfo::onDisableMeasure);
//	connect(m_pCopyAction, &QAction::triggered, this, &PanelComparatorInfo::onCopy);
	connect(m_pCopyCellAction, &QAction::triggered, this, &PanelComparatorInfo::onCopyCell);
	connect(m_pComparatorPropertyAction, &QAction::triggered, this, &PanelComparatorInfo::onComparatorProperty);

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

void PanelComparatorInfo::measureKindChanged(Measure::Kind measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = measureKind;
}


// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::connectionTypeChanged(Metrology::ConnectionType connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	m_connectionType = connectionType;
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

	if (m_pEnableMeasureAction == nullptr || m_pDisableMeasureAction == nullptr)
	{
		return;
	}

	if (comparatorEx->enableMeasure() == true)
	{
		m_pEnableMeasureAction->setEnabled(false);
		m_pDisableMeasureAction->setEnabled(true);
	}
	else
	{
		m_pEnableMeasureAction->setEnabled(true);
		m_pDisableMeasureAction->setEnabled(false);
	}

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
			onComparatorProperty();
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

	std::vector<IoSignalParam> ioParamList;

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

		ioParamList.push_back(ioParam);
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

void PanelComparatorInfo::onEnableMeasure()
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

	comparatorEx->setEnableMeasure(true);

	emit updateSignalInList(inParam.hash());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::onDisableMeasure()
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

	comparatorEx->setEnableMeasure(false);

	emit updateSignalInList(inParam.hash());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::onCopyCell()
{
	if (m_pView == nullptr)
	{
		return;
	}

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(m_pView->model()->data(m_pView->currentIndex()).toString());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelComparatorInfo::onComparatorProperty()
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
