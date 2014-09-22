#include "Stable.h"
#include "SignalsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"


const int SC_STR_ID = 0,
SC_EXT_STR_ID = 1,
SC_NAME = 2,
SC_CHANNEL = 3,
SC_DATA_FORMAT = 4,
SC_DATA_SIZE = 5,
SC_LOW_ADC = 6,
SC_HIGH_ADC = 7,
SC_LOW_LIMIT = 8,
SC_HIGH_LIMIT = 9,
SC_UNIT = 10,
SC_ADJUSTMENT = 11,
SC_DROP_LIMIT = 12,
SC_EXCESS_LIMIT = 13,
SC_UNBALANCE_LIMIT = 14,
SC_INPUT_LOW_LIMIT = 15,
SC_INPUT_HIGH_LIMIT = 16,
SC_INPUT_UNIT = 17,
SC_INPUT_SENSOR = 18,
SC_OUTPUT_LOW_LIMIT = 19,
SC_OUTPUT_HIGH_LIMIT = 20,
SC_OUTPUT_UNIT = 21,
SC_OUTPUT_SENSOR = 22,
SC_ACQUIRE = 23,
SC_CALCULATED = 24,
SC_NORMAL_STATE = 25,
SC_DECIMAL_PLACES = 26,
SC_APERTURE = 27,
SC_IN_OUT_TYPE = 28,
SC_IN_OUT_NO = 29,
SC_DEVICE = 30;


const char* Columns[] =
{
	"ID",
	"External ID",
	"Name",
	"Channel",
	"Data format",
	"Data size"
	"Low ADC",
	"High ADC",
	"Low limit",
	"High limit",
	"Unit",
	"Adjustment",
	"Drop limit",
	"Excess limit",
	"Unbalance limit",
	"InputLow limit",
	"InputHigh limit",
	"Input unit",
	"Input sensor"
	"Output low Limit",
	"Output high Limit",
	"Output unit",
	"Output sensor",
	"Acquire",
	"Calculated",
	"Normal state",
	"Decimal places"
	"Aperture",
	"Input-output type"
	"Input-output nomber"
	"Device"
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);


SignalsModel::SignalsModel(DbController* dbController, QWidget *parent) :
	QAbstractTableModel(parent),
	m_parentWindow(parent),
	m_dbController(dbController)
{
}

SignalsModel::~SignalsModel()
{

}

int SignalsModel::rowCount(const QModelIndex &) const
{
	return m_signalIDs.count();
}

int SignalsModel::columnCount(const QModelIndex &) const
{
	return COLUMNS_COUNT;
}

QVariant SignalsModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (row == m_signalIDs.count())
	{
		return QVariant();
	}
	const Signal* signal = m_signalSet.getConstSignal(m_signalIDs[row]);
	if (signal == nullptr)
	{
		//emit signalDataRequest(m_signalIDs[row]);
		return QVariant();
	}
	if (role == Qt::DisplayRole)
	{
		switch (col)
		{
			case SC_STR_ID: return signal->strID();
			case SC_EXT_STR_ID: return signal->extStrID();
			case SC_NAME: return signal->name();
			case SC_CHANNEL: return signal->channel();
			case SC_DATA_FORMAT: return signal->dataFormat();
			case SC_DATA_SIZE: return signal->dataSize();
			case SC_LOW_ADC: return signal->lowADC();
			case SC_HIGH_ADC: return signal->highADC();
			case SC_LOW_LIMIT: return signal->lowLimit();
			case SC_HIGH_LIMIT: return signal->highLimit();
			case SC_UNIT: return signal->unitID();
			case SC_ADJUSTMENT: return signal->adjustment();
			case SC_EXCESS_LIMIT: return signal->excessLimit();
			case SC_UNBALANCE_LIMIT: return signal->unbalanceLimit();
			case SC_INPUT_LOW_LIMIT: return signal->inputLowLimit();
			case SC_INPUT_HIGH_LIMIT: return signal->inputHighLimit();
			case SC_INPUT_UNIT: return signal->inputUnitID();
			case SC_INPUT_SENSOR: return signal->inputSensorID();
			case SC_OUTPUT_LOW_LIMIT: return signal->outputLowLimit();
			case SC_OUTPUT_HIGH_LIMIT: return signal->outputHighLimit();
			case SC_OUTPUT_UNIT: return signal->outputUnitID();
			case SC_OUTPUT_SENSOR: return signal->outputSensorID();
			case SC_ACQUIRE: return signal->acquire();
			case SC_CALCULATED: return signal->calculated();
			case SC_NORMAL_STATE: return signal->normalState();
			case SC_DECIMAL_PLACES: return signal->decimalPlaces();
			case SC_APERTURE: return signal->aperture();
			case SC_IN_OUT_TYPE: return signal->inOutType();
			case SC_IN_OUT_NO: return signal->inOutNo();
			case SC_DEVICE: return signal->deviceID();
		}
	}

	return QVariant();
}

QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return QString(Columns[section]);
		}
		if (orientation == Qt::Vertical)
		{
			if (section < m_signalIDs.count())
			{
				return m_signalIDs[section];
			}
		}
	}
	return QVariant();
}

bool SignalsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		bool added = false;
		int row = index.row();
		if (row == m_signalIDs.count())
		{
			/*beginInsertRows(QModelIndex(), row, row);
			Signal signal;
			m_signals.append(signal);
			endInsertRows();
			added = true;*/
		}

		Signal signal/* = m_signals[index.row()]*/;

		switch (index.column())
		{
			case SC_STR_ID: signal.setStrID(value.toString()); break;
			case SC_EXT_STR_ID: signal.setExtStrID(value.toString()); break;
			case SC_NAME: signal.setName(value.toString()); break;
			case SC_DATA_FORMAT: signal.setDataFormat(value.toInt()); break;
			case SC_DATA_SIZE: signal.setDataSize(value.toInt()); break;
			case SC_LOW_ADC: signal.setLowADC(value.toInt()); break;
			case SC_HIGH_ADC: signal.setHighADC(value.toInt()); break;
			case SC_LOW_LIMIT: signal.setLowLimit(value.toDouble()); break;
			case SC_HIGH_LIMIT: signal.setHighLimit(value.toDouble()); break;
			case SC_UNIT: signal.setUnitID(value.toInt()); break;
			case SC_ADJUSTMENT: signal.setAdjustment(value.toDouble()); break;
			case SC_EXCESS_LIMIT: signal.setExcessLimit(value.toDouble()); break;
			case SC_UNBALANCE_LIMIT: signal.setUnbalanceLimit(value.toDouble()); break;
			case SC_INPUT_LOW_LIMIT: signal.setInputLowLimit(value.toDouble()); break;
			case SC_INPUT_HIGH_LIMIT: signal.setInputHighLimit(value.toDouble()); break;
			case SC_INPUT_UNIT: signal.setInputUnitID(value.toInt()); break;
			case SC_INPUT_SENSOR: signal.setInputSensorID(value.toInt()); break;
			case SC_OUTPUT_LOW_LIMIT: signal.setOutputLowLimit(value.toDouble()); break;
			case SC_OUTPUT_HIGH_LIMIT: signal.setOutputHighLimit(value.toDouble()); break;
			case SC_OUTPUT_UNIT: signal.setOutputUnitID(value.toInt()); break;
			case SC_OUTPUT_SENSOR: signal.setOutputSensorID(value.toInt()); break;
			case SC_ACQUIRE: signal.setAcquire(value.toBool()); break;
			case SC_CALCULATED: signal.setCalculated(value.toBool()); break;
			case SC_NORMAL_STATE: signal.setNormalState(value.toInt()); break;
			case SC_DECIMAL_PLACES: signal.setDecimalPlaces(value.toInt()); break;
			case SC_APERTURE: signal.setAperture(value.toDouble()); break;
			//case SC_IN_OUT_TYPE: signal.setInOutType(value.toInt()); break;
			case SC_IN_OUT_NO: signal.setInOutNo(value.toInt()); break;
			case SC_DEVICE: signal.setDeviceID(value.toInt()); break;
		}

		/*if (added)
		{
			emit signalAdded(signal);
		}
		else
		{
			emit signalChanged(signal);
		}*/
	}

	qDebug() << "setData with role: " << role;

	emit dataChanged(index, index, QVector<int>() << role);

	return true;
}

Qt::ItemFlags SignalsModel::flags(const QModelIndex &index) const
{
	if (index.column() == SC_CHANNEL)
	{
		return QAbstractTableModel::flags(index);
	}
	else
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
}

void SignalsModel::loadSignals()
{
	//QSet<int> signalsIDs;

	if (m_signalIDs.count() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, m_signalIDs.count() - 1);
		m_signalIDs.clear();
		m_signalSet.removeAll();
		endRemoveRows();
	}

	dbController()->getSignalsIDs(&m_signalIDs, m_parentWindow);

	beginInsertRows(QModelIndex(), 0, m_signalIDs.count() - 1);

	if (!dbController()->getSignals(&m_signalSet, m_parentWindow))
	{
		QMessageBox::warning(m_parentWindow, tr("Warning"), tr("Could not load signals"));
	}

	endInsertRows();

	emit cellsSizeChanged();
}

DbController *SignalsModel::dbController()
{
	return m_dbController;
}

//
//
// SignalsTabPage
//
//
SignalsTabPage::SignalsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	/*m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_equipmentView->addAction(m_addSystemAction);
	m_equipmentView->addAction(m_addCaseAction);
	m_equipmentView->addAction(m_addSubblockAction);
	m_equipmentView->addAction(m_addBlockAction);*/

	// Property View
	//
	//m_propertyView = new QTextEdit();
	m_signalsModel = new SignalsModel(dbcontroller, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsModel);
	connect(m_signalsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_signalsView, SLOT(resizeColumnsToContents()));
	connect(m_signalsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_signalsView, SLOT(resizeRowsToContents()));
	connect(m_signalsModel, SIGNAL(cellsSizeChanged()), m_signalsView, SLOT(resizeColumnsToContents()));
	connect(m_signalsModel, SIGNAL(cellsSizeChanged()), m_signalsView, SLOT(resizeRowsToContents()));
	m_signalsView->resizeColumnsToContents();
	m_signalsView->resizeRowsToContents();

	// Splitter
	//
	//m_splitter = new QSplitter();

	//m_splitter->addWidget(m_equipmentView);
	//m_splitter->addWidget(m_propertyView);

	//m_splitter->setStretchFactor(0, 2);
	//m_splitter->setStretchFactor(1, 1);

	//m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	//
	// Layouts
	//

	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_signalsView);

	//pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &SignalsTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	//theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
	//theSettings.writeUserScope();
}

void SignalsTabPage::CreateActions()
{
	/*m_addSystemAction = new QAction(tr("Add System"), this);
	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
	//m_addSystemAction->setEnabled(false);
	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);
	*/
	return;
}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_signalsModel->loadSignals();

	return;
}

void SignalsTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}
