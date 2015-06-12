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
		Q_PROPERTY(int DoubleValue READ doubleValue WRITE setDoubleValue)

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
			DoubleType
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

		int intValue() const;
		void setIntValue(int intValue);

		double doubleValue() const;
		void setDoubleValue(double doubleValue);

		// Data
		//
	private:
		ConstType m_type = ConstType::IntegralType;
		int m_intValue = 0;
		double m_doubleValue = 0.0;
	};


}
#endif // SCHEMEITEMCONST_H
