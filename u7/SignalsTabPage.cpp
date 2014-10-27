#include "Stable.h"
#include "SignalsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QDesktopWidget>
#include <QCheckBox>
#include <QPlainTextEdit>

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
SC_DEVICE_STR_ID = 29,
SC_LAST_CHANGE_USER = 30;


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
	"Last change user",
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);



SignalsDelegate::SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, SignalsModel* model, QObject *parent) :
	QStyledItemDelegate(parent),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo),
	m_signalSet(signalSet),
	m_model(model)
{

}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();

	if (!m_model->checkoutSignal(index.row()))
	{
		return nullptr;
	}

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
		case SC_LOW_ADC:
		case SC_HIGH_ADC:
		case SC_NORMAL_STATE:
		case SC_DECIMAL_PLACES:
		{
			QLineEdit* le = new QLineEdit(parent);
			le->setValidator(new QIntValidator(le));
			return le;
		}
		case SC_DATA_SIZE:
		{
			if (m_signalSet.count() > index.row() && m_signalSet[index.row()].type() == SignalType::discrete)
			{
				return nullptr;
			}
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
			return cb;
		}
		case SC_UNIT:
		case SC_INPUT_UNIT:
		case SC_OUTPUT_UNIT:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(m_unitInfo.toList());
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
			return cb;
		}
		case SC_ACQUIRE:
		case SC_CALCULATED:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(QStringList() << tr("No") << tr("Yes"));
			return cb;
		}
		case SC_IN_OUT_TYPE:
		{
			QComboBox* cb = new QComboBox(parent);
			for (int i = 0; i < IN_OUT_TYPE_COUNT; i++)
			{
				cb->addItem(InOutTypeStr[i]);
			}
			return cb;
		}
		case SC_LAST_CHANGE_USER:
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
		case SC_LOW_ADC: if (le) le->setText(QString::number(m_signalSet[row].lowADC())); break;
		case SC_HIGH_ADC: if (le) le->setText(QString::number(m_signalSet[row].highADC())); break;
		case SC_NORMAL_STATE: if (le) le->setText(QString::number(m_signalSet[row].normalState())); break;
		case SC_DECIMAL_PLACES: if (le) le->setText(QString::number(m_signalSet[row].decimalPlaces())); break;

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
		case SC_LAST_CHANGE_USER:
		case SC_CHANNEL:
		default:
			assert(false);
	}
}

void SignalsDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
	int col = index.column();
	int row = index.row();
	if (row >= m_signalSet.count())
	{
		return;
	}
	QLineEdit* le = dynamic_cast<QLineEdit*>(editor);
	QComboBox* cb = dynamic_cast<QComboBox*>(editor);
	Signal& s = m_signalSet[row];
	switch (col)
	{
		// LineEdit
		//
		case SC_STR_ID: if (le) s.setStrID(le->text()); break;
		case SC_EXT_STR_ID: if (le) s.setExtStrID(le->text()); break;
		case SC_NAME: if (le) s.setName(le->text()); break;
		case SC_DEVICE_STR_ID: if (le) s.setDeviceStrID(le->text()); break;

		case SC_DATA_SIZE: if (le) s.setDataSize(le->text().toInt()); break;
		case SC_LOW_ADC: if (le) s.setLowADC(le->text().toInt()); break;
		case SC_HIGH_ADC: if (le) s.setHighADC(le->text().toInt()); break;
		case SC_NORMAL_STATE: if (le) s.setNormalState(le->text().toInt()); break;
		case SC_DECIMAL_PLACES: if (le) s.setDecimalPlaces(le->text().toInt()); break;

		case SC_LOW_LIMIT: if (le) s.setLowLimit(le->text().toDouble()); break;
		case SC_HIGH_LIMIT: if (le) s.setHighLimit(le->text().toDouble()); break;
		case SC_ADJUSTMENT: if (le) s.setAdjustment(le->text().toDouble()); break;
		case SC_DROP_LIMIT: if (le) s.setDropLimit(le->text().toDouble()); break;
		case SC_EXCESS_LIMIT: if (le) s.setExcessLimit(le->text().toDouble()); break;
		case SC_UNBALANCE_LIMIT: if (le) s.setUnbalanceLimit(le->text().toDouble()); break;
		case SC_INPUT_LOW_LIMIT: if (le) s.setInputLowLimit(le->text().toDouble()); break;
		case SC_INPUT_HIGH_LIMIT: if (le) s.setInputHighLimit(le->text().toDouble()); break;
		case SC_OUTPUT_LOW_LIMIT: if (le) s.setOutputLowLimit(le->text().toDouble()); break;
		case SC_OUTPUT_HIGH_LIMIT: if (le) s.setOutputHighLimit(le->text().toDouble()); break;
		case SC_APERTURE: if (le) s.setAperture(le->text().toDouble()); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) s.setDataFormat(m_dataFormatInfo.key(cb->currentIndex())); break;
		case SC_UNIT: if (cb) s.setUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_INPUT_UNIT: if (cb) s.setInputUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_OUTPUT_UNIT: if (cb) s.setOutputUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_INPUT_SENSOR: if (cb) s.setInputSensorID(cb->currentIndex()); break;
		case SC_OUTPUT_SENSOR: if (cb) s.setOutputSensorID(cb->currentIndex()); break;
		case SC_ACQUIRE: if (cb) s.setAcquire(cb->currentIndex() == 0 ? false : true); break;
		case SC_CALCULATED: if (cb) s.setCalculated(cb->currentIndex() == 0 ? false : true); break;
		case SC_IN_OUT_TYPE: if (cb) s.setInOutType(SignalInOutType(cb->currentIndex())); break;
		case SC_LAST_CHANGE_USER:
		case SC_CHANNEL:
		default:
			assert(false);
			return;
	}

	ObjectState state;
	m_model->dbController()->setSignalWorkcopy(&s, &state, m_model->parrentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		m_model->showError(state);
	}
}

void SignalsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
	editor->setGeometry(option.rect);
	QComboBox* cb = dynamic_cast<QComboBox*>(editor);
	if (cb)
	{
		cb->showPopup();
	}
}

bool SignalsDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &index)
{
	if (event->type() == QEvent::MouseButtonDblClick)
	{
		QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
		if (mouseEvent == nullptr)
		{
			assert(false);
			return false;
		}
		emit itemDoubleClicked(index.row());
		return true;
	}
	return false;
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

QString SignalsModel::getSensorStr(int sensorID) const
{
	if (sensorID >= 0 && sensorID < SENSOR_TYPE_COUNT)
	{
		return SensorTypeStr[sensorID];
	}
	else
	{
		return tr("Unknown sensor");
	}
}

QString SignalsModel::getUserStr(int userID) const
{
	if (m_usernameMap.contains(userID))
	{
		return m_usernameMap[userID];
	}
	else
	{
		return tr("Unknown user ID = %1").arg(userID);
	}
}

void SignalsModel::changeCheckedoutSignalActionsVisibility()
{
	for (int i = 0; i < m_signalSet.count(); i++)
	{
		const Signal& signal = m_signalSet[i];
		if (signal.checkedOut() && signal.userID() == dbController()->currentUser().userId())
		{
			emit setCheckedoutSignalActionsVisibility(true);
			return;
		}
	}
	emit setCheckedoutSignalActionsVisibility(false);
}

bool SignalsModel::checkoutSignal(int index)
{
	Signal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	dbController()->checkoutSignals(&signalsIDs, &objectStates, parrentWindow());
	if (objectStates.count() == 0)
	{
		return false;
	}
	showErrors(objectStates);
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_ALREADY_CHECKED_OUT
				&& objectState.userId != dbController()->currentUser().userId())
		{
			return false;
		}
	}
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

QString SignalsModel::errorMessage(const ObjectState& state) const
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 could not be checked out").arg(state.id);
		case ERR_SIGNAL_ALREADY_CHECKED_OUT: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_usernameMap[state.userId]);
		case ERR_SIGNAL_DELETED: return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND: return tr("Signal %1 not found").arg(state.id);
		default:
			assert(false);
			return tr("Unknown error %1").arg(state.errCode);
	}
}

void SignalsModel::showError(const ObjectState& state) const
{
	if (state.errCode != ERR_SIGNAL_OK)
	{
		QMessageBox::critical(m_parentWindow, tr("Error"), errorMessage(state));
	}
}

void SignalsModel::showErrors(const QVector<ObjectState>& states) const
{
	QString message;
	foreach (const ObjectState& state, states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(state) + "\n";
		}
	}
	if (!message.isEmpty())
	{
		QMessageBox::critical(m_parentWindow, tr("Error"), message);
	}
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
			case SC_LAST_CHANGE_USER: return getUserStr(signal.userID());
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
			case SC_UNIT: return getUnitStr(signal.unitID());

			case SC_ADJUSTMENT: return signal.adjustment();
			case SC_DROP_LIMIT: return signal.dropLimit();
			case SC_EXCESS_LIMIT: return signal.excessLimit();
			case SC_UNBALANCE_LIMIT: return signal.unbalanceLimit();

			case SC_INPUT_LOW_LIMIT: return signal.inputLowLimit();
			case SC_INPUT_HIGH_LIMIT: return signal.inputHighLimit();
			case SC_INPUT_UNIT: return getUnitStr(signal.inputUnitID());
			case SC_INPUT_SENSOR: return getSensorStr(signal.inputSensorID());

			case SC_OUTPUT_LOW_LIMIT: return signal.outputLowLimit();
			case SC_OUTPUT_HIGH_LIMIT: return signal.outputHighLimit();
			case SC_OUTPUT_UNIT: return getUnitStr(signal.outputUnitID());
			case SC_OUTPUT_SENSOR: return getSensorStr(signal.outputSensorID());

			case SC_ACQUIRE: return signal.acquire() ? "Yes" : "No";
			case SC_CALCULATED: return signal.calculated() ? "Yes" : "No";
			case SC_NORMAL_STATE: return signal.normalState();
			case SC_DECIMAL_PLACES: return signal.decimalPlaces();
			case SC_APERTURE: return signal.aperture();
			case SC_IN_OUT_TYPE: return (signal.inOutType() < IN_OUT_TYPE_COUNT) ? InOutTypeStr[signal.inOutType()] : tr("Unknown type");
			case SC_DEVICE_STR_ID: return signal.deviceStrID();

			default:
				assert(false);
		}
	}

	return QVariant();
}

QVariant SignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Vertical && role == Qt::DecorationRole)
	{
		const Signal& signal = m_signalSet[section];
		if (signal.checkedOut())
		{
			if (signal.userID() == dbController()->currentUser().userId())
			{
				switch (signal.instanceAction())
				{
					case InstanceAction::added: return plus;
					case InstanceAction::modified: return pencil;
					case InstanceAction::deleted: return cross;
					default:
						assert(false);
						return QVariant();
				}
			}
			else
			{
				return lock;
			}
		}
	}
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
			case SC_LAST_CHANGE_USER:
			case SC_CHANNEL:
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
	if (index.column() == SC_CHANNEL || index.column() == SC_LAST_CHANGE_USER)
	{
		return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
	}
	else
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
}

void SignalsModel::loadSignals()
{
	bool signalsCleared = false;
	if (m_signalSet.count() != 0)
	{
		signalsCleared = true;
		emit aboutToClearSignals();
	}
	clearSignals();

	std::vector<DbUser> list;
	m_dbController->getUserList(&list, m_parentWindow);
	m_usernameMap.clear();
	for (int i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}

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

		if (signalsCleared)
		{
			emit signalsRestored();
		}

		emit cellsSizeChanged();
	}

	changeCheckedoutSignalActionsVisibility();
}

void SignalsModel::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		beginRemoveRows(QModelIndex(), 0, m_signalSet.count() - 1);
		m_signalSet.clear();
		endRemoveRows();
	}

	m_dataFormatInfo.clear();
	m_unitInfo.clear();

	emit cellsSizeChanged();
}

QVector<int> SignalsModel::getSameChannelSignals(int row)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[row].signalGroupID() != 0)
	{
		QVector<int>& sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[row].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(m_signalSet.keyIndex(id));
		}
	}
	else
	{
		sameChannelSignalRows.append(row);
	}
	return sameChannelSignalRows;
}

bool SignalsModel::isEditableSignal(int row)
{
	Signal& s = m_signalSet[row];
	if (s.checkedOut() && s.userID() != dbController()->currentUser().userId())
	{
		return false;
	}
	return true;
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

bool SignalsModel::editSignal(int row)
{
	if (!checkoutSignal(row))
	{
		return false;
	}

	Signal signal = m_signalSet[row];
	SignalPropertiesDialog dlg(signal, signal.type(), m_dataFormatInfo, m_unitInfo, m_parentWindow);

	if (dlg.exec() == QDialog::Accepted)
	{
		m_signalSet[row]= signal;

		ObjectState state;
		dbController()->setSignalWorkcopy(&signal, &state, parrentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			showError(state);
		}

		loadSignals();
		return true;
	}
	else
	{
		// Because signals was checkedout and should be updated
		//
		loadSignals();
	}
	return false;
}

void SignalsModel::deleteSignalGroups(const QSet<int>& signalGroupIDs)
{
	foreach (const int groupID, signalGroupIDs)
	{
		QVector<int> signalIDs = m_signalSet.getChannelSignalsID(groupID);
		foreach (const int signalID, signalIDs)
		{
			deleteSignal(signalID);
		}
	}
	loadSignals();
}

void SignalsModel::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, parrentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

DbController *SignalsModel::dbController()
{
	return m_dbController;
}

const DbController *SignalsModel::dbController() const
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

	QToolBar* toolBar = new QToolBar(this);

	// Property View
	//
	m_signalsModel = new SignalsModel(dbcontroller, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	SignalsDelegate* delegate = m_signalsModel->createDelegate();
	m_signalsView->setItemDelegate(delegate);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(m_signalsModel, &SignalsModel::dataChanged, m_signalsView, &QTableView::resizeColumnsToContents);
	connect(m_signalsModel, &SignalsModel::dataChanged, m_signalsView, &QTableView::resizeRowsToContents);
	connect(m_signalsModel, &SignalsModel::cellsSizeChanged, m_signalsView, &QTableView::resizeColumnsToContents);
	connect(m_signalsModel, &SignalsModel::cellsSizeChanged, m_signalsView, &QTableView::resizeRowsToContents);
	connect(m_signalsView->itemDelegate(), &SignalsDelegate::closeEditor, m_signalsView, &QTableView::resizeColumnsToContents);
	connect(m_signalsView->itemDelegate(), &SignalsDelegate::closeEditor, m_signalsView, &QTableView::resizeRowsToContents);
	connect(delegate, &SignalsDelegate::itemDoubleClicked, m_signalsModel, &SignalsModel::editSignal);
	connect(delegate, &SignalsDelegate::closeEditor, m_signalsModel, &SignalsModel::loadSignals);

	connect(m_signalsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SignalsTabPage::changeSignalActionsVisibility);

	connect(m_signalsModel, &SignalsModel::aboutToClearSignals, this, &SignalsTabPage::saveSelection);
	connect(m_signalsModel, &SignalsModel::signalsRestored, this, &SignalsTabPage::restoreSelection);

	m_signalsView->resizeColumnsToContents();
	m_signalsView->resizeRowsToContents();

	// Create Actions
	//
	CreateActions(toolBar);

	//
	// Layouts
	//

	QVBoxLayout* pMainLayout = new QVBoxLayout();

	pMainLayout->addWidget(toolBar);
	pMainLayout->addWidget(m_signalsView);

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
}

void SignalsTabPage::CreateActions(QToolBar *toolBar)
{
	QAction* action = new QAction(tr("Refresh signal list"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::loadSignals);
	toolBar->addAction(action);

	action = new QAction(tr("Create signal"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::addSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(tr("Edit signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::editSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(tr("Delete signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(tr("Undo signal changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(tr("Show pending changes..."), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::showPendingChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	changeSignalActionsVisibility();
}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);

	m_signalsModel->loadSignals();
}

void SignalsTabPage::projectClosed()
{
	this->setEnabled(false);

	m_signalsModel->clearSignals();
}

void SignalsTabPage::editSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
    if (selection.count() == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
    }
    for (int i = 0; i < selection.count(); i++)
    {
		if (!m_signalsModel->editSignal(selection[i].row()))
		{
			break;
		}
	}
}

void SignalsTabPage::deleteSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
    if (selection.count() == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
    }
	QSet<int> deletedSignalGroupIDs;
    for (int i = 0; i < selection.count(); i++)
    {
		int row = selection[i].row();
		int groupId = m_signalsModel->signal(row).signalGroupID();
		if (groupId != 0)
		{
			deletedSignalGroupIDs.insert(groupId);
		}
		else
		{
			m_signalsModel->deleteSignal(m_signalsModel->key(row));
		}
	}
	m_signalsModel->deleteSignalGroups(deletedSignalGroupIDs);
}

void SignalsTabPage::undoSignalChanges()
{
	UndoSignalsDialog dlg(m_signalsModel, this);

	dlg.setCheckStates(m_signalsView->selectionModel()->selectedRows(), true);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::showPendingChanges()
{
	CheckinSignalsDialog dlg(m_signalsModel, m_signalsView->selectionModel()->selectedRows(), this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::changeSignalActionsVisibility()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
    if (selection.count() == 0)
    {
		emit setSignalActionsVisibility(false);
    }
	else
	{
		for (int i = 0; i < selection.count(); i++)
		{
			if (m_signalsModel->isEditableSignal(selection[i].row()))
			{
				emit setSignalActionsVisibility(true);
				return;
			}
		}
		emit setSignalActionsVisibility(false);
	}
}

void SignalsTabPage::saveSelection()
{
	// Save signal id list of selected rows and signal id with column number of focused cell
	//
	selectedRowsSignalID.clear();
	QModelIndexList& selectedList = m_signalsView->selectionModel()->selectedRows(0);
	foreach (const QModelIndex& index, selectedList)
	{
		selectedRowsSignalID.append(m_signalsModel->key(index.row()));
	}
	QModelIndex index = m_signalsView->currentIndex();
	focusedCellSignalID = m_signalsModel->key(index.row());
	focusedCellColumn = index.column();
	horizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	verticalScrollPosition = m_signalsView->verticalScrollBar()->value();
}

void SignalsTabPage::restoreSelection()
{
	foreach (int id, selectedRowsSignalID)
	{
		m_signalsView->selectRow(m_signalsModel->getKeyIndex(id));
	}
	m_signalsView->setCurrentIndex(m_signalsModel->index(m_signalsModel->getKeyIndex(focusedCellSignalID), focusedCellColumn));
	m_signalsView->horizontalScrollBar()->setValue(horizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(verticalScrollPosition);
}


CheckedoutSignalsModel::CheckedoutSignalsModel(SignalsModel* sourceModel, QObject* parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{
	setSourceModel(sourceModel);
	states.resize(rowCount());
}

QVariant CheckedoutSignalsModel::data(const QModelIndex& index, int role) const
{
	if (index.column() == SC_STR_ID && role == Qt::CheckStateRole)
	{
		return states[index.row()];
	}
	return QSortFilterProxyModel::data(index, role);
}

bool CheckedoutSignalsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.column() == SC_STR_ID && role == Qt::CheckStateRole)
	{
		setCheckState(index.row(), Qt::CheckState(value.toInt()));
		return true;
	}
	return QSortFilterProxyModel::setData(index, value, role);
}

Qt::ItemFlags CheckedoutSignalsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);
	flags &= ~Qt::ItemIsEditable;
	if (index.column() == SC_STR_ID)
	{
		flags |= Qt::ItemIsUserCheckable;
	}
	return flags;
}

bool CheckedoutSignalsModel::filterAcceptsRow(int source_row, const QModelIndex&) const
{
	const Signal& signal = m_sourceModel->signal(source_row);
	return signal.checkedOut() && signal.userID() == m_sourceModel->dbController()->currentUser().userId();
}

void CheckedoutSignalsModel::initCheckStates(const QModelIndexList& list, bool fromSourceModel)
{
	for (int i = 0; i < list.count(); i++)
	{
		QModelIndex proxyIndex = fromSourceModel ? mapFromSource(list[i]) : list[i];
		if (proxyIndex.isValid())
		{
			setCheckState(proxyIndex.row(), Qt::Checked);
		}
	}
}

void CheckedoutSignalsModel::setAllCheckStates(bool state)
{
	for (int i = 0; i < states.count(); i++)
	{
		states[i] = state ? Qt::Checked : Qt::Unchecked;
	}
	emit dataChanged(index(0, 0), index(states.count() - 1, 0), QVector<int>() << Qt::CheckStateRole);
}

void CheckedoutSignalsModel::setCheckState(int row, Qt::CheckState state)
{
	QVector<int>& sourceRows = m_sourceModel->getSameChannelSignals(mapToSource(index(row, 0)).row());
	foreach (const int sourceRow, sourceRows)
	{
		QModelIndex changedIndex = mapFromSource(m_sourceModel->index(sourceRow, 0));
		if (!changedIndex.isValid())
		{
			continue;
		}
		states[changedIndex.row()] = state;
		emit dataChanged(changedIndex, changedIndex, QVector<int>() << Qt::CheckStateRole);
	}
}


CheckinSignalsDialog::CheckinSignalsDialog(SignalsModel *sourceModel, QModelIndexList selection, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Pending changes"));

	m_splitter = new QSplitter(Qt::Vertical, this);

	QVBoxLayout* vl1 = new QVBoxLayout;
	QVBoxLayout* vl2 = new QVBoxLayout;
	vl2->setMargin(0);

	m_proxyModel = new CheckedoutSignalsModel(sourceModel, this);

	if (selection.count() != 0)
	{
		m_proxyModel->initCheckStates(selection);
	}

	m_commentEdit = new QPlainTextEdit(this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl2->addWidget(selectAll);

	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_proxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	m_signalsView->resizeColumnsToContents();
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	QAction* undoAction = new QAction(tr("Undo signal changes"), this);
	connect(undoAction, &QAction::triggered, this, &CheckinSignalsDialog::openUndoDialog);
	m_signalsView->addAction(undoAction);

	vl2->addWidget(m_signalsView);

	vl2->addWidget(new QLabel(tr("Comment:"), this));

	QWidget* w = new QWidget(this);

	w->setLayout(vl2);

	m_splitter->addWidget(w);
	m_splitter->addWidget(m_commentEdit);

	vl1->addWidget(m_splitter);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addStretch();

	QPushButton* checkinSelectedButton = new QPushButton(tr("Checkin"), this);
	connect(checkinSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::checkinSelected);
	hl->addWidget(checkinSelectedButton);

	QPushButton* undoSelectedButton = new QPushButton(tr("Undo"), this);
	connect(undoSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::undoSelected);
	hl->addWidget(undoSelectedButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, &QPushButton::clicked, this, &CheckinSignalsDialog::cancel);
	hl->addWidget(cancelButton);

	vl1->addLayout(hl);

	setLayout(vl1);

	QSettings settings;
	resize(settings.value("Pending changes dialog: size", qApp->desktop()->size() * 3 / 4).toSize());
	m_splitter->setChildrenCollapsible(false);

	QList<int> list = m_splitter->sizes();
	list[0] = height();
	list[1] = m_commentEdit->height();
	m_splitter->setSizes(list);

	m_splitter->restoreState(settings.value("Pending changes dialog: splitter position", m_splitter->saveState()).toByteArray());
}

void CheckinSignalsDialog::checkinSelected()
{
	QString commentText = m_commentEdit->toPlainText();
	if (commentText.isEmpty())
	{
		QMessageBox::warning(m_sourceModel->parrentWindow(), tr("Warning"), tr("Checkin comment is empty"));
		return;
	}
	QVector<int> IDs;
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, SC_STR_ID);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << m_sourceModel->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parrentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	states.resize(IDs.size());
	m_sourceModel->dbController()->checkinSignals(&IDs, commentText, &states, this);
	m_sourceModel->showErrors(states);

	saveGeometry();

	accept();
}

void CheckinSignalsDialog::undoSelected()
{
	QVector<int> IDs;
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, SC_STR_ID);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << m_sourceModel->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parrentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	foreach (int ID, IDs)
	{
		ObjectState state;
		m_sourceModel->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parrentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}
	if (!states.isEmpty())
	{
		m_sourceModel->showErrors(states);
	}

	saveGeometry();

	accept();
}

void CheckinSignalsDialog::cancel()
{
	saveGeometry();

	reject();
}

void CheckinSignalsDialog::openUndoDialog()
{
	UndoSignalsDialog dlg(m_sourceModel, this);

	dlg.setCheckStates(m_signalsView->selectionModel()->selectedRows(), false);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_sourceModel->loadSignals();
}

void CheckinSignalsDialog::saveGeometry()
{
	QSettings settings;
	settings.setValue("Pending changes dialog: size", size());
	settings.setValue("Pending changes dialog: splitter position", m_splitter->saveState());
}



UndoSignalsDialog::UndoSignalsDialog(SignalsModel* sourceModel, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Undo signals to precheckout state"));

	QSettings settings;
	resize(settings.value("Undo signals dialog: size", qApp->desktop()->size() * 3 / 4).toSize());

	QVBoxLayout* vl = new QVBoxLayout;

	m_proxyModel = new CheckedoutSignalsModel(sourceModel, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl->addWidget(selectAll);

	QTableView* signalsView = new QTableView(this);
	signalsView->setModel(m_proxyModel);
	signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	signalsView->resizeColumnsToContents();
	signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	signalsView->horizontalHeader()->setHighlightSections(false);

	vl->addWidget(signalsView);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &UndoSignalsDialog::undoSelected);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	vl->addWidget(buttonBox);

	setLayout(vl);
}

void UndoSignalsDialog::setCheckStates(QModelIndexList selection, bool fromSourceModel)
{
	if (!selection.isEmpty())
	{
		m_proxyModel->initCheckStates(selection, fromSourceModel);
	}
}

void UndoSignalsDialog::undoSelected()
{
	QVector<int> IDs;
	for (int i = 0; i < m_proxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_proxyModel->index(i, SC_STR_ID);
		if (m_proxyModel->data(proxyIndex, Qt::CheckStateRole) != Qt::Checked)
		{
			continue;
		}
		int sourceRow = m_proxyModel->mapToSource(proxyIndex).row();
		IDs << m_sourceModel->key(sourceRow);
	}
	if (IDs.count() == 0)
	{
		QMessageBox::warning(m_sourceModel->parrentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	foreach (int ID, IDs)
	{
		ObjectState state;
		m_sourceModel->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parrentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}
	if (!states.isEmpty())
	{
		m_sourceModel->showErrors(states);
	}

	QSettings settings;
	settings.setValue("Undo signals dialog: size", size());

	accept();
}
