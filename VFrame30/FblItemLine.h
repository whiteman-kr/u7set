#pragma once

#include "PosConnectionImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class FblItemLine : public PosConnectionImpl, public FblItem
	{
		Q_OBJECT

	public:
		FblItemLine(void);
		FblItemLine(SchemaUnit itemunit);
		virtual ~FblItemLine(void);
		
	protected:
		virtual void propertyDemand(const QString& prop) override;
		
		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;


		// Properties and Data
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		E::LineStyle lineStyle() const;
		void setLineStyle(E::LineStyle value);

	protected:
		double m_weight;					// Line weight, pixels/inchces depends on UnitDocPt
		QColor m_lineColor;
		E::LineStyle m_lineStyle = E::SolidLine;
	};
}
