#include "SchemaItemControl.h"
#include "PropertyNames.h"

namespace VFrame30
{
	SchemaItemControl::SchemaItemControl(void) :
		SchemaItemControl(SchemaUnit::Inch)
	{
		// This contructor can be call in case of loading this object
		//
	}

	SchemaItemControl::SchemaItemControl(SchemaUnit unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::styleSheet, PropertyNames::controlCategory, true, SchemaItemControl::styleSheet, SchemaItemControl::setStyleSheet);

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

		controlMessage->set_stylesheet(m_styleSheet.toStdString());

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

		setStyleSheet(QString::fromStdString(controlMessage.stylesheet()));		// Text setters can have some string optimization for default values

		return true;
	}

	QWidget* SchemaItemControl::createWidget(QWidget* /*parent*/, bool /*editMode*/) const
	{
		// Implement in derived class
		//
		assert(false);
		return nullptr;
	}

	void SchemaItemControl::updateWidgetProperties(QWidget* widget) const
	{
		if (widget == nullptr)
		{
			assert(widget);
			return;
		}

		bool updateRequired = false;

		if (widget->isEnabled() == isCommented() ||
			widget->styleSheet() != styleSheet())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			widget->setUpdatesEnabled(false);

			widget->setDisabled(isCommented());
			widget->setStyleSheet(styleSheet());

			widget->setUpdatesEnabled(true);
		}

		return;
	}

	void SchemaItemControl::updateWdgetPosAndSize(QWidget* widget, double zoom)
	{
		if (widget == nullptr)
		{
			assert(widget);
			return;
		}

		bool updateRequired = false;

		QPoint pos = {static_cast<int>(leftDocPt() * zoom / 100.0),
					  static_cast<int>(topDocPt() * zoom / 100.0)};

		QSize size = {static_cast<int>(widthDocPt() * zoom / 100.0),
					  static_cast<int>(heightDocPt() * zoom / 100.0)};

		if (widget->pos() != pos ||
			widget->size() != size)
		{
			 updateRequired = true;
		}

		if (updateRequired == true)
		{
			widget->setUpdatesEnabled(false);

			if (widget->pos() != pos)
			{
				widget->move(pos);
			}

			if (widget->size() != size)
			{
				widget->resize(size);
			}

			widget->setUpdatesEnabled(true);
		}
	}

	// Properties and Data
	//
	const QString& SchemaItemControl::styleSheet() const
	{
		return m_styleSheet;
	}

	void SchemaItemControl::setStyleSheet(QString value)
	{
		m_styleSheet = value;
	}

}

