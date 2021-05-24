#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	class SchemaItemConst : public FblItemRect
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemConst>;

		// Declarations
		//
	public:
		enum ConstType
		{
			IntegerType,
			FloatType,
			Discrete
		};
		Q_ENUM(ConstType)

	public:
		SchemaItemConst();
		SchemaItemConst(SchemaUnit unit);
		virtual ~SchemaItemConst();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Public Methods
		//
	public:
		QString valueToString() const;

		virtual QString buildName() const final;
		virtual QString toolTipText(int dpiX, int dpiY) const final;

		// Properties
		//
	public:
		SchemaItemConst::ConstType type() const;
		void setType(SchemaItemConst::ConstType value);

		bool isIntegral() const;
		bool isFloat() const;
		bool isDiscrete() const;

		const Afb::AfbParamValue& signedInt32Value() const;
		void setSignedInt32Value(const Afb::AfbParamValue& intValue);
		qint32 signedInt32NativeValue() const;
		void setSignedInt32NativeValue(qint32 v);

		const Afb::AfbParamValue& floatValue() const;
		void setFloatValue(const Afb::AfbParamValue& value);
		float floatNativeValue() const;
		void setFloatNativeValue(float v);

		const Afb::AfbParamValue& discreteValue() const;
		void setDiscreteValue(const Afb::AfbParamValue& discreteValue);
		quint16 discreteNativeValue() const;
		void setDiscreteNativeValue(quint16);

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		// Data
		//
	private:
		ConstType m_type = ConstType::FloatType;

		struct ConstValue
		{
			Afb::AfbParamValue signedInt32{E::SignalType::Analog, E::DataFormat::SignedInt, 32};
			Afb::AfbParamValue float32{E::SignalType::Analog, E::DataFormat::Float, 32};
			Afb::AfbParamValue discrete{E::SignalType::Analog, E::DataFormat::UnsignedInt, 16};		// It is made analog uint because if we make discrete then in PropertyEditor it will be edited as bool (true/false)
		} m_value;

		int m_precision = 2;
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;		// Used only when m_type == ConstType::FloatType;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
	};

}
