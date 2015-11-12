#ifndef SCHEMEITEMCONST_H
#define SCHEMEITEMCONST_H

#include "FblItemRect.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT SchemeItemConst : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemConst>;
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
		SchemeItemConst();
		SchemeItemConst(SchemeUnit unit);
		virtual ~SchemeItemConst();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* layer) const override;

		// Public Methods
		//
	public:
		QString valueToString() const;

		virtual QString buildName() const override;

		// Properties
		//
	public:
		SchemeItemConst::ConstType type() const;
		void setType(SchemeItemConst::ConstType value);

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


#endif // SCHEMEITEMCONST_H
