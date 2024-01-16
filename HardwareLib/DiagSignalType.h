#pragma once

#include "DeviceObject.h"
#include "../UtilsLib/XmlHelper.h"

namespace Hardware
{
	//
	//
	// DiagSignalType
	//
	//
	struct DiagSignalType
	{
		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

		void save(Proto::DiagSignalType* message) const;
		bool load(const Proto::DiagSignalType& message);
		
		// Data
		//
		QUuid uuid;

		bool systemSignalType = false; // Means that it can't be changed or deleted.
		QString signalTypeId;
		E::DiagSignalType type = E::DiagSignalType::Analog;
		E::DiagByteOrder byteOrder = E::DiagByteOrder::LittleEndian;

		QString units;

		// Discrete specific
		//
		bool inverseValue = false;
		int normalState = 0;
		QString normalStateString0 = QStringLiteral("0");
		QString normalStateString1 = QStringLiteral("1");

		// Analog conversion
		//
		E::DiagAnalogFormat analogFormat = E::DiagAnalogFormat::UnsignedInt;

		double adcHighLimit = 0.0;
		double adcLowLimit = 255.0;

		double valueHighLimit = 0.0;
		double valueLowLimit = 255.0;

		double valueMultiplier = 1.0; // y = mx + b, m = valueMultiplier
		double valueOffset = 0;       // b = valueOffset

		bool useLimits = false;       // Use adcLimits and valueLimits for analog value.
	};


	//
	//
	// DiagSignalTypeObject
	//
	//
	class DiagSignalTypeObject final : public PropertyObject,
								 public Proto::ObjectSerialization<DiagSignalTypeObject>,
								 public std::enable_shared_from_this<DiagSignalTypeObject>
	{
		Q_OBJECT

	protected:
		explicit DiagSignalTypeObject(QObject* parent = nullptr);

	public:
		virtual ~DiagSignalTypeObject() = default;

		// Serialization
		//
	protected:
		friend Proto::ObjectSerialization<DiagSignalTypeObject>; // for call CreateObject from Proto::ObjectSerialization

	public:
		[[nodiscard]] static std::shared_ptr<DiagSignalTypeObject> CreateObject(QObject* parent = nullptr);
		[[nodiscard]] static std::shared_ptr<DiagSignalTypeObject> CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Properties
		//
	public:
		[[nodiscard]] QUuid uuid() const;
		void setUuid(QUuid uuid);

		[[nodiscard]] bool isSystemSignalType() const;
		[[nodiscard]] bool systemSignalType() const;
		void setSystemSignalType(bool value);

		[[nodiscard]] const QString& signalTypeId() const;
		void setSignalTypeId(const QString& value);

		[[nodiscard]] E::DiagSignalType type() const;
		void setType(E::DiagSignalType value);

		[[nodiscard]] E::DiagByteOrder byteOrder() const;
		void setByteOrder(E::DiagByteOrder value);

		// Discrete specific properties
		//
		[[nodiscard]] bool inverseValue() const;
		void setInverseValue(bool value);

		[[nodiscard]] int normalState() const;
		void setNormalState(int value);

		[[nodiscard]] const QString& normalStateString0() const;
		void setNormalStateString0(const QString& value);

		[[nodiscard]] const QString& normalStateString1() const;
		void setNormalStateString1(const QString& value);

		// Analog conversion specific properties
		//
		[[nodiscard]] E::DiagAnalogFormat analogFormat() const;
		void setAnalogFormat(E::DiagAnalogFormat value);

		[[nodiscard]] double adcHighLimit() const;
		void setAdcHighLimit(double value);

		[[nodiscard]] double adcLowLimit() const;
		void setAdcLowLimit(double value);

		[[nodiscard]] double valueHighLimit() const;
		void setValueHighLimit(double value);

		[[nodiscard]] double valueLowLimit() const;
		void setValueLowLimit(double value);

		[[nodiscard]] double valueMultiplier() const;
		void setValueMultiplier(double value);

		[[nodiscard]] double valueOffset() const;
		void setValueOffset(double value);

		[[nodiscard]] bool useLimits() const;
		void setUseLimits(bool value);

		[[nodiscard]] const QString& units() const;
		void setUnits(const QString& value);

		// Data
		//
	private:
		DiagSignalType m_data;


	public:
		static const char* mimeType;    // = "application/x-radiydiagsignaltype";
	};


	// TODO: DiagSetpoint, add setpoints to diag signal type
	//

	//class DiagSetpoint final : public PropertyObject
	//{
	//	Q_OBJECT

	//public:
	//	DiagSetpoint();

	//private:
	//	double m_value = 0.0;
	//	double m_hysteresis = 0.0;

	//	E::CmpType m_type = E::CmpType::Greate;
	//	int	Level;	

	//	CString Caption;
	//	COLORREF Color;	

	//};
} // namespace Hardware
