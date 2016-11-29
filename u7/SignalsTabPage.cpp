#include "Stable.h"

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
#include <QCompleter>
#include <QStringListModel>
#include <QGroupBox>
#include <QSet>

#include "../lib/DbController.h"

#include "SignalsTabPage.h"
#include "Settings.h"
#include "../lib/SignalProperties.h"
#include "SignalPropertiesDialog.h"
#include "./Forms/ComparePropertyObjectDialog.h"

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
SC_DROP_LIMIT = 16,
SC_EXCESS_LIMIT = 17,
SC_UNBALANCE_LIMIT = 18,
SC_OUTPUT_MODE = 19,
SC_DECIMAL_PLACES = 20,
SC_APERTURE = 21,
SC_FILTERING_TIME = 22,
SC_SPREAD_TOLERANCE = 23,
SC_BYTE_ORDER = 24,
SC_ENABLE_TUNING = 25,
SC_TUNING_DEFAULT_VALUE = 26,
SC_LAST_CHANGE_USER = 27;


const char* Columns[] =
{
	"AppSignalID",
	"CustomAppSignalID",
	"Caption",
	"Channel",
	"A/D",
	"Data format",
	"Data\nsize",
	"Normal\nstate",
	"Acquire",
	"Input-output\ntype",
	"EquipmentID",
	"Low ADC",
	"High ADC",
	"Low\nEngeneering Units",
	"High\nEngeneering Units",
	"Unit",
	"Low\nValid Range",
	"High\nValid Range",
	"Unbalance limit",
	"Output\nmode",
	"Decimal\nplaces",
	"Aperture",
	"Filtering time",
	"Spred Tolerance",
	"Byte order",
	"Enable\ntuning",
	"Tuning\ndefault value",
	"Last change user",
};

const int COLUMNS_COUNT = sizeof(Columns) / sizeof(char*);

const int DEFAULT_COLUMN_WIDTH = 75;

const QVector<int> defaultColumnVisibility =
{
	SC_STR_ID,
	SC_EXT_STR_ID,
	SC_NAME,
	SC_TYPE,
	SC_IN_OUT_TYPE,
	SC_DEVICE_STR_ID,
	SC_LOW_LIMIT,
	SC_HIGH_LIMIT
};

SignalsModel* SignalsModel::m_instance = nullptr;



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
			if (m_signalSet.count() > index.row() && m_signalSet[index.row()].signalType() == E::SignalType::Discrete)
			{
				return nullptr;
			}
			QLineEdit* le = new QLineEdit(parent);
			le->setValidator(new QIntValidator(le));
			return le;
		}
		case SC_LOW_LIMIT:
		case SC_HIGH_LIMIT:
		case SC_DROP_LIMIT:
		case SC_EXCESS_LIMIT:
		case SC_UNBALANCE_LIMIT:
		case SC_APERTURE:
		case SC_FILTERING_TIME:
		case SC_SPREAD_TOLERANCE:
		case SC_TUNING_DEFAULT_VALUE:
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
			cb->addItems(m_dataFormatInfo.getValuesList());
			return cb;
		}
		case SC_UNIT:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(m_unitInfo.getValuesList());
			return cb;
		}
		case SC_OUTPUT_MODE:
		{
			QComboBox* cb = new QComboBox(parent);
			for (int i = 0; i < OUTPUT_MODE_COUNT; i++)
			{
				cb->addItem(OutputModeStr[i]);
			}
			return cb;
		}
		case SC_ACQUIRE:
		case SC_ENABLE_TUNING:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(QStringList() << tr("False") << tr("True"));
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
		case SC_STR_ID: if (le) le->setText(s.appSignalID()); break;
		case SC_EXT_STR_ID: if (le) le->setText(s.customAppSignalID()); break;
		case SC_NAME: if (le) le->setText(s.caption()); break;
		case SC_DEVICE_STR_ID: if (le) le->setText(s.equipmentID()); break;

		case SC_DATA_SIZE: if (le) le->setText(QString::number(s.dataSize())); break;
		case SC_LOW_ADC: if (le) le->setText(QString::number(s.lowADC())); break;
		case SC_HIGH_ADC: if (le) le->setText(QString::number(s.highADC())); break;
		case SC_NORMAL_STATE: if (le) le->setText(QString::number(s.normalState())); break;
		case SC_DECIMAL_PLACES: if (le) le->setText(QString::number(s.decimalPlaces())); break;

		case SC_LOW_LIMIT: if (le) le->setText(QString("%1").arg(s.lowEngeneeringUnits())); break;
		case SC_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(s.highEngeneeringUnits())); break;
		case SC_DROP_LIMIT: if (le) le->setText(QString("%1").arg(s.lowValidRange())); break;
		case SC_EXCESS_LIMIT: if (le) le->setText(QString("%1").arg(s.highValidRange())); break;
		case SC_UNBALANCE_LIMIT: if (le) le->setText(QString("%1").arg(s.unbalanceLimit())); break;
		case SC_APERTURE: if (le) le->setText(QString("%1").arg(s.aperture())); break;
		case SC_FILTERING_TIME: if (le) le->setText(QString("%1").arg(s.filteringTime())); break;
		case SC_SPREAD_TOLERANCE: if (le) le->setText(QString("%1").arg(s.spreadTolerance())); break;
		case SC_TUNING_DEFAULT_VALUE: if (le) le->setText(QString("%1").arg(s.tuningDefaultValue())); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) cb->setCurrentIndex(m_dataFormatInfo.keyIndex(s.analogSignalFormatInt())); break;
		case SC_UNIT: if (cb) cb->setCurrentIndex(m_unitInfo.keyIndex(s.unitID())); break;
		case SC_OUTPUT_MODE: if (cb) cb->setCurrentIndex(s.outputMode()); break;
		case SC_ACQUIRE: if (cb) cb->setCurrentIndex(s.acquire()); break;
		case SC_ENABLE_TUNING: if (cb) cb->setCurrentIndex(s.enableTuning()); break;
		case SC_IN_OUT_TYPE: if (cb) cb->setCurrentIndex(TO_INT(s.inOutType())); break;
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
		case SC_STR_ID: if (le) s.setAppSignalID(le->text()); break;
		case SC_EXT_STR_ID: if (le) s.setCustomAppSignalID(le->text()); break;
		case SC_NAME: if (le) s.setCaption(le->text()); break;
		case SC_DEVICE_STR_ID: if (le) s.setEquipmentID(le->text()); break;

		case SC_DATA_SIZE: if (le) s.setDataSize(le->text().toInt()); break;
		case SC_LOW_ADC: if (le) s.setLowADC(le->text().toInt()); break;
		case SC_HIGH_ADC: if (le) s.setHighADC(le->text().toInt()); break;
		case SC_NORMAL_STATE: if (le) s.setNormalState(le->text().toInt()); break;
		case SC_DECIMAL_PLACES: if (le) s.setDecimalPlaces(le->text().toInt()); break;

		case SC_LOW_LIMIT: if (le) s.setLowEngeneeringUnits(le->text().toDouble()); break;
		case SC_HIGH_LIMIT: if (le) s.setHighEngeneeringUnits(le->text().toDouble()); break;
		case SC_DROP_LIMIT: if (le) s.setLowValidRange(le->text().toDouble()); break;
		case SC_EXCESS_LIMIT: if (le) s.setHighValidRange(le->text().toDouble()); break;
		case SC_UNBALANCE_LIMIT: if (le) s.setUnbalanceLimit(le->text().toDouble()); break;
		case SC_APERTURE: if (le) s.setAperture(le->text().toDouble()); break;
		case SC_FILTERING_TIME: if (le) s.setFilteringTime(le->text().toDouble()); break;
		case SC_SPREAD_TOLERANCE: if (le) s.setSpreadTolerance(le->text().toDouble()); break;
		case SC_TUNING_DEFAULT_VALUE: if (le) s.setTuningDefaultValue(le->text().toDouble()); break;
		// ComboBox
		//
		case SC_DATA_FORMAT: if (cb) s.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(m_dataFormatInfo.keyAt(cb->currentIndex()))); break;
		case SC_UNIT: if (cb) s.setUnitID(m_unitInfo.keyAt(cb->currentIndex())); break;
		case SC_OUTPUT_MODE: if (cb) s.setOutputMode(static_cast<E::OutputMode>(cb->currentIndex())); break;
		case SC_ACQUIRE: if (cb) s.setAcquire(cb->currentIndex() == 0 ? false : true); break;
		case SC_ENABLE_TUNING: if (cb) s.setEnableTuning(cb->currentIndex() == 0 ? false : true); break;
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

bool SignalsDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &)
{
	if (event->type() == QEvent::MouseButtonDblClick)
	{
		emit itemDoubleClicked();
		return true;
	}
	return false;
}


SignalsModel::SignalsModel(DbController* dbController, SignalsTabPage* parent) :
	QAbstractTableModel(parent),
	m_parentWindow(parent),
	m_dbController(dbController)
{
	m_instance = this;
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

QString SignalsModel::getOutputModeStr(int outputMode) const
{
	if (outputMode >= 0 && outputMode < OUTPUT_MODE_COUNT)
	{
		return OutputModeStr[outputMode];
	}
	else
	{
		return tr("Unknown output mode");
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
	dbController()->checkoutSignals(&signalsIDs, &objectStates, parentWindow());
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
	loadSignal(index, true);
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
	dbController()->checkoutSignals(&signalsIDs, &objectStates, parentWindow());
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
	loadSignal(index, true);
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

QString SignalsModel::errorMessage(const ObjectState& state) const
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_ALREADY_CHECKED_OUT: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_usernameMap[state.userId]);
		case ERR_SIGNAL_DELETED: return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND: return tr("Signal %1 not found").arg(state.id);
		case ERR_SIGNAL_EXISTS: return "";				// error message is displayed by PGSql driver
		default:
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
			if (message.isEmpty() == false)
			{
				message += "\n";
			}

			message += errorMessage(state);
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
		if (signal.signalType() == E::SignalType::Analog)
		{
			switch (col)
			{
				case SC_LAST_CHANGE_USER: return signal.checkedOut() ? getUserStr(signal.userID()) : "";
				case SC_STR_ID: return signal.appSignalID();
				case SC_EXT_STR_ID: return signal.customAppSignalID();
				case SC_NAME: return signal.caption();
				case SC_CHANNEL: return E::valueToString<E::Channel>(signal.channelInt());
				case SC_TYPE: return QChar('A');
				case SC_DATA_FORMAT:
					if (m_dataFormatInfo.contains(signal.analogSignalFormatInt()))
					{
						return m_dataFormatInfo.value(signal.analogSignalFormatInt());
					}
					else
					{
						return tr("Unknown data format");
					}

				case SC_DATA_SIZE: return signal.dataSize();
				case SC_LOW_ADC: return QString("0x%1").arg(signal.lowADC(), 4, 16, QChar('0'));
				case SC_HIGH_ADC: return QString("0x%1").arg(signal.highADC(), 4, 16, QChar('0'));
				case SC_LOW_LIMIT: return signal.lowEngeneeringUnits();
				case SC_HIGH_LIMIT: return signal.highEngeneeringUnits();
				case SC_UNIT: return getUnitStr(signal.unitID());

				case SC_DROP_LIMIT: return signal.lowValidRange();
				case SC_EXCESS_LIMIT: return signal.highValidRange();
				case SC_UNBALANCE_LIMIT: return signal.unbalanceLimit();

				case SC_OUTPUT_MODE: return getOutputModeStr(signal.outputMode());

				case SC_ACQUIRE: return signal.acquire() ? tr("True") : tr("False");
				case SC_ENABLE_TUNING: return signal.enableTuning() ? tr("True") : tr("False");

				case SC_NORMAL_STATE: return signal.normalState();
				case SC_DECIMAL_PLACES: return signal.decimalPlaces();
				case SC_APERTURE: return signal.aperture();
				case SC_FILTERING_TIME: return signal.filteringTime();
				case SC_SPREAD_TOLERANCE: return signal.spreadTolerance();
				case SC_TUNING_DEFAULT_VALUE: return signal.tuningDefaultValue();

				case SC_IN_OUT_TYPE: return (TO_INT(signal.inOutType()) < IN_OUT_TYPE_COUNT) ? InOutTypeStr[TO_INT(signal.inOutType())] : tr("Unknown type");
				case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());

				case SC_DEVICE_STR_ID: return signal.equipmentID();

				default:
					assert(false);
			}
		}
		else
		{
			switch (col)
			{
				case SC_LAST_CHANGE_USER: return signal.checkedOut() ? getUserStr(signal.userID()) : "";
				case SC_STR_ID: return signal.appSignalID();
				case SC_EXT_STR_ID: return signal.customAppSignalID();
				case SC_NAME: return signal.caption();
				case SC_CHANNEL: return E::valueToString<E::Channel>(signal.channelInt());
				case SC_TYPE: return QChar('D');
				case SC_DATA_FORMAT:
					if (m_dataFormatInfo.contains(signal.analogSignalFormatInt()))
					{
						return m_dataFormatInfo.value(signal.analogSignalFormatInt());
					}
					else
					{
						return tr("Unknown data format");
					}

				case SC_DATA_SIZE: return signal.dataSize();
				case SC_ACQUIRE: return signal.acquire() ? tr("True") : tr("False");
				case SC_ENABLE_TUNING: return signal.enableTuning() ? tr("True") : tr("False");
				case SC_TUNING_DEFAULT_VALUE: return signal.tuningDefaultValue();
				case SC_IN_OUT_TYPE: return (TO_INT(signal.inOutType()) < IN_OUT_TYPE_COUNT) ? InOutTypeStr[TO_INT(signal.inOutType())] : tr("Unknown type");
				case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());
				case SC_DEVICE_STR_ID: return signal.equipmentID();

				case SC_LOW_ADC:
				case SC_HIGH_ADC:
				case SC_LOW_LIMIT:
				case SC_HIGH_LIMIT:
				case SC_UNIT:

				case SC_DROP_LIMIT:
				case SC_EXCESS_LIMIT:
				case SC_UNBALANCE_LIMIT:

				case SC_OUTPUT_MODE:

				case SC_NORMAL_STATE:
				case SC_DECIMAL_PLACES:
				case SC_APERTURE:
				case SC_FILTERING_TIME:
				case SC_SPREAD_TOLERANCE:
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
				switch (signal.instanceAction().value())
				{
					case VcsItemAction::Added: return plus;
					case VcsItemAction::Modified: return pencil;
					case VcsItemAction::Deleted: return cross;
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

		Signal& signal = m_signalSet[row];

		switch (index.column())
		{
			case SC_STR_ID: signal.setAppSignalID(value.toString()); break;
			case SC_EXT_STR_ID: signal.setCustomAppSignalID(value.toString()); break;
			case SC_NAME: signal.setCaption(value.toString()); break;
			case SC_DATA_FORMAT: signal.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(value.toInt())); break;
			case SC_DATA_SIZE: signal.setDataSize(value.toInt()); break;
			case SC_LOW_ADC: signal.setLowADC(value.toInt()); break;
			case SC_HIGH_ADC: signal.setHighADC(value.toInt()); break;
			case SC_LOW_LIMIT: signal.setLowEngeneeringUnits(value.toDouble()); break;
			case SC_HIGH_LIMIT: signal.setHighEngeneeringUnits(value.toDouble()); break;
			case SC_UNIT: signal.setUnitID(value.toInt()); break;
			case SC_DROP_LIMIT: signal.setLowValidRange(value.toDouble()); break;
			case SC_EXCESS_LIMIT: signal.setHighValidRange(value.toDouble()); break;
			case SC_UNBALANCE_LIMIT: signal.setUnbalanceLimit(value.toDouble()); break;
			case SC_OUTPUT_MODE: signal.setOutputMode(static_cast<E::OutputMode>(value.toInt())); break;
			case SC_ACQUIRE: signal.setAcquire(value.toBool()); break;
			case SC_ENABLE_TUNING: signal.setEnableTuning(value.toBool()); break;
			case SC_NORMAL_STATE: signal.setNormalState(value.toInt()); break;
			case SC_DECIMAL_PLACES: signal.setDecimalPlaces(value.toInt()); break;
			case SC_APERTURE: signal.setAperture(value.toDouble()); break;
			case SC_FILTERING_TIME: signal.setFilteringTime(value.toDouble()); break;
			case SC_SPREAD_TOLERANCE: signal.setSpreadTolerance(value.toDouble()); break;
			case SC_TUNING_DEFAULT_VALUE: signal.setTuningDefaultValue(value.toDouble()); break;
			case SC_BYTE_ORDER: signal.setByteOrder(E::ByteOrder(value.toInt())); break;
			case SC_DEVICE_STR_ID: signal.setEquipmentID(value.toString()); break;
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
	}

	changeCheckedoutSignalActionsVisibility();
}

void SignalsModel::loadSignal(int row, bool updateView)
{
	dbController()->getLatestSignal(key(row), &m_signalSet[row], parentWindow());
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
		dbController()->getLatestSignal(keys[i], &m_signalSet[row], parentWindow());
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

Signal*SignalsModel::getSignalByStrID(const QString signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
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
	QDialog signalTypeDialog(m_parentWindow, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	QFormLayout* fl = new QFormLayout(&signalTypeDialog);

	QLineEdit* deviceIdEdit = new QLineEdit(&signalTypeDialog);
	deviceIdEdit->setText("");

	fl->addRow(tr("EquipmentID"), deviceIdEdit);

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

	signalTypeDialog.setWindowTitle("Create signals");
	signalTypeDialog.setFixedSize(600, 200);

	if (signalTypeDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	int channelCount = signalChannelCountEdit->text().toInt();
	int signalCount = signalCountEdit->text().toInt();

	Signal signal;

	QSettings settings(QSettings::UserScope, qApp->organizationName());

	if (E::SignalType(signalTypeCombo->currentIndex()) == E::SignalType::Analog)
	{
		signal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
		signal.setDataSize(FLOAT32_SIZE);
		signal.setSignalType(E::SignalType::Analog);
	}
	else
	{
		signal.setDataSize(DISCRETE_SIZE);
		signal.setSignalType(E::SignalType::Discrete);
	}
	signal.setLowADC(settings.value("SignalsTabPage/LastEditedSignal/lowADC").toInt());
	signal.setHighADC(settings.value("SignalsTabPage/LastEditedSignal/highADC").toInt());
	signal.setLowEngeneeringUnits(settings.value("SignalsTabPage/LastEditedSignal/lowEngeneeringUnits").toDouble());
	signal.setHighEngeneeringUnits(settings.value("SignalsTabPage/LastEditedSignal/highEngeneeringUnits").toDouble());
	int unit = settings.value("SignalsTabPage/LastEditedSignal/unitID").toInt();
	if (unit != -1)
	{
		signal.setUnitID(m_unitInfo.keyAt(unit));
	}
	signal.setLowValidRange(settings.value("SignalsTabPage/LastEditedSignal/lowValidRange").toDouble());
	signal.setHighValidRange(settings.value("SignalsTabPage/LastEditedSignal/highValidRange").toDouble());
	signal.setUnbalanceLimit(settings.value("SignalsTabPage/LastEditedSignal/unbalanceLimit").toDouble());

	signal.setInputLowLimit(settings.value("SignalsTabPage/LastEditedSignal/inputLowLimit").toDouble());
	signal.setInputHighLimit(settings.value("SignalsTabPage/LastEditedSignal/inputHighLimit").toDouble());
	unit = settings.value("SignalsTabPage/LastEditedSignal/inputUnitID").toInt();
	if (unit != -1)
	{
		signal.setInputUnitID(m_unitInfo.keyAt(unit));
	}
	signal.setInputSensorID(settings.value("SignalsTabPage/LastEditedSignal/inputSensorID").toInt());

	signal.setOutputLowLimit(settings.value("SignalsTabPage/LastEditedSignal/outputLowLimit").toDouble());
	signal.setOutputHighLimit(settings.value("SignalsTabPage/LastEditedSignal/outputHighLimit").toDouble());
	unit = settings.value("SignalsTabPage/LastEditedSignal/outputUnitID").toInt();
	if (unit != -1)
	{
		signal.setOutputUnitID(m_unitInfo.keyAt(unit));
	}
	signal.setOutputSensorID(settings.value("SignalsTabPage/LastEditedSignal/outputSensorID").toInt());

	signal.setOutputMode(static_cast<E::OutputMode>(settings.value("SignalsTabPage/LastEditedSignal/outputMode").toInt()));

	signal.setAcquire(settings.value("SignalsTabPage/LastEditedSignal/acquire").toBool());
	signal.setCalculated(settings.value("SignalsTabPage/LastEditedSignal/calculated").toBool());
	signal.setNormalState(settings.value("SignalsTabPage/LastEditedSignal/normalState").toInt());
	signal.setDecimalPlaces(settings.value("SignalsTabPage/LastEditedSignal/decimalPlaces").toInt());
	signal.setAperture(settings.value("SignalsTabPage/LastEditedSignal/aperture").toDouble());
	signal.setFilteringTime(settings.value("SignalsTabPage/LastEditedSignal/filteringTime").toDouble());
	signal.setSpreadTolerance(settings.value("SignalsTabPage/LastEditedSignal/spreadTolerance").toDouble());
	signal.setInOutType(E::SignalInOutType::Internal);
	signal.setByteOrder(E::ByteOrder(settings.value("SignalsTabPage/LastEditedSignal/byteOrder").toInt()));

	if (!deviceIdEdit->text().isEmpty())
	{
		signal.setEquipmentID(deviceIdEdit->text());
	}

	SignalPropertiesDialog dlg(signal, m_unitInfo, false, nullptr, m_parentWindow);

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
				QString strID = signal.appSignalID();

				if (signalCount > 1)
				{
					strID = (strID + "_sig%1").arg(s);
				}

				if (channelCount > 1)
				{
					strID = (strID + "_%1").arg(QChar('A' + i));
				}

				signalVector[i].setAppSignalID(strID.toUpper());
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

bool SignalsModel::editSignals(QVector<int> ids)
{
	loadSignalSet(ids, false);

	bool readOnly = false;
	QVector<Signal*> signalVector;

	for (int i = 0; i < ids.count(); i++)
	{
		Signal& signal = m_signalSet[m_signalSet.keyIndex(ids[i])];
		if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId())
		{
			readOnly = true;
		}

		signalVector.append(&signal);
	}

	SignalPropertiesDialog dlg(signalVector, m_unitInfo, readOnly, this, m_parentWindow);

	QObject::connect(&dlg, &SignalPropertiesDialog::onError, m_parentWindow, &SignalsTabPage::showError, Qt::QueuedConnection);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<ObjectState> states;
		for (int i = 0; i < ids.count(); i++)
		{
			if (dlg.isEditedSignal(ids[i]))
			{
				ObjectState state;
				dbController()->setSignalWorkcopy(signalVector[i], &state, parentWindow());
				states.append(state);
			}
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
	dbController()->setSignalWorkcopy(&signal, &state, parentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
	int row = m_signalSet.keyIndex(signal.ID());
	emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
}

QList<int> SignalsModel::cloneSignals(const QSet<int>& signalIDs)
{
	QList<int> resultSignalIDs;
	m_signalSet.buildID2IndexMap();

	auto idMaker = [](QString prefix, QString id) {
		if (id[0] == '#')
		{
			return '#' + prefix + id.mid(1);
		}
		else
		{
			return prefix + id;
		}
	};

	QSet<int> clonedSignalIDs;
	QList<int> signalIDsList = signalIDs.toList();
	qSort(signalIDsList);
	for (const int signalID : signalIDsList)
	{
		if (clonedSignalIDs.contains(signalID))
		{
			continue;
		}

		const Signal&& signal = m_signalSet.value(signalID);
		E::SignalType type = signal.signalType();
		QVector<int> groupSignalIDs;

		if (signal.signalGroupID() == 0)
		{
			groupSignalIDs.append(signal.ID());
		}
		else
		{
			groupSignalIDs = m_signalSet.getChannelSignalsID(signal);
		}
		qSort(groupSignalIDs);

		for (int groupSignalID : groupSignalIDs)
		{
			clonedSignalIDs.insert(groupSignalID);
		}

		QString prefix = "CLONE_";
		int prefixNumerator = 1;
		bool hasConflict;
		do
		{
			hasConflict = false;
			for (int groupSignalID : groupSignalIDs)
			{
				if (m_signalSet.contains(idMaker(prefix, m_signalSet.value(groupSignalID).appSignalID())))
				{
					hasConflict = true;
					break;
				}
			}
			if (hasConflict)
			{
				prefixNumerator++;
				prefix = QString("CLONE%1_").arg(prefixNumerator);
			}
		}
		while (hasConflict && prefixNumerator < 1000);

		if (prefixNumerator >= 1000)
		{
			assert(false);
			return QList<int>();
		}

		QVector<Signal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const Signal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;

			groupSignals[i].setAppSignalID(idMaker(prefix, groupSignal.appSignalID()));
			groupSignals[i].setCustomAppSignalID(idMaker(prefix, groupSignal.customAppSignalID()));
		}

		dbController()->addSignal(type, &groupSignals, m_parentWindow);
		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs.push_back(groupSignals[i].ID());
		}
	}
	loadSignals();
	return resultSignalIDs;
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

void SignalsModel::deleteSignals(const QSet<int>& signalIDs)
{
	foreach (const int signalID, signalIDs)
	{
		deleteSignal(signalID);
	}
	loadSignals();
}

void SignalsModel::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, parentWindow());
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

	m_signalTypeFilterCombo = new QComboBox(this);
	m_signalTypeFilterCombo->addItem(tr("All signals"), ST_ANY);
	m_signalTypeFilterCombo->addItem(tr("Analog signals"), ST_ANALOG);
	m_signalTypeFilterCombo->addItem(tr("Discrete signals"), ST_DISCRETE);

	m_signalIdFieldCombo = new QComboBox(this);
	m_signalIdFieldCombo->addItem(tr("AppSignalID"), FI_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("CustomAppSignalID"), FI_CUSTOM_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("EquipmentID"), FI_EQUIPMENT_ID);

	QToolBar* toolBar = new QToolBar(this);

	connect(GlobalMessanger::instance(), &GlobalMessanger::showDeviceApplicationSignals, this, &SignalsTabPage::changeSignalIdFilter);

	QToolBar* filterToolBar = new QToolBar(this);

	m_filterEdit = new QLineEdit(this);
	filterToolBar->addWidget(new QLabel("Filter ", this));
	filterToolBar->addWidget(m_signalTypeFilterCombo);
	filterToolBar->addWidget(new QLabel(" by ", this));
	filterToolBar->addWidget(m_signalIdFieldCombo);
	filterToolBar->addWidget(new QLabel(" complies ", this));
	filterToolBar->addWidget(m_filterEdit);

	QSettings settings(QSettings::UserScope, qApp->organizationName());
	m_filterHistory = settings.value("SignalsTabPage/filterHistory").toStringList();

	m_completer = new QCompleter(m_filterHistory, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_filterEdit->setCompleter(m_completer);
	connect(m_filterEdit, &QLineEdit::textEdited, [=](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_filterEdit, &QLineEdit::setText);

	QPushButton* applyButton = new QPushButton("Apply", this);
	connect(applyButton, &QPushButton::clicked, this, &SignalsTabPage::applySignalIdFilter);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SignalsTabPage::applySignalIdFilter);
	filterToolBar->addWidget(applyButton);

	QPushButton* resetButton = new QPushButton("Reset", this);
	connect(resetButton, &QPushButton::clicked, this, &SignalsTabPage::resetSignalIdFilter);
	filterToolBar->addWidget(resetButton);

	// Property View
	//
	m_signalsModel = new SignalsModel(dbcontroller, this);
	m_signalsProxyModel = new SignalsProxyModel(m_signalsModel, this);
	m_signalsView = new QTableView(this);
	m_signalsView->setModel(m_signalsProxyModel);
	m_signalsView->setSortingEnabled(true);
	m_signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_signalsView->verticalHeader()->setFixedWidth(DEFAULT_COLUMN_WIDTH);
	SignalsDelegate* delegate = m_signalsModel->createDelegate(m_signalsProxyModel);
	m_signalsView->setItemDelegate(delegate);
	m_signalsView->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);

	QHeaderView* horizontalHeader = m_signalsView->horizontalHeader();
	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	horizontalHeader->setHighlightSections(false);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));
	horizontalHeader->setDefaultSectionSize(150);
	horizontalHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setColumnWidth(SC_STR_ID, 400);
	m_signalsView->setColumnWidth(SC_EXT_STR_ID, 400);
	m_signalsView->setColumnWidth(SC_NAME, 400);
	m_signalsView->setColumnWidth(SC_DEVICE_STR_ID, 400);

	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		m_signalsView->setColumnWidth(i, settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
														m_signalsView->columnWidth(i)).toInt());
	}

	m_tableHeadersContextMenuActions = new QActionGroup(this);
	m_tableHeadersContextMenuActions->setExclusive(false);
	QAction* columnAction = m_tableHeadersContextMenuActions->addAction("All columns");
	columnAction->setCheckable(true);
	columnAction->setChecked(settings.value("SignalsTabPage/ColumnVisibility/All columns", true).toBool());

	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		columnAction = m_tableHeadersContextMenuActions->addAction(QString(Columns[i]).replace("\n", " "));
		columnAction->setCheckable(true);
		bool checked = settings.value(QString("SignalsTabPage/ColumnVisibility/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
									  defaultColumnVisibility.contains(i)).toBool();
		columnAction->setChecked(checked);
		m_signalsView->setColumnHidden(i, !checked);
	}

	m_signalsView->horizontalHeader()->addActions(m_tableHeadersContextMenuActions->actions());
	connect(m_tableHeadersContextMenuActions, &QActionGroup::triggered, this, &SignalsTabPage::changeColumnVisibility);
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &SignalsTabPage::saveColumnWidth);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	connect(m_signalsModel, &SignalsModel::signalActivated, m_signalsView, &QTableView::selectRow);
	connect(delegate, &SignalsDelegate::itemDoubleClicked, this, &SignalsTabPage::editSignal);
	connect(m_signalTypeFilterCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SignalsTabPage::changeSignalTypeFilter);

	connect(m_signalsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SignalsTabPage::changeSignalActionsVisibility);

	connect(m_signalsModel, &SignalsModel::aboutToClearSignals, this, &SignalsTabPage::saveSelection);
	connect(m_signalsModel, &SignalsModel::signalsRestored, this, &SignalsTabPage::restoreSelection);

	// Create Actions
	//
	CreateActions(toolBar);

	//
	// Layouts
	//

	QVBoxLayout* pMainLayout = new QVBoxLayout();

	pMainLayout->addWidget(toolBar);
	pMainLayout->addWidget(filterToolBar);
	pMainLayout->addWidget(m_signalsView);

	setLayout(pMainLayout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SignalsTabPage::projectClosed);

	connect(GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SignalsTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	QSettings settings(QSettings::UserScope, qApp->organizationName());

	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		QAction* columnVisibilityAction = m_tableHeadersContextMenuActions->actions()[i + 1];
		if (columnVisibilityAction->isChecked())
		{
			saveColumnWidth(i);
		}
		saveColumnVisibility(i, columnVisibilityAction->isChecked());
	}
	settings.setValue("SignalsTabPage/ColumnVisibility/All columns", m_tableHeadersContextMenuActions->actions()[0]->isChecked());
}

QStringList SignalsTabPage::createSignal(DbController* dbController, const QStringList& lmIdList, int schemaCounter, const QString& schemaId, const QString& schemaCaption, QWidget* parent)
{
	QVector<Signal> signalVector;

	QDialog signalCreationSettingsDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	QFormLayout* fl = new QFormLayout;

	QList<QCheckBox*> checkBoxList;
	QStringList selectedLmIdList;

	QVBoxLayout* vl = new QVBoxLayout;

	QGroupBox *groupBox = new QGroupBox("EquipmentID for signals", &signalCreationSettingsDialog);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	QVBoxLayout* groupBoxLayout = new QVBoxLayout;
	groupBox->setLayout(groupBoxLayout);
	vl->addWidget(groupBox);

	for (QString lmId : lmIdList)
	{
		QCheckBox* enableLmCheck = new QCheckBox(lmId, &signalCreationSettingsDialog);
		enableLmCheck->setChecked(true);

		groupBoxLayout->addWidget(enableLmCheck);
		checkBoxList.append(enableLmCheck);

		connect(enableLmCheck, &QCheckBox::toggled, [&checkBoxList, enableLmCheck](bool checked){
			if (checked)
			{
				return;
			}
			bool hasCheckedLm = false;
			for (QCheckBox* lmCheck : checkBoxList)
			{
				if (lmCheck->isChecked())
				{
					hasCheckedLm = true;
					break;
				}
			}
			if (!hasCheckedLm)
			{
				enableLmCheck->setChecked(true);
			}
		});
	}

	QComboBox* signalTypeCombo = new QComboBox(&signalCreationSettingsDialog);
	signalTypeCombo->addItems(QStringList() << tr("Analog") << tr("Discrete"));
	signalTypeCombo->setCurrentIndex(0);

	fl->addRow(tr("Signal type"), signalTypeCombo);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::accepted, &signalCreationSettingsDialog, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &signalCreationSettingsDialog, &QDialog::reject);

	vl->addLayout(fl);
	vl->addStretch();
	vl->addWidget(buttonBox);

	signalCreationSettingsDialog.setLayout(vl);

	signalCreationSettingsDialog.setWindowTitle("Signal creation settings");

	if (signalCreationSettingsDialog.exec() != QDialog::Accepted)
	{
		return QStringList();
	}

	E::SignalType type = static_cast<E::SignalType>(signalTypeCombo->currentIndex());

	for (QCheckBox* check : checkBoxList)
	{
		if (check->isChecked())
		{
			selectedLmIdList << check->text();
		}
	}

	int channelNo = 0;
	for (QString lmId : selectedLmIdList)
	{
		QString signalSuffix = QString("%1%2").arg(type == E::SignalType::Analog ? "A" : "D").arg(schemaCounter, 4, 10, QChar('0'));
		if (lmIdList.count() > 1)
		{
			signalSuffix += QString("_%1").arg(QChar('A' + channelNo));
		}

		QString newSignalExtStrId = QString("%1_%2").arg(schemaId).arg(signalSuffix);
		newSignalExtStrId.replace("#", "");
		QString newSignalStrId = newSignalExtStrId;

		QString newSignalCaption = QString("App signal %1 at schema \"%2\"").arg(signalSuffix).arg(schemaCaption);

		if (newSignalExtStrId[0] == QChar('#'))
		{
			newSignalExtStrId = newSignalExtStrId.mid(1);
		}
		if (newSignalStrId[0] != QChar('#'))
		{
			newSignalStrId = "#" + newSignalStrId;
		}

		Signal newSignal;

		newSignal.setSignalType(type);

		if (type == E::SignalType::Analog)
		{
			newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
			newSignal.setDataSize(FLOAT32_SIZE);
		}
		else
		{
			newSignal.setDataSize(DISCRETE_SIZE);
		}

		newSignal.setAppSignalID(newSignalStrId);
		newSignal.setCustomAppSignalID(newSignalExtStrId);
		newSignal.setEquipmentID(lmId);
		newSignal.setCaption(newSignalCaption);
		signalVector.push_back(newSignal);

		channelNo++;
	}

	if (signalVector.isEmpty())
	{
		return QStringList();
	}

	QVector<Signal*> signalPtrVector;
	for (Signal& signal : signalVector)
	{
		signalPtrVector.push_back(&signal);
	}
	SignalPropertiesDialog dlg(signalPtrVector, *Signal::unitList.get(), false, nullptr, parent);

	if (dlg.exec() != QDialog::Accepted )
	{
		return QStringList();
	}

	if (dbController->addSignal(type, &signalVector, parent) == false)
	{
		return QStringList();
	}

	QStringList result;
	for (Signal& signal : signalVector)
	{
		result << signal.appSignalID();
	}

	return result;
}

void SignalsTabPage::CreateActions(QToolBar *toolBar)
{
	QAction* action = new QAction(QIcon(":/Images/Images/update.png"), tr("Refresh signal list"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::loadSignals);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/undo.png"), tr("Undo signal changes"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::undoSignalChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/checkin.png"), tr("CheckIn"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::checkIn);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/changes.png"), tr("Show pending changes..."), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::showPendingChanges);
	connect(m_signalsModel, &SignalsModel::setCheckedoutSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/plus.png"), tr("Create signal"), this);
	connect(action, &QAction::triggered, m_signalsModel, &SignalsModel::addSignal);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/copy.png"), tr("Clone signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::cloneSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/cross.png"), tr("Delete signal"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::deleteSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
	m_signalsView->addAction(action);
	toolBar->addAction(action);

	action = new QAction(QIcon(":/Images/Images/pencil.png"), tr("Properties"), this);
	connect(action, &QAction::triggered, this, &SignalsTabPage::editSignal);
	connect(this, &SignalsTabPage::setSignalActionsVisibility, action, &QAction::setVisible);
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
		m_signalsView->selectRow(m_signalsProxyModel->mapFromSource(m_signalsModel->index(m_signalsModel->keyIndex(selectedSignalId[i]), 0)).row());
	}
	m_signalsView->setSelectionMode(selectionMode);
}

void SignalsTabPage::cloneSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}

	QSet<int> clonedSignalIDs;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		int id = m_signalsModel->key(row);
		clonedSignalIDs.insert(id);
	}

	m_selectedRowsSignalID = m_signalsModel->cloneSignals(clonedSignalIDs);
	if (!m_selectedRowsSignalID.isEmpty())
	{
		m_focusedCellSignalID = m_selectedRowsSignalID[0];
	}

	m_signalsView->clearSelection();

	restoreSelection();
}

void SignalsTabPage::deleteSignal()
{
	QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
	if (selection.count() == 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
	}
	QSet<int> deletedSignalIDs;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		int groupId = m_signalsModel->signal(row).signalGroupID();
		if (groupId != 0)
		{
			deletedSignalIDs.unite(m_signalsModel->getChannelSignalsID(groupId).toList().toSet());
		}
		else
		{
			deletedSignalIDs.insert(m_signalsModel->key(row));
		}
	}
	m_signalsModel->deleteSignals(deletedSignalIDs);
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

	CheckinSignalsDialog dlg(tr("Pending changes"), m_signalsModel, sourceSelection.indexes(), true, this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::checkIn()
{
	const QItemSelection& proxySelection = m_signalsView->selectionModel()->selection();
	const QItemSelection& sourceSelection = m_signalsProxyModel->mapSelectionToSource(proxySelection);

	CheckinSignalsDialog dlg(tr("CheckIn"), m_signalsModel, sourceSelection.indexes(), false, this);

	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	m_signalsModel->loadSignals();
}

void SignalsTabPage::changeSignalActionsVisibility()
{
	if (m_changingSelectionManualy)
	{
		return;
	}
	if (!m_signalsView->selectionModel()->hasSelection())
	{
		emit setSignalActionsVisibility(false);
	}
	else
	{
		QModelIndexList&& selection = m_signalsView->selectionModel()->selectedIndexes();
		QSet<int> checkedRows;
		for (int i = 0; i < selection.count(); i++)
		{
			int row = selection[i].row();
			if (checkedRows.contains(row))
			{
				continue;
			}
			checkedRows.insert(row);
			row = m_signalsProxyModel->mapToSource(selection[i]).row();
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
	m_changingSelectionManualy = true;

	QModelIndex currentSourceIndex = m_signalsModel->index(m_signalsModel->keyIndex(m_focusedCellSignalID), m_focusedCellColumn);
	QModelIndex currentProxyIndex = m_signalsProxyModel->mapFromSource(currentSourceIndex);

	QItemSelection selection;

	//int selectionRowCount = 0;
	foreach (int id, m_selectedRowsSignalID)
	{
		int rowNo = m_signalsModel->keyIndex(id);

		QModelIndex leftIndex  = m_signalsModel->index(rowNo, 0);
		QModelIndex rightIndex = m_signalsModel->index(rowNo, m_signalsModel->columnCount() -1);

		QItemSelection rowSelection(leftIndex, rightIndex);
		selection.merge(rowSelection, QItemSelectionModel::Select);

		/*selectionRowCount++;

		if (selectionRowCount > 256)
		{
			// Selection limits has been added, because m_signalsView->selectionModel()->select(...) becomes extremely slow
			break;
		}*/
	}

	m_signalsView->selectionModel()->setCurrentIndex(currentProxyIndex, QItemSelectionModel::Select);
	m_signalsView->selectionModel()->select(currentProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	//m_signalsView->selectionModel()->select(m_signalsProxyModel->mapSelectionFromSource(selection), QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_signalsView->horizontalScrollBar()->setValue(m_lastHorizontalScrollPosition);
	m_signalsView->verticalScrollBar()->setValue(m_lastVerticalScrollPosition);

	m_signalsView->scrollTo(currentProxyIndex);

	m_changingSelectionManualy = false;
	changeSignalActionsVisibility();
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
}

void SignalsTabPage::changeSignalIdFilter(QStringList strIds, bool refreshSignalList)
{
	// Update signals
	//
	if (refreshSignalList == true)
	{
		m_signalsModel->loadSignals();
	}

	// Reset signal type filter
	//
	if (sender() != nullptr && typeid(*sender()) == typeid(GlobalMessanger))
	{
		for (int i = 0; i < m_signalTypeFilterCombo->count(); i++)
		{
			if (m_signalTypeFilterCombo->itemData(i) == ST_ANY)
			{
				m_signalTypeFilterCombo->setCurrentIndex(i);
			}
		}
		m_signalsProxyModel->setSignalTypeFilter(ST_ANY);
		m_signalIdFieldCombo->setCurrentIndex(FI_EQUIPMENT_ID);
		m_signalsProxyModel->setIdFilterField(FI_EQUIPMENT_ID);
	}

	// Set signal id filter
	//
	m_signalsProxyModel->setSignalIdFilter(strIds);

	// Set signal id filter editor text and save filter history
	//
	QString newFilter = strIds.join(" | ");
	while (newFilter.indexOf("  ") != -1)
	{
		newFilter.replace("  ", " ");
	}

	if (!newFilter.isEmpty() && !m_filterHistory.contains(newFilter))
	{
		m_filterHistory.append(newFilter);

		QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
		assert(model != nullptr);
		if (model != nullptr)
		{
			model->setStringList(m_filterHistory);
		}

		QSettings settings(QSettings::UserScope, qApp->organizationName());
		settings.setValue("SignalsTabPage/filterHistory", m_filterHistory);
	}

	m_filterEdit->setText(newFilter);

	GlobalMessanger::instance()->fireChangeCurrentTab(this);
}

void SignalsTabPage::applySignalIdFilter()
{
	m_signalsProxyModel->setIdFilterField(m_signalIdFieldCombo->currentIndex());
	changeSignalIdFilter(m_filterEdit->text().trimmed().split("|", QString::SkipEmptyParts), false);
}

void SignalsTabPage::resetSignalIdFilter()
{
	m_signalsProxyModel->setSignalIdFilter(QStringList());
	m_signalsProxyModel->setSignalTypeFilter(ST_ANY);
	m_filterEdit->setText("");
	m_signalTypeFilterCombo->setCurrentIndex(0);
}

void SignalsTabPage::changeColumnVisibility(QAction* action)
{
	int actionIndex = m_tableHeadersContextMenuActions->actions().indexOf(action);
	int columnIndex = actionIndex - 1;
	if (actionIndex == 0)
	{
		for (int i = 0; i < COLUMNS_COUNT; i++)
		{
			if (!action->isChecked())
			{
				saveColumnWidth(i);
			}
			saveColumnVisibility(i, action->isChecked());
			m_signalsView->setColumnHidden(i, !action->isChecked());
			m_tableHeadersContextMenuActions->actions()[i + 1]->setChecked(action->isChecked());
		}
	}
	else
	{
		if (!action->isChecked())
		{
			saveColumnWidth(columnIndex);
		}
		saveColumnVisibility(columnIndex, action->isChecked());
		m_signalsView->setColumnHidden(columnIndex, !action->isChecked());
		if (action->isChecked() && m_signalsView->columnWidth(columnIndex) == 0)
		{
			QSettings settings(QSettings::UserScope, qApp->organizationName());
			int newValue = settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[columnIndex]).replace("/", "|")).replace("\n", " "), DEFAULT_COLUMN_WIDTH).toInt();
			if (newValue == 0)
			{
				newValue = DEFAULT_COLUMN_WIDTH;
			}
			m_signalsView->setColumnWidth(columnIndex, newValue);
		}
	}
	if (m_signalsView->horizontalHeader()->hiddenSectionCount() == COLUMNS_COUNT)
	{
		m_signalsView->showColumn(SC_STR_ID);
		m_tableHeadersContextMenuActions->actions()[SC_STR_ID + 1]->setChecked(true);
		saveColumnVisibility(SC_STR_ID, true);
	}
}

void SignalsTabPage::showError(QString message)
{
	QMessageBox::warning(this, "Error", message);
}

void SignalsTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isSignal() == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<SignalProperties> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<Signal> outSignals;

			bool ok = db()->getSpecificSignals(&signalIds, compareData.sourceChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				source = std::make_shared<SignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Signal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				source = std::make_shared<SignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<SignalProperties> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			std::vector<int> signalIds;
			signalIds.push_back(object.id());

			std::vector<Signal> outSignals;

			bool ok = db()->getSpecificSignals(&signalIds, compareData.targetChangeset, &outSignals, this);
			if (ok == true && outSignals.size() == 1)
			{
				target = std::make_shared<SignalProperties>(outSignals.front());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			assert(false);
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Signal outSignal;

			bool ok = db()->getLatestSignal(object.id(), &outSignal, this);
			if (ok == true)
			{
				target = std::make_shared<SignalProperties>(outSignal);
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}

void SignalsTabPage::saveColumnWidth(int index)
{
	int width = m_signalsView->columnWidth(index);
	if (width == 0)
	{
		return;
	}
	QSettings settings(QSettings::UserScope, qApp->organizationName());
	settings.setValue(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[index]).replace("/", "|")).replace("\n", " "), width);
}

void SignalsTabPage::saveColumnVisibility(int index, bool visible)
{
	QSettings settings(QSettings::UserScope, qApp->organizationName());
	settings.setValue(QString("SignalsTabPage/ColumnVisibility/%1").arg(QString(Columns[index]).replace("/", "|")).replace("\n", " "), visible);
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


CheckinSignalsDialog::CheckinSignalsDialog(QString title, SignalsModel *sourceModel, QModelIndexList selection, bool showUndoButton, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(title);

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
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	m_signalsView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_signalsView->horizontalHeader()->setHighlightSections(false);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));

	QSettings settings(QSettings::UserScope, qApp->organizationName());
	m_signalsView->setColumnWidth(0, settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[0]).replace("/", "|")).replace("\n", " "),
													m_signalsView->columnWidth(0)).toInt() + 30);	// basic column width + checkbox size
	for (int i = 1; i < COLUMNS_COUNT; i++)
	{
		bool checked = settings.value(QString("SignalsTabPage/ColumnVisibility/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
									  defaultColumnVisibility.contains(i)).toBool();
		m_signalsView->setColumnHidden(i, !checked);

		if (checked)
		{
			m_signalsView->setColumnWidth(i, settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
															m_signalsView->columnWidth(i)).toInt());
		}
	}

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

	QPushButton* checkinSelectedButton = new QPushButton(tr("CheckIn"), this);
	connect(checkinSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::checkinSelected);
	hl->addWidget(checkinSelectedButton);

	if (showUndoButton)
	{
		QPushButton* undoSelectedButton = new QPushButton(tr("Undo"), this);
		connect(undoSelectedButton, &QPushButton::clicked, this, &CheckinSignalsDialog::undoSelected);
		hl->addWidget(undoSelectedButton);
	}

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, &QPushButton::clicked, this, &CheckinSignalsDialog::cancel);
	hl->addWidget(cancelButton);

	vl1->addLayout(hl);

	setLayout(vl1);

	resize(settings.value("PendingChangesDialog/size", qApp->desktop()->screenGeometry(this).size() * 3 / 4).toSize());
	m_splitter->setChildrenCollapsible(false);

	QList<int> list = m_splitter->sizes();
	list[0] = height();
	list[1] = m_commentEdit->height();
	m_splitter->setSizes(list);

	m_splitter->restoreState(settings.value("PendingChangesDialog/splitterPosition", m_splitter->saveState()).toByteArray());
}

void CheckinSignalsDialog::checkinSelected()
{
	QString commentText = m_commentEdit->toPlainText();
	if (commentText.isEmpty())
	{
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("Checkin comment is empty"));
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
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
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
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	foreach (int ID, IDs)
	{
		ObjectState state;
		m_sourceModel->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parentWindow());
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
	QSettings settings(QSettings::UserScope, qApp->organizationName());
	settings.setValue("PendingChangesDialog/size", size());
	settings.setValue("PendingChangesDialog/splitterPosition", m_splitter->saveState());
}



UndoSignalsDialog::UndoSignalsDialog(SignalsModel* sourceModel, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Undo signal changes"));

	QSettings settings(QSettings::UserScope, qApp->organizationName());
	resize(settings.value("UndoSignalsDalog/size", qApp->desktop()->screenGeometry(this).size() * 3 / 4).toSize());

	QVBoxLayout* vl = new QVBoxLayout;

	QTableView* signalsView = new QTableView(this);
	m_proxyModel = new CheckedoutSignalsModel(sourceModel, signalsView, this);

	QCheckBox* selectAll = new QCheckBox(tr("Select all"), this);
	connect(selectAll, &QCheckBox::toggled, m_proxyModel, &CheckedoutSignalsModel::setAllCheckStates);
	vl->addWidget(selectAll);

	signalsView->setModel(m_proxyModel);
	signalsView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

	signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(signalsView->fontMetrics().height() * 1.4));

	signalsView->setColumnWidth(0, settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[0]).replace("/", "|")).replace("\n", " "),
												  signalsView->columnWidth(0)).toInt() + 30);	// basic column width + checkbox size
	for (int i = 1; i < COLUMNS_COUNT; i++)
	{
		bool checked = settings.value(QString("SignalsTabPage/ColumnVisibility/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
									  defaultColumnVisibility.contains(i)).toBool();
		signalsView->setColumnHidden(i, !checked);

		if (checked)
		{
			signalsView->setColumnWidth(i, settings.value(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " "),
														  signalsView->columnWidth(i)).toInt());
		}
	}

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
		QMessageBox::warning(m_sourceModel->parentWindow(), tr("Warning"), tr("No one signal was selected!"));
		return;
	}
	QVector<ObjectState> states;
	foreach (int ID, IDs)
	{
		ObjectState state;
		m_sourceModel->dbController()->undoSignalChanges(ID, &state, m_sourceModel->parentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}
	if (!states.isEmpty())
	{
		m_sourceModel->showErrors(states);
	}

	QSettings settings(QSettings::UserScope, qApp->organizationName());
	settings.setValue("UndoSignalsDialog/size", size());

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
	const Signal& currentSignal = m_sourceModel->signal(source_row);
	if (!(m_signalType == ST_ANY || m_signalType == currentSignal.signalTypeInt()))
	{
		return false;
	}
	if (m_strIdMasks.isEmpty())
	{
		return true;
	}
	QString strId;
	switch (m_idFilterField)
	{
		case FI_APP_SIGNAL_ID:
			strId = currentSignal.appSignalID().trimmed();
			break;
		case FI_CUSTOM_APP_SIGNAL_ID:
			strId = currentSignal.customAppSignalID().trimmed();
			break;
		case FI_EQUIPMENT_ID:
			strId = currentSignal.equipmentID().trimmed();
			break;
		default:
			assert(false);
			return false;
	}
	for (QString idMask : m_strIdMasks)
	{
		QRegExp rx(idMask.trimmed());
		rx.setPatternSyntax(QRegExp::Wildcard);
		if (rx.exactMatch(strId))
		{
			return true;
		}
	}
	return false;
}

void SignalsProxyModel::setSignalTypeFilter(int signalType)
{
	if (m_signalType != signalType)
	{
		m_signalType = signalType;

		invalidateFilter();
	}
}

void SignalsProxyModel::setSignalIdFilter(QStringList strIds)
{
	bool equal = true;
	if (m_strIdMasks.count() != strIds.count())
	{
		equal = false;
	}
	else
	{
		for (int i = 0; i < m_strIdMasks.count(); i++)
		{
			if (m_strIdMasks[i] != strIds[i])
			{
				equal = false;
				break;
			}
		}
	}
	if (!equal)
	{
		m_strIdMasks = strIds;

		invalidateFilter();
	}
}

void SignalsProxyModel::setIdFilterField(int field)
{
	if (m_idFilterField != field)
	{
		m_idFilterField = field;

		invalidateFilter();
	}
}
