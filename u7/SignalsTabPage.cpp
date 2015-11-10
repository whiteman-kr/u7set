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
#include <QRadioButton>
#include <QButtonGroup>
#include <QToolButton>
#include <QTimer>

const int SC_STR_ID = 0,
SC_EXT_STR_ID = 1,
SC_NAME = 2,
SC_CHANNEL = 3,
SC_TYPE = 4,
SC_DATA_FORMAT = 5,
SC_DATA_SIZE = 6,
SC_NORMAL_STATE = 7,
SC_ACQUIRE = 8,
SC_IN_OUT_TYPE = 9,
SC_DEVICE_STR_ID = 10,
SC_LOW_ADC = 11,
SC_HIGH_ADC = 12,
SC_LOW_LIMIT = 13,
SC_HIGH_LIMIT = 14,
SC_UNIT = 15,
SC_ADJUSTMENT = 16,
SC_DROP_LIMIT = 17,
SC_EXCESS_LIMIT = 18,
SC_UNBALANCE_LIMIT = 19,
SC_INPUT_LOW_LIMIT = 20,
SC_INPUT_HIGH_LIMIT = 21,
SC_INPUT_UNIT = 22,
SC_INPUT_SENSOR = 23,
SC_OUTPUT_LOW_LIMIT = 24,
SC_OUTPUT_HIGH_LIMIT = 25,
SC_OUTPUT_UNIT = 26,
SC_OUTPUT_RANGE_MODE = 27,
SC_OUTPUT_SENSOR = 28,
SC_CALCULATED = 29,
SC_DECIMAL_PLACES = 30,
SC_APERTURE = 31,
SC_FILTERING_TIME = 32,
SC_MAX_DIFFERENCE = 33,
SC_BYTE_ORDER = 34,
SC_LAST_CHANGE_USER = 35;


const char* Columns[] =
{
	"ID",
	"External ID",
	"Name",
	"Channel",
	"A/D",
	"Data format",
	"Data size",
	"Normal state",
	"Acquire",
	"Input-output type",
	"Device ID",
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
	"Output range mode",
	"Output sensor",
	"Calculated",
	"Decimal places",
	"Aperture",
	"Filtering time",
	"Max difference",
	"Byte order",
	"Last change user",
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);



SignalsDelegate::SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* proxyModel, QObject *parent) :
	QStyledItemDelegate(parent),
	m_dataFormatInfo(dataFormatInfo),
	m_unitInfo(unitInfo),
	m_signalSet(signalSet),
	m_model(model),
	m_proxyModel(proxyModel)
{

}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();

	if (!m_model->checkoutSignal(row))
	{
		return nullptr;
	}

	m_model->loadSignal(row, false);

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
			if (m_signalSet.count() > index.row() && m_signalSet[index.row()].type() == E::SignalType::Discrete)
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
		case SC_FILTERING_TIME:
		case SC_MAX_DIFFERENCE:
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
		case SC_OUTPUT_RANGE_MODE:
		{
			QComboBox* cb = new QComboBox(parent);
			for (int i = 0; i < OUTPUT_RANGE_MODE_COUNT; i++)
			{
				cb->addItem(OutputRangeModeStr[i]);
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
		case SC_BYTE_ORDER:
		{
			QComboBox* cb = new QComboBox(parent);

			auto byteOrderList = E::enumValues<E::ByteOrder>();

			for (auto bo : byteOrderList)
			{
				cb->addItem(bo.second);
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
	int row = m_proxyModel->mapToSource(index).row();
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
		case SC_STR_ID: if (le) le->setText(s.strID()); break;
		case SC_EXT_STR_ID: if (le) le->setText(s.extStrID()); break;
		case SC_NAME: if (le) le->setText(s.name()); break;
		case SC_DEVICE_STR_ID: if (le) le->setText(s.deviceStrID()); break;

		case SC_DATA_SIZE: if (le) le->setText(QString::number(s.dataSize())); break;
		case SC_LOW_ADC: if (le) le->setText(QString::number(s.lowADC())); break;
		case SC_HIGH_ADC: if (le) le->setText(QString::number(s.highADC())); break;
		case SC_NORMAL_STATE: if (le) le->setText(QString::number(s.normalState())); break;
		case SC_DECIMAL_PLACES: if (le) le->setText(QString::number(s.decimalPlaces())); break;

		case SC_LOW_LIMIT: if (le) le->setText(QString("%1").arg(s.lowLimit())); break;
		case SC_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(s.highLimit())); break;
		case SC_ADJUSTMENT: if (le) le->setText(QString("%1").arg(s.adjustment())); break;
		case SC_DROP_LIMIT: if (le) le->setText(QString("%1").arg(s.dropLimit())); break;
		case SC_EXCESS_LIMIT: if (le) le->setText(QString("%1").arg(s.excessLimit())); break;
		case SC_UNBALANCE_LIMIT: if (le) le->setText(QString("%1").arg(s.unbalanceLimit())); break;
		case SC_INPUT_LOW_LIMIT: if (le) le->setText(QString("%1").arg(s.inputLowLimit())); break;
		case SC_INPUT_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(s.inputHighLimit())); break;
		case SC_OUTPUT_LOW_LIMIT: if (le) le->setText(QString("%1").arg(s.outputLowLimit())); break;
		case SC_OUTPUT_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(s.outputHighLimit())); break;
		case SC_APERTURE: if (le) le->setText(QString("%1").arg(s.aperture())); break;
		case SC_FILTERING_TIME: if (le) le->setText(QString("%1").arg(s.filteringTime())); break;
		case SC_MAX_DIFFERENCE: if (le) le->setText(QString("%1").arg(s.maxDifference())); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) cb->setCurrentIndex(m_dataFormatInfo.keyIndex(s.dataFormatInt())); break;
		case SC_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(s.unitID())); break;
		case SC_INPUT_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(s.inputUnitID())); break;
		case SC_OUTPUT_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(s.outputUnitID())); break;
		case SC_INPUT_SENSOR: if (cb) cb->setCurrentIndex(s.inputSensorID()); break;
		case SC_OUTPUT_SENSOR: if (cb) cb->setCurrentIndex(s.outputSensorID()); break;
		case SC_OUTPUT_RANGE_MODE: if (cb) cb->setCurrentIndex(s.outputRangeMode()); break;
		case SC_ACQUIRE: if (cb) cb->setCurrentIndex(s.acquire()); break;
		case SC_CALCULATED: if (cb) cb->setCurrentIndex(s.calculated()); break;
		case SC_IN_OUT_TYPE: if (cb) cb->setCurrentIndex(s.inOutType()); break;
		case SC_BYTE_ORDER: if (cb) cb->setCurrentIndex(s.byteOrderInt()); break;
		case SC_LAST_CHANGE_USER:
		case SC_CHANNEL:
		case SC_TYPE:
		default:
			assert(false);
	}
}

void SignalsDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();
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
		case SC_FILTERING_TIME: if (le) s.setFilteringTime(le->text().toDouble()); break;
		case SC_MAX_DIFFERENCE: if (le) s.setMaxDifference(le->text().toDouble()); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) s.setDataFormat(static_cast<E::DataFormat>(m_dataFormatInfo.key(cb->currentIndex()))); break;
		case SC_UNIT: if (cb) s.setUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_INPUT_UNIT: if (cb) s.setInputUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_OUTPUT_UNIT: if (cb) s.setOutputUnitID(m_unitInfo.key(cb->currentIndex())); break;
		case SC_INPUT_SENSOR: if (cb) s.setInputSensorID(cb->currentIndex()); break;
		case SC_OUTPUT_SENSOR: if (cb) s.setOutputSensorID(cb->currentIndex()); break;
		case SC_OUTPUT_RANGE_MODE: if (cb) s.setOutputRangeMode(OutputRangeMode(cb->currentIndex())); break;
		case SC_ACQUIRE: if (cb) s.setAcquire(cb->currentIndex() == 0 ? false : true); break;
		case SC_CALCULATED: if (cb) s.setCalculated(cb->currentIndex() == 0 ? false : true); break;
		case SC_IN_OUT_TYPE: if (cb) s.setInOutType(SignalInOutType(cb->currentIndex())); break;
		case SC_BYTE_ORDER: if (cb) s.setByteOrder(E::ByteOrder(cb->currentIndex())); break;
		case SC_LAST_CHANGE_USER:
		case SC_CHANNEL:
		case SC_TYPE:
		default:
			assert(false);
			return;
	}

	m_model->saveSignal(s);
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
		emit itemDoubleClicked(m_proxyModel->mapToSource(index).row());
		return true;
	}
	return false;
}


SignalsModel::SignalsModel(DbController* dbController, SignalsTabPage* parent) :
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

QString SignalsModel::getOutputRangeModeStr(int outputRangeMode) const
{
	if (outputRangeMode >= 0 && outputRangeMode < OUTPUT_RANGE_MODE_COUNT)
	{
		return OutputRangeModeStr[outputRangeMode];
	}
	else
	{
		return tr("Unknown output range mode");
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

bool SignalsModel::checkoutSignal(int index, QString& message)
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
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}
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
		if (signal.type() == E::SignalType::Analog)
		{
			switch (col)
			{
				case SC_LAST_CHANGE_USER: return getUserStr(signal.userID());
				case SC_STR_ID: return signal.strID();
				case SC_EXT_STR_ID: return signal.extStrID();
				case SC_NAME: return signal.name();
				case SC_CHANNEL: return signal.channel();
				case SC_TYPE: return QChar('A');
				case SC_DATA_FORMAT:
					if (m_dataFormatInfo.contains(signal.dataFormatInt()))
					{
						return m_dataFormatInfo.value(signal.dataFormatInt());
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
				case SC_OUTPUT_RANGE_MODE: return getOutputRangeModeStr(signal.outputRangeMode());

				case SC_ACQUIRE: return signal.acquire() ? tr("Yes") : tr("No");
				case SC_CALCULATED: return signal.calculated() ? tr("Yes") : tr("No");
				case SC_NORMAL_STATE: return signal.normalState();
				case SC_DECIMAL_PLACES: return signal.decimalPlaces();
				case SC_APERTURE: return signal.aperture();
				case SC_FILTERING_TIME: return signal.filteringTime();
				case SC_MAX_DIFFERENCE: return signal.maxDifference();
				case SC_IN_OUT_TYPE: return (signal.inOutType() < IN_OUT_TYPE_COUNT) ? InOutTypeStr[signal.inOutType()] : tr("Unknown type");
				case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());
				case SC_DEVICE_STR_ID: return signal.deviceStrID();

				default:
					assert(false);
			}
		}
		else
		{
			switch (col)
			{
				case SC_LAST_CHANGE_USER: return getUserStr(signal.userID());
				case SC_STR_ID: return signal.strID();
				case SC_EXT_STR_ID: return signal.extStrID();
				case SC_NAME: return signal.name();
				case SC_CHANNEL: return signal.channel();
				case SC_TYPE: return QChar('D');
				case SC_DATA_FORMAT:
					if (m_dataFormatInfo.contains(signal.dataFormatInt()))
					{
						return m_dataFormatInfo.value(signal.dataFormatInt());
					}
					else
					{
						return tr("Unknown data format");
					}

				case SC_DATA_SIZE: return signal.dataSize();
				case SC_ACQUIRE: return signal.acquire() ? tr("Yes") : tr("No");
				case SC_IN_OUT_TYPE: return (signal.inOutType() < IN_OUT_TYPE_COUNT) ? InOutTypeStr[signal.inOutType()] : tr("Unknown type");
				case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());
				case SC_DEVICE_STR_ID: return signal.deviceStrID();

				case SC_LOW_ADC:
				case SC_HIGH_ADC:
				case SC_LOW_LIMIT:
				case SC_HIGH_LIMIT:
				case SC_UNIT:

				case SC_ADJUSTMENT:
				case SC_DROP_LIMIT:
				case SC_EXCESS_LIMIT:
				case SC_UNBALANCE_LIMIT:

				case SC_INPUT_LOW_LIMIT:
				case SC_INPUT_HIGH_LIMIT:
				case SC_INPUT_UNIT:
				case SC_INPUT_SENSOR:

				case SC_OUTPUT_LOW_LIMIT:
				case SC_OUTPUT_HIGH_LIMIT:
				case SC_OUTPUT_UNIT:
				case SC_OUTPUT_SENSOR:
				case SC_OUTPUT_RANGE_MODE:

				case SC_CALCULATED:
				case SC_NORMAL_STATE:
				case SC_DECIMAL_PLACES:
				case SC_APERTURE:
				case SC_FILTERING_TIME:
				case SC_MAX_DIFFERENCE:
					return QVariant();

				default:
					assert(false);
			}
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
					case InstanceAction::Added: return plus;
					case InstanceAction::Modified: return pencil;
					case InstanceAction::Deleted: return cross;
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
			case SC_DATA_FORMAT: signal.setDataFormat(static_cast<E::DataFormat>(value.toInt())); break;
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
			case SC_OUTPUT_RANGE_MODE: signal.setOutputRangeMode(OutputRangeMode(value.toInt())); break;
			case SC_ACQUIRE: signal.setAcquire(value.toBool()); break;
			case SC_CALCULATED: signal.setCalculated(value.toBool()); break;
			case SC_NORMAL_STATE: signal.setNormalState(value.toInt()); break;
			case SC_DECIMAL_PLACES: signal.setDecimalPlaces(value.toInt()); break;
			case SC_APERTURE: signal.setAperture(value.toDouble()); break;
			case SC_FILTERING_TIME: signal.setFilteringTime(value.toDouble()); break;
			case SC_MAX_DIFFERENCE: signal.setMaxDifference(value.toDouble()); break;
			case SC_IN_OUT_TYPE: signal.setInOutType(SignalInOutType(value.toInt())); break;
			case SC_BYTE_ORDER: signal.setByteOrder(E::ByteOrder(value.toInt())); break;
			case SC_DEVICE_STR_ID: signal.setDeviceStrID(value.toString()); break;
			case SC_LAST_CHANGE_USER:
			case SC_CHANNEL:
			case SC_TYPE:
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
	if (index.column() == SC_CHANNEL || index.column() == SC_LAST_CHANGE_USER || index.column() == SC_TYPE)
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
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}

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

void SignalsModel::loadSignal(int row, bool updateView)
{
	dbController()->getLatestSignal(key(row), &m_signalSet[row], parrentWindow());
	if (updateView)
	{
		emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
	}
}

void SignalsModel::loadSignalSet(QVector<int> keys, bool updateView)
{
	for (int i = 0; i < keys.count(); i++)
	{
		int row = keyIndex(keys[i]);
		dbController()->getLatestSignal(keys[i], &m_signalSet[row], parrentWindow());
		if (updateView)
		{
			emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
		}
	}
}

void SignalsModel::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		beginRemoveRows(QModelIndex(), 0, m_signalSet.count() - 1);
		m_signalSet.clear();
		endRemoveRows();
	}

	m_unitInfo.clear();
}

QVector<int> SignalsModel::getSameChannelSignals(int row)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[row].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[row].signalGroupID());
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

	QLineEdit* deviceIdEdit = new QLineEdit(&signalTypeDialog);
	deviceIdEdit->setText("");

	fl->addRow(tr("Device ID"), deviceIdEdit);

	QComboBox* signalTypeCombo = new QComboBox(&signalTypeDialog);
	signalTypeCombo->addItems(QStringList() << tr("Analog") << tr("Discrete"));
	signalTypeCombo->setCurrentIndex(0);

	fl->addRow(tr("Signal type"), signalTypeCombo);

	QLineEdit* signalChannelCountEdit = new QLineEdit(&signalTypeDialog);
	signalChannelCountEdit->setText("1");
	QRegExp channelRegExp("[1-6]");
	QValidator *validator = new QRegExpValidator(channelRegExp, &signalTypeDialog);
	signalChannelCountEdit->setValidator(validator);

	fl->addRow(tr("Signal channel count"), signalChannelCountEdit);

	QLineEdit* signalCountEdit = new QLineEdit(&signalTypeDialog);
	signalCountEdit->setText("1");
	QRegExp countRegExp("[1-9]\\d{0,3}");
	validator = new QRegExpValidator(countRegExp, &signalTypeDialog);
	signalCountEdit->setValidator(validator);

	fl->addRow(tr("Signal count"), signalCountEdit);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::accepted, &signalTypeDialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &signalTypeDialog, &QDialog::reject);

	fl->addRow(buttonBox);

	signalTypeDialog.setLayout(fl);

	if (signalTypeDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	int channelCount = signalChannelCountEdit->text().toInt();
	int signalCount = signalCountEdit->text().toInt();

	Signal signal;

	QSettings settings;

	if (E::SignalType(signalTypeCombo->currentIndex()) == E::SignalType::Analog)
	{
		signal.setDataFormat(E::DataFormat::Float);
	}
	else
	{
		signal.setDataFormat(static_cast<E::DataFormat>(m_dataFormatInfo.key(settings.value("LastEditedSignal: dataFormat").toInt())));
	}
	signal.setDataSize(settings.value("LastEditedSignal: dataSize").toInt());
	signal.setLowADC(settings.value("LastEditedSignal: lowADC").toInt());
	signal.setHighADC(settings.value("LastEditedSignal: highADC").toInt());
	signal.setLowLimit(settings.value("LastEditedSignal: lowLimit").toDouble());
	signal.setHighLimit(settings.value("LastEditedSignal: highLimit").toDouble());
	int unit = settings.value("LastEditedSignal: unitID").toInt();
	if (unit != -1)
	{
		signal.setUnitID(m_unitInfo.key(unit));
	}
	signal.setAdjustment(settings.value("LastEditedSignal: adjustment").toDouble());
	signal.setDropLimit(settings.value("LastEditedSignal: dropLimit").toDouble());
	signal.setExcessLimit(settings.value("LastEditedSignal: excessLimit").toDouble());
	signal.setUnbalanceLimit(settings.value("LastEditedSignal: unbalanceLimit").toDouble());

	signal.setInputLowLimit(settings.value("LastEditedSignal: inputLowLimit").toDouble());
	signal.setInputHighLimit(settings.value("LastEditedSignal: inputHighLimit").toDouble());
	unit = settings.value("LastEditedSignal: inputUnitID").toInt();
	if (unit != -1)
	{
		signal.setInputUnitID(m_unitInfo.key(unit));
	}
	signal.setInputSensorID(settings.value("LastEditedSignal: inputSensorID").toInt());

	signal.setOutputLowLimit(settings.value("LastEditedSignal: outputLowLimit").toDouble());
	signal.setOutputHighLimit(settings.value("LastEditedSignal: outputHighLimit").toDouble());
	unit = settings.value("LastEditedSignal: outputUnitID").toInt();
	if (unit != -1)
	{
		signal.setOutputUnitID(m_unitInfo.key(unit));
	}
	signal.setOutputSensorID(settings.value("LastEditedSignal: outputSensorID").toInt());

	signal.setOutputRangeMode(OutputRangeMode(settings.value("LastEditedSignal: outputRangeMode").toInt()));

	signal.setAcquire(settings.value("LastEditedSignal: acquire").toBool());
	signal.setCalculated(settings.value("LastEditedSignal: calculated").toBool());
	signal.setNormalState(settings.value("LastEditedSignal: normalState").toInt());
	signal.setDecimalPlaces(settings.value("LastEditedSignal: decimalPlaces").toInt());
	signal.setAperture(settings.value("LastEditedSignal: aperture").toDouble());
	signal.setFilteringTime(settings.value("LastEditedSignal: filteringTime").toDouble());
	signal.setMaxDifference(settings.value("LastEditedSignal: maxDifference").toDouble());
	signal.setInOutType(SignalInOutType(settings.value("LastEditedSignal: inOutType").toInt()));
	signal.setByteOrder(E::ByteOrder(settings.value("LastEditedSignal: byteOrder").toInt()));

	SignalPropertiesDialog dlg(signal, E::SignalType(signalTypeCombo->currentIndex()), m_dataFormatInfo, m_unitInfo, false, nullptr, m_parentWindow);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<Signal*> resultSignalVector;
		resultSignalVector.reserve(signalCount * channelCount);
		for (int s = 0; s < signalCount; s++)
		{
			QVector<Signal> signalVector;
			for (int i = 0; i < channelCount; i++)
			{
				signalVector << signal;
				QString strID = signal.strID();
				QString name = signal.name();

				if (!deviceIdEdit->text().isEmpty())
				{
					name += " Device " + deviceIdEdit->text();
					signal.setDeviceStrID(deviceIdEdit->text());
				}

				if (signalCount > 1)
				{
					strID = (strID + "_sig%1").arg(s);
					name = (name + " Signal %1").arg(s);
				}

				if (channelCount > 1)
				{
					strID = (strID + "_%1").arg(QChar('A' + i));
				}

				signalVector[i].setStrID(strID.toUpper());
				signalVector[i].setName(name);
			}

			if (dbController()->addSignal(E::SignalType(signalTypeCombo->currentIndex()), &signalVector, m_parentWindow))
			{
				for (int i = 0; i < signalVector.count(); i++)
				{
					Signal* newSignal = new Signal;

					*newSignal = signalVector[i];

					resultSignalVector.append(newSignal);
				};
			}
		}
		if (!resultSignalVector.isEmpty())
		{
			beginInsertRows(QModelIndex(), m_signalSet.count(), m_signalSet.count() + resultSignalVector.count() - 1);
			for (int i = 0; i < resultSignalVector.count(); i++)
			{
				Signal* s = resultSignalVector[i];
				m_signalSet.append(s->ID(), s);
			}
			endInsertRows();
			emit dataChanged(createIndex(m_signalSet.count() - resultSignalVector.count(), 0), createIndex(m_signalSet.count() - 1, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
			emit signalActivated(m_signalSet.count() - resultSignalVector.count());
		}
	}

	emit setCheckedoutSignalActionsVisibility(true);
}

void SignalsModel::showError(QString message)
{
	QMessageBox::critical(m_parentWindow, tr("Error"), message);
}

bool SignalsModel::editSignal(int row)
{
	loadSignal(row, false);

	Signal signal = m_signalSet[row];
	int readOnly = false;
	if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId())
	{
		readOnly = true;
	}

	SignalPropertiesDialog dlg(signal, signal.type(), m_dataFormatInfo, m_unitInfo, readOnly, this, m_parentWindow);

	QObject::connect(&dlg, &SignalPropertiesDialog::onError, m_parentWindow, &SignalsTabPage::showError, Qt::QueuedConnection);

	if (dlg.exec() == QDialog::Accepted)
	{
		ObjectState state;
		dbController()->setSignalWorkcopy(&signal, &state, parrentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			showError(state);
		}

		loadSignal(row);
		emit signalActivated(row);
		return true;
	}

	loadSignal(row);	//Signal could be checked out but not changed
	return false;
}

bool SignalsModel::editSignals(QVector<int> ids)
{
	loadSignalSet(ids, false);

	int readOnly = false;
	QVector<Signal*> signalVector;
	E::SignalType signalType = E::SignalType::Analog;

	for (int i = 0; i < ids.count(); i++)
	{
		Signal& signal = m_signalSet[m_signalSet.keyIndex(ids[i])];
		if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId())
		{
			readOnly = true;
		}
		if (signal.type() == E::SignalType::Discrete)
		{
			signalType = E::SignalType::Discrete;
		}
		signalVector.append(&signal);
	}

	SignalPropertiesDialog dlg(signalVector, signalType, m_dataFormatInfo, m_unitInfo, readOnly, this, m_parentWindow);

	QObject::connect(&dlg, &SignalPropertiesDialog::onError, m_parentWindow, &SignalsTabPage::showError, Qt::QueuedConnection);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<ObjectState> states;
		for (int i = 0; i < ids.count(); i++)
		{
			ObjectState state;
			dbController()->setSignalWorkcopy(signalVector[i], &state, parrentWindow());
			states.append(state);
		}
		showErrors(states);

		loadSignalSet(ids);
		return true;
	}

	loadSignalSet(ids);	//Signal could be checked out but not changed
	return false;
}

void SignalsModel::saveSignal(Signal& signal)
{
	ObjectState state;
	dbController()->setSignalWorkcopy(&signal, &state, parrentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
	int row = m_signalSet.keyIndex(signal.ID());
	emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
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
	MainTabPage(dbcontroller, parent),
	m_scrollTimer(new QTimer(this))
{
	assert(dbcontroller != nullptr);

	m_signalTypeFilterCombo = new QComboBox(this);
	m_signalTypeFilterCombo->addItem(tr("All signals"), ST_ANY);
	m_signalTypeFilterCombo->addItem(tr("Analog signals"), ST_ANALOG);
	m_signalTypeFilterCombo->addItem(tr("Discrete signals"), ST_DISCRETE);

	QToolBar* toolBar = new QToolBar(this);
	toolBar->addWidget(m_signalTypeFilterCombo);

	// Property View
	//
	m_signalsModel = new SignalsModel(dbcontroller, this);
	m_signalsProxyModel = new SignalsProxyModel(m_signalsModel, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsProxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	SignalsDelegate* delegate = m_signalsModel->createDelegate(m_signalsProxyModel);
	m_signalsView->setItemDelegate(delegate);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);
	connect(m_signalsView->horizontalScrollBar(), &QScrollBar::valueChanged, this, &SignalsTabPage::saveScrollPosition);
	connect(m_signalsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &SignalsTabPage::saveScrollPosition);
	connect(m_scrollTimer, &QTimer::timeout, this, &SignalsTabPage::checkScrollPosition);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(m_signalsModel, &SignalsModel::dataChanged, this, &SignalsTabPage::updateCellsSize);
	connect(m_signalsModel, &SignalsModel::cellsSizeChanged, this, &SignalsTabPage::updateCellsSize);
	connect(m_signalsModel, &SignalsModel::signalActivated, m_signalsView, &QTableView::selectRow);
	connect(delegate, &SignalsDelegate::itemDoubleClicked, m_signalsModel, &SignalsModel::editSignal);
	connect(m_signalTypeFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalsTabPage::changeSignalTypeFilter);

	connect(m_signalsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SignalsTabPage::changeSignalActionsVisibility);

	connect(m_signalsModel, &SignalsModel::aboutToClearSignals, this, &SignalsTabPage::saveSelection);
	connect(m_signalsModel, &SignalsModel::signalsRestored, this, &SignalsTabPage::restoreSelection);

	// Create Actions
	//
	CreateActions(toolBar);

	m_autoResizeToContents = new QCheckBox(tr("Autoresize to contents"));
	m_autoResizeToContents->setChecked(true);
	connect(m_autoResizeToContents, &QCheckBox::clicked, this, &SignalsTabPage::updateCellsSize);

	toolBar->addWidget(m_autoResizeToContents);

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
	QAction* action = new QAction(QIcon(":/Images/Images/update.png"), tr("Refresh signal list"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::loadSignals);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/plus.png"), tr("Create signal"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::addSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/pencil.png"), tr("Edit signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::editSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/cross.png"), tr("Delete signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/undo.png"), tr("Undo signal changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/changes.png"), tr("Show pending changes..."), this);
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
	QVector<int> selectedSignalId;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		selectedSignalId.append(m_signalsModel->key(row));
	}

	m_signalsModel->editSignals(selectedSignalId);

	m_signalsView->selectionModel()->clearSelection();
	QAbstractItemView::SelectionMode selectionMode = m_signalsView->selectionMode();
	m_signalsView->setSelectionMode(QAbstractItemView::MultiSelection);
	for (int i = 0; i < selectedSignalId.count(); i++)
	{
		m_signalsView->selectRow(m_signalsModel->keyIndex(selectedSignalId[i]));
	}
	m_signalsView->setSelectionMode(selectionMode);
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
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
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

	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);
	dlg.setCheckStates(sourceSelection.indexes(), true);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::showPendingChanges()
{
	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);

	CheckinSignalsDialog dlg(m_signalsModel, sourceSelection.indexes(), this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::updateCellsSize()
{
	if (m_autoResizeToContents->isChecked())
	{
		QModelIndex topLeft = m_signalsView->indexAt(m_signalsView->rect().topLeft());
		QModelIndex bottomRight = m_signalsView->indexAt(m_signalsView->rect().bottomRight());

		for (int column = topLeft.column(); column <= bottomRight.column(); column++)
		{
			m_signalsView->resizeColumnToContents(column);
		}

		for (int row = topLeft.row(); row <= bottomRight.row(); row++)
		{
			m_signalsView->resizeRowToContents(row);
		}

		QModelIndex newBottomRight = m_signalsView->indexAt(m_signalsView->rect().bottomRight());
		while (bottomRight.row() < newBottomRight.row())
		{
			for (int row = bottomRight.row(); row <= newBottomRight.row(); row++)
			{
				m_signalsView->resizeRowToContents(row);
			}
			bottomRight = newBottomRight;
			newBottomRight = m_signalsView->indexAt(m_signalsView->rect().bottomRight());
		}
	}
}

void SignalsTabPage::saveScrollPosition()
{
	if (!m_autoResizeToContents->isChecked())
	{
		return;
	}
	m_lastVerticalScrollPosition = m_signalsView->verticalScrollBar()->value();
	m_lastHorizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	if (m_scrollTimer->isActive())
	{
		m_scrollTimer->stop();
	}
	m_scrollTimer->start(1000);
}

void SignalsTabPage::checkScrollPosition()
{
	if (!m_autoResizeToContents->isChecked())
	{
		return;
	}
	if (m_lastVerticalScrollPosition == m_signalsView->verticalScrollBar()->value() &&
			m_lastHorizontalScrollPosition == m_signalsView->horizontalScrollBar()->value())
	{
		updateCellsSize();
		m_scrollTimer->stop();
	}
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
			int row = m_signalsProxyModel->mapToSource(selection[i]).row();
			if (m_signalsModel->isEditableSignal(row))
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
	m_selectedRowsSignalID.clear();
	QModelIndexList selectedList = m_signalsView->selectionModel()->selectedRows(0);
	foreach (const QModelIndex& index, selectedList)
	{
		int row = m_signalsProxyModel->mapToSource(index).row();
		m_selectedRowsSignalID.append(m_signalsModel->key(row));
	}
	QModelIndex index = m_signalsView->currentIndex();
	if (index.isValid())
	{
		int row = m_signalsProxyModel->mapToSource(index).row();
		m_focusedCellSignalID = m_signalsModel->key(row);
		m_focusedCellColumn = index.column();
	}
	m_lastHorizontalScrollPosition = m_signalsView->horizontalScrollBar()->value();
	m_lastVerticalScrollPosition = m_signalsView->verticalScrollBar()->value();
}

void SignalsTabPage::restoreSelection()
{
	foreach (int id, m_selectedRowsSignalID)
	{
		QModelIndex sourceIndex = m_signalsModel->index(m_signalsModel->keyIndex(id), 0);
		QModelIndex proxyIndex = m_signalsProxyModel->mapFromSource(sourceIndex);
		m_signalsView->selectRow(proxyIndex.row());
	}
	QModelIndex sourceIndex = m_signalsModel->index(m_signalsModel->keyIndex(m_focusedCellSignalID), m_focusedCellColumn);
	m_signalsView->setCurrentIndex(m_signalsProxyModel->mapFromSource(sourceIndex));
	m_signalsView->horizontalScrollBar()->setValue(m_lastHorizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(m_lastVerticalScrollPosition);
}

void SignalsTabPage::changeSignalTypeFilter(int selectedType)
{
	saveSelection();
	int singalType = m_signalTypeFilterCombo->itemData(selectedType, Qt::UserRole).toInt();
	m_signalsProxyModel->setSignalTypeFilter(singalType);
	restoreSelection();

	switch(singalType)
	{
		case ST_DISCRETE:
			for (int i = SC_LOW_ADC; i < SC_LAST_CHANGE_USER; i++)
			{
				m_signalsView->setColumnHidden(i, true);
			}
			break;
		case ST_ANALOG:
		case ST_ANY:
			for (int i = SC_LOW_ADC; i < SC_LAST_CHANGE_USER; i++)
			{
				m_signalsView->setColumnHidden(i, false);
			}
			break;
		default:
			assert(false);
	}
	m_signalsView->resizeColumnsToContents();
}

void SignalsTabPage::showError(QString message)
{
	QMessageBox::warning(this, "Error", message);
}


CheckedoutSignalsModel::CheckedoutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel),
	m_view(view)
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
		QModelIndexList list = m_view->selectionModel()->selectedRows(0);
		for (int i = 0; i < list.count(); i++)
		{
			setCheckState(list[i].row(), Qt::CheckState(value.toInt()));
		}
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
	QVector<int> sourceRows = m_sourceModel->getSameChannelSignals(mapToSource(index(row, 0)).row());
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

	m_signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, m_signalsView, this);

	if (selection.count() != 0)
	{
		m_proxyModel->initCheckStates(selection);
	}

	m_commentEdit = new QPlainTextEdit(this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl2->addWidget(selectAll);

	m_signalsView->setModel(m_proxyModel);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	m_signalsView->resizeColumnsToContents();
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	QAction* undoAction = new QAction(QIcon(":/Images/Images/undo.png"), tr("Undo signal changes"), this);
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

	QTableView* signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl->addWidget(selectAll);

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


SignalsProxyModel::SignalsProxyModel(SignalsModel *sourceModel, QObject *parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{
	setSourceModel(sourceModel);
}

bool SignalsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
	return m_signalType == ST_ANY || m_signalType == m_sourceModel->signal(source_row).typeInt();
}

void SignalsProxyModel::setSignalTypeFilter(int signalType)
{
	beginResetModel();
	m_signalType = signalType;
	endResetModel();
}
