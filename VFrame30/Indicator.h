#pragma once
#include "Settings.h"

namespace VFrame30
{
	class SchemaItemIndicator;
	class CDrawParam;
	class Schema;
	class SchemaLayer;

	//
	// IndicatorComponent base class
	//
	class Indicator
	{
	public:
		Indicator() = delete;
		explicit Indicator(SchemaUnit itemUnit);
		virtual ~Indicator() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) = 0;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) = 0;
		virtual bool save(Proto::SchemaItemIndicator* message) const = 0;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const = 0;

	public:
		// Get/set are working in regional units, for drawing use variables
		//
		template <typename TYPE>
		TYPE regionalGetter(const TYPE& variable) const;			// Get/set are working in regional units, for drawing use variables

		template <typename TYPE>
		void regionalSetter(TYPE value, TYPE* variable);

		void setUnits(SchemaUnit itemUnit);

	protected:
		SchemaUnit m_itemUnit = SchemaUnit::Display;
	};



	template <typename TYPE>
	TYPE Indicator::regionalGetter(const TYPE& variable) const
	{
		if (m_itemUnit == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(variable);
		}
		else
		{
			double pt = variable;
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	template <typename TYPE>
	void Indicator::regionalSetter(TYPE value, TYPE* variable)
	{
		Q_ASSERT(variable);

		if (m_itemUnit == SchemaUnit::Display)
		{
			*variable = CUtils::RoundDisplayPoint(value);
		}
		else
		{
			*variable = CUtils::ConvertPoint(value, Settings::regionalUnit(), SchemaUnit::Inch, 0);
		}
	}

}
