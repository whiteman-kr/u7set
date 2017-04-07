#pragma once

#include "PosRectImpl.h"

class QWidget;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemControl : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemControl(void);
		explicit SchemaItemControl(SchemaUnit unit);
		virtual ~SchemaItemControl(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
	public:
		virtual QWidget* createWidget(QWidget* parent, double zoom) const;
		virtual void updateWidgetProperties(QWidget* widget, double zoom) const;

		// Properties and Data
		//
	public:
//		double weight() const;
//		void setWeight(double weight);

//		QColor lineColor() const;
//		void setLineColor(QColor color);

	private:
//		double m_weight = 0.0;				// Line weight, in pixels or inches depends on UnitDocPt
//		QColor m_lineColor;
	};
}
