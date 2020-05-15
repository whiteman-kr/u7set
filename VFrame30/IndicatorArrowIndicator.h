#pragma once
#include "Indicator.h"

namespace VFrame30
{
	//
	// ArrowIndicator
	//
	class IndicatorArrowIndicator : public Indicator
	{
		Q_OBJECT
	public:
		IndicatorArrowIndicator() = delete;
		explicit IndicatorArrowIndicator(SchemaUnit itemUnit);
		virtual ~IndicatorArrowIndicator() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;
		virtual bool save(Proto::SchemaItemIndicator* message) const override;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const override;

	public:
		double startValue() const;
		void setStartValue(double value);

		double endValue() const;
		void setEndValue(double value);

		double startAngle() const;
		void setStartAngle(double value);

		double spanAngle() const;
		void setSpanAngle(double value);

	private:
		double m_startValue = 0;
		double m_endValue = 100.0;

		double m_startAngle = 330;			// Zero degrees is at the 9 o'clock position.
		double m_spanAngle = 240;
	};

}
