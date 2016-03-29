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
			IntegralType,
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

		// Data
		//
	private:
		ConstType m_type = ConstType::FloatType;
		int m_intValue = 0;
		double m_floatValue = 0.0;
		int m_precision = 2;
	};

}
