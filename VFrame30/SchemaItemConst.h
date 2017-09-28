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
			FloatType
		};
		Q_ENUM(ConstType)

	public:
		SchemaItemConst();
		SchemaItemConst(SchemaUnit unit);
		virtual ~SchemaItemConst();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

	protected:
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Public Methods
		//
	public:
		QString valueToString() const;

		virtual QString buildName() const override;

		// Properties
		//
	public:
		SchemaItemConst::ConstType type() const;
		void setType(SchemaItemConst::ConstType value);

		bool isIntegral() const;
		bool isFloat() const;

		int intValue() const;
		void setIntValue(int intValue);

		double floatValue() const;
		void setFloatValue(double floatValue);

		int precision() const;
		void setPrecision(int value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		// Data
		//
	private:
		ConstType m_type = ConstType::FloatType;
		int m_intValue = 0;
		double m_floatValue = 0.0;
		int m_precision = 6;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
	};

}
