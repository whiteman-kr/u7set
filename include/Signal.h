#ifndef SIGNAL_H
#define SIGNAL_H

#include <QString>
#include "../include/DbStruct.h"


enum SignalType
{
	analog = 0,
	discrete = 1
};

Q_DECLARE_METATYPE(SignalType);

enum SignalInOutType
{
	input = 0,
	output = 1,
	internal = 2
};


enum DataFormatType
{
	binary_LE_unsigned = 1,
	binary_LE_signed = 2,
	binary_BE_unsigned = 3,
	binary_BE_signed = 4,
};


enum InstanceAction
{
	added = 1,
	modified = 2,
	deleted = 3
};


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

const int SENSOR_TYPE_COUNT = sizeof(SensorTypeStr) / sizeof(const char*);


const int NO_UNIT_ID = 1;


struct Unit
{
	int ID;
	QString nameEn;
	QString nameRu;
};


struct DataFormat
{
	int ID;
	QString name;
};


typedef QHash<int, QString> HashIntQString;

template <typename KEY, typename VALUE>
class OrderedHash
{
private:
	QVector<KEY> m_vector;
	QHash<KEY, VALUE> m_hash;

public:
	OrderedHash();
};


const QString DATE_TIME_FORMAT_STR("yyyy-MM-ddTHH:mm:ss");

class Signal
{
private:
	OrderedHash<int, QString> qq;

	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	bool m_checkedOut = false;
	int m_userID = 0;
	int m_channel = 1;
	SignalType m_type = SignalType::analog;
	QDateTime m_created;
	bool m_deleted = false;
	QDateTime m_instanceCreated;
	InstanceAction m_instanceAction = InstanceAction::added;

	QString m_strID;
	QString m_extStrID;
	QString m_name;
	int m_dataFormat = static_cast<int>(DataFormatType::binary_LE_unsigned);
	int m_dataSize;
	int m_lowADC = 0;
	int m_highADC = 0;
	double m_lowLimit = 0;
	double m_highLimit = 0;
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
	int m_outputSensorID = 0;
	bool m_acquire = true;
	bool m_calculated = false;
	int m_normalState = 0;
	int m_decimalPlaces = 2;
	double m_aperture = 0;
	SignalInOutType m_inOutType = SignalInOutType::internal;
	QString m_deviceStrID;

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
	void setType(SignalType type) { m_type = type; }
	void setCreated(const QDateTime& created) { m_created = created; }
	void setCreated(const QString& createdStr) { m_created = QDateTime::fromString(createdStr, DATE_TIME_FORMAT_STR); }
	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceCreated(const QDateTime& instanceCreated) { m_instanceCreated = instanceCreated; }
	void setInstanceCreated(const QString& instanceCreatedStr) { m_instanceCreated = QDateTime::fromString(instanceCreatedStr, DATE_TIME_FORMAT_STR); }
	void setInstanceAction(InstanceAction action) { m_instanceAction = action; }

public:
	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }
	int channel() const { return m_channel; }
	SignalType type() const { return m_type; }
	QDateTime created() const { return m_created; }
	bool deleted() const { return m_deleted; }
	QDateTime instanceCreated() const { return m_instanceCreated; }
	InstanceAction instanceAction() const { return m_instanceAction; }

	QString strID() const { return m_strID; }
	void setStrID(const QString& strID) { m_strID = strID; }

	QString extStrID() const { return m_extStrID; }
	void setExtStrID(const QString& extStrID) { m_extStrID = extStrID; }

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	int dataFormat() const { return m_dataFormat; }
	void setDataFormat(int dataFormat) { m_dataFormat = dataFormat; }

	int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }

	int lowADC() const { return m_lowADC; }
	void setLowADC(int lowADC) { m_lowADC = lowADC; }

	int highADC() const { return m_highADC; }
	void setHighADC(int highADC) { m_highADC = highADC;}

	double lowLimit() const { return m_lowLimit; }
	void setLowLimit(double lowLimit) { m_lowLimit = lowLimit; }

	double highLimit() const { return m_highLimit; }
	void setHighLimit(double highLimit) { m_highLimit = highLimit; }

	int unitID() const { return m_unitID; }
	void setUnitID(int unitID) { m_unitID = unitID; }

	double adjustment() const { return m_adjustment; }
	void setAdjustment(double adjustment) { m_adjustment = adjustment; }

	double dropLimit() const { return m_dropLimit; }
	void setDropLimit(double dropLimit) { m_dropLimit = dropLimit; }

	double excessLimit() const { return m_excessLimit; }
	void setExcessLimit(double excessLimit) { m_excessLimit = excessLimit; }

	double unbalanceLimit() const { return m_unbalanceLimit; }
	void setUnbalanceLimit(double unbalanceLimit) { m_unbalanceLimit = unbalanceLimit; }

	double inputLowLimit() const { return m_inputLowLimit; }
	void setInputLowLimit(double inputLowLimit) { m_inputLowLimit = inputLowLimit; }

	double inputHighLimit() const { return m_inputHighLimit; }
	void setInputHighLimit(double inputHighLimit) { m_inputHighLimit = inputHighLimit; }

	int inputUnitID() const { return m_inputUnitID; }
	void setInputUnitID(int inputUnitID) { m_inputUnitID = inputUnitID; }

	int inputSensorID() const { return m_inputSensorID; }
	void setInputSensorID(int inputSensorID) { m_inputSensorID = inputSensorID; }

	double outputLowLimit() const { return m_outputLowLimit; }
	void setOutputLowLimit(double outputLowLimit) { m_outputLowLimit = outputLowLimit; }

	double outputHighLimit() const { return m_outputHighLimit; }
	void setOutputHighLimit(double outputHighLimit) { m_outputHighLimit = outputHighLimit; }

	int outputUnitID() const { return m_outputUnitID; }
	void setOutputUnitID(int outputUnitID) { m_outputUnitID = outputUnitID; }

	int outputSensorID() const { return m_outputSensorID; }
	void setOutputSensorID(int outputSensorID) { m_outputSensorID = outputSensorID; }

	bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

	bool calculated() const { return m_calculated; }
	void setCalculated(bool calculated) { m_calculated = calculated; }

	int normalState() const { return m_normalState; }
	void setNormalState	(int normalState) { m_normalState = normalState; }

	int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

	double aperture() const { return m_aperture; }
	void setAperture(double aperture) { m_aperture = aperture; }

	SignalInOutType inOutType() const { return m_inOutType; }
	void setInOutType(SignalInOutType inOutType) { m_inOutType = inOutType; }

	QString deviceStrID() const { return m_deviceStrID; }
	void setDeviceStrID(const QString& deviceStrID) { m_deviceStrID = deviceStrID; }

	friend class DbWorker;
};


class SignalSet
{
private:
	QHash<int, Signal*> m_signalSet;

public:
	SignalSet();
	virtual ~SignalSet();

	void insert(const Signal& signal);
	Signal* getSignal(int signalID);
	const Signal* getConstSignal(int signalID) const;
	bool haveSignal(int signalID);

	bool contains(int signalID);

	void removeAll();
};


#endif // SIGNAL_H
