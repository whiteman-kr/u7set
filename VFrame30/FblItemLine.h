#pragma once

#include "PosConnectionImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FblItemLine : public PosConnectionImpl, public FblItem
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<FblItemLine>;
#endif

	protected:
		FblItemLine(void);
		FblItemLine(SchemaUnit itemunit);
	public:
		virtual ~FblItemLine(void);
		
	public:
		
		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;


		// Properties and Data
	public:
		virtual bool IsFblItem() const override;

		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

	protected:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
	};
}
