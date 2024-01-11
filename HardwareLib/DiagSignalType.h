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
	class DiagSignalType final : public PropertyObject,
								 public Proto::ObjectSerialization<DiagSignalType>,
								 public std::enable_shared_from_this<DiagSignalType>
	{
		Q_OBJECT

	protected:
		explicit DiagSignalType(QObject* parent = nullptr);

	public:
		virtual ~DiagSignalType() = default;

		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

		// Serialization
		//
	protected:
		friend Proto::ObjectSerialization<DiagSignalType>; // for call CreateObject from Proto::ObjectSerialization

	public:
		[[nodiscard]] static std::shared_ptr<DiagSignalType> CreateObject(QObject* parent = nullptr);
		[[nodiscard]] static std::shared_ptr<DiagSignalType> CreateObject(const Proto::Envelope& message);

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
		QUuid m_uuid;

		bool m_systemSignalType = false; // Means that it can't be changed or deleted.
		QString m_signalTypeId;
		E::DiagSignalType m_type = E::DiagSignalType::Analog;
		E::DiagByteOrder m_byteOrder = E::DiagByteOrder::LittleEndian;

		QString m_units;

		// Discrete specific
		//
		bool m_inverseValue = false;
		int m_normalState = 0;
		QString m_normalStateString0 = QStringLiteral("0");
		QString m_normalStateString1 = QStringLiteral("1");

		// Analog conversion
		//
		E::DiagAnalogFormat m_analogFormat = E::DiagAnalogFormat::UnsignedInt;

		double m_adcHighLimit = 0.0;
		double m_adcLowLimit = 255.0;

		double m_valueHighLimit = 0.0;
		double m_valueLowLimit = 255.0;

		double m_valueMultiplier = 1.0; // y = mx + b, m = m_valueMultiplier
		double m_valueOffset = 0;       // b = m_valueOffset

		bool m_useLimits = false;       // Use adcLimits and valueLimits for analog value.

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
