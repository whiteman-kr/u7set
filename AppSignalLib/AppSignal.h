#pragma once

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <QDateTime>

#include "../UtilsLib/Address16.h"
#include "TuningValue.h"

namespace Proto
{
	class SignalSpecPropValue;
	class ProtoAppSignalData;
	class AppSignal;
}

class XmlWriteHelper;
class XmlReadHelper;

class AppSignalSpecPropValues;

struct ID_AppSignalID
{
	int ID = -1;
	int signalGroupID = -1;
	QString appSignalID;
};

Q_DECLARE_METATYPE(ID_AppSignalID)

using AppSignalStateFlagsMap = std::map<E::AppSignalStateFlagType, QString>;

class AppSignal
{
	friend class DbWorker;
	friend class AppSignalSet;
	friend class AppSignalSetProvider;
	friend class SignalTests;
	friend class DbControllerSignalTests;

public:
	static const QString CAPTION_VALIDATOR;
	static const QString IDENTIFICATORS_VALIDATOR;

public:
	AppSignal();
	AppSignal(const AppSignal& s);
	AppSignal(const ID_AppSignalID& ids);
	virtual ~AppSignal();

	QString initFromDeviceSignal(const QString& deviceSignalEquipmentID,
								E::SignalType deviceSignalType,
								E::SignalFunction deviceSignalFunction,
								const QString& appSignalID,
								const QString& customAppSignalID,
								const QString& appSignalCaption,
								const QString& appSignalBusTypeID,
								E::AnalogAppSignalFormat analogAppSignalFormat,
								const QString& appSignalSpecPropsStruct,
								bool enableTuning,
								const QVariant& tuningLowBound,
								const QVariant& tuningHighBound,
								const QVariant& tuningDefaultValue);

	void clear();

	void initSpecificProperties();

	bool isLoaded() const { return m_loaded; }
	void setLoaded(bool loaded) { m_loaded = loaded; }

	// Signal identificators

	const QString& appSignalID() const { return m_appSignalID; }
	void setAppSignalID(const QString& appSignalID) { m_appSignalID = appSignalID; }

	QString customAppSignalID() const { return m_customAppSignalID; }
	void setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }
	bool customAppSignalIDContainsMacro() const { return m_customAppSignalID.contains(TemplateMacro::START_TOKEN); }

	QString caption() const { return m_caption; }
	void setCaption(const QString& caption) { m_caption = caption; }
	bool captionContainsMacro() const { return m_caption.contains(TemplateMacro::START_TOKEN); }

	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	QString busTypeID() const { return m_busTypeID; }
	void setBusTypeID(const QString& busTypeID) { m_busTypeID = busTypeID; }

	E::Channel channel() const { return m_channel; }

	bool excludeFromBuild() const { return m_excludeFromBuild; }
	void setExcludeFromBuild(bool excludeFromBuild) { m_excludeFromBuild = excludeFromBuild; }

	bool isAutoSignal() const { return m_isAutoSignal; }
	void setAutoSignal(bool autoSignal) { m_isAutoSignal = autoSignal; }

	// Signal type

	E::SignalType signalType() const { return m_signalType; }
	void setSignalType(E::SignalType type);

	bool isAnalog() const { return m_signalType == E::SignalType::Analog; }
	bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }
	bool isBus() const { return m_signalType == E::SignalType::Bus; }

	E::SignalInOutType inOutType() const { return m_inOutType; }
	void setInOutType(E::SignalInOutType inOutType) { m_inOutType = inOutType; }

	bool isInput() const { return m_inOutType == E::SignalInOutType::Input; }
	bool isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
	bool isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }
	bool isSwCalculated() const { return m_inOutType == E::SignalInOutType::SoftwareCalculated; }

	E::SoftwareCalcFunction swCalcFunction() const;
	void setSwCalcFunction(E::SoftwareCalcFunction func);

	// Signal format

	int dataSize() const { return m_dataSize; }
	void setDataSize(int dataSize) { m_dataSize = dataSize; }
	void setDataSizeW(int sizeW);
	void setDataSizeByType(E::SignalType type, E::AnalogAppSignalFormat analogFormat);

	int sizeW() const { return (m_dataSize / SIZE_16BIT + ((m_dataSize % SIZE_16BIT) ? 1 : 0)); }
	int sizeBit() const { return m_dataSize; }

	E::ByteOrder byteOrder() const { return m_byteOrder; }
	void setByteOrder(E::ByteOrder byteOrder) { m_byteOrder = byteOrder; }

	E::AnalogAppSignalFormat analogSignalFormat() const { return m_analogSignalFormat; }
	void setAnalogSignalFormat(E::AnalogAppSignalFormat dataFormat);

	E::DataFormat dataFormat() const;

	bool isCompatibleFormat(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat, E::ByteOrder byteOrder) const;
	bool isCompatibleFormat(const SignalAddress16& sa16) const;
	bool isCompatibleFormat(const AppSignal& s) const;
	bool isCompatibleFormat(E::SignalType signalType, const QString& busTypeID) const;

	bool invertSignal() const;
	void setInvertSignal(bool invert);

	bool reserved() const;
	void setReserved(bool reserved);

	// Analog signal properties

	int lowADC(QString* err = nullptr) const;
	void setLowADC(int lowADC);

	int highADC(QString* err = nullptr) const;
	void setHighADC(int highADC);

	int lowDAC(QString* err = nullptr) const;
	void setLowDAC(int lowDAC);

	int highDAC(QString* err = nullptr) const;
	void setHighDAC(int highDAC);

	double lowEngineeringUnits(QString* err = nullptr) const;
	void setLowEngineeringUnits(double lowEngineeringUnits);

	double highEngineeringUnits(QString* err = nullptr) const;
	void setHighEngineeringUnits(double highEngineeringUnits);

	double lowPhysicalUnits(QString* err = nullptr) const;
	void setLowPhysicalUnits(double lowPhUnits);

	double highPhysicalUnits(QString* err = nullptr) const;
	void setHighPhysicalUnits(double highPhUnits);

	bool isReverseEngineeringLimits() const;

	double lowValidRange(QString* err = nullptr) const;
	void setLowValidRange(double lowValidRange);

	double highValidRange(QString* err = nullptr) const;
	void setHighValidRange(double highValidRange);

	double filteringTime(QString* err = nullptr) const;
	void setFilteringTime(double filteringTime);

	double spreadTolerance(QString* err = nullptr) const;
	void setSpreadTolerance(double spreadTolerance);

	// Analog input/output signal properties

	double electricLowLimit(QString* err = nullptr) const;
	void setElectricLowLimit(double electricLowLimit);

	double electricHighLimit(QString* err = nullptr) const;
	void setElectricHighLimit(double electricHighLimit);

	E::ElectricUnit electricUnit(QString* err = nullptr) const;
	void setElectricUnit(E::ElectricUnit electricUnit);

	double rloadOhm(QString* err = nullptr) const;
	void setRloadOhm(double rload_Ohm);

	E::SensorType sensorType(QString* err = nullptr) const;
	void setSensorType(E::SensorType sensorType);

	E::OutputMode outputMode(QString* err = nullptr) const;
	void setOutputMode(E::OutputMode outputMode);

	double r0_Ohm(QString* err = nullptr) const;
	void setR0_Ohm(double r0_Ohm);

	// Tuning signal properties

	bool enableTuning() const { return m_enableTuning; }
	bool isTunable() const { return m_enableTuning; }
	void setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

	TuningValue tuningDefaultValue() const { return m_tuningDefaultValue; }
	void setTuningDefaultValue(const TuningValue& value) { m_tuningDefaultValue = value; }

	TuningValue tuningLowBound() const { return m_tuningLowBound; }
	void setTuningLowBound(const TuningValue& value) { m_tuningLowBound = value; }

	TuningValue tuningHighBound() const { return m_tuningHighBound; }
	void setTuningHighBound(const TuningValue& value) { m_tuningHighBound = value; }

	// Signal properties for MATS

	bool acquire() const { return m_acquire; }
	void setAcquire(bool acquire) { m_acquire = acquire; }

	bool isAcquired() const { return m_acquire; }

	bool archive() const { return m_archive; }
	void setArchive(bool archive) { m_archive = archive; }

	bool log() const { return m_log; }
	void setLog(bool log) { m_log = log; }

	bool isArchived() const { return m_archive; }

	int decimalPlaces() const { return m_decimalPlaces; }
	void setDecimalPlaces(int decimalPlaces) { m_decimalPlaces = decimalPlaces; }

	double coarseAperture() const { return m_coarseAperture; }
	void setCoarseAperture(double aperture) { m_coarseAperture = aperture; }

	double fineAperture() const { return m_fineAperture; }
	void setFineAperture(double aperture) { m_fineAperture = aperture; }

	E::ApertureType apertureType() const { return m_apertureType; }
	void setApertureType(E::ApertureType type) { m_apertureType = type; }

	// Specific properties

	QString specPropStruct() const { return m_specPropStruct; }
	void setSpecPropStruct(const QString& specPropsStruct);
	Hash specPropStructHash() const;

	bool createSpecPropValues();

	void setProtoSpecPropValues(const QByteArray& protoSpecPropValues) { m_protoSpecPropValues = protoSpecPropValues; }
	const QByteArray& protoSpecPropValues() const { return m_protoSpecPropValues; }

	void cacheSpecPropValues() const;

	bool getSpecPropBool(const QString& name, QString* err) const;
	double getSpecPropDouble(const QString& name, QString* err) const;
	int getSpecPropInt(const QString& name, QString* err) const;
	unsigned int getSpecPropUInt(const QString& name, QString* err) const;
	int getSpecPropEnum(const QString& name, QString* err) const;
	bool getSpecPropValue(const QString& name, QVariant* qv, bool* isEnum, QString* err) const;
	bool isSpecPropExists(const QString& name) const;

	bool setSpecPropBool(const QString& name, bool value);
	bool setSpecPropDouble(const QString& name, double value);
	bool setSpecPropInt(const QString& name, int value);
	bool setSpecPropUInt(const QString& name, unsigned int value);
	bool setSpecPropEnum(const QString& name, int enumValue);
	bool setSpecPropValue(const QString& name, const QVariant& qv, bool isEnum);

	//

	QStringList tags() const;
	const std::set<QString>& tagsSet() const { return m_tags; }
	std::set<QString>& tagsSet() { return m_tags; }
	QString tagsStr() const { return tags().join(QChar::LineFeed); }

	void setTags(const QStringList& tags);
	void setTags(const std::set<QString>& tags);
	void setTagsStr(const QString& tagsStr);

	bool hasTags() const { return m_tags.size() > 0; }
	bool hasTag(const QString& tag) const { return m_tags.find(tag.toLower().trimmed()) != m_tags.end(); }
	int tagsCount() const { return static_cast<int>(m_tags.size()); }

	void appendTag(const QString& tag);
	void appendTags(const QStringList& tags);
	void appendTags(const std::set<QString>& tags);

	void removeTag(const QString& tag);
	void removeTags(const QStringList& tags);
	void removeTags(const std::set<QString>& tags);

	void clearTags();

	//

	void saveProtoData(QByteArray* protoDataArray) const;
	void saveProtoData(Proto::ProtoAppSignalData* protoData) const;

	void loadProtoData(const char* protoDataPtr, int protoDataSize);
	void loadProtoData(const QByteArray& protoDataArray);

	// Signal fields from database

	int ID() const { return m_ID; }
	int signalGroupID() const { return m_signalGroupID; }
	int signalInstanceID() const { return m_signalInstanceID; }
	int changesetID() const { return m_changesetID; }
	bool checkedOut() const { return m_checkedOut; }
	int userID() const { return m_userID; }
	bool deleted() const { return m_deleted; }

	QDateTime created() const;
	QDateTime instanceCreated() const;

	qint64 createdMcs() const { return m_createdMcs; }
	qint64 instanceCreatedMcs() const { return m_instanceCreatedMcs; }

	E::VcsItemAction instanceAction() const { return m_instanceAction; }

	// Signal properties calculated in compile-time

	Hash hash() const { assert(m_hash !=0); return m_hash; }
	void setHash(Hash hash) { m_hash = hash; }

	QString unit() const { return m_unit; }
	void setUnit(const QString& unit) { m_unit = unit; }

	Address16 ioBufAddr() const;
	void setIoBufAddr(const Address16& addr);

	Address16 tuningAddr() const { return m_tuningAddr; }
	void setTuningAddr(const Address16& addr) { m_tuningAddr = addr; }

	Address16 tuningAbsAddr() const { return m_tuningAbsAddr; }
	void setTuningAbsAddr(const Address16& addr) { m_tuningAbsAddr = addr; }

	Address16 ualAddr() const { return m_ualAddr; }
	void setUalAddr(const Address16& addr) { m_ualAddr = addr; }
	bool ualAddrIsValid() const { return m_ualAddr.isValid(); }

	Address16 regBufAddr() const { return m_regBufAddr; }
	void setRegBufAddr(const Address16& addr) { m_regBufAddr = addr; }

	Address16 regValueAddr() const { return m_regValueAddr; }
	void setRegValueAddr(const Address16& addr) { m_regValueAddr = addr; }

	Address16 regValidityAddr() const { return m_regValidityAddr; }
	void setRegValidityAddr(const Address16& addr) { m_regValidityAddr = addr; }

	Address16 actualAddr(E::LogicModuleRamAccess* lmRamAccess = nullptr) const;

	void resetAddresses();

	E::LogicModuleRamAccess lmRamAccess() const { return m_lmRamAccess; }
	void setLmRamAccess(E::LogicModuleRamAccess access) { m_lmRamAccess = access; }

	QString regValueAddrStr() const;

	bool needConversion() const { return m_needConversion; }
	void setNeedConversion(bool need) { m_needConversion = need; }

	bool isConst() const { return m_isConst; }
	void setIsConst(bool isConst) { m_isConst = isConst; }

	double constValue() const { return m_constValue; }
	void setConstValue(double constValue) { m_constValue = constValue; }

	bool isEndpoint() const { return m_isEndpoint; }
	void setEndpoint(bool ep) { m_isEndpoint = ep; }

	//

	void writeToAzpzXml(XmlWriteHelper& xml) const;

	void writeDoubleSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName = QString()) const;
	void writeIntSpecPropAttribute(XmlWriteHelper& xml, const QString& propName, const QString& attributeName = QString()) const;

	void writeToXml(XmlWriteHelper& xml) const;
	bool readFromXml(XmlReadHelper& xml);
	bool readTuningValuesFromXml(XmlReadHelper& xml);

	void saveToProto(Proto::AppSignal* s) const;
	void loadFromProto(const Proto::AppSignal &s);

	bool equalWithAppSignal(const AppSignal& s) const;

	void initCalculatedProperties();

	bool addFlagSignalID(E::AppSignalStateFlagType flagType, const QString& appSignalID);
	QString getFlagSignalID(E::AppSignalStateFlagType flagType) const;
	QStringList getFlagSignalsIDs() const;
	bool hasFlagsSignals() const;

	const AppSignalStateFlagsMap& stateFlagsSignals() const { return m_stateFlagsSignals; }

	void initTuningValues();

	static AppSignal* createDiscreteSignal(E::SignalInOutType inOutType,
										  const QString& appSignalID,
										  const QString& customAppSignalID,
										  const QString& caption,
										  const QString& equipmentID);

	static QString removeNumberSign(const QString& appSignalID);

	void trimTextFields();
	void uppercaseAppSignalID(bool uppercase);

private:
	// Private setters for fields, wich can't be changed outside DB engine
	// Should be used only by friends
	//
	friend class SignalPropertiesDialog;

	void setID(int signalID) { m_ID = signalID; }
	void setSignalGroupID(int signalGroupID) { m_signalGroupID = signalGroupID; }
	void setSignalInstanceID(int signalInstanceID) { m_signalInstanceID = signalInstanceID; }
	void setChangesetID(int changesetID) { m_changesetID = changesetID; }
	void setCheckedOut(bool checkedOut) { m_checkedOut = checkedOut; }
	void setUserID(int userID) { m_userID = userID; }
	void setChannel(E::Channel channel) { m_channel = channel; }

	void setCreated(qint64 timestampMcs) { m_createdMcs = timestampMcs; }
	void setCreated(const QDateTime& dt) { m_createdMcs = dt.toMSecsSinceEpoch() * 1000; }

	void setInstanceCreated(qint64 timestampMcs) { m_instanceCreatedMcs = timestampMcs; }
	void setInstanceCreated(const QDateTime& dt) { m_instanceCreatedMcs = dt.toMSecsSinceEpoch() * 1000; }

	void setDeleted(bool deleted) { m_deleted = deleted; }
	void setInstanceAction(E::VcsItemAction action) { m_instanceAction = action; }
	void initCreatedDates();

	QString* appSignalIDPtr() { return &m_appSignalID; }
	QString* customAppSignalIDPtr() { return &m_customAppSignalID; }
	QString* equipmentIDPtr() { return &m_equipmentID; }
	QString* specPropStructPtr() { return &m_specPropStruct; }

	QByteArray* protoSpecPropValuesPtr() { return &m_protoSpecPropValues; }

	bool isCompatibleFormatPrivate(E::SignalType signalType, E::DataFormat dataFormat, int size, E::ByteOrder byteOrder, const QString& busTypeID) const;

	void updateTuningValuesType();

	QString specPropNotExistErr(const QString &propName) const;

private:
	// Signal identificators
	//
	QString m_appSignalID;
	QString m_customAppSignalID;
	QString m_caption;
	QString m_equipmentID;											// should be transformed to portEquipmentID
	QString m_lmEquipmentID;										// now fills in compile time only
	QString m_busTypeID;											// only for: m_signalType == E::SignalType::Bus

	E::Channel m_channel = E::Channel::A;
	
	// Signal type
	//
	E::SignalType m_signalType = E::SignalType::Analog;
	E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
	E::SoftwareCalcFunction m_swCalcFunction = E::SoftwareCalcFunction::None;

	// Signal format
	//
	int m_dataSize = 32;											// signal data size in bits
	E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

	// Analog signal properties
	//
	E::AnalogAppSignalFormat m_analogSignalFormat =					// only for m_signalType == E::SignalType::Analog
							E::AnalogAppSignalFormat::Float32;		// discrete signals is always treat as UnsignedInt and dataSize == 1
	QString m_unit;

	//

	bool m_excludeFromBuild = false;
	bool m_isAutoSignal = false;

	bool m_acquire = true;
	bool m_archive = true;
	bool m_log = false;
	bool m_invertSignal = false;
	bool m_reserved = false;

	bool m_loaded = false; // == false - only m_ID and m_appSignalID fields is initialized from database
						   // == true - all Signal fields is initialized from database

	bool m_deleted = false;
	bool m_checkedOut = false;

	bool m_isConst = false;
	bool m_isEndpoint = false;
	bool m_needConversion = false;

	// Tuning signal properties
	//
	bool m_enableTuning = false;
	TuningValue m_tuningDefaultValue;
	TuningValue m_tuningLowBound;
	TuningValue m_tuningHighBound;

	// Signal properties for MATS
	//
	int m_decimalPlaces = 2;
	E::ApertureType m_apertureType = E::ApertureType::RangePercent;
	double m_coarseAperture = 1;
	double m_fineAperture = 0.5;

	// Signal specific properties
	//
	QString m_specPropStruct;
	mutable Hash m_specPropStructHash = 0;				// cached value, used only in signal editing
	QByteArray m_protoSpecPropValues;					// serialized protobuf message Proto::PropertyValues

	mutable std::shared_ptr<AppSignalSpecPropValues> m_cachedSpecPropValues;

	std::set<QString> m_tags;

	// Signal fields from database
	//
	int m_ID = 0;
	int m_signalGroupID = 0;
	int m_signalInstanceID = 0;
	int m_changesetID = 0;
	int m_userID = 0;

	E::VcsItemAction m_instanceAction = E::VcsItemAction::Added;

	qint64 m_createdMcs = 0;				// in microseconds, as in database
	qint64 m_instanceCreatedMcs = 0;		// in microseconds, as in database


	// Signal properties calculated in compile-time
	//
	Hash m_hash = 0;						// == calcHash(m_appSignalID)

	Address16 m_ioBufAddr;					// signal address in i/o modules buffers for signals of input/output modules (input and output signals)
											// or

	Address16 m_tuningAddr;					// address of tunable signal  from beginning of tuning buffer

	Address16 m_tuningAbsAddr;				// absolute address of tunable signal
											// For analogs m_tuningAddr and m_tuningAbsAddr are EQUAL!
											// For discretes m_tuningAddr and m_tuningAbsAddr are different, due to 32-bits packing of discretes!

	Address16 m_ualAddr;					// signal address is used in UAL
											// may be equal to m_ioBufAddr, m_tuningAddr, m_regValueAddr or not
											// this address should be used in all signals value read/write operations in UAL

	Address16 m_regBufAddr;					// absolute signal address in registration buffer (LM's memory address)

	Address16 m_regValueAddr;				// signal Value address in FSC data packet
	Address16 m_regValidityAddr;			// signal Validity address in FSC data packet

	E::LogicModuleRamAccess m_lmRamAccess = E::LogicModuleRamAccess::Undefined;

	double m_constValue = 0;

	AppSignalStateFlagsMap m_stateFlagsSignals;
};

class AppSignalSet
{
private:

	static const int SINGLE_CHANNEL = 0;

	class SignalsGroups
	{
	public:
		void swap(SignalsGroups& signalGroups);
		void clear();
		void insert(const AppSignal* appSignal);
		void remove(const AppSignal* appSignal);
		void remove(int groupID, int signalID);

		bool getGroupSignalIDs(int signalID, int groupID, std::vector<int>* signalsIDs) const;

	private:
		std::map<int, std::vector<int>> m_groups;		// signalGroupID => set of signalIDs
														// signalGroupID == SINGLE_CHANNEL is NOT placed in this map!
	};

public:
	AppSignalSet();
	virtual ~AppSignalSet();

	void swap(AppSignalSet& appSignalSet);

	void clear();
	void reserve(int n);

	// return pair is <newAppSignalPtr, newAppSignalIndex>
	//
	std::pair<AppSignal*, int> append(AppSignal* newSignal);		// takes ownership on "newSignal"
	std::pair<AppSignal*, int> append(const AppSignal& signal);		// appends new AppSignal(signal) (make copy)
	std::pair<AppSignal*, int> append(const ID_AppSignalID& id);

	void removeSignals(const std::vector<int>& signalToRemoveIDs);

	bool contains(const QString& appSignalID) const;
	int count() const;
	int size() const;
	bool isEmpty() const;

	void enableIdGeneration();

	const std::vector<AppSignal*>& signalsVector() const;

	std::vector<AppSignal*>::iterator begin();
	std::vector<AppSignal*>::const_iterator begin() const;

	std::vector<AppSignal*>::iterator end();
	std::vector<AppSignal*>::const_iterator end() const;

	AppSignal* getSignal(const QString& appSignalID);
	const AppSignal* getSignal(const QString& appSignalID) const;

	AppSignal* getSignal(int signalID);
	const AppSignal* getSignal(int signalID) const;

	AppSignal* getSignalByHash(Hash appSignalIDHash);
	const AppSignal* getSignalByHash(Hash appSignalIDHash) const;

	AppSignal* at(int index);
	const AppSignal* at(int index) const;

	int signalIndex(int signalID) const;

	bool getChannelSignalsID(int signalID, std::vector<int>* channelSignalIDs) const;
	bool getChannelSignalsID(const AppSignal& signal, std::vector<int>* channelSignalIDs) const;
	bool getChannelSignalsID(int signalID, int groupID, std::vector<int>* channelSignalIDs) const;

	void appSignalIdsListSorted(bool removeNumberSign, QStringList* list) const;

	std::pair<AppSignal*, int> updateSignal(const AppSignal& s);

	bool serializeFromProtoFile(const QString& filePath);

	inline static const int BAD_INDEX = -1;
	inline static const int BAD_ID = -1;

private:
	const AppSignal* privateGetSignal(const QString& appSignalID) const;
	const AppSignal* privateGetSignalByID(int signalID) const;
	const AppSignal* privateGetSignalByHash(Hash appSignalIDHash) const;
	const AppSignal* privateAt(int index) const;

private:
	std::vector<AppSignal*> m_signals;
	std::map<int, qsizetype> m_idToIndex;			// signal.ID => Index in m_signals
	std::map<Hash, qsizetype> m_hashToIndex;		// Hash(AppSignalID) => Index in m_signals

	SignalsGroups m_groups;

	bool m_enableIdGeneration = false;
};

class AppSignals
{
public:
	~AppSignals();

	void clear();

	void insert(const ::Proto::AppSignal& protoAppSignal);

	bool containsID(const QString& appSignalID) const;
	bool containsHash(Hash hash) const;

	const AppSignal* getSignalByID(const QString& appSignalID) const;		// rename => getByAppSignalID

	const AppSignal* getSignalByHash(Hash hash) const;

	const AppSignal* getSignalByIndex(int index) const;

	bool isEmpty() const;
	size_t count() const;

	std::vector<AppSignal*>::iterator begin();
	std::vector<AppSignal*>::const_iterator begin() const;

	std::vector<AppSignal*>::iterator end();
	std::vector<AppSignal*>::const_iterator end() const;

private:
	std::vector<AppSignal*> m_signals;				// dynamic AppSignal object owner
	std::map<Hash, AppSignal*> m_hashToSignal;		// Hash => appSignal
};


