#include "Stable.h"
#include "SchemaItemControl.h"
#include "MacrosExpander.h"

namespace VFrame30
{
	SchemaItemControl::SchemaItemControl(void) :
		SchemaItemControl(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemControl::SchemaItemControl(SchemaUnit unit)
	{
//		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemControl::weight, SchemaItemControl::setWeight);

		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemControl::~SchemaItemControl(void)
	{
	}

	// Serialization
	//
	bool SchemaItemControl::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemControl* controlMessage = message->mutable_schemaitem()->mutable_control();
		Q_UNUSED(controlMessage);

		//rectMessage->set_linecolor(m_lineColor.rgba());

		return true;
	}

	bool SchemaItemControl::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_control() == false)
		{
			assert(message.schemaitem().has_control());
			return false;
		}

		const Proto::SchemaItemControl& controlMessage = message.schemaitem().control();
		Q_UNUSED(controlMessage);

		//m_weight = rectMessage.weight();

		return true;
	}

	QWidget* SchemaItemControl::createWidget(QWidget* /*parent*/, double /*zoom*/) const
	{
		// Implement in derived class
		//
		assert(false);
		return nullptr;
	}

	void SchemaItemControl::updateWidgetProperties(QWidget* widget, double /*zoom*/) const
	{
		if (widget == nullptr)
		{
			assert(widget);
			return;
		}

		if (widget->isEnabled() == isCommented())
		{
			widget->setDisabled(isCommented());
		}

		return;
	}


	// Properties and Data
	//

	//	// LineColor property
//	//
//	QColor SchemaItemControl::lineColor() const
//	{
//		return m_lineColor;
//	}

//	void SchemaItemControl::setLineColor(QColor color)
//	{
//		m_lineColor = color;
//	}

}

