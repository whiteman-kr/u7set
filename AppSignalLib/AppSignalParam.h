#pragma once

#include <memory>
#include <set>

#include "TuningValue.h"

class AppSignal;
class AppSignalSpecPropValues;

namespace Proto
{
	class AppSignal;
}

struct AppSignalParamMimeType
{
	static const char* value;	// = "application/x-appsignalparam";	Data in format ::Proto::AppSiagnalParamSet
};


/*! \class AppSignalParam
	\ingroup groupParamsStates
	\brief Describes signal parameters in Monitor and TuningClient applications.

	AppSignalParam class describes signal parameters in Monitor and TuningClient applications. This state is received from ApplicationDataService by Monitor or
	from TuningService by Monitor or TuningClient.

	\ref VFrame30::ScriptAppSignalController "ScriptAppSignalController" class accessed by global <b>signals</b> object is used for
	requesting signal parameters from Application Data Service.

	\ref VFrame30::TuningController "TuningController" class accessed by global <b>tuning</b> object is used for
	requesting signal parameters from Tuning Service.

	\warning
	TuningController is always available in TuningClient. In Monitor it is available only in non-safety projects when Tuning function is enabled.

	\n
	\warning
	After requesting signal state it is highly recommended to check function return values, because errors can occur. For example,
	connection to a service can be down, or signal with specified identifier could not exist.

	<b>Example:</b>

	\code
	var appSignalIdA = "#APPSIGNAL01";	// Signal A
	var appSignalIdB = "#APPSIGNAL02";	// Signal B

	// Get parameters from Application Data Service
	//
	var param1 = signals.signalParam(appSignalIdA);

	// Get parameters from Tuning Service
	//
	var param2 = tuning.signalParam(appSignalIdB);

	if (param1 == undefined || param2 == undefined)
	{
		// Some signals do not exist or getting their parameters failed
		//
		return;
	}

	// Output parameters of signal A
	//
	schemaItemValue.Text = param1.caption;

	// Check if signal B is analog and input
	//
	if (param2.isAnalog)
	{
		if (param2.isInput)
		{
			// Create a string with template "caption, units"
			//
			var units = param2.unit;

			var text = param2.caption + ", " + units;
			...
		}
	}
	\endcode
*/
class AppSignalParam
{
	Q_GADGET

	/// \brief Contains a 64-bit hash of a signal
	Q_PROPERTY(Hash hash READ hash)
	Q_PROPERTY(Hash Hash READ hash)

	/// \brief Application Signal Identifier
	Q_PROPERTY(QString appSignalID READ appSignalId)
	Q_PROPERTY(QString AppSignalID READ appSignalId)

	/// \brief Custom Application Signal Identifier
	Q_PROPERTY(QString customAppSignalID READ customSignalId)
	Q_PROPERTY(QString CustomAppSignalID READ customSignalId)

	Q_PROPERTY(QString customSignalID READ customSignalId)	// same as CustomAppSignalID, for compatibility
	Q_PROPERTY(QString CustomSignalID READ customSignalId)	// same as CustomAppSignalID, for compatibility

	/// \brief Signal Caption
	Q_PROPERTY(QString caption READ caption)
	Q_PROPERTY(QString Caption READ caption)

	/// \brief Signal EquipmentID, can be port EquipmentID for inputs/ouputs or LmEquipmentID for internal signals.
	Q_PROPERTY(QString equipmentID READ equipmentId)
	Q_PROPERTY(QString EquipmentID READ equipmentId)

	/// \brief Signal Equipment ID
	Q_PROPERTY(QString lmEquipmentID READ lmEquipmentId)
	Q_PROPERTY(QString LmEquipmentID READ lmEquipmentId)

	/// \brief Signal Measure Units
	Q_PROPERTY(QString unit READ unit)
	Q_PROPERTY(QString Unit READ unit)

	/// \brief Signal precision (digits after point)
	Q_PROPERTY(int precision READ precision)
	Q_PROPERTY(int Precision READ precision)

	/// \brief Signal channel
	Q_PROPERTY(E::Channel channel READ channel)
	Q_PROPERTY(E::Channel Channel READ channel)

	/// \brief Low Valid Range
	Q_PROPERTY(double lowValidRange READ lowValidRange)
	Q_PROPERTY(double LowValidRange READ lowValidRange)

	/// \brief High Valid Range
	Q_PROPERTY(double highValidRange READ highValidRange)
	Q_PROPERTY(double HighValidRange READ highValidRange)

	/// \brief Low Engineering Units
	Q_PROPERTY(double lowEngineeringUnits READ lowEngineeringUnits)
	Q_PROPERTY(double LowEngineeringUnits READ lowEngineeringUnits)

	/// \brief High Engineering Units
	Q_PROPERTY(double highEngineeringUnits READ highEngineeringUnits)
	Q_PROPERTY(double HighEngineeringUnits READ highEngineeringUnits)

	/// \brief Signal is tunable
	Q_PROPERTY(bool enableTuning READ enableTuning)
	Q_PROPERTY(bool EnableTuning READ enableTuning)

	/// \brief Default value of tunable signal
	Q_PROPERTY(QVariant tuningDefaultValue READ tuningDefaultValueToVariant)
	Q_PROPERTY(QVariant TuningDefaultValue READ tuningDefaultValueToVariant)

	/// \brief Low tuning limit of the signal
	Q_PROPERTY(QVariant tuningLowBound READ tuningLowBoundToVariant)
	Q_PROPERTY(QVariant TuningLowBound READ tuningLowBoundToVariant)

	/// \brief High tuning limit of the signal
	Q_PROPERTY(QVariant tuningHighBound READ tuningHighBoundToVariant)
	Q_PROPERTY(QVariant TuningHighBound READ tuningHighBoundToVariant)

	/// \brief Signal is input
	Q_PROPERTY(bool isInput READ isInput)
	Q_PROPERTY(bool IsInput READ isInput)

	/// \brief Signal is output
	Q_PROPERTY(bool isOutput READ isOutput)
	Q_PROPERTY(bool IsOutput READ isOutput)

	/// \brief Signal is internal
	Q_PROPERTY(bool isInternal READ isInternal)
	Q_PROPERTY(bool IsInternal READ isInternal)

	/// \brief Signal is analog
	Q_PROPERTY(bool isAnalog READ isAnalog)
	Q_PROPERTY(bool IsAnalog READ isAnalog)

	/// \brief Signal is discrete
	Q_PROPERTY(bool isDiscrete READ isDiscrete)
	Q_PROPERTY(bool IsDiscrete READ isDiscrete)

	/// \brief Signal is endpoint
	Q_PROPERTY(bool isEndpoint READ isEndpoint)
	Q_PROPERTY(bool IsEndpoint READ isEndpoint)

	/// \brief Signal state is inverted (applicable only for discrete signals)
	Q_PROPERTY(bool isInverted READ isInverted)
	Q_PROPERTY(bool IsInverted READ isInverted)

	/// \brief Signal is reserved
	Q_PROPERTY(bool isReserved READ isReserved)
	Q_PROPERTY(bool IsReserved READ isReserved)

public:
	AppSignalParam(const AppSignal& signal);

	// Shallow copy.
	//
	AppSignalParam();
	
	AppSignalParam(const AppSignalParam&);
	AppSignalParam(AppSignalParam&&) noexcept;

	AppSignalParam& operator=(const AppSignalParam&);
	AppSignalParam& operator=(AppSignalParam&&) noexcept;


	bool load(const Proto::AppSignal& message);
	void load(const AppSignal& signal);
	void save(::Proto::AppSignal* message) const;

	// Make a deep copy of the AppSignalParam.
	//
	AppSignalParam clone() const;

	// Properties
	//
public:
	[[nodiscard]] Hash hash() const;
	void setHash(Hash value);

	[[nodiscard]] QString appSignalId() const;
	void setAppSignalId(const QString& value);

	[[nodiscard]] QString customSignalId() const;
	void setCustomSignalId(const QString& value);

	[[nodiscard]] QString caption() const;
	void setCaption(const QString& value);

	[[nodiscard]] QString equipmentId() const;
	void setEquipmentId(const QString& value);

	[[nodiscard]] QString lmEquipmentId() const;
	void setLmEquipmentId(const QString& value);

	[[nodiscard]] E::Channel channel() const;
	void setChannel(E::Channel value);

	[[nodiscard]] bool isInput() const;
	[[nodiscard]] bool isOutput() const;
	[[nodiscard]] bool isInternal() const;
	[[nodiscard]] E::SignalInOutType inOutType() const;
	void setInOutType(E::SignalInOutType value);

	[[nodiscard]] bool isAnalog() const;
	[[nodiscard]] bool isDiscrete() const;
	[[nodiscard]] bool isBus() const;
	[[nodiscard]] E::SignalType type() const;
	void setType(E::SignalType value);

	[[nodiscard]] TuningValueType tuningType() const;

	[[nodiscard]] E::AnalogAppSignalFormat analogSignalFormat() const;
	void setAnalogSignalFormat(E::AnalogAppSignalFormat value);

	[[nodiscard]] E::ByteOrder byteOrder() const;
	void setByteOrder(E::ByteOrder value);

	[[nodiscard]] QString unit() const;
	void setUnit(const QString& value);

	[[nodiscard]] double lowValidRange() const;
	[[nodiscard]] double highValidRange() const;

	[[nodiscard]] double lowEngineeringUnits() const;
	void setLowEngineeringUnits(double value);

	[[nodiscard]] double highEngineeringUnits() const;
	void setHighEngineeringUnits(double value);

	[[nodiscard]] double inputLowLimit() const;
	[[nodiscard]] double inputHighLimit() const;
	[[nodiscard]] E::ElectricUnit inputUnitId() const;
	[[nodiscard]] E::SensorType inputSensorType() const;

	[[nodiscard]] double outputLowLimit() const;
	[[nodiscard]] double outputHighLimit() const;
	[[nodiscard]] int outputUnitId() const;
	[[nodiscard]] E::OutputMode outputMode() const;
	[[nodiscard]] E::SensorType outputSensorType() const;

	[[nodiscard]] int precision() const;
	void setPrecision(int value);

	[[nodiscard]] double fineAperture() const;
	void setFineAperture(double value);

	[[nodiscard]] double coarseAperture() const;
	void setCoarseAperture(double value);

	[[nodiscard]] double filteringTime() const;
	void setFilteringTime(double value);

	[[nodiscard]] double spreadTolerance() const;
	void setSpreadTolerance(double value);

	[[nodiscard]] bool enableTuning() const;
	void setEnableTuning(bool value);

	[[nodiscard]] bool isEndpoint() const;
	void setEndpoint(bool value);

	[[nodiscard]] bool isInverted() const;
	void setInverted(bool value);

	[[nodiscard]] bool isReserved() const;
	void setReserved(bool value);

	[[nodiscard]] TuningValue tuningDefaultValue() const;
	[[nodiscard]] QVariant tuningDefaultValueToVariant() const;
	void setTuningDefaultValue(const TuningValue& value);

	[[nodiscard]] TuningValue tuningLowBound() const;
	[[nodiscard]] QVariant tuningLowBoundToVariant() const;
	void setTuningLowBound(const TuningValue& value);

	[[nodiscard]] TuningValue tuningHighBound() const;
	[[nodiscard]] QVariant tuningHighBoundToVariant() const;
	void setTuningHighBound(const TuningValue& value);

	[[nodiscard]] std::set<QString> tags() const;
	[[nodiscard]] QStringList tagStringList() const;
	void setTags(std::set<QString> tags);

	const QString& specificPropertyStruct() const;
	const QByteArray& protoSpecificPropertyValues() const;

	const AppSignalSpecPropValues& specificPropertyValues() const;

public:
	/// @brief Check if signal has specified tag
	Q_INVOKABLE bool hasTag(const QString& tag) const;
	
	/// @brief Get specific property value by name
	Q_INVOKABLE QVariant specificPropertyValue(const QString& propertyName) const;

	/// @brief Check if specific property exists
	Q_INVOKABLE bool specificPropertyExists(const QString& propertyName) const;

public:
	static const int NO_UNIT_ID = 1;

private:
	void detach();

	struct PrivateData
	{
		bool load(const Proto::AppSignal& message);
		void load(const AppSignal& signal);
		void save(::Proto::AppSignal* message) const;

		Hash m_hash = UNDEFINED_HASH;				// Hash from m_appSignalId
		QString m_appSignalId;
		QString m_customSignalId;
		QString m_caption;
		QString m_equipmentId;
		QString m_lmEquipmentId;

		E::Channel m_channel = E::Channel::A;
		E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;
		E::SignalType m_signalType = E::SignalType::Analog;
		E::AnalogAppSignalFormat m_analogSignalFormat = E::AnalogAppSignalFormat::Float32;
		E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

		QString m_unit;

		double m_lowValidRange = 0;
		double m_highValidRange = 100;
		double m_lowEngineeringUnits = 0;
		double m_highEngineeringUnits = 100;

		double m_electricLowLimit = 0;									// low electric value for input range
		double m_electricHighLimit = 0;									// high electric value for input range
		E::ElectricUnit m_electricUnit = E::ElectricUnit::NoUnit;		// electric unit for input range (mA, mV, Ohm, V ....)
		E::SensorType m_sensorType = E::SensorType::NoSensor;			// electric sensor type for input range (was created for m_inputUnitID)

		double m_outputLowLimit = 0;									// low physical value for output range
		double m_outputHighLimit = 0;									// high physical value for output range
		int m_outputUnitId = NO_UNIT_ID;								// physical unit for output range (kg, mm, Pa ...)
		E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;		// output electric range (or mode ref. OutputModeStr[])
		E::SensorType m_outputSensorType = E::SensorType::NoSensor;		// electric sensor type for output range (was created for m_outputMode)

		int m_precision = 2;
		double m_coarseAperture = 1;
		double m_fineAperture = 0.5;
		double m_filteringTime = 0.005;
		double m_spreadTolerance = 2;
		bool m_enableTuning = false;
		bool m_endpoint = false;
		bool m_inverted = false;
		bool m_reserved = false;
		TuningValue m_tuningDefaultValue;
		TuningValue m_tuningLowBound;
		TuningValue m_tuningHighBound;

		QString m_specPropStruct;
		QByteArray m_specPropValues;
		
		std::unique_ptr<::AppSignalSpecPropValues> m_specificPropertyValues;

		std::set<QString> m_tags;
	};

	std::shared_ptr<PrivateData> m_data = std::make_shared<PrivateData>();
};

Q_DECLARE_METATYPE(AppSignalParam)
