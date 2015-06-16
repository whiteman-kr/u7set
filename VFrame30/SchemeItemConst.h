#ifndef SCHEMEITEMCONST_H
#define SCHEMEITEMCONST_H

#include "FblItemRect.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT SchemeItemConst : public FblItemRect
	{
		Q_OBJECT

		Q_PROPERTY(ConstType Type READ type WRITE setType)
		Q_PROPERTY(int IntegralValue READ intValue WRITE setIntValue)
		Q_PROPERTY(int FloatValue READ floatValue WRITE setFloatValue)

		Q_ENUMS(ConstType)

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<VideoItem>::DerivedType<SchemeItemConst>;
#endif

		// Declarations
		//
	public:
		enum ConstType
		{
			IntegralType,
			FloatType
		};

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

		// Data
		//
	private:
		ConstType m_type = ConstType::IntegralType;
		int m_intValue = 0;
		double m_floatValue = 0.0;
	};

}

Q_DECLARE_METATYPE(VFrame30::SchemeItemConst::ConstType)

#endif // SCHEMEITEMCONST_H
