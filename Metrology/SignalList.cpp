#include "SignalList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalListTable::m_showCustomID = true;
bool SignalListTable::m_showADCInHex = true;

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::SignalListTable(QObject*)
{
	connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &SignalListTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::~SignalListTable()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::columnCount(const QModelIndex&) const
{
	return SIGNAL_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::rowCount(const QModelIndex&) const
{
	return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SIGNAL_LIST_COLUMN_COUNT)
		{
			result = SignalListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalListTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	MetrologySignal* pSignal = signal(row);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case SIGNAL_LIST_COLUMN_RACK:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_ID:					result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:		result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_CAPTION:			result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_CHASSIS:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_MODULE:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_PLACE:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_ADC:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_IN_PH_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_IN_EL_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_IN_EL_SENSOR:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_OUT_PH_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_OUT_EL_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_OUT_EL_SENSOR:		result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_TUN_SIGNAL:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL:	result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalListTable::text(int row, int column, MetrologySignal* pSignal) const
{
	if (row < 0 || row >= signalCount())
	{
		return QString();
	}

	if (column < 0 || column > SIGNAL_LIST_COLUMN_COUNT)
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
		case SIGNAL_LIST_COLUMN_RACK:				result = param.location().rack().caption();	break;
		case SIGNAL_LIST_COLUMN_ID:					result = m_showCustomID == true ? param.customAppSignalID() : param.appSignalID();	break;
		case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:		result = param.location().equipmentID();	break;
		case SIGNAL_LIST_COLUMN_CAPTION:			result = param.caption();					break;
		case SIGNAL_LIST_COLUMN_CHASSIS:			result = param.location().chassisStr();		break;
		case SIGNAL_LIST_COLUMN_MODULE:				result = param.location().moduleStr();		break;
		case SIGNAL_LIST_COLUMN_PLACE:				result = param.location().placeStr();		break;
		case SIGNAL_LIST_COLUMN_ADC:				result = param.adcRangeStr(m_showADCInHex);	break;
		case SIGNAL_LIST_COLUMN_IN_PH_RANGE:		result = param.inputPhysicalRangeStr();		break;
		case SIGNAL_LIST_COLUMN_IN_EL_RANGE:		result = param.inputElectricRangeStr();		break;
		case SIGNAL_LIST_COLUMN_IN_EL_SENSOR:		result = param.inputElectricSensor();		break;
		case SIGNAL_LIST_COLUMN_OUT_PH_RANGE:		result = param.outputPhysicalRangeStr();	break;
		case SIGNAL_LIST_COLUMN_OUT_EL_RANGE:		result = param.outputElectricRangeStr();	break;
		case SIGNAL_LIST_COLUMN_OUT_EL_SENSOR:		result = param.outputElectricSensor();		break;
		case SIGNAL_LIST_COLUMN_TUN_SIGNAL:			result = param.enableTuningStr();			break;
		case SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL:	result = param.tuningDefaultValueStr();		break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.size();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal* SignalListTable::signal(int index) const
{
	MetrologySignal* signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			 signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::set(const QList<MetrologySignal*> list_add)
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

void SignalListTable::clear()
{
	int count = signalCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			for(int i = count - 1; i >= 0; i--)
			{
				if (m_signalList[i] != nullptr)
				{
					delete m_signalList[i];
				}
			}

			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::updateSignalParam(const Hash& signalHash)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
		{
			 MetrologySignal* pSignal = m_signalList[i];
			 if (pSignal == nullptr)
			 {
				 continue;
			 }

			if (pSignal->param().hash() == signalHash)
			{
				pSignal->setParam(theSignalBase.signalParam(signalHash));

				break;
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

E::SignalType		SignalListDialog::m_typeAD		= E::SignalType::Analog;
E::SignalInOutType	SignalListDialog::m_typeIO		= E::SignalInOutType::Input;
int					SignalListDialog::m_currenIndex = 0;

// -------------------------------------------------------------------------------------------------------------------

SignalListDialog::SignalListDialog(bool hasButtons, QWidget *parent) :
	QDialog(parent)
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->m_pConfigSocket != nullptr)
	{
		connect(pMainWindow->m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &SignalListDialog::updateList, Qt::QueuedConnection);
	}

	createInterface(hasButtons);
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalListDialog::~SignalListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createInterface(bool hasButtons)
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Signals.png"));
	setWindowTitle(tr("Signals"));
	resize(QApplication::desktop()->availableGeometry().width() - 200, 500);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);

	m_pPrintAction = m_pSignalMenu->addAction(tr("&Print ..."));
	m_pPrintAction->setIcon(QIcon(":/icons/Print.png"));
	m_pPrintAction->setShortcut(Qt::CTRL + Qt::Key_P);

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

	m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Properties ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
	m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
	m_pTypeAnalogAction->setCheckable(true);
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
	m_pTypeDiscreteAction->setCheckable(true);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);

	m_pViewTypeIOMenu = new QMenu(tr("Type I/O"), this);
	m_pTypeInputAction = m_pViewTypeIOMenu->addAction(tr("Input"));
	m_pTypeInputAction->setCheckable(true);
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction = m_pViewTypeIOMenu->addAction(tr("Internal"));
	m_pTypeInternalAction->setCheckable(true);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
	m_pTypeOutputAction = m_pViewTypeIOMenu->addAction(tr("Output"));
	m_pTypeOutputAction->setCheckable(true);
	m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

	m_pViewShowMenu = new QMenu(tr("Show"), this);
	m_pShowCustomIDAction = m_pViewShowMenu->addAction(tr("Custom ID"));
	m_pShowCustomIDAction->setCheckable(true);
	m_pShowCustomIDAction->setChecked(m_signalTable.showCustomID());
	m_pShowCustomIDAction->setShortcut(Qt::CTRL + Qt::Key_Tab);
	m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
	m_pShowADCInHexAction->setCheckable(true);
	m_pShowADCInHexAction->setChecked(m_signalTable.showADCInHex());

	m_pViewMenu->addMenu(m_pViewTypeADMenu);
	m_pViewMenu->addMenu(m_pViewTypeIOMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addMenu(m_pViewShowMenu);

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pPrintAction, &QAction::triggered, this, &SignalListDialog::printSignal);
	connect(m_pExportAction, &QAction::triggered, this, &SignalListDialog::exportSignal);

	connect(m_pFindAction, &QAction::triggered, this, &SignalListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &SignalListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &SignalListDialog::selectAll);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalListDialog::signalProperties);

	connect(m_pTypeAnalogAction, &QAction::triggered, this, &SignalListDialog::showTypeAnalog);
	connect(m_pTypeDiscreteAction, &QAction::triggered, this, &SignalListDialog::showTypeDiscrete);
	connect(m_pTypeInputAction, &QAction::triggered, this, &SignalListDialog::showTypeInput);
	connect(m_pTypeInternalAction, &QAction::triggered, this, &SignalListDialog::showTypeInternal);
	connect(m_pTypeOutputAction, &QAction::triggered, this, &SignalListDialog::showTypeOutput);
	connect(m_pShowCustomIDAction, &QAction::triggered, this, &SignalListDialog::showCustomID);
	connect(m_pShowADCInHexAction, &QAction::triggered, this, &SignalListDialog::showADCInHex);


	m_pView = new QTableView(this);
	m_pView->setModel(&m_signalTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(m_pView, &QTableView::doubleClicked , this, &SignalListDialog::onListDoubleClicked);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);

	if (hasButtons == true)
	{
		m_pView->setSelectionMode(QAbstractItemView::SingleSelection);

		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

		connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalListDialog::onOk);
		connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalListDialog::reject);

		mainLayout->addWidget(m_buttonBox);
	}

	setLayout(mainLayout);

	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &SignalListDialog::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(SignalListColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalListDialog::onColumnAction);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addMenu(m_pViewTypeADMenu);
	m_pContextMenu->addMenu(m_pViewTypeIOMenu);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSignalPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::updateList()
{
	updateVisibleColunm();

	m_signalTable.clear();

	QList<MetrologySignal*> signalList;

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		MetrologySignal signal = theSignalBase.signal(i);
		if (signal.param().isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam& param = signal.param();

		if (param.signalType() != m_typeAD || param.inOutType() != m_typeIO)
		{
			continue;
		}

		signalList.append(new MetrologySignal(signal));
	}

	m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::updateVisibleColunm()
{
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
	m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

	for(int c = 0; c < SIGNAL_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(SIGNAL_LIST_COLUMN_EQUIPMENT_ID, true);


	switch (m_typeAD)
	{
		case E::SignalType::Analog:

			switch (m_typeIO)
			{
				case E::SignalInOutType::Input:
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_SENSOR, true);
					break;

				case E::SignalInOutType::Internal:
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_SENSOR, true);
					break;

				case E::SignalInOutType::Output:
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_IN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_SENSOR, true);
					break;

				default:
					assert(0);
			}

			break;

		case E::SignalType::Discrete:

			hideColumn(SIGNAL_LIST_COLUMN_ADC, true);
			hideColumn(SIGNAL_LIST_COLUMN_IN_PH_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_IN_EL_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_IN_EL_SENSOR, true);
			hideColumn(SIGNAL_LIST_COLUMN_OUT_PH_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_OUT_EL_SENSOR, true);

			break;


		default:
			assert(0);
	}

	if (m_typeIO != E::SignalInOutType::Internal)
	{
		hideColumn(SIGNAL_LIST_COLUMN_TUN_SIGNAL, true);
		hideColumn(SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL, true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= SIGNAL_LIST_COLUMN_COUNT)
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

bool SignalListDialog::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
		{
			if (m_buttonBox == nullptr)
			{
				signalProperties();
			}
			else
			{
				emit onOk();
			}
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::printSignal()
{

}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::exportSignal()
{
	ExportData* dialog = new ExportData(m_pView, tr("Signals"));
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::copy()
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

void SignalListDialog::signalProperties()
{
	QModelIndex visibleIndex = m_pView->currentIndex();

	int index = visibleIndex .row();
	if (index < 0 || index >= m_signalTable.signalCount())
	{
		return;
	}

	MetrologySignal* pSignal = m_signalTable.signal(index);
	if (pSignal == nullptr)
	{
		return;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return;
	}

	SignalPropertyDialog dialog(param);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeAnalog()
{
	m_typeAD = E::SignalType::Analog;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeDiscrete()
{
	m_typeAD = E::SignalType::Discrete;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInput()
{
	m_typeIO = E::SignalInOutType::Input;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeInternal()
{
	m_typeIO = E::SignalInOutType::Internal;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showTypeOutput()
{
	m_typeIO = E::SignalInOutType::Output;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showCustomID()
{
	m_signalTable.setShowCustomID(m_pShowCustomIDAction->isChecked());

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::showADCInHex()
{
	m_signalTable.setShowADCInHex(m_pShowADCInHexAction->isChecked());

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		if (m_pColumnAction[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onListDoubleClicked(const QModelIndex&)
{
	if (m_buttonBox == nullptr)
	{
		signalProperties();
	}
	else
	{
		emit onOk();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListDialog::onOk()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalTable.signalCount())
	{
		return;
	}

	MetrologySignal* pSignal = m_signalTable.signal(index);
	if (pSignal == nullptr)
	{
		return;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return;
	}

	m_selectedSignalHash = param.hash();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
