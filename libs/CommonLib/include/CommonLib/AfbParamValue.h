#pragma once
#include "Types.h"

namespace Afb
{
	//
	// AfbParamValue -- optional (value or variable) AfbValue
	//
	class AfbParamValue
	{
	public:
		AfbParamValue() = default;
		explicit AfbParamValue(E::SignalType type, E::DataFormat dataFormat, quint16 size);
		AfbParamValue(const AfbParamValue&) = default;
		~AfbParamValue() = default;
		AfbParamValue& operator=(const AfbParamValue&) = default;

		bool operator==(const AfbParamValue&) const = default;
		bool operator!=(const AfbParamValue&) const = default;

	public:
		[[nodiscard]] QString toString() const;
		[[nodiscard]] QString toString(char numberFormat, int precision) const;
		bool fromString(const QString& str);

		[[nodiscard]] int validate(const QString& str) const;

		[[nodiscard]] QVariant toVariant() const;		// Returns AfbParamValue as QVariant
		bool fromVariant(const QVariant& v);			// Set *this to QVariant (AfbParamValue)

	public:
		[[nodiscard]] E::SignalType type() const;
		void setType(E::SignalType type);

		[[nodiscard]] E::DataFormat dataFormat() const;
		void setDataFormat(E::DataFormat dataFormat);

		[[nodiscard]] bool isAnalog() const;
		[[nodiscard]] bool isDiscrete() const;

		[[nodiscard]] int size() const;
		void setSize(int value);

		[[nodiscard]] QVariant value() const;
		bool setValue(const QVariant& v);

		[[nodiscard]] const QString& reference() const;
		void setReference(const QString& value);

		[[nodiscard]] bool hasReference() const;
		[[nodiscard]] bool checkValue() const;

		// Data
		//
	private:
		E::SignalType m_type = E::SignalType::Analog;
		E::DataFormat m_dataFormat = E::DataFormat::Float;
		quint16 m_size = 0;				// BitWidth

		QVariant m_value;
		QString m_reference;			// Reference to variable e.g. '$(Schema.VarName)'
	};

}	// namespace Afb


Q_DECLARE_METATYPE(Afb::AfbParamValue)
