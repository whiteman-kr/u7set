#include "Stable.h"
#include "SignalsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QDialogButtonBox>


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
SC_DEVICE_STR_ID = 29;


const char* Columns[] =
{
	"ID",
	"External ID",
	"Name",
	"Channel",
	"Data format",
	"Data size",
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
	"Input sensor",
	"Output low Limit",
	"Output high Limit",
	"Output unit",
	"Output sensor",
	"Acquire",
	"Calculated",
	"Normal state",
	"Decimal places",
	"Aperture",
	"Input-output type",
	"Device ID",
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);


SignalsDelegate::SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, QObject *parent) :
	QStyledItemDelegate(parent),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo),
	m_signalSet(signalSet)
{

}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();
	switch (col)
	{
		// LineEdit
		//
		case SC_STR_ID:
		{
			QLineEdit* le = new QLineEdit(parent);
			QRegExp rx4ID("^#[A-Za-z][A-Za-z\\d_]*$");
			le->setValidator(new QRegExpValidator(rx4ID, le));
			return le;
		}
		case SC_EXT_STR_ID:
		{
			QLineEdit* le = new QLineEdit(parent);
			QRegExp rx4ExtID("^[A-Za-z][A-Za-z\\d_]*$");
			le->setValidator(new QRegExpValidator(rx4ExtID, le));
			return le;
		}
		case SC_NAME:
		{
			QLineEdit* le = new QLineEdit(parent);
			QRegExp rx4Name("^.+$");
			le->setValidator(new QRegExpValidator(rx4Name, le));
			return le;
		}
		case SC_DATA_SIZE:
		case SC_LOW_ADC:
		case SC_HIGH_ADC:
		case SC_NORMAL_STATE:
		case SC_DECIMAL_PLACES:
		{
			QLineEdit* le = new QLineEdit(parent);
			le->setValidator(new QIntValidator(le));
			return le;
		}
		case SC_LOW_LIMIT:
		case SC_HIGH_LIMIT:
		case SC_ADJUSTMENT:
		case SC_DROP_LIMIT:
		case SC_EXCESS_LIMIT:
		case SC_UNBALANCE_LIMIT:
		case SC_INPUT_LOW_LIMIT:
		case SC_INPUT_HIGH_LIMIT:
		case SC_OUTPUT_LOW_LIMIT:
		case SC_OUTPUT_HIGH_LIMIT:
		case SC_APERTURE:
		{
			QLineEdit* le = new QLineEdit(parent);
			le->setValidator(new QDoubleValidator(le));
			return le;
		}
		case SC_DEVICE_STR_ID:
		{
			QLineEdit* le = new QLineEdit(parent);
			return le;
		}
		// ComboBox
		//
		case SC_DATA_FORMAT:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(m_dataFormatInfo.toList());
			cb->showPopup();
			return cb;
		}
		case SC_UNIT:
		case SC_INPUT_UNIT:
		case SC_OUTPUT_UNIT:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(m_unitInfo.toList());
			cb->showPopup();
			return cb;
		}
		case SC_INPUT_SENSOR:
		case SC_OUTPUT_SENSOR:
		{
			QComboBox* cb = new QComboBox(parent);
			for (int i = 0; i < SENSOR_TYPE_COUNT; i++)
			{
				cb->addItem(SensorTypeStr[i]);
			}
			cb->showPopup();
			return cb;
		}
		case SC_ACQUIRE:
		case SC_CALCULATED:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(QStringList() << tr("No") << tr("Yes"));
			cb->showPopup();
			return cb;
		}
		case SC_IN_OUT_TYPE:
		{
			QComboBox* cb = new QComboBox(parent);
			for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
			{
				cb->addItem(InOutTypeStr[i]);
			}
			cb->showPopup();
			return cb;
		}
		case SC_CHANNEL:
		default:
			assert(false);
			return QStyledItemDelegate::createEditor(parent, option, index);
	}
}

void SignalsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	int col = index.column();
	int row = index.row();
	if (row >= m_signalSet.count())
	{
		return;
	}
	QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
	QComboBox* cb = dynamic_cast<QComboBox*>(editor);
	switch (col)
	{
		// LineEdit
		//
		case SC_STR_ID: if (le) le->setText(m_signalSet[row].strID()); break;
		case SC_EXT_STR_ID: if (le) le->setText(m_signalSet[row].extStrID()); break;
		case SC_NAME: if (le) le->setText(m_signalSet[row].name()); break;
		case SC_DEVICE_STR_ID: if (le) le->setText(m_signalSet[row].deviceStrID()); break;

		case SC_DATA_SIZE: if (le) le->setText(QString::number(m_signalSet[row].dataSize())); break;
		case SC_LOW_ADC: if (le) le->setText(QString::number(m_signalSet[row].dataSize())); break;
		case SC_HIGH_ADC: if (le) le->setText(QString::number(m_signalSet[row].dataSize())); break;
		case SC_NORMAL_STATE: if (le) le->setText(QString::number(m_signalSet[row].dataSize())); break;
		case SC_DECIMAL_PLACES: if (le) le->setText(QString::number(m_signalSet[row].dataSize())); break;

		case SC_LOW_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].lowLimit())); break;
		case SC_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].highLimit())); break;
		case SC_ADJUSTMENT: if (le) le->setText(QString("%1").arg(m_signalSet[row].adjustment())); break;
		case SC_DROP_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].dropLimit())); break;
		case SC_EXCESS_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].excessLimit())); break;
		case SC_UNBALANCE_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].unbalanceLimit())); break;
		case SC_INPUT_LOW_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].inputLowLimit())); break;
		case SC_INPUT_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].inputHighLimit())); break;
		case SC_OUTPUT_LOW_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].outputLowLimit())); break;
		case SC_OUTPUT_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(m_signalSet[row].outputHighLimit())); break;
		case SC_APERTURE: if (le) le->setText(QString("%1").arg(m_signalSet[row].aperture())); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) cb->setCurrentIndex(m_dataFormatInfo.keyIndex(m_signalSet[row].dataFormat())); break;
		case SC_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(m_signalSet[row].unitID())); break;
		case SC_INPUT_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(m_signalSet[row].inputUnitID())); break;
		case SC_OUTPUT_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(m_signalSet[row].outputUnitID())); break;
		case SC_INPUT_SENSOR: if (cb) cb->setCurrentIndex(m_signalSet[row].inputSensorID()); break;
		case SC_OUTPUT_SENSOR: if (cb) cb->setCurrentIndex(m_signalSet[row].outputSensorID()); break;
		case SC_ACQUIRE: if (cb) cb->setCurrentIndex(m_signalSet[row].acquire()); break;
		case SC_CALCULATED: if (cb) cb->setCurrentIndex(m_signalSet[row].calculated()); break;
		case SC_IN_OUT_TYPE: if (cb) cb->setCurrentIndex(m_signalSet[row].inOutType()); break;
		case SC_CHANNEL:
		default:
			assert(false);
	}
}

void SignalsDelegate::setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const
{

}

void SignalsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
	editor->setGeometry(option.rect);
}


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
	return m_signalSet.count();
}

int SignalsModel::columnCount(const QModelIndex &) const
{
	return COLUMNS_COUNT;
}


QString SignalsModel::getUnitStr(int unitID) const
{
	if (m_unitInfo.contains(unitID))
	{
		return m_unitInfo.value(unitID);
	}

	return tr("Unknown unit");
}


QVariant SignalsModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (row == m_signalSet.count())
	{
		return QVariant();
	}

	const Signal& signal = m_signalSet[row];

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (col)
		{
			case SC_STR_ID: return signal.strID();
			case SC_EXT_STR_ID: return signal.extStrID();
			case SC_NAME: return signal.name();
			case SC_CHANNEL: return signal.channel();
			case SC_DATA_FORMAT:
				if (m_dataFormatInfo.contains(signal.dataFormat()))
				{
					return m_dataFormatInfo.value(signal.dataFormat());
				}
				else
				{
					return tr("Unknown data format");
				}

			case SC_DATA_SIZE: return signal.dataSize();
			case SC_LOW_ADC: return QString("0x%1").arg(signal.lowADC(), 4, 16, QChar('0'));
			case SC_HIGH_ADC: return QString("0x%1").arg(signal.highADC(), 4, 16, QChar('0'));
			case SC_LOW_LIMIT: return signal.lowLimit();
			case SC_HIGH_LIMIT: return signal.highLimit();
			case SC_UNIT:
				return getUnitStr(signal.unitID());

			case SC_ADJUSTMENT: return signal.adjustment();
			case SC_DROP_LIMIT: return signal.dropLimit();
			case SC_EXCESS_LIMIT: return signal.excessLimit();
			case SC_UNBALANCE_LIMIT: return signal.unbalanceLimit();
			case SC_INPUT_LOW_LIMIT: return signal.inputLowLimit();
			case SC_INPUT_HIGH_LIMIT: return signal.inputHighLimit();
			case SC_INPUT_UNIT:
				return getUnitStr(signal.inputUnitID());

			case SC_INPUT_SENSOR: return SensorTypeStr[signal.inputSensorID()];
			case SC_OUTPUT_LOW_LIMIT: return signal.outputLowLimit();
			case SC_OUTPUT_HIGH_LIMIT: return signal.outputHighLimit();
			case SC_OUTPUT_UNIT:
				return getUnitStr(signal.outputUnitID());

			case SC_OUTPUT_SENSOR: return SensorTypeStr[signal.outputSensorID()];
			case SC_ACQUIRE: return signal.acquire() ? "Yes" : "No";
			case SC_CALCULATED: return signal.calculated() ? "Yes" : "No";
			case SC_NORMAL_STATE: return signal.normalState();
			case SC_DECIMAL_PLACES: return signal.decimalPlaces();
			case SC_APERTURE: return signal.aperture();
			case SC_IN_OUT_TYPE: return signal.inOutType();
			case SC_DEVICE_STR_ID: return signal.deviceStrID();

			default:
				assert(false);
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
			if (section < m_signalSet.count())
			{
				return m_signalSet.key(section);
			}
		}
	}
	return QVariant();
}

bool SignalsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		int row = index.row();

		assert(row < m_signalSet.count());

		Signal& signal = m_signalSet[index.row()];

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
			case SC_DROP_LIMIT: signal.setDropLimit(value.toDouble()); break;
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
			case SC_IN_OUT_TYPE: signal.setInOutType(SignalInOutType(value.toInt())); break;
			case SC_DEVICE_STR_ID: signal.setDeviceStrID(value.toString()); break;
			default:
				assert(false);
		}

		emit dataChanged(index, index, QVector<int>() << Qt::EditRole << Qt::DisplayRole);
	}
	else
	{
		QAbstractTableModel::setData(index, value, role);
	}

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
	if (m_signalSet.count() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, m_signalSet.count() - 1);
		m_signalSet.clear();
		endRemoveRows();
	}

	//dbController()->getSignalsIDs(&m_signalIDs, m_parentWindow);
	dbController()->getDataFormats(&m_dataFormatInfo, m_parentWindow);
	dbController()->getUnits(&m_unitInfo, m_parentWindow);

	if (!dbController()->getSignals(&m_signalSet, m_parentWindow))
	{
		QMessageBox::warning(m_parentWindow, tr("Warning"), tr("Could not load signals"));
	}

	if (m_signalSet.count() > 0)
	{
		beginInsertRows(QModelIndex(), 0, m_signalSet.count() - 1);
		endInsertRows();

		emit cellsSizeChanged();
	}
}

void SignalsModel::addSignal()
{
	QDialog signalTypeDialog;
	QFormLayout* fl = new QFormLayout(&signalTypeDialog);

	QComboBox* signalTypeCombo = new QComboBox(&signalTypeDialog);
	signalTypeCombo->addItems(QStringList() << tr("Analog") << tr("Discrete"));
	signalTypeCombo->setCurrentIndex(0);

	fl->addRow(tr("Signal type"), signalTypeCombo);

	QLineEdit* signalChannelCount = new QLineEdit(&signalTypeDialog);
	signalChannelCount->setText("1");
	QRegExp rx("[1-9]\\d{0,1}");
	QValidator *validator = new QRegExpValidator(rx, &signalTypeDialog);
	signalChannelCount->setValidator(validator);

	fl->addRow(tr("Signal channel count"), signalChannelCount);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::accepted, &signalTypeDialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &signalTypeDialog, &QDialog::reject);

	fl->addRow(buttonBox);

	signalTypeDialog.setLayout(fl);

	if (signalTypeDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	int channelCount = signalChannelCount->text().toInt();

	Signal signal;

	SignalPropertiesDialog dlg(signal, SignalType(signalTypeCombo->currentIndex()), m_dataFormatInfo, m_unitInfo, m_parentWindow);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<Signal> signalVector;
		for (int i = 0; i < channelCount; i++)
		{
			signalVector << signal;
			if (channelCount > 1)
			{
				signalVector[i].setStrID((signal.strID() + "_%1").arg(QChar('A' + i)));
			}
		}
		if (dbController()->addSignal(SignalType(signalTypeCombo->currentIndex()), &signalVector, m_parentWindow))
		{
			beginInsertRows(QModelIndex(), m_signalSet.count(), m_signalSet.count() + signalVector.count() - 1);
			for (int i = 0; i < signalVector.count(); i++)
			{
				m_signalSet.append(signalVector[i].ID(), signalVector[i]);
			}
			endInsertRows();
			emit cellsSizeChanged();
		}
	}
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

	// Set context menu to Equipment View
	//
	/*m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_equipmentView->addAction(m_addSystemAction);
	m_equipmentView->addAction(m_addCaseAction);
	m_equipmentView->addAction(m_addSubblockAction);
	m_equipmentView->addAction(m_addBlockAction);*/

	// Property View
	//
	m_signalsModel = new SignalsModel(dbcontroller, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsModel);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	//m_signalsView->verticalHeader()->doubleClicked();
	connect(m_signalsModel, &SignalsModel::dataChanged, m_signalsView, &QTableView::resizeColumnsToContents);
	connect(m_signalsModel, &SignalsModel::dataChanged, m_signalsView, &QTableView::resizeRowsToContents);
	connect(m_signalsModel, &SignalsModel::cellsSizeChanged, m_signalsView, &QTableView::resizeColumnsToContents);
	connect(m_signalsModel, &SignalsModel::cellsSizeChanged, m_signalsView, &QTableView::resizeRowsToContents);
	m_signalsView->resizeColumnsToContents();
	m_signalsView->resizeRowsToContents();

	// Create Actions
	//
	CreateActions();

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
	QAction* action = new QAction(tr("Create signal"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::addSignal);
	m_signalsView->addAction(action);
	/*m_signalsView->addAction(menu->addAction(new QAction("Edit signal", this)));
	m_signalsView->addAction(menu->addAction(new QAction("Delete signal", this)));
	m_signalsView->addAction(menu->addAction(new QAction("Restore signal", this)));*/
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

void SignalsTabPage::contextMenuRequested(QPoint)
{
	//menu->popup(table->viewport()->mapToGlobal(pos));
}
