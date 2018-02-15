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
#include <QStandardItemModel>

#include "../lib/DbController.h"

#include "SignalsTabPage.h"
#include "Settings.h"
#include "../lib/SignalProperties.h"
#include "../lib/WidgetUtils.h"
#include "SignalPropertiesDialog.h"
#include "./Forms/ComparePropertyObjectDialog.h"
#include "BusStorage.h"

const int SC_STR_ID = 0,
SC_EXT_STR_ID = 1,
SC_DEVICE_STR_ID = 2,
SC_BUS_TYPE_ID = 3,
SC_NAME = 4,
SC_CHANNEL = 5,
SC_TYPE = 6,
SC_ACQUIRE = 7,
SC_IN_OUT_TYPE = 8,
SC_DATA_SIZE = 9,
SC_ANALOG_SIGNAL_FORMAT = 10,
SC_LOW_ADC = 11,
SC_HIGH_ADC = 12,
SC_LOW_LIMIT = 13,
SC_HIGH_LIMIT = 14,
SC_UNIT = 15,
SC_DROP_LIMIT = 16,
SC_EXCESS_LIMIT = 17,
SC_OUTPUT_MODE = 18,
SC_DECIMAL_PLACES = 19,
SC_APERTURE = 20,
SC_FILTERING_TIME = 21,
SC_SPREAD_TOLERANCE = 22,
SC_BYTE_ORDER = 23,
SC_ENABLE_TUNING = 24,
SC_TUNING_DEFAULT_VALUE = 25,
SC_LAST_CHANGE_USER = 26;


const char* Columns[] =
{
	"AppSignalID",
	"CustomAppSignalID",
	"EquipmentID",
	"BusTypeID",
	"Caption",
	"Channel",
	"A/D/B",
	"Acquire",
	"Input-output\ntype",
	"Data\nsize",
	"Analog\nSignal format",
	"Low ADC",
	"High ADC",
	"Low\nEngeneering Units",
	"High\nEngeneering Units",
	"Unit",
	"Low\nValid Range",
	"High\nValid Range",
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



SignalsDelegate::SignalsDelegate(SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* proxyModel, QObject *parent) :
	QStyledItemDelegate(parent),
	m_signalSet(signalSet),
	m_model(model),
	m_proxyModel(proxyModel)
{
	connect(this, &QAbstractItemDelegate::closeEditor, this, &SignalsDelegate::onCloseEditorEvent);
}

QWidget *SignalsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int col = index.column();
	int row = m_proxyModel->mapToSource(index).row();

	m_model->loadSignal(m_model->key(row), false);	// get current checkedOut state

	Signal& s = m_signalSet[row];
	if (!s.checkedOut())
	{
		signalIdForUndoOnCancelEditing = s.ID();
	}
	else
	{
		signalIdForUndoOnCancelEditing = -1;
	}

	if (!m_model->checkoutSignal(row))
	{
		return nullptr;
	}

	m_model->loadSignal(m_model->key(row), false);	// update new checkedOut state on view

	switch (col)
	{
		// LineEdit
		//
		case SC_STR_ID:
		case SC_EXT_STR_ID:
		case SC_BUS_TYPE_ID:
		{
			QLineEdit* le = new QLineEdit(parent);
			QRegExp rx4ID("^[#]?[A-Za-z\\d_]*$");
			le->setValidator(new QRegExpValidator(rx4ID, le));
			return le;
		}
		case SC_NAME:
		case SC_UNIT:
		{
			QLineEdit* le = new QLineEdit(parent);
			QRegExp rx4Name("^.+$");
			le->setValidator(new QRegExpValidator(rx4Name, le));
			return le;
		}
		case SC_LOW_ADC:
		case SC_HIGH_ADC:
		case SC_DECIMAL_PLACES:
		{
			QLineEdit* le = new QLineEdit(parent);
			le->setValidator(new QIntValidator(le));
			return le;
		}
		case SC_DATA_SIZE:
		{
			return nullptr;	//Read only
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
		case SC_ANALOG_SIGNAL_FORMAT:
		{
			QComboBox* cb = new QComboBox(parent);
			cb->addItems(E::enumKeyStrings<E::AnalogAppSignalFormat>());
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
			return nullptr;	//Read only
			QComboBox* cb = new QComboBox(parent);

			cb->addItems(E::enumKeyStrings<E::SignalInOutType>());

			return cb;
		}
		case SC_BYTE_ORDER:
		{
			return nullptr;	//Read only
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
		case SC_BUS_TYPE_ID: if (le) le->setText(s.busTypeID()); break;
		case SC_NAME: if (le) le->setText(s.caption()); break;
		case SC_DEVICE_STR_ID: if (le) le->setText(s.equipmentID()); break;

		case SC_DATA_SIZE: if (le) le->setText(QString::number(s.dataSize())); break;
		case SC_LOW_ADC: if (le) le->setText(QString::number(s.lowADC())); break;
		case SC_HIGH_ADC: if (le) le->setText(QString::number(s.highADC())); break;
		case SC_DECIMAL_PLACES: if (le) le->setText(QString::number(s.decimalPlaces())); break;

		case SC_LOW_LIMIT: if (le) le->setText(QString("%1").arg(s.lowEngeneeringUnits())); break;
		case SC_HIGH_LIMIT: if (le) le->setText(QString("%1").arg(s.highEngeneeringUnits())); break;
		case SC_DROP_LIMIT: if (le) le->setText(QString("%1").arg(s.lowValidRange())); break;
		case SC_EXCESS_LIMIT: if (le) le->setText(QString("%1").arg(s.highValidRange())); break;
		case SC_APERTURE: if (le) le->setText(QString("%1").arg(s.coarseAperture())); break;
		case SC_FILTERING_TIME: if (le) le->setText(QString("%1").arg(s.filteringTime())); break;
		case SC_SPREAD_TOLERANCE: if (le) le->setText(QString("%1").arg(s.spreadTolerance())); break;
		case SC_TUNING_DEFAULT_VALUE: if (le) le->setText(QString("%1").arg(s.tuningDefaultValue().toDouble())); break;
		// ComboBox
		//
		case SC_ANALOG_SIGNAL_FORMAT: assert(false);/* WhiteMan if (cb) cb->setCurrentIndex(m_dataFormatInfo.keyIndex(s.analogSignalFormatInt()));*/ break;
		case SC_UNIT: if (le) le->setText(s.unit()); break;
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
		case SC_STR_ID:
		{
			if (le)
			{
				QString strId = le->text();
				if (strId.isEmpty() || strId[0] != '#')
				{
					strId = '#' + strId;
				}
				s.setAppSignalID(strId.trimmed());
			}
			break;
		}
		break;
		case SC_EXT_STR_ID:
		{
			if (le)
			{
				QString strId = le->text();
				if (!strId.isEmpty() && strId[0] == '#')
				{
					strId = strId.mid(1);
				}
				s.setCustomAppSignalID(strId.trimmed());
			}
			break;
		}
		case SC_BUS_TYPE_ID:
		{
			if (le)
			{
				QString busId = le->text();
				if (!busId.isEmpty() && busId[0] == '#')
				{
					busId = busId.mid(1);
				}
				s.setBusTypeID(busId.trimmed());
			}
			break;
		}
		case SC_NAME: if (le) s.setCaption(le->text().trimmed()); break;
		case SC_DEVICE_STR_ID: if (le) s.setEquipmentID(le->text().trimmed()); break;

		case SC_DATA_SIZE: if (le) s.setDataSize(le->text().toInt()); break;
		case SC_LOW_ADC: if (le) s.setLowADC(le->text().toInt()); break;
		case SC_HIGH_ADC: if (le) s.setHighADC(le->text().toInt()); break;
		case SC_DECIMAL_PLACES: if (le) s.setDecimalPlaces(le->text().toInt()); break;

		case SC_LOW_LIMIT: if (le) s.setLowEngeneeringUnits(le->text().toDouble()); break;
		case SC_HIGH_LIMIT: if (le) s.setHighEngeneeringUnits(le->text().toDouble()); break;
		case SC_DROP_LIMIT: if (le) s.setLowValidRange(le->text().toDouble()); break;
		case SC_EXCESS_LIMIT: if (le) s.setHighValidRange(le->text().toDouble()); break;
		case SC_APERTURE: if (le) s.setCoarseAperture(le->text().toDouble()); break;
		case SC_FILTERING_TIME: if (le) s.setFilteringTime(le->text().toDouble()); break;
		case SC_SPREAD_TOLERANCE: if (le) s.setSpreadTolerance(le->text().toDouble()); break;
		case SC_TUNING_DEFAULT_VALUE: if (le) s.setTuningDefaultValue(TuningValue::createFromDouble(s.signalType(), s.analogSignalFormat(), le->text().toDouble())); break;
		// ComboBox
		//
		case SC_ANALOG_SIGNAL_FORMAT: assert(false); /* WhiteMan if (cb) s.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(m_dataFormatInfo.keyAt(cb->currentIndex())));*/ break;
		case SC_UNIT: if (le) s.setUnit(le->text().trimmed()); break;
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
	signalIdForUndoOnCancelEditing = -1;
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

void SignalsDelegate::onCloseEditorEvent(QWidget*, QAbstractItemDelegate::EndEditHint hint)
{
	if (hint == QAbstractItemDelegate::RevertModelCache && signalIdForUndoOnCancelEditing != -1)
	{
		m_model->undoSignal(signalIdForUndoOnCancelEditing);
		signalIdForUndoOnCancelEditing = -1;
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

QString SignalsModel::getSensorStr(int sensorType) const
{
	if (sensorType >= 0 && sensorType < SENSOR_TYPE_COUNT)
	{
		return SensorTypeStr[sensorType];
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
		if (signal.checkedOut() && (signal.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
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
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
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
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id, true);
	}
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

bool SignalsModel::checkoutSignal(int index, QString& message)
{
	Signal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
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
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			return false;
		}
	}
	for (int id : signalsIDs)
	{
		loadSignal(id, true);
	}
	emit setCheckedoutSignalActionsVisibility(true);
	return true;
}

bool SignalsModel::undoSignal(int id)
{
	const Signal& s = m_signalSet[m_signalSet.keyIndex(id)];
	if (!s.checkedOut())
	{
		return false;
	}

	QVector<int> signalsIDs;
	if (s.signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(s.signalGroupID());
	}
	else
	{
		signalsIDs << id;
	}
	QVector<ObjectState> states;

	for (int id : signalsIDs)
	{
		ObjectState state;
		dbController()->undoSignalChanges(id, &state, parentWindow());
		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}

	if (!states.isEmpty())
	{
		showErrors(states);
	}

	for (int id : signalsIDs)
	{
		loadSignal(id, true);
	}

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
		QString message = errorMessage(state);
		if (!message.isEmpty())
		{
			QMessageBox::critical(m_parentWindow, tr("Error"), errorMessage(state));
		}
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
		switch (signal.signalType())
		{
			case E::SignalType::Analog:
				switch (col)
				{
					case SC_LAST_CHANGE_USER: return signal.checkedOut() ? getUserStr(signal.userID()) : "";
					case SC_STR_ID: return signal.appSignalID();
					case SC_EXT_STR_ID: return signal.customAppSignalID();
					case SC_BUS_TYPE_ID: return signal.busTypeID();
					case SC_NAME: return signal.caption();
					case SC_CHANNEL: return E::valueToString<E::Channel>(signal.channelInt());
					case SC_TYPE: return QChar('A');
					case SC_ANALOG_SIGNAL_FORMAT: return E::valueToString<E::AnalogAppSignalFormat>(signal.analogSignalFormat());
					case SC_DATA_SIZE: return signal.dataSize();
					case SC_LOW_ADC: return QString("0x%1").arg(signal.lowADC(), 4, 16, QChar('0'));
					case SC_HIGH_ADC: return QString("0x%1").arg(signal.highADC(), 4, 16, QChar('0'));
					case SC_LOW_LIMIT: return signal.lowEngeneeringUnits();
					case SC_HIGH_LIMIT: return signal.highEngeneeringUnits();
					case SC_UNIT: return signal.unit();

					case SC_DROP_LIMIT: return signal.lowValidRange();
					case SC_EXCESS_LIMIT: return signal.highValidRange();

					case SC_OUTPUT_MODE: return getOutputModeStr(signal.outputMode());

					case SC_ACQUIRE: return signal.acquire() ? tr("True") : tr("False");
					case SC_ENABLE_TUNING: return signal.enableTuning() ? tr("True") : tr("False");

					case SC_DECIMAL_PLACES: return signal.decimalPlaces();
					case SC_APERTURE: return signal.coarseAperture();
					case SC_FILTERING_TIME: return signal.filteringTime();
					case SC_SPREAD_TOLERANCE: return signal.spreadTolerance();
					case SC_TUNING_DEFAULT_VALUE: return signal.tuningDefaultValue().toDouble();

					case SC_IN_OUT_TYPE: return E::valueToString<E::SignalInOutType>(signal.inOutType());

					case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());

					case SC_DEVICE_STR_ID: return signal.equipmentID();

					default:
						assert(false);
				}
				break;

			case E::SignalType::Discrete:
			case E::SignalType::Bus:
				switch (col)
				{
					case SC_LAST_CHANGE_USER: return signal.checkedOut() ? getUserStr(signal.userID()) : "";
					case SC_STR_ID: return signal.appSignalID();
					case SC_EXT_STR_ID: return signal.customAppSignalID();
					case SC_BUS_TYPE_ID: return signal.busTypeID();
					case SC_NAME: return signal.caption();
					case SC_CHANNEL: return E::valueToString<E::Channel>(signal.channelInt());
					case SC_TYPE: return (signal.signalType() == E::SignalType::Discrete) ? QChar('D') : QChar('B');
					case SC_ANALOG_SIGNAL_FORMAT: return "";

					case SC_DATA_SIZE: return signal.dataSize();
					case SC_ACQUIRE: return signal.acquire() ? tr("True") : tr("False");
					case SC_ENABLE_TUNING: return signal.enableTuning() ? tr("True") : tr("False");
					case SC_TUNING_DEFAULT_VALUE: return signal.tuningDefaultValue().toDouble();
					case SC_IN_OUT_TYPE: return E::valueToString<E::SignalInOutType>(signal.inOutType());
					case SC_BYTE_ORDER: return E::valueToString<E::ByteOrder>(signal.byteOrderInt());
					case SC_DEVICE_STR_ID: return signal.equipmentID();

					case SC_LOW_ADC:
					case SC_HIGH_ADC:
					case SC_LOW_LIMIT:
					case SC_HIGH_LIMIT:
					case SC_UNIT:

					case SC_DROP_LIMIT:
					case SC_EXCESS_LIMIT:

					case SC_OUTPUT_MODE:

					case SC_DECIMAL_PLACES:
					case SC_APERTURE:
					case SC_FILTERING_TIME:
					case SC_SPREAD_TOLERANCE:
						return QVariant();

					default:
						assert(false);
				}
				break;

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
			if (signal.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
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

		Signal& s = m_signalSet[row];

		switch (index.column())
		{
			case SC_STR_ID: s.setAppSignalID(value.toString()); break;
			case SC_EXT_STR_ID: s.setCustomAppSignalID(value.toString()); break;
			case SC_BUS_TYPE_ID: s.setBusTypeID(value.toString()); break;
			case SC_NAME: s.setCaption(value.toString()); break;
			case SC_ANALOG_SIGNAL_FORMAT: s.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(value.toInt())); break;
			case SC_DATA_SIZE: s.setDataSize(value.toInt()); break;
			case SC_LOW_ADC: s.setLowADC(value.toInt()); break;
			case SC_HIGH_ADC: s.setHighADC(value.toInt()); break;
			case SC_LOW_LIMIT: s.setLowEngeneeringUnits(value.toDouble()); break;
			case SC_HIGH_LIMIT: s.setHighEngeneeringUnits(value.toDouble()); break;
			case SC_UNIT: s.setUnit(value.toString()); break;
			case SC_DROP_LIMIT: s.setLowValidRange(value.toDouble()); break;
			case SC_EXCESS_LIMIT: s.setHighValidRange(value.toDouble()); break;
			case SC_OUTPUT_MODE: s.setOutputMode(static_cast<E::OutputMode>(value.toInt())); break;
			case SC_ACQUIRE: s.setAcquire(value.toBool()); break;
			case SC_ENABLE_TUNING: s.setEnableTuning(value.toBool()); break;
			case SC_DECIMAL_PLACES: s.setDecimalPlaces(value.toInt()); break;
			case SC_APERTURE: s.setCoarseAperture(value.toDouble()); break;
			case SC_FILTERING_TIME: s.setFilteringTime(value.toDouble()); break;
			case SC_SPREAD_TOLERANCE: s.setSpreadTolerance(value.toDouble()); break;
			case SC_TUNING_DEFAULT_VALUE: s.setTuningDefaultValue(TuningValue::createFromDouble(s.signalType(), s.analogSignalFormat(), value.toDouble())); break;
			case SC_BYTE_ORDER: s.setByteOrder(E::ByteOrder(value.toInt())); break;
			case SC_DEVICE_STR_ID: s.setEquipmentID(value.toString()); break;
			case SC_LAST_CHANGE_USER:
			case SC_CHANNEL:
			case SC_TYPE:
			default:
				assert(false);
		}

		loadSignal(s.ID());
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

	if (!dbController()->getSignals(&m_signalSet, false, m_parentWindow))
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

void SignalsModel::loadSignal(int signalId, bool updateView)
{
	int row = keyIndex(signalId);
	if (row == -1)
	{
		return;
	}
	dbController()->getLatestSignal(signalId, &m_signalSet[row], parentWindow());
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
	if (s.checkedOut() && (s.userID() != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator()))
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
	signalTypeCombo->addItems(QStringList() << tr("Analog") << tr("Discrete") << tr("Bus"));
	signalTypeCombo->setCurrentIndex(1);

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

	QSettings settings;

	signal.setSignalType(static_cast<E::SignalType>(signalTypeCombo->currentIndex()));

	switch (signal.signalType())
	{
		case E::SignalType::Analog:
			signal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
			signal.setDataSize(FLOAT32_SIZE);
			break;

		case E::SignalType::Discrete:
			signal.setDataSize(DISCRETE_SIZE);
			break;

		case E::SignalType::Bus:
		default:
			break;
	}

	auto loader = [&settings](const QString& name)
	{
		return settings.value(lastEditedSignalFieldValuePlace + name);
	};

	signal.setLowADC(loader(lowADCCaption).toInt());
	signal.setHighADC(loader(highADCCaption).toInt());
	signal.setLowEngeneeringUnits(loader(lowEngeneeringUnitsCaption).toDouble());
	signal.setHighEngeneeringUnits(loader(highEngeneeringUnitsCaption).toDouble());
	signal.setUnit(loader(unitCaption).toString());
	signal.setLowValidRange(loader(lowValidRangeCaption).toDouble());
	signal.setHighValidRange(loader(highValidRangeCaption).toDouble());

	signal.setElectricLowLimit(loader(electricLowLimitCaption).toDouble());
	signal.setElectricHighLimit(loader(electricHighLimitCaption).toDouble());
	signal.setElectricUnit(static_cast<E::ElectricUnit>(loader(electricUnitCaption).toInt()));
	signal.setSensorType(static_cast<E::SensorType>(loader(sensorTypeCaption).toInt()));
	signal.setOutputMode(static_cast<E::OutputMode>(loader(outputModeCaption).toInt()));

	signal.setAcquire(loader(acquireCaption).toBool());
	signal.setDecimalPlaces(loader(decimalPlacesCaption).toInt());
	signal.setCoarseAperture(loader(coarseApertureCaption).toDouble());
	signal.setFineAperture(loader(fineApertureCaption).toDouble());
	signal.setFilteringTime(loader(filteringTimeCaption).toDouble());
	signal.setSpreadTolerance(loader(spreadToleranceCaption).toDouble());
	signal.setInOutType(E::SignalInOutType::Internal);
	signal.setByteOrder(E::ByteOrder::BigEndian);

	if (!deviceIdEdit->text().isEmpty())
	{
		signal.setEquipmentID(deviceIdEdit->text());
	}

	int signalCounter = dbController()->nextCounterValue();
	if (signalCounter >= 0)
	{
		QString newId = QString(E::valueToString<E::SignalType>(signal.signalType()).toUpper() + "_%1").arg(signalCounter, 3, 10, QLatin1Char('0'));
		signal.setAppSignalID('#' + newId);
		signal.setCustomAppSignalID(newId);
		signal.setCaption(newId);
	}

	SignalPropertiesDialog dlg(dbController(), QVector<Signal*>() << &signal, false, false, m_parentWindow);

	trimSignalTextFields(signal);

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
				QString suffix;

				if (signalCount > 1)
				{
					suffix = QString("_SIG%1").arg(s, 3, 10, QChar('0'));
				}

				if (channelCount > 1)
				{
					suffix += "_" + QString('A' + i);
				}

				signalVector[i].setAppSignalID((signalVector[i].appSignalID() + suffix).toUpper());
				signalVector[i].setCustomAppSignalID((signalVector[i].customAppSignalID() + suffix).toUpper());
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
			int firstInsertedSignalId = resultSignalVector[0]->ID();
			beginInsertRows(QModelIndex(), m_signalSet.count(), m_signalSet.count() + resultSignalVector.count() - 1);
			for (int i = 0; i < resultSignalVector.count(); i++)
			{
				Signal* s = resultSignalVector[i];
				m_signalSet.append(s->ID(), s);
			}
			endInsertRows();
			emit dataChanged(createIndex(m_signalSet.count() - resultSignalVector.count(), 0), createIndex(m_signalSet.count() - 1, columnCount() - 1), QVector<int>() << Qt::EditRole << Qt::DisplayRole);
			emit signalsRestored(firstInsertedSignalId);
		}
	}

	emit setCheckedoutSignalActionsVisibility(true);
}

void SignalsModel::showError(QString message)
{
	if (!message.isEmpty())
	{
		QMessageBox::critical(m_parentWindow, tr("Error"), message);
	}
}

bool SignalsModel::editSignals(QVector<int> ids)
{
	loadSignalSet(ids, false);

	bool readOnly = false;
	QVector<Signal*> signalVector;

	for (int i = 0; i < ids.count(); i++)
	{
		Signal& signal = m_signalSet[m_signalSet.keyIndex(ids[i])];
		if (signal.checkedOut() && signal.userID() != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
		{
			readOnly = true;
		}

		signalVector.append(&signal);
	}

	SignalPropertiesDialog dlg(dbController(), signalVector, readOnly, true, m_parentWindow);

	if (dlg.exec() == QDialog::Accepted)
	{
		QVector<ObjectState> states;
		for (int i = 0; i < ids.count(); i++)
		{
			if (dlg.isEditedSignal(ids[i]))
			{
				ObjectState state;
				trimSignalTextFields(*signalVector[i]);
				dbController()->setSignalWorkcopy(signalVector[i], &state, parentWindow());
				states.append(state);
			}
		}
		showErrors(states);

		loadSignalSet(ids);
		changeCheckedoutSignalActionsVisibility();
		return true;
	}

	if (dlg.hasEditedSignals())
	{
		loadSignalSet(ids);	//Signal could be checked out but not changed
		changeCheckedoutSignalActionsVisibility();
	}
	return false;
}

void SignalsModel::trimSignalTextFields(Signal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}

void SignalsModel::saveSignal(Signal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);
	dbController()->setSignalWorkcopy(&signal, &state, parentWindow());
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
		loadSignal(signal.ID());
		return;
	}
	int row = m_signalSet.keyIndex(signal.ID());
	emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
}

QList<int> SignalsModel::cloneSignals(const QSet<int>& signalIDs)
{
	QList<int> resultSignalIDs;
	m_signalSet.buildID2IndexMap();

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

		QString suffix = "_CLONE";
		int suffixNumerator = 1;
		bool hasConflict;
		do
		{
			hasConflict = false;
			for (int groupSignalID : groupSignalIDs)
			{
				if (m_signalSet.contains(m_signalSet.value(groupSignalID).appSignalID() + suffix))
				{
					hasConflict = true;
					break;
				}
			}
			if (hasConflict)
			{
				suffixNumerator++;
				suffix = QString("_CLONE%1").arg(suffixNumerator);
			}
		}
		while (hasConflict && suffixNumerator < 1000);

		if (suffixNumerator >= 1000)
		{
			assert(false);
			return QList<int>();
		}

		QVector<Signal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const Signal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;
			trimSignalTextFields(groupSignals[i]);

			groupSignals[i].setAppSignalID(groupSignal.appSignalID() + suffix);
			groupSignals[i].setCustomAppSignalID(groupSignal.customAppSignalID() + suffix);
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
	changeCheckedoutSignalActionsVisibility();
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
	m_signalTypeFilterCombo->addItem(tr("Bus signals"), ST_BUS);

	m_signalIdFieldCombo = new QComboBox(this);
	m_signalIdFieldCombo->addItem(tr("Any"), FI_ANY);
	m_signalIdFieldCombo->addItem(tr("AppSignalID"), FI_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("CustomAppSignalID"), FI_CUSTOM_APP_SIGNAL_ID);
	m_signalIdFieldCombo->addItem(tr("EquipmentID"), FI_EQUIPMENT_ID);
	m_signalIdFieldCombo->addItem(tr("Caption"), FI_CAPTION);

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

	QSettings settings;
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
	horizontalHeader->setSectionsMovable(true);

	m_signalsView->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalsView->fontMetrics().height() * 1.4));
	m_signalsView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
	horizontalHeader->setDefaultSectionSize(150);
	horizontalHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
	m_signalsView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_signalsView->setColumnWidth(SC_STR_ID, 400);
	m_signalsView->setColumnWidth(SC_EXT_STR_ID, 400);
	m_signalsView->setColumnWidth(SC_BUS_TYPE_ID, 400);
	m_signalsView->setColumnWidth(SC_NAME, 400);
	m_signalsView->setColumnWidth(SC_DEVICE_STR_ID, 400);

	QMap<int, int> positionMap;

	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		QString columnName = QString("%1").arg(QString(Columns[i]).replace("/", "|")).replace("\n", " ");
		m_signalsView->setColumnWidth(i, settings.value(QString("SignalsTabPage/ColumnWidth/") + columnName, m_signalsView->columnWidth(i)).toInt());
		horizontalHeader->setSectionHidden(i, !settings.value(QString("SignalsTabPage/ColumnVisibility/") + columnName, true).toBool());
		int position = settings.value(QString("SignalsTabPage/ColumnPosition/") + columnName, i).toInt();
		positionMap.insert(position, i);
	}

	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		int logicalIndex = positionMap[i];
		int oldVisualIndex = horizontalHeader->visualIndex(logicalIndex);
		horizontalHeader->moveSection(oldVisualIndex, i);
	}

	QAction* columnsAction = new QAction("Columns", m_signalsView);
	connect(columnsAction, &QAction::triggered, this, &SignalsTabPage::editColumnsVisibilityAndOrder);
	horizontalHeader->addAction(columnsAction);
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &SignalsTabPage::saveColumnWidth);

	m_signalsView->setStyleSheet("QTableView::item:focus{background-color:darkcyan}");

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
	auto header = m_signalsView->horizontalHeader();
	for (int i = 0; i < COLUMNS_COUNT; i++)
	{
		bool visible = !header->isSectionHidden(i);
		if (visible)
		{
			saveColumnWidth(i);
		}
		saveColumnVisibility(i, visible);
		saveColumnPosition(i, header->visualIndex(i));
	}
}

QStringList SignalsTabPage::createSignal(DbController* dbc, int counter, QString schemaId, QString schemaCaption, CreatingSignalOptions* options, QWidget* parent)
{
	assert(options);

	if (options->lmEquipmentIdList.isEmpty())
	{
		QMessageBox::warning(parent, "Error", "Lm list is empty");
		assert(false);
	}

	QVector<Signal> signalVector;

	QDialog signalCreationSettingsDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	QFormLayout* fl = new QFormLayout;

	QList<QCheckBox*> lmCheckBoxList;
	QList<QLineEdit*> appSignalIdEditList;
	QList<QLineEdit*> customSignalIdEditList;

	QVBoxLayout* vl = new QVBoxLayout;

	QGroupBox *equipmentGroupBox = new QGroupBox("EquipmentID for signals", &signalCreationSettingsDialog);
	equipmentGroupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	QGridLayout* equipmentGroupBoxLayout = new QGridLayout;
	equipmentGroupBox->setLayout(equipmentGroupBoxLayout);
	vl->addWidget(equipmentGroupBox);

	int row = 0;
	equipmentGroupBoxLayout->addWidget(new QLabel("EquipmentID", &signalCreationSettingsDialog), row, 0);
	equipmentGroupBoxLayout->addWidget(new QLabel("AppSignalID", &signalCreationSettingsDialog), row, 1);
	equipmentGroupBoxLayout->addWidget(new QLabel("CustomSignalID", &signalCreationSettingsDialog), row, 2);

	bool atLeastOneLmIsChecked = false;

	for (QString lmId : options->lmEquipmentIdList)
	{
		QCheckBox* enableLmCheck = new QCheckBox(lmId, &signalCreationSettingsDialog);
		if (options->selectedEquipmentIdList.contains(lmId))
		{
			enableLmCheck->setChecked(true);
			atLeastOneLmIsChecked = true;
		}

		equipmentGroupBoxLayout->addWidget(enableLmCheck, row + 1, 0);
		lmCheckBoxList.append(enableLmCheck);

		connect(enableLmCheck, &QCheckBox::toggled, [&lmCheckBoxList, enableLmCheck](bool checked){
			if (checked)
			{
				return;
			}
			bool hasCheckedLm = false;
			for (QCheckBox* lmCheck : lmCheckBoxList)
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

		QString appSignalID;
		if (row < options->appSignalIdList.count())
		{
			appSignalID = options->appSignalIdList[row];
		}

		QString customSignalID;
		if (row < options->customSignalIdList.count())
		{
			customSignalID = options->customSignalIdList[row];
		}

		if (customSignalID.isEmpty())
		{
			if (appSignalID.isEmpty())
			{
				customSignalID = schemaId;
				if (options->lmEquipmentIdList.count() > 1)
				{
					customSignalID += QString("_%1").arg(QChar('A'+row));
				}
			}
			else
			{
				customSignalID = appSignalID;
				customSignalID.remove('#');
			}
		}

		if (appSignalID.isEmpty())
		{
			appSignalID = customSignalID;
		}

		if (customSignalID[0] == QChar('#'))
		{
			customSignalID = customSignalID.mid(1);
		}
		if (appSignalID[0] != QChar('#'))
		{
			appSignalID = "#" + appSignalID;
		}

		QLineEdit* appSignalIdEdit = new QLineEdit(appSignalID, &signalCreationSettingsDialog);
		equipmentGroupBoxLayout->addWidget(appSignalIdEdit, row + 1, 1);
		appSignalIdEditList.append(appSignalIdEdit);

		QLineEdit* customSignalIdEdit = new QLineEdit(customSignalID, &signalCreationSettingsDialog);
		equipmentGroupBoxLayout->addWidget(customSignalIdEdit, row + 1, 2);
		customSignalIdEditList.append(customSignalIdEdit);

		row++;
	}

	if (atLeastOneLmIsChecked == false)
	{
		for (QCheckBox* lmCheckBox : lmCheckBoxList)
		{
			lmCheckBox->setChecked(true);
		}
	}

	QGroupBox *signalTypeGroupBox = new QGroupBox("Signal type", &signalCreationSettingsDialog);
	signalTypeGroupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	QVBoxLayout* signalTypeGroupBoxLayout = new QVBoxLayout;
	signalTypeGroupBox->setLayout(signalTypeGroupBoxLayout);

	QButtonGroup* signalTypeButtonGroup = new QButtonGroup(&signalCreationSettingsDialog);

	signalTypeButtonGroup->setExclusive(true);

	static const QString discreteCaption("Discrete");
	static const QString analogFloat32Caption("Analog Float32");
	static const QString analogSignedInt32Caption("Analog SignedInt32");
	static const QString busCaption("Bus");

	QVector<QRadioButton*> buttons;
	QRadioButton* busButton;
	buttons.push_back(new QRadioButton(discreteCaption, signalTypeGroupBox));
	buttons.push_back(new QRadioButton(analogFloat32Caption, signalTypeGroupBox));
	buttons.push_back(new QRadioButton(analogSignedInt32Caption, signalTypeGroupBox));
	buttons.push_back(busButton = new QRadioButton(busCaption, signalTypeGroupBox));

	QComboBox* busTypeIdComboBox = new QComboBox(&signalCreationSettingsDialog);
	busTypeIdComboBox->setEditable(true);
	busTypeIdComboBox->setValidator(new QRegExpValidator(QRegExp(cacheValidator), busTypeIdComboBox));
	busTypeIdComboBox->setVisible(false);

	BusStorage busStorage(dbc);
	// Load buses
	//
	QString errorMessage;

	if (busStorage.load(&errorMessage) == false)
	{
		QMessageBox::critical(parent, qAppName(), tr("Bus loading error: %1").arg(errorMessage));
	}

	int count = busStorage.count();

	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<VFrame30::Bus> bus = busStorage.get(i);

		busTypeIdComboBox->addItem(bus->busTypeId());
	}

	QSettings settings;
	static const QString defaultBusTypeIdCaption("SignalsTabPage/onSignalCreationFromLogicSchema/defaultBusTypeId");

	if (options->defaultBusTypeId.isEmpty())
	{
		QString defaultBusTypeId = settings.value(defaultBusTypeIdCaption,
												  QString("BUSTYPEID_%1").arg(dbc->nextCounterValue(), 4, 10, QLatin1Char('0'))).toString();
		busTypeIdComboBox->setEditText(defaultBusTypeId);
	}
	else
	{
		busTypeIdComboBox->setEditText(options->defaultBusTypeId);
	}

	QLabel* busTypeIdLabel = new QLabel("BusTypeID", &signalCreationSettingsDialog);
	busTypeIdLabel->setVisible(false);

	fl->addRow(busTypeIdLabel, busTypeIdComboBox);

	connect(busButton, &QRadioButton::toggled, busTypeIdLabel, &QLabel::setVisible);
	connect(busButton, &QRadioButton::toggled, busTypeIdComboBox, &QComboBox::setVisible);

	static const QString defaultSignalTypeCaption("SignalsTabPage/onSignalCreationFromLogicSchema/defaultSignalType");

	int defaultSignalType = options->defaultSignalTypeIndex;
	if (defaultSignalType == -1)
	{
		defaultSignalType = settings.value(defaultSignalTypeCaption, 0).toInt();
	}

	if (defaultSignalType >= buttons.count())
	{
		defaultSignalType = buttons.count() - 1;
	}

	buttons[defaultSignalType]->setChecked(true);

	int index = 0;

	for (auto b : buttons)
	{
		signalTypeButtonGroup->addButton(b, index++);
		signalTypeGroupBoxLayout->addWidget(b);
	}

	vl->addWidget(signalTypeGroupBox);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, &QDialogButtonBox::rejected, &signalCreationSettingsDialog, &QDialog::reject);
	connect(buttonBox, &QDialogButtonBox::accepted, [lmCheckBoxList, appSignalIdEditList, customSignalIdEditList, &signalCreationSettingsDialog](){
		for (int i = 0; i < lmCheckBoxList.count(); i++)
		{
			if (lmCheckBoxList[i]->isChecked() == false)
			{
				continue;
			}

			if (appSignalIdEditList[i]->text().isEmpty())
			{
				QMessageBox::warning(&signalCreationSettingsDialog, "Error", "One of AppSignalIDs is empty");
				return;
			}
			if (customSignalIdEditList[i]->text().isEmpty())
			{
				QMessageBox::warning(&signalCreationSettingsDialog, "Error", "One of CustomSignalIDs is empty");
				return;
			}

			for (int j = i + 1; j < lmCheckBoxList.count(); j++)
			{
				if (lmCheckBoxList[j]->isChecked() == false)
				{
					continue;
				}

				if (appSignalIdEditList[i]->text() == appSignalIdEditList[j]->text())
				{
					QMessageBox::warning(&signalCreationSettingsDialog, "Error", QString("AppSignalID %1 repeats").arg(appSignalIdEditList[i]->text()));
					return;
				}

				if (customSignalIdEditList[i]->text() == customSignalIdEditList[j]->text())
				{
					QMessageBox::warning(&signalCreationSettingsDialog, "Error", QString("CustomSignalID %1 repeats").arg(customSignalIdEditList[i]->text()));
					return;
				}
			}
		}
		signalCreationSettingsDialog.accept();
	});

	vl->addLayout(fl);
	vl->addStretch();
	vl->addWidget(buttonBox);

	signalCreationSettingsDialog.setLayout(vl);

	signalCreationSettingsDialog.setWindowTitle("Signal creation settings");

	if (options->settingsWindowPositionRect.isValid() == true)
	{
		setWindowPosition(&signalCreationSettingsDialog, options->settingsWindowPositionRect);
	}

	if (signalCreationSettingsDialog.exec() != QDialog::Accepted)
	{
		return QStringList();
	}

	options->settingsWindowPositionRect = signalCreationSettingsDialog.geometry();

	settings.setValue(defaultSignalTypeCaption, signalTypeButtonGroup->checkedId());

	QRadioButton* checkedSignalTypeButton = dynamic_cast<QRadioButton*>(signalTypeButtonGroup->checkedButton());
	if (checkedSignalTypeButton == nullptr)
	{
		return QStringList();
	}

	E::SignalType type;
	switch (checkedSignalTypeButton->text()[0].unicode())
	{
		case 'A':
			type = E::SignalType::Analog;
			break;

		case 'B':
			type = E::SignalType::Bus;
			settings.setValue(defaultBusTypeIdCaption, busTypeIdComboBox->currentText());
			options->defaultBusTypeId = busTypeIdComboBox->currentText();
			break;

		case 'D':
			type = E::SignalType::Discrete;
			break;
	}

	options->appSignalIdList.clear();
	options->customSignalIdList.clear();
	options->selectedEquipmentIdList.clear();
	options->defaultSignalTypeIndex = signalTypeButtonGroup->checkedId();

	int channelNo = 0;
	for (int i = 0; i < lmCheckBoxList.count(); i++)
	{
		QString currentAppSignalId = appSignalIdEditList[i]->text();
		options->appSignalIdList << currentAppSignalId;

		QString currentCustomSignalId = customSignalIdEditList[i]->text();
		options->customSignalIdList << currentCustomSignalId;

		QCheckBox* check = lmCheckBoxList[i];
		if (check->isChecked() == false)
		{
			continue;
		}
		QString lmId = check->text();
		options->selectedEquipmentIdList << lmId;

		if (currentCustomSignalId[0] == QChar('#'))
		{
			currentCustomSignalId = currentCustomSignalId.mid(1);
		}
		if (currentAppSignalId[0] != QChar('#'))
		{
			currentAppSignalId = "#" + currentAppSignalId;
		}

		QString signalSuffix = QString("%1%2").arg(QChar(E::valueToString<E::SignalType>(type)[0])).arg(counter, 4, 10, QChar('0'));
		if (options->lmEquipmentIdList.count() > 1)
		{
			signalSuffix += QString("_%1").arg(QChar('A' + channelNo));
		}

		QString newSignalCaption = QString("App signal %1 at schema \"%2\"").arg(signalSuffix).arg(schemaCaption);

		Signal newSignal;

		newSignal.setSignalType(type);

		switch (type)
		{
			case E::SignalType::Analog:
				if (checkedSignalTypeButton->text() == analogFloat32Caption)
				{
					newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
					newSignal.setDataSize(FLOAT32_SIZE);
				}

				if (checkedSignalTypeButton->text() == analogSignedInt32Caption)
				{
					newSignal.setAnalogSignalFormat(E::AnalogAppSignalFormat::SignedInt32);
					newSignal.setDataSize(SIGNED_INT32_SIZE);
				}
				break;

			case E::SignalType::Discrete:
				newSignal.setDataSize(DISCRETE_SIZE);
				break;

			case E::SignalType::Bus:
				newSignal.setBusTypeID(busTypeIdComboBox->currentText());
				break;

			default:
				break;
		}

		newSignal.setAppSignalID(currentAppSignalId);
		newSignal.setCustomAppSignalID(currentCustomSignalId);
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
	SignalPropertiesDialog dlg(dbc, signalPtrVector, false, false, parent);

	if (dlg.exec() != QDialog::Accepted )
	{
		return QStringList();
	}

	for (Signal& signal : signalVector)
	{
		SignalsModel::trimSignalTextFields(signal);
	}

	if (dbc->addSignal(type, &signalVector, parent) == false)
	{
		return QStringList();
	}

	SignalsModel* model = SignalsModel::instance();
	model->loadSignals();

	QList<int> selectIdList;
	QStringList result;
	for (Signal& signal : signalVector)
	{
		result << signal.appSignalID();
		selectIdList << signal.ID();
	}

	model->parentWindow()->setSelection(selectIdList);

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

void SignalsTabPage::keyPressEvent(QKeyEvent* e)
{
	if(e->type() == QKeyEvent::KeyPress && e->matches(QKeySequence::Copy))
	{
		QModelIndexList selection = m_signalsView->selectionModel()->selectedRows(0);
		if (selection.count() == 0)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No one signal was selected!"));
		}
		QString selectedSignalIds;
		for (int i = 0; i < selection.count(); i++)
		{
			int row = m_signalsProxyModel->mapToSource(selection[i]).row();
			selectedSignalIds.append(m_signalsModel->signal(row).appSignalID() + "\n");
		}

		QApplication::clipboard()->setText(selectedSignalIds);
	}
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
		return;
	}

	int currentRow = m_signalsProxyModel->mapToSource(m_signalsView->currentIndex()).row();
	int currentColumn = m_signalsView->currentIndex().column();
	int currentId = m_signalsModel->key(currentRow);

	QVector<int> selectedSignalId;
	for (int i = 0; i < selection.count(); i++)
	{
		int row = m_signalsProxyModel->mapToSource(selection[i]).row();
		selectedSignalId.append(m_signalsModel->key(row));
	}

	m_signalsModel->editSignals(selectedSignalId);

	m_signalsView->scrollTo(m_signalsProxyModel->mapFromSource(m_signalsModel->index(m_signalsModel->keyIndex(currentId), currentColumn)));
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

void SignalsTabPage::editColumnsVisibilityAndOrder()
{
	QDialog dlg(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	QStandardItemModel *model = new QStandardItemModel(&dlg);
	auto header = m_signalsView->horizontalHeader();
	for (int i = 0; i < header->count(); i++)
	{
		auto item = new QStandardItem;
		item->setCheckable(true);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		model->setItem(i, item);
	}

	// Helper functions
	//
	auto isHidden = [header](int logicalIndex){
		return header->isSectionHidden(logicalIndex) || header->sectionSize(logicalIndex) == 0;
	};
	auto updateHidden = [model](int visualIndex, bool hidden) {
		model->setData(model->index(visualIndex, 0), hidden ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
	};
	auto setHidden = [header](int logicalIndex, bool hidden) {
		header->setSectionHidden(logicalIndex, hidden);
		if (!hidden && header->sectionSize(logicalIndex) == 0)
		{
			header->resizeSection(logicalIndex, header->defaultSectionSize());
		}
	};

	// Update state of items from signal table header
	//
	auto updateItems = [=](){
		for (int i = 0; i < header->count(); i++)
		{
			int logicalIndex = header->logicalIndex(i);
			updateHidden(i, isHidden(logicalIndex));
			model->setData(model->index(i, 0), m_signalsModel->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString().replace('\n', ' '), Qt::DisplayRole);
		}
	};
	updateItems();

	// Child widgets layout
	//
	QListView* listView = new QListView(&dlg);
	listView->setModel(model);
	listView->setCurrentIndex(model->index(0,0));
	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(listView);
	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);
	QPushButton* upButton = new QPushButton("Up", &dlg);
	vl->addWidget(upButton);
	QPushButton* downButton = new QPushButton("Down", &dlg);
	vl->addWidget(downButton);
	vl->addStretch();
	dlg.setLayout(hl);

	//Window geometry
	//
	setWindowPosition(&dlg, "ColumnsVisibilityDialog/geometry");

	// Show/Hide column
	//
	connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item){
		int visualIndex = item->row();
		int logicalIndex = header->logicalIndex(visualIndex);
		setHidden(logicalIndex, item->checkState() != Qt::Checked);

		saveColumnVisibility(logicalIndex, item->checkState() == Qt::Checked);

		//Check if no visible column left
		//
		for (int i = 0; i < model->rowCount(); i++)
		{
			if (!isHidden(i))
			{
				return;
			}
		}
		setHidden(0, false);
		saveColumnVisibility(0, true);
		updateHidden(header->visualIndex(0), false);
	});

	// Move column left (move item up)
	//
	connect(upButton, &QPushButton::pressed, [=](){
		int visualIndex = listView->currentIndex().row();
		if (visualIndex == 0)
		{
			return;
		}

		header->moveSection(visualIndex, visualIndex - 1);

		listView->setCurrentIndex(model->index(visualIndex - 1, 0));
		updateItems();
		saveColumnPosition(header->logicalIndex(visualIndex), visualIndex);
		saveColumnPosition(header->logicalIndex(visualIndex - 1), visualIndex - 1);
	});

	// Move column right (move item down)
	//
	connect(downButton, &QPushButton::pressed, [=](){
		int visualIndex = listView->currentIndex().row();
		if (visualIndex == model->rowCount() - 1)
		{
			return;
		}

		header->moveSection(visualIndex, visualIndex + 1);

		listView->setCurrentIndex(model->index(visualIndex + 1, 0));
		updateItems();
		saveColumnPosition(header->logicalIndex(visualIndex), visualIndex);
		saveColumnPosition(header->logicalIndex(visualIndex + 1), visualIndex + 1);
	});

	dlg.exec();

	QSettings settings;
	settings.setValue("ColumnsVisibilityDialog/geometry", dlg.geometry());
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

void SignalsTabPage::setSelection(const QList<int>& selectedRowsSignalID, int focusedCellSignalID)
{
	if (selectedRowsSignalID.isEmpty())
	{
		return;
	}
	if (focusedCellSignalID == -1)
	{
		focusedCellSignalID = selectedRowsSignalID.last();
	}
	m_selectedRowsSignalID = selectedRowsSignalID;

	int focusedRow = m_signalsModel->keyIndex(focusedCellSignalID);

	m_lastVerticalScrollPosition = m_signalsView->rowViewportPosition(focusedRow);
	m_lastHorizontalScrollPosition = 0;

	restoreSelection(focusedCellSignalID);
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

void SignalsTabPage::restoreSelection(int focusedSignalId)
{
	m_changingSelectionManualy = true;

	if (focusedSignalId != -1)
	{
		m_focusedCellSignalID = focusedSignalId;
		m_focusedCellColumn = 0;
	}

	QModelIndex currentSourceIndex = m_signalsModel->index(m_signalsModel->keyIndex(m_focusedCellSignalID), m_focusedCellColumn);
	QModelIndex currentProxyIndex = m_signalsProxyModel->mapFromSource(currentSourceIndex);

	/*QItemSelection selection;

	int selectionRowCount = 0;
	foreach (int id, m_selectedRowsSignalID)
	{
		int rowNo = m_signalsModel->keyIndex(id);

		QModelIndex leftIndex  = m_signalsModel->index(rowNo, 0);
		QModelIndex rightIndex = m_signalsModel->index(rowNo, m_signalsModel->columnCount() -1);

		QItemSelection rowSelection(leftIndex, rightIndex);
		selection.merge(rowSelection, QItemSelectionModel::Select);

		selectionRowCount++;

		if (selectionRowCount > 256)
		{
			// Selection limits has been added, because m_signalsView->selectionModel()->select(...) becomes extremely slow
			break;
		}
	}

	m_signalsView->selectionModel()->select(m_signalsProxyModel->mapSelectionFromSource(selection), QItemSelectionModel::Select | QItemSelectionModel::Rows);*/

	m_signalsView->selectionModel()->setCurrentIndex(currentProxyIndex, QItemSelectionModel::Select);
	m_signalsView->selectionModel()->select(currentProxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

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
		case ST_BUS:
			for (int i = SC_ANALOG_SIGNAL_FORMAT; i < SC_LAST_CHANGE_USER; i++)
			{
				m_signalsView->setColumnHidden(i, true);
			}
			break;

		case ST_ANALOG:
		case ST_ANY:
			for (int i = SC_ANALOG_SIGNAL_FORMAT; i < SC_LAST_CHANGE_USER; i++)
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

		QSettings settings;
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

void SignalsTabPage::showError(QString message)
{
	if (!message.isEmpty())
	{
		QMessageBox::warning(this, "Error", message);
	}
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
	QSettings settings;
	settings.setValue(QString("SignalsTabPage/ColumnWidth/%1").arg(QString(Columns[index]).replace("/", "|")).replace("\n", " "), width);
}

void SignalsTabPage::saveColumnVisibility(int index, bool visible)
{
	QSettings settings;
	settings.setValue(QString("SignalsTabPage/ColumnVisibility/%1").arg(QString(Columns[index]).replace("/", "|")).replace("\n", " "), visible);
}

void SignalsTabPage::saveColumnPosition(int index, int position)
{
	QSettings settings;
	settings.setValue(QString("SignalsTabPage/ColumnPosition/%1").arg(QString(Columns[index]).replace("/", "|")).replace("\n", " "), position);
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
	return signal.checkedOut() && (signal.userID() == m_sourceModel->dbController()->currentUser().userId() || m_sourceModel->dbController()->currentUser().isAdminstrator());
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

	QSettings settings;
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

	m_splitter->setChildrenCollapsible(false);

	setWindowPosition(this, "PendingChangesDialog/geometry");

	QList<int> list = m_splitter->sizes();
	list[0] = height();
	list[1] = m_commentEdit->height();
	m_splitter->setSizes(list);

	m_splitter->restoreState(settings.value("PendingChangesDialog/splitterPosition", m_splitter->saveState()).toByteArray());
}

void CheckinSignalsDialog::checkinSelected()
{
	saveDialogGeometry();

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

	accept();
}

void CheckinSignalsDialog::undoSelected()
{
	saveDialogGeometry();

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

	accept();
}

void CheckinSignalsDialog::cancel()
{
	saveDialogGeometry();

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

void CheckinSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
}

void CheckinSignalsDialog::saveDialogGeometry()
{
	QSettings settings;
	settings.setValue("PendingChangesDialog/geometry", geometry());
	settings.setValue("PendingChangesDialog/splitterPosition", m_splitter->saveState());
}



UndoSignalsDialog::UndoSignalsDialog(SignalsModel* sourceModel, QWidget* parent) :
	QDialog(parent),
	m_sourceModel(sourceModel)
{
	setWindowTitle(tr("Undo signal changes"));

	setWindowPosition(this, "UndoSignalsDialog/geometry");

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

	QSettings settings;
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
	connect(buttonBox, &QDialogButtonBox::rejected, this, &UndoSignalsDialog::saveDialogGeometry);
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

void UndoSignalsDialog::saveDialogGeometry()
{
	QSettings settings;
	settings.setValue("UndoSignalsDialog/geometry", geometry());
}

void UndoSignalsDialog::undoSelected()
{
	saveDialogGeometry();

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

	accept();
}

void UndoSignalsDialog::closeEvent(QCloseEvent* event)
{
	saveDialogGeometry();

	QDialog::closeEvent(event);
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

	for (QString idMask : m_strIdMasks)
	{
		QRegExp rx(idMask.trimmed());
		rx.setPatternSyntax(QRegExp::Wildcard);
		bool result = false;

		switch (m_idFilterField)
		{
			case FI_ANY:
				result = rx.exactMatch(currentSignal.appSignalID().trimmed()) ||
						rx.exactMatch(currentSignal.customAppSignalID().trimmed()) ||
						rx.exactMatch(currentSignal.equipmentID().trimmed()) ||
						rx.exactMatch(currentSignal.caption().trimmed());
				break;
			case FI_APP_SIGNAL_ID:
				result = rx.exactMatch(currentSignal.appSignalID().trimmed());
				break;
			case FI_CUSTOM_APP_SIGNAL_ID:
				result = rx.exactMatch(currentSignal.customAppSignalID().trimmed());
				break;
			case FI_EQUIPMENT_ID:
				result = rx.exactMatch(currentSignal.equipmentID().trimmed());
				break;
			case FI_CAPTION:
				result = rx.exactMatch(currentSignal.caption().trimmed());
				break;
			default:
				assert(false);
				return false;
		}
		if (result == true)
		{
			return true;
		}
	}
	return false;
}

bool SignalsProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
	QVariant l = m_sourceModel->data(source_left);
	QVariant r = m_sourceModel->data(source_right);

	if (l == r)
	{
		const Signal& sl = m_sourceModel->signal(source_left.row());
		const Signal& sr = m_sourceModel->signal(source_right.row());

		return sl.appSignalID() < sr.appSignalID();
	}
	else
	{
		return QSortFilterProxyModel::lessThan(source_left, source_right);
	}
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
