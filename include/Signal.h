#ifndef SIGNAL_H
#define SIGNAL_H

#include <QString>
#include <QMultiHash>
#include "../include/Types.h"
#include "../include/DbStruct.h"
#include "../include/OrderedHash.h"
#include "../include/DeviceObject.h"
#include "../include/DataSource.h"
#include "../VFrame30/Afb.h"


class QXmlStreamAttributes;


const char* const InOutTypeStr[] =
{
	"Input",
	"Output",
	"Internal"
};

const int IN_OUT_TYPE_COUNT = sizeof(InOutTypeStr) / sizeof(InOutTypeStr[0]);


const char* const SensorTypeStr[] =
{
    "Not used",

    "TSP-50P-1.391",
    "TSP-100P-1.391",
    "TSP-50P-1.385",
    "TSP-100P-1.385",
    "TSM-50M-1.428",
	"TSM-100M-1.428",
    "TSM-50M-1.426",
    "TSM-100M-1.426",
    "TSP-21",
    "TSM-23",
    "Rheochord",

    "ТХА(K)",
    "TXK(L) 84",
    "TXK(L) 94",
    "TNN(N)",

    "BPS",
};

const int SENSOR_TYPE_COUNT = sizeof(SensorTypeStr) / sizeof(SensorTypeStr[0]);


const char* const OutputRangeModeStr[] =
{
	"0..5 V",
	"4..20 mA",
	"-10..10 V",
	"0..5 mA",
};

const int OUTPUT_RANGE_MODE_COUNT = sizeof(OutputRangeModeStr) / sizeof(OutputRangeModeStr[0]);


const int NO_UNIT_ID = 1;


struct Unit
{
	int ID;
	QString nameEn;
	QString nameRu;
};


struct DataFormatPair
{
	int ID;
	QString name;
};


// signal address struct
//
// offset	- signal offset in memory in 16-bit words
// bitNo	- discrete signal offset in memory in bits 0..15,
//			  for analog signals always 0
//
//
class Address16
{
private:
	int m_offset = -1;
	int m_bit = -1;

public:
	Address16() {}
	Address16(int offset, int bit) : m_offset(offset), m_bit(bit) {}

	void set(int offset, int bit) { m_offset = offset; m_bit = bit; }
	void setOffset(int offset) { m_offset = offset; }
	void setBit(int bit) { m_bit = bit; }

	int addWord(int wordCount)
	{
		m_offset += wordCount;
		return wordCount;
	}

	int addBit(int bitCount)
	{
		int old_offset = m_offset;
		int totalBitCount = m_offset * 16 + m_bit + bitCount;

		m_offset = totalBitCount / 16;
		m_bit = totalBitCount % 16;

		return m_offset - old_offset;
	}

	void add1Word() { addBit(1); }
	void add1Bit() { addBit(1); }

	int wordAlign()
	{
		int offset = 0;

		if (m_bit != 0)
		{
			m_offset++;
			m_bit = 0;

			offset = 1;
		}

		return offset;
	}

	int offset() const { return m_offset; }
	int bit() const { return m_bit; }

	void reset() { 	m_offset = -1; m_bit = -1; }

	bool isValid() const { return m_offset != -1 && m_bit != -1; }

	QString toString() const { return QString("%1:%2").arg(m_offset).arg(m_bit); }
	void fromString(QString str);
};


typedef OrderedHash<int, QString> UnitList;


class DataFormatList : public OrderedHash<int, QString>
{
public:
	DataFormatList();
};


const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class Signal : public PropertyObject
{
	Q_OBJECT

private:
	void InitProperties();

	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	int m_channel = 1;
	E::SignalType m_type = E::SignalType::Analog;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	E::InstanceAction m_instanceAction = E::InstanceAction::Added;

	QString m_strID;
	QString m_extStrID;
	QString m_name;
	E::DataFormat m_dataFormat = E::DataFormat::Float;
	int m_dataSize = 32;
	int m_lowADC = 0;
	int m_highADC = 0xFFFF;
	double m_lowLimit = 0;
	double m_highLimit = 100;
	int m_unitID = NO_UNIT_ID;
	double m_adjustment = 0;
	double m_dropLimit = 0;
	double m_excessLimit = 0;
	double m_unbalanceLimit = 0;
	double m_inputLowLimit = 0;
	double m_inputHighLimit = 0;
	int m_inputUnitID = NO_UNIT_ID;
	int m_inputSensorID = 0;
	double m_outputLowLimit = 0;
	double m_outputHighLimit = 0;
	int m_outputUnitID = NO_UNIT_ID;
	E::OutputRangeMode m_outputRangeMode = E::OutputRangeMode::Plus0_Plus5_V;
	int m_outputSensorID = 0;
	bool m_acquire = true;
	bool m_calculated = false;
	int m_normalState = 0;
	int m_decimalPlaces = 2;
	double m_aperture = 0;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
	QString m_deviceStrID;
	double m_filteringTime = 0.05;
	double m_maxDifference = 0.5;
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	Address16 m_iobufferAddr;			// only for modules input/output signals
										// signal address in i/o modules buffers

	Address16 m_ramAddr;				// signal address in LM RAM
	Address16 m_regAddr;				// signal address in FSC data packet (registration address)

	// Private setters for fields, witch can't be changed outside DB engine
	// Should be used only by friends
	//
	void setID(int signalID) { m_ID = signalID; }
	void setSignalGroupID(int signalGroupID) { m_signalGroupID = signalGroupID; }
	void setSignalInstanceID(int signalInstanceID) { m_signalInstanceID = signalInstanceID; }
	void setChangesetID(int changesetID) { m_changesetID = changesetID; }
	void setCheckedOut(int checkedOut) { m_checkedOut = checkedOut; }
	void setUserID(int userID) { m_userID = userID; }
	void setChannel(int channel) { m_channel = channel; }
	void setCreated(const QDateTime& created) { m_created = created; }
	void setCreated(const QString& createdStr) { m_created = QDateTime::fromString(createdStr, DATE_TIME_FORMAT_STR); }
	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceCreated(const QDateTime& instanceCreated) { m_instanceCreated = instanceCreated; }
	void setInstanceCreated(const QString& instanceCreatedStr) { m_instanceCreated = QDateTime::fromString(instanceCreatedStr, DATE_TIME_FORMAT_STR); }
	void setInstanceAction(E::InstanceAction action) { m_instanceAction = action; }

public:
	Signal();

	Signal(const Signal& signal) :
		PropertyObject()
	{
		*this = signal;

		InitProperties();
	}

	Signal(const Hardware::DeviceSignal& deviceSignal);

	Signal& operator = (const Signal& signal);

	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }
	int channel() const { return m_channel; }

	int typeInt() const { return TO_INT(m_type); }
	E::SignalType type() const { return m_type; }
	void setType(E::SignalType type) { m_type = type; }

	bool isAnalog() const { return m_type == E::SignalType::Analog; }
	bool isDiscrete() const { return m_type == E::SignalType::Discrete; }

	bool isInput() const { return m_inOutType == E::SignalInOutType::Input; }
	bool isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
	bool isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

	bool isRegistered() const { return acquire(); }

	int sizeW() const { return (m_dataSize / 16 + (m_dataSize % 16 ? 1 : 0)); }

	QDateTime created() const { return m_created; }
	bool deleted() const { return m_deleted; }
	QDateTime instanceCreated() const { return m_instanceCreated; }
	E::InstanceAction instanceAction() const { return m_instanceAction; }

	Address16& iobufferAddr() { return m_iobufferAddr; }
	Address16& ramAddr() { return m_ramAddr; }
	Address16& regAddr() { return m_regAddr; }

	const Address16& iobufferAddr() const { return m_iobufferAddr; }
	const Address16& ramAddr() const { return m_ramAddr; }
	const Address16& regAddr() const { return m_regAddr; }

	void resetAddresses() { m_ramAddr.reset(); m_regAddr.reset(); }

	void setRamAddr(const Address16& ramAddr) { m_ramAddr = ramAddr; }
	void setRegAddr(const Address16& regAddr) { m_regAddr = regAddr; }

	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(bool));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(double));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const QString&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::OutputRangeMode));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::SignalInOutType));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(E::ByteOrder));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(const Address16&));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, DataFormatList& dataFormatInfo, void (Signal::*setter)(E::DataFormat));
	void serializeField(const QXmlStreamAttributes& attr, QString fieldName, UnitList& unitInfo, void (Signal::*setter)(int));
	void serializeSensorField(const QXmlStreamAttributes& attr, QString fieldName, void (Signal::*setter)(int));

	void serializeFields(const QXmlStreamAttributes& attr, DataFormatList& dataFormatInfo, UnitList& unitInfo);

    Q_INVOKABLE QString strID() const { return m_strID; }
	void setStrID(const QString& strID) { m_strID = strID; }

    Q_INVOKABLE QString extStrID() const { return m_extStrID; }
	void setExtStrID(const QString& extStrID) { m_extStrID = extStrID; }

    Q_INVOKABLE QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	Q_INVOKABLE E::DataFormat dataFormat() const { return m_dataFormat; }
	Q_INVOKABLE int dataFormatInt() const { return TO_INT(m_dataFormat); }
	void setDataFormat(E::DataFormat dataFormat) { m_dataFormat = dataFormat; }

    Q_INVOKABLE int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }

    Q_INVOKABLE int lowADC() const { return m_lowADC; }
	void setLowADC(int lowADC) { m_lowADC = lowADC; }

    Q_INVOKABLE int highADC() const { return m_highADC; }
	void setHighADC(int highADC) { m_highADC = highADC;}

    Q_INVOKABLE double lowLimit() const { return m_lowLimit; }
	void setLowLimit(double lowLimit) { m_lowLimit = lowLimit; }

    Q_INVOKABLE double highLimit() const { return m_highLimit; }
	void setHighLimit(double highLimit) { m_highLimit = highLimit; }

    Q_INVOKABLE int unitID() const { return m_unitID; }
	void setUnitID(int unitID) { m_unitID = unitID; }

    Q_INVOKABLE double adjustment() const { return m_adjustment; }
	void setAdjustment(double adjustment) { m_adjustment = adjustment; }

    Q_INVOKABLE double dropLimit() const { return m_dropLimit; }
	void setDropLimit(double dropLimit) { m_dropLimit = dropLimit; }

    Q_INVOKABLE double excessLimit() const { return m_excessLimit; }
	void setExcessLimit(double excessLimit) { m_excessLimit = excessLimit; }

    Q_INVOKABLE double unbalanceLimit() const { return m_unbalanceLimit; }
	void setUnbalanceLimit(double unbalanceLimit) { m_unbalanceLimit = unbalanceLimit; }

    Q_INVOKABLE double inputLowLimit() const { return m_inputLowLimit; }
	void setInputLowLimit(double inputLowLimit) { m_inputLowLimit = inputLowLimit; }

    Q_INVOKABLE double inputHighLimit() const { return m_inputHighLimit; }
	void setInputHighLimit(double inputHighLimit) { m_inputHighLimit = inputHighLimit; }

    Q_INVOKABLE int inputUnitID() const { return m_inputUnitID; }
	void setInputUnitID(int inputUnitID) { m_inputUnitID = inputUnitID; }

    Q_INVOKABLE int inputSensorID() const { return m_inputSensorID; }
	void setInputSensorID(int inputSensorID) { m_inputSensorID = inputSensorID; }

    Q_INVOKABLE double outputLowLimit() const { return m_outputLowLimit; }
	void setOutputLowLimit(double outputLowLimit) { m_outputLowLimit = outputLowLimit; }

    Q_INVOKABLE double outputHighLimit() const { return m_outputHighLimit; }
	void setOutputHighLimit(double outputHighLimit) { m_outputHighLimit = outputHighLimit; }

    Q_INVOKABLE int outputUnitID() const { return m_outputUnitID; }
	void setOutputUnitID(int outputUnitID) { m_outputUnitID = outputUnitID; }

	E::OutputRangeMode outputRangeMode() const { return m_outputRangeMode; }
	Q_INVOKABLE int jsOutputRangeMode() const { return static_cast<int>(outputRangeMode());}
	void setOutputRangeMode(E::OutputRangeMode outputRangeMode) { m_outputRangeMode = outputRangeMode; }

    Q_INVOKABLE int outputSensorID() const { return m_outputSensorID; }
	void setOutputSensorID(int outputSensorID) { m_outputSensorID = outputSensorID; }

    Q_INVOKABLE bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

    Q_INVOKABLE bool calculated() const { return m_calculated; }
	void setCalculated(bool calculated) { m_calculated = calculated; }

    Q_INVOKABLE int normalState() const { return m_normalState; }
	void setNormalState	(int normalState) { m_normalState = normalState; }

    Q_INVOKABLE int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

    Q_INVOKABLE double aperture() const { return m_aperture; }
	void setAperture(double aperture) { m_aperture = aperture; }

	Q_INVOKABLE E::SignalInOutType inOutType() const { return m_inOutType; }
	void setInOutType(E::SignalInOutType inOutType) { m_inOutType = inOutType; }

    Q_INVOKABLE QString deviceStrID() const { return m_deviceStrID; }
	void setDeviceStrID(const QString& deviceStrID) { m_deviceStrID = deviceStrID; }

	Q_INVOKABLE double filteringTime() const { return m_filteringTime; }
	void setFilteringTime(double filteringTime) { m_filteringTime = filteringTime; }

	Q_INVOKABLE double maxDifference() const { return m_maxDifference; }
	void setMaxDifference(double maxDifference) { m_maxDifference = maxDifference; }

	Q_INVOKABLE E::ByteOrder byteOrder() const { return m_byteOrder; }
	Q_INVOKABLE int byteOrderInt() const { return TO_INT(m_byteOrder); }
	void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

	bool isCompatibleDataFormat(Afb::AfbDataFormat afbDataFormat) const;

	void setReadOnly(bool value);
	static std::shared_ptr<UnitList> m_unitList;

	friend class DbWorker;
};


typedef PtrOrderedHash<int, Signal> SignalPtrOrderedHash;


class SignalSet : public SignalPtrOrderedHash
{
private:
	QMultiHash<int, int> m_groupSignals;

public:
	SignalSet();
	virtual ~SignalSet();

	virtual void clear() override;

	virtual void append(const int& signalID, Signal* signal);
	virtual void remove(const int& signalID);
	virtual void removeAt(const int index);

	QVector<int> getChannelSignalsID(const Signal& signal);
	QVector<int> getChannelSignalsID(int signalGroupID);

	void reserve(int n);

	void resetAddresses();
};


void SerializeSignalsFromXml(const QString& filePath, UnitList& unitInfo, SignalSet& signalSet);
void InitDataSources(QHash<quint32, DataSource>& dataSources, Hardware::DeviceObject* deviceRoot, const SignalSet& signalSet);



#endif // SIGNAL_H
