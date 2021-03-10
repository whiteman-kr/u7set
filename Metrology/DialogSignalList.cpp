#include "DialogSignalList.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalListTable::SignalListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalListTable::~SignalListTable()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
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
			result = qApp->translate("DialogSignalList", SignalListColumn[section]);
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
			case SIGNAL_LIST_COLUMN_APP_ID:				result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_CUSTOM_ID:			result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:		result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_CAPTION:			result = Qt::AlignLeft;		break;
			case SIGNAL_LIST_COLUMN_RACK:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_CHASSIS:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_MODULE:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_PLACE:				result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS:	result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_ADC_RANGE:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_EL_RANGE:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_EL_SENSOR:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_PH_RANGE:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_EN_RANGE:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_TUN_SIGNAL:			result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL:	result = Qt::AlignCenter;	break;
			case SIGNAL_LIST_COLUMN_TUN_RANGE:			result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::BackgroundRole)
	{
		switch (column)
		{
			case SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS:

				if (pSignal->param().isAnalog() == true && pSignal->param().location().shownOnSchemas() == true)
				{
					return QColor(0xA0, 0xFF, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_ADC_RANGE:

				if (pSignal->param().highADC() - pSignal->param().lowADC() <= 0)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_EL_RANGE:

				if (pSignal->param().electricRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_EL_SENSOR:

				if (pSignal->param().electricSensorType() == E::SensorType::NoSensor)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_PH_RANGE:

				if (pSignal->param().physicalRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_EN_RANGE:

			    if (pSignal->param().engineeringRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;

			case SIGNAL_LIST_COLUMN_TUN_RANGE:

				if (pSignal->param().tuningRangeIsValid() == false)
				{
					return QColor(0xFF, 0xA0, 0xA0);
				}
				break;
		}
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalListTable::text(int row, int column, Metrology::Signal* pSignal) const
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
		case SIGNAL_LIST_COLUMN_APP_ID:				result = param.appSignalID();					break;
		case SIGNAL_LIST_COLUMN_CUSTOM_ID:			result = param.customAppSignalID();				break;
		case SIGNAL_LIST_COLUMN_EQUIPMENT_ID:		result = param.equipmentID();					break;
		case SIGNAL_LIST_COLUMN_CAPTION:			result = param.caption();						break;
		case SIGNAL_LIST_COLUMN_RACK:				result = param.location().rack().caption();		break;
		case SIGNAL_LIST_COLUMN_CHASSIS:			result = param.location().chassisStr();			break;
		case SIGNAL_LIST_COLUMN_MODULE:				result = param.location().moduleStr();			break;
		case SIGNAL_LIST_COLUMN_PLACE:				result = param.location().placeStr();			break;
		case SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS:	result = qApp->translate("MetrologySignal", param.location().shownOnSchemasStr().toUtf8());	break;
		case SIGNAL_LIST_COLUMN_ADC_RANGE:			result = param.adcRangeStr(true);				break;
		case SIGNAL_LIST_COLUMN_EL_RANGE:			result = param.electricRangeStr();				break;
		case SIGNAL_LIST_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();			break;
		case SIGNAL_LIST_COLUMN_PH_RANGE:			result = param.physicalRangeStr();				break;
		case SIGNAL_LIST_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();			break;
		case SIGNAL_LIST_COLUMN_TUN_SIGNAL:			result = param.enableTuningStr();				break;
		case SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL:	result = param.tuningDefaultValueStr();			break;
		case SIGNAL_LIST_COLUMN_TUN_RANGE:			result = param.tuningRangeStr();				break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalListTable::signalCount() const
{
	QMutexLocker l(&m_signalMutex);

	return m_signalList.count();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalListTable::signal(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_signalList.count())
	{
		return nullptr;
	}

	return m_signalList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalListTable::set(const QVector<Metrology::Signal*>& list_add)
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

			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

E::SignalType		DialogSignalList::m_typeAD = E::SignalType::Analog;
E::SignalInOutType	DialogSignalList::m_typeIO = E::SignalInOutType::Input;

// -------------------------------------------------------------------------------------------------------------------

DialogSignalList::DialogSignalList(bool hasButtons, QWidget* parent) :
	DialogList(0.6, 0.4, hasButtons, parent)
{
	createInterface();
	DialogSignalList::updateList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogSignalList::~DialogSignalList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::createInterface()
{
	setWindowTitle(tr("Signals"));

	// menu
	//
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);
	m_pViewTypeADMenu = new QMenu(tr("Type A/D"), this);
	m_pViewTypeIOMenu = new QMenu(tr("Type I/O"), this);

	// action
	//
	m_pSignalMenu->addAction(m_pExportAction);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pPropertyAction);

	m_pTypeAnalogAction = m_pViewTypeADMenu->addAction(tr("Analog"));
	m_pTypeAnalogAction->setCheckable(true);
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction = m_pViewTypeADMenu->addAction(tr("Discrete"));
	m_pTypeDiscreteAction->setCheckable(true);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction = m_pViewTypeADMenu->addAction(tr("Bus"));
	m_pTypeBusAction->setCheckable(true);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);

	m_pTypeInputAction = m_pViewTypeIOMenu->addAction(tr("Input"));
	m_pTypeInputAction->setCheckable(true);
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction = m_pViewTypeIOMenu->addAction(tr("Internal"));
	m_pTypeInternalAction->setCheckable(true);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
	m_pTypeOutputAction = m_pViewTypeIOMenu->addAction(tr("Output"));
	m_pTypeOutputAction->setCheckable(true);
	m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

	m_pViewMenu->addMenu(m_pViewTypeADMenu);
	m_pViewMenu->addMenu(m_pViewTypeIOMenu);

	//
	//
	addMenu(m_pSignalMenu);
	addMenu(m_pEditMenu);
	addMenu(m_pViewMenu);

	//
	//
	connect(m_pTypeAnalogAction, &QAction::triggered, this, &DialogSignalList::showTypeAnalog);
	connect(m_pTypeDiscreteAction, &QAction::triggered, this, &DialogSignalList::showTypeDiscrete);
	connect(m_pTypeBusAction, &QAction::triggered, this, &DialogSignalList::showTypeBus);
	connect(m_pTypeInputAction, &QAction::triggered, this, &DialogSignalList::showTypeInput);
	connect(m_pTypeInternalAction, &QAction::triggered, this, &DialogSignalList::showTypeInternal);
	connect(m_pTypeOutputAction, &QAction::triggered, this, &DialogSignalList::showTypeOutput);

	//
	//
	setModel(&m_signalTable);
	DialogList::createHeaderContexMenu(SIGNAL_LIST_COLUMN_COUNT, SignalListColumn, SignalListColumnWidth);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::createContextMenu()
{
	addContextMenu(m_pViewTypeADMenu);
	addContextMenu(m_pViewTypeIOMenu);
	addContextSeparator();
	addContextAction(m_pCopyAction);
	addContextSeparator();
	addContextAction(m_pPropertyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::updateVisibleColunm()
{
	m_pTypeAnalogAction->setChecked(m_typeAD == E::SignalType::Analog);
	m_pTypeDiscreteAction->setChecked(m_typeAD == E::SignalType::Discrete);
	m_pTypeBusAction->setChecked(m_typeAD == E::SignalType::Bus);
	m_pTypeInputAction->setChecked(m_typeIO == E::SignalInOutType::Input);
	m_pTypeInternalAction->setChecked(m_typeIO == E::SignalInOutType::Internal);
	m_pTypeOutputAction->setChecked(m_typeIO == E::SignalInOutType::Output);

	for(int c = 0; c < SIGNAL_LIST_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(SIGNAL_LIST_COLUMN_CUSTOM_ID, true);
	hideColumn(SIGNAL_LIST_COLUMN_EQUIPMENT_ID, true);
	hideColumn(SIGNAL_LIST_COLUMN_SHOWN_ON_SCHEMS, true);

	switch (m_typeAD)
	{
		case E::SignalType::Analog:

			switch (m_typeIO)
			{
				case E::SignalInOutType::Input:
				case E::SignalInOutType::Output:
					hideColumn(SIGNAL_LIST_COLUMN_ADC_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_PH_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_TUN_RANGE, true);
					break;

				case E::SignalInOutType::Internal:
					hideColumn(SIGNAL_LIST_COLUMN_ADC_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_EL_RANGE, true);
					hideColumn(SIGNAL_LIST_COLUMN_EL_SENSOR, true);
					hideColumn(SIGNAL_LIST_COLUMN_PH_RANGE, true);
					break;

				default:
					assert(0);
			}

			break;

		case E::SignalType::Discrete:

			hideColumn(SIGNAL_LIST_COLUMN_ADC_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EL_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EL_SENSOR, true);
			hideColumn(SIGNAL_LIST_COLUMN_PH_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EN_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_TUN_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_TUN_RANGE, true);

			break;

		case E::SignalType::Bus:

			hideColumn(SIGNAL_LIST_COLUMN_ADC_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EL_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EL_SENSOR, true);
			hideColumn(SIGNAL_LIST_COLUMN_PH_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_EN_RANGE, true);
			hideColumn(SIGNAL_LIST_COLUMN_TUN_SIGNAL, true);
			hideColumn(SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL, true);
			hideColumn(SIGNAL_LIST_COLUMN_TUN_RANGE, true);

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

void DialogSignalList::updateList()
{
	updateVisibleColunm();

	m_signalTable.clear();

	QVector<Metrology::Signal*> signalList;

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

		if (param.signalType() != m_typeAD || param.inOutType() != m_typeIO)
		{
			continue;
		}

		signalList.append(pSignal);
	}

	m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::onProperties()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	QModelIndex visibleIndex = pView->currentIndex();

	int index = visibleIndex .row();
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

	DialogSignalProperty dialog(param, this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeAnalog()
{
	m_typeAD = E::SignalType::Analog;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeDiscrete()
{
	m_typeAD = E::SignalType::Discrete;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeBus()
{
	m_typeAD = E::SignalType::Bus;

	updateList();
}


// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeInput()
{
	m_typeIO = E::SignalInOutType::Input;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeInternal()
{
	m_typeIO = E::SignalInOutType::Internal;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::showTypeOutput()
{
	m_typeIO = E::SignalInOutType::Output;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalList::onOk()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
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

	m_selectedSignalHash = param.hash();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
