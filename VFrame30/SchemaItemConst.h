#pragma once

#include "FblItemRect.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT SchemaItemConst : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemConst>;
#endif

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

		int intValue() const;
		void setIntValue(int intValue);

		double floatValue() const;
		void setFloatValue(double floatValue);

		int discreteValue() const;
		void setDiscreteValue(int discreteValue);

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
			int intValue = 0;
			double floatValue = 0.0;
			int discreteValue = 0;
		} m_value;

		int m_precision = 2;
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;		// Used only when m_type == ConstType::FloatType;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
	};

}
