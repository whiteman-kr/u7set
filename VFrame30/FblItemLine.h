#pragma once

#include "PosConnectionImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FblItemLine : public PosConnectionImpl, public FblItem
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
		void setWeight(const double& weight);

		QColor lineColor() const;
		void setLineColor(const QColor& color);

	protected:
		double m_weight;					// Line weight, pixels/inchces depends on UnitDocPt
		QColor m_lineColor;
	};
}
