#include "TuningSignalList.h"

#include "ProcessData.h"
#include "ObjectProperties.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSourceTable::TuningSourceTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceTable::~TuningSourceTable()
{
	QMutexLocker l(&m_sourceMutex);

	m_sourceIdList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceTable::columnCount(const QModelIndex&) const
{
	return TUN_SOURCE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceTable::rowCount(const QModelIndex&) const
{
	return sourceCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < TUN_SOURCE_LIST_COLUMN_COUNT)
		{
			result = qApp->translate("TuningSignalListDialog.h", TuningSourceColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= sourceCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	TuningSource src = source(row);

	TuningSourceState sourceState = src.state();

	if (column == TUN_SOURCE_LIST_COLUMN_IS_REPLY || column == TUN_SOURCE_LIST_COLUMN_REQUESTS || column == TUN_SOURCE_LIST_COLUMN_REPLIES || column == TUN_SOURCE_LIST_COLUMN_COMMANDS)
	{
		// get fresh state from base
		//
		 sourceState = theSignalBase.tuning().sourceBase().state(src.sourceID());
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case TUN_SOURCE_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case TUN_SOURCE_LIST_COLUMN_IP:				result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_CHANNEL:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_IS_REPLY:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_REQUESTS:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_REPLIES:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_COMMANDS:		result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == TUN_SOURCE_LIST_COLUMN_REQUESTS || column == TUN_SOURCE_LIST_COLUMN_REPLIES || column == TUN_SOURCE_LIST_COLUMN_COMMANDS)
		{
			if (sourceState.isReply() == false)
			{
				return QColor(Qt::darkGray);
			}
		}

		return QVariant();
	}


	if (role == Qt::BackgroundRole)
	{
		if (column == TUN_SOURCE_LIST_COLUMN_IS_REPLY)
		{
			if (sourceState.isReply() == false)
			{
				return QColor(0xFF, 0xA0, 0xA0) ;
			}
			else
			{
				return QColor(0xA0, 0xFF, 0xA0) ;
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, src, sourceState);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSourceTable::text(int row, int column, const TuningSource& source, const TuningSourceState& state) const
{
	if (row < 0 || row >= sourceCount())
	{
		return QString();
	}

	if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = source.equipmentID();							break;
		case TUN_SOURCE_LIST_COLUMN_CAPTION:		result = source.caption();								break;
		case TUN_SOURCE_LIST_COLUMN_IP:				result = source.serverIP() + " (" + QString::number(source.serverPort()) + ")";	break;
		case TUN_SOURCE_LIST_COLUMN_CHANNEL:		result = source.channel();								break;
		case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:		result = source.subSystem();							break;
		case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:		result = QString::number(source.lmNumber());			break;
		case TUN_SOURCE_LIST_COLUMN_IS_REPLY:		result = state.isReply() == false ? "No" : "Yes";		break;
		case TUN_SOURCE_LIST_COLUMN_REQUESTS:		result = QString::number(state.requestCount());			break;
		case TUN_SOURCE_LIST_COLUMN_REPLIES:		result = QString::number(state.replyCount());			break;
		case TUN_SOURCE_LIST_COLUMN_COMMANDS:		result = QString::number(state.commandQueueSize());		break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::updateColumn(int column)
{
	if (column < 0 || column >= TUN_SOURCE_LIST_COLUMN_COUNT)
	{
		return;
	}

	int count = rowCount();

	for (int row = 0; row < count; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceTable::sourceCount() const
{
	QMutexLocker l(&m_sourceMutex);

	return m_sourceIdList.count();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource TuningSourceTable::source(int index) const
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= m_sourceIdList.count())
	{
		return TuningSource();
	}

	return m_sourceIdList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::set(const QVector<TuningSource>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceIdList = list_add;

		m_sourceMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::clear()
{
	int count = m_sourceIdList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceIdList.clear();

		m_sourceMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSignalTable::TuningSignalTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalTable::~TuningSignalTable()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalTable::columnCount(const QModelIndex&) const
{
	return TUN_SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalTable::rowCount(const QModelIndex&) const
{
	return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSignalTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < TUN_SIGNAL_LIST_COLUMN_COUNT)
		{
			result = qApp->translate("TuningSignalListDialog.h", TuningSignalColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSignalTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= signalCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > TUN_SIGNAL_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	Metrology::Signal* pSignal = signal(row);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case TUN_SIGNAL_LIST_COLUMN_RACK:			result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case TUN_SIGNAL_LIST_COLUMN_STATE:			result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_DEFAULT:		result = Qt::AlignCenter;	break;
			case TUN_SIGNAL_LIST_COLUMN_RANGE:			result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == TUN_SIGNAL_LIST_COLUMN_DEFAULT)
		{
			return QColor(Qt::darkGray);
		}

		return QVariant();
	}


	if (role == Qt::BackgroundRole)
	{
		if (column == TUN_SIGNAL_LIST_COLUMN_STATE)
		{
			if (pSignal->state().valid() == false)
			{
				return theOptions.signalInfo().colorFlagValid();
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignalTable::text(int row, int column, Metrology::Signal* pSignal) const
{
	if (row < 0 || row >= signalCount())
	{
		return QString();
	}

	if (column < 0 || column > TUN_SIGNAL_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSignal == nullptr)
	{
		return QString();
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case TUN_SIGNAL_LIST_COLUMN_RACK:			result = param.location().rack().caption();	break;
		case TUN_SIGNAL_LIST_COLUMN_APP_ID:			result = param.appSignalID();				break;
		case TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID:		result = param.customAppSignalID();			break;
		case TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID:	result = param.equipmentID();				break;
		case TUN_SIGNAL_LIST_COLUMN_CAPTION:		result = param.caption();					break;
		case TUN_SIGNAL_LIST_COLUMN_STATE:			result = signalStateStr(pSignal);			break;
		case TUN_SIGNAL_LIST_COLUMN_DEFAULT:		result = param.tuningDefaultValueStr();		break;
		case TUN_SIGNAL_LIST_COLUMN_RANGE:			result = param.tuningRangeStr();			break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignalTable::signalStateStr(Metrology::Signal* pSignal) const
{
	if (pSignal == nullptr)
	{
		return QString();
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	if (pSignal->state().valid() == false)
	{
		return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
	}

	QString stateStr, formatStr;

	switch (param.signalType())
	{
		case E::SignalType::Analog:

			formatStr = QString::asprintf("%%.%df", param.decimalPlaces());

			stateStr = QString::asprintf(formatStr.toAscii(), pSignal->state().value());

			break;

		case E::SignalType::Discrete:

			stateStr = pSignal->state().value() == 0.0 ? QString("No") : QString("Yes");

			break;

		case E::SignalType::Bus:

			stateStr.clear();

			break;

		default:
			assert(0);
	}

	return stateStr;
}


// -------------------------------------------------------------------------------------------------------------------

void TuningSignalTable::updateColumn(int column)
{
	if (column < 0 || column >= TUN_SIGNAL_LIST_COLUMN_COUNT)
	{
		return;
	}

	int count = rowCount();

	for (int row = 0; row < count; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalTable::signalCount() const
{
	QMutexLocker l(&m_signalMutex);

	return m_signalList.count();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* TuningSignalTable::signal(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_signalList.count())
	{
		return nullptr;
	}

	return m_signalList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalTable::set(const QVector<Metrology::Signal*>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList = list_add;

		m_signalMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalTable::clear()
{
	int count = signalCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

E::SignalType	TuningSignalListDialog::m_typeAD = E::SignalType::Analog;
bool			TuningSignalListDialog::m_showSource = false;

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListDialog::TuningSignalListDialog(QWidget *parent) :
	QDialog(parent)
{
	createInterface();
	updateSourceList();
	updateSignalList();

	startSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalListDialog::~TuningSignalListDialog()
{
	stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Tuning.png"));
	setWindowTitle(tr("Tuning signals"));
	resize(1000, 600);
	move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);

	m_pSetValueAction = m_pSignalMenu->addAction(tr("&Set value ..."));
	m_pSetValueAction->setIcon(QIcon(":/icons/ChangeState.png"));
	//m_pChangeStateAction->setShortcut(Qt::CTRL + Qt::Key_Enter);

	m_pSignalMenu->addSeparator();

	m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
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

	m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
	m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
	m_pTypeAnalogAction->setCheckable(true);
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
	m_pTypeDiscreteAction->setCheckable(true);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction = m_pViewTypeADMenu->addAction(tr("Bus"));
	m_pTypeBusAction->setCheckable(true);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);


	m_pViewShowMenu = new QMenu(tr("Show"), this);
	m_pShowSoucreAction = m_pViewShowMenu->addAction(tr("Sources"));
	m_pShowSoucreAction->setCheckable(true);
	m_pShowSoucreAction->setChecked(m_showSource);

	m_pViewMenu->addMenu(m_pViewTypeADMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addMenu(m_pViewShowMenu);

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pSetValueAction, &QAction::triggered, this, &TuningSignalListDialog::setSignalState);
	connect(m_pExportAction, &QAction::triggered, this, &TuningSignalListDialog::exportSignal);

	connect(m_pFindAction, &QAction::triggered, this, &TuningSignalListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &TuningSignalListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &TuningSignalListDialog::selectAll);

	connect(m_pTypeAnalogAction, &QAction::triggered, this, &TuningSignalListDialog::showTypeAnalog);
	connect(m_pTypeDiscreteAction, &QAction::triggered, this, &TuningSignalListDialog::showTypeDiscrete);
	connect(m_pTypeBusAction, &QAction::triggered, this, &TuningSignalListDialog::showTypeBus);
	connect(m_pShowSoucreAction, &QAction::triggered, this, &TuningSignalListDialog::showSources);


	m_pSourceView = new QTableView(this);
	m_pSourceView->setModel(&m_sourceTable);
	QSize sourceCellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pSourceView->verticalHeader()->setDefaultSectionSize(sourceCellSize.height());

	for(int column = 0; column < TUN_SOURCE_LIST_COLUMN_COUNT; column++)
	{
		m_pSourceView->setColumnWidth(column, TuningSourceColumnWidth[column]);
	}

	m_pSourceView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pSourceView->setFixedHeight(120);

	if (m_showSource == false)
	{
		m_pSourceView->hide();
	}

	m_pSignalView = new QTableView(this);
	m_pSignalView->setModel(&m_signalTable);
	QSize signalCellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pSignalView->verticalHeader()->setDefaultSectionSize(signalCellSize.height());

	for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pSignalView->setColumnWidth(column, TuningSignalColumnWidth[column]);
	}

	m_pSignalView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pSignalView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pSignalView->setWordWrap(false);

	connect(m_pSignalView, &QTableView::doubleClicked , this, &TuningSignalListDialog::onSignalListDoubleClicked);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pSourceView);
	mainLayout->addWidget(m_pSignalView);

	setLayout(mainLayout);

	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pSignalView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSignalView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &TuningSignalListDialog::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pSignalView);

	for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("TuningSignalListDialog.h", TuningSignalColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &TuningSignalListDialog::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pSetValueAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addMenu(m_pViewTypeADMenu);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);

	// init context menu
	//
	m_pSignalView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSignalView, &QTableWidget::customContextMenuRequested, this, &TuningSignalListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateSourceList()
{
	// update source list
	//
	m_sourceTable.clear();

	QVector<TuningSource> sourceList;

	int souceCount = theSignalBase.tuning().sourceBase().count();
	for(int i = 0; i < souceCount; i++)
	{
		const TuningSource& src = theSignalBase.tuning().sourceBase().source(i);
		if (src.sourceID() == 0)
		{
			continue;
		}

		sourceList.append(src);
	}

	m_sourceTable.set(sourceList);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateSignalList()
{
	updateVisibleColunm();

	// update signal list

	m_signalTable.clear();

	QVector<Metrology::Signal*> signalList;

	int signalCount = theSignalBase.tuning().signalBase().count();
	for(int i = 0; i < signalCount; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.tuning().signalBase().signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.signalType() != m_typeAD)
		{
			continue;
		}

		signalList.append(pSignal);
	}

	m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateState()
{
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_IS_REPLY);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REQUESTS);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REPLIES);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_COMMANDS);

	m_signalTable.updateColumn(TUN_SIGNAL_LIST_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::updateVisibleColunm()
{
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);

	m_pShowSoucreAction->setChecked(m_showSource);

	for(int c = 0; c < TUN_SIGNAL_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID, true);
	hideColumn(TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID, true);

	if (m_typeAD == E::SignalType::Discrete)
	{
		hideColumn(TUN_SIGNAL_LIST_COLUMN_RANGE, true);
	}

	if (m_typeAD == E::SignalType::Bus)
	{
		hideColumn(TUN_SIGNAL_LIST_COLUMN_STATE, true);
		hideColumn(TUN_SIGNAL_LIST_COLUMN_DEFAULT, true);
		hideColumn(TUN_SIGNAL_LIST_COLUMN_RANGE, true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= TUN_SIGNAL_LIST_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pSignalView->hideColumn(column);
		m_pColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pSignalView->showColumn(column);
		m_pColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::startSignalStateTimer()
{
	if (m_updateSignalStateTimer == nullptr)
	{
		m_updateSignalStateTimer = new QTimer(this);
		connect(m_updateSignalStateTimer, &QTimer::timeout, this, &TuningSignalListDialog::updateState);
	}

	m_updateSignalStateTimer->start(250); // 250 ms
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::stopSignalStateTimer()
{
	if (m_updateSignalStateTimer != nullptr)
	{
		m_updateSignalStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool TuningSignalListDialog::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			setSignalState();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::setSignalState()
{
	int index = m_pSignalView->currentIndex().row();
	if (index < 0 || index >= m_signalTable.signalCount())
	{
		return;
	}

	Metrology::Signal* pSignal = m_signalTable.signal(index);
	if (pSignal == nullptr)
	{
		return;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return;
	}

	if (param.isBus() == true || param.isInternal() == false)
	{
		return;
	}

	TuningSignalStateDialog* dialog = new TuningSignalStateDialog(param);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::exportSignal()
{
	ExportData* dialog = new ExportData(m_pSignalView, false, "TuningSignals");
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::find()
{
	FindData* dialog = new FindData(m_pSignalView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::copy()
{
	CopyData copyData(m_pSignalView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showTypeAnalog()
{
	m_typeAD = E::SignalType::Analog;

	updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showTypeDiscrete()
{
	m_typeAD = E::SignalType::Discrete;

	updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showTypeBus()
{
	m_typeAD = E::SignalType::Bus;

	updateSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::showSources()
{
	m_showSource = m_pShowSoucreAction->isChecked();

	if (m_showSource == true)
	{
		m_pSourceView->show();
	}
	else
	{
		m_pSourceView->hide();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < TUN_SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		if (m_pColumnAction[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalListDialog::onSignalListDoubleClicked(const QModelIndex&)
{
	setSignalState();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSignalStateDialog::TuningSignalStateDialog(const Metrology::SignalParam& param, QWidget *parent) :
	QDialog(parent),
	m_param(param)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalStateDialog::~TuningSignalStateDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalStateDialog::createInterface()
{
	setWindowFlags(Qt::Window  | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Tuning.png"));
	setWindowTitle(tr("Signal state"));

	if (m_param.isValid() == false)
	{
		QMessageBox::critical(this, windowTitle(), tr("It is not possible to change signal state!"));
		return;
	}

	// main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	switch(m_param.signalType())
	{
		case E::SignalType::Analog:
			{
				QLabel* stateLabel = new QLabel(tr("Please, input new state of analog signal:"));
				stateLabel->setAlignment(Qt::AlignHCenter);

				QRegExp rx("^[-]{0,1}[0-9]*[.]{1}[0-9]*$");
				QValidator *validator = new QRegExpValidator(rx, this);

				m_stateEdit = new QLineEdit(QString::number(theSignalBase.signalState(m_param.hash()).value() ));
				m_stateEdit->setAlignment(Qt::AlignHCenter);
				m_stateEdit->setValidator(validator);

				QLabel* rangeLabel = new QLabel(m_param.tuningRangeStr());
				rangeLabel->setAlignment(Qt::AlignHCenter);

				// buttons
				//
				QHBoxLayout *buttonLayout = new QHBoxLayout ;

				QPushButton* okButton = new QPushButton(tr("Ok"));
				QPushButton* cancelButton = new QPushButton(tr("Cancel"));

				connect(okButton, &QPushButton::clicked, this, &TuningSignalStateDialog::onOk);
				connect(cancelButton, &QPushButton::clicked, this, &TuningSignalStateDialog::reject);

				buttonLayout->addWidget(okButton);
				buttonLayout->addWidget(cancelButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addWidget(m_stateEdit);
				mainLayout->addWidget(rangeLabel);
				mainLayout->addStretch();
				mainLayout->addLayout(buttonLayout);
			}
			break;

		case E::SignalType::Discrete:
			{
				QLabel* stateLabel = new QLabel(tr("Please, select new state of discrete signal:"));

				// buttons
				//
				QHBoxLayout *buttonLayout = new QHBoxLayout ;

				QPushButton* yesButton = new QPushButton(tr("Yes"));
				QPushButton* noButton = new QPushButton(tr("No"));

				connect(yesButton, &QPushButton::clicked, this, &TuningSignalStateDialog::onYes);
				connect(noButton, &QPushButton::clicked, this, &TuningSignalStateDialog::onNo);

				buttonLayout->addWidget(yesButton);
				buttonLayout->addWidget(noButton);

				// main Layout
				//
				mainLayout->addWidget(stateLabel);
				mainLayout->addLayout(buttonLayout);
			}
			break;

		default:
			assert(0);
	}

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalStateDialog::onOk()
{
	double state = m_stateEdit->text().toDouble();

	if (state < m_param.tuningLowBound().toDouble()|| state > m_param.tuningHighBound().toDouble())
	{
		QString str, formatStr;

		formatStr = QString::asprintf("%%.%df", m_param.decimalPlaces());

		str = tr("Failed input value: %1").arg(QString::asprintf(formatStr.toAscii(), state));
		str += tr("\nRange of signal: %1").arg(m_param.tuningRangeStr());

		QMessageBox::critical(this, windowTitle(), str);
		return;
	}

	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), state);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalStateDialog::onYes()
{
	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), 1);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalStateDialog::onNo()
{
	theSignalBase.tuning().appendCmdFowWrite(m_param.hash(), m_param.tuningValueType(), 0);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
