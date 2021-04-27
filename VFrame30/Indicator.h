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
	class Indicator : public QObject
	{
		Q_OBJECT
	public:
		Indicator() = delete;
		explicit Indicator(SchemaUnit itemUnit);
		virtual ~Indicator() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) = 0;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) = 0;
		virtual bool save(Proto::SchemaItemIndicator* message) const = 0;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const = 0;

	signals:
		void updatePropertiesList();

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
			return VFrame30::RoundDisplayPoint(variable);
		}
		else
		{
			double pt = variable;
			pt = VFrame30::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return VFrame30::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	template <typename TYPE>
	void Indicator::regionalSetter(TYPE value, TYPE* variable)
	{
		Q_ASSERT(variable);

		if (m_itemUnit == SchemaUnit::Display)
		{
			*variable = VFrame30::RoundDisplayPoint(value);
		}
		else
		{
			*variable = VFrame30::ConvertPoint(value, Settings::regionalUnit(), SchemaUnit::Inch, 0);
		}
	}

}
