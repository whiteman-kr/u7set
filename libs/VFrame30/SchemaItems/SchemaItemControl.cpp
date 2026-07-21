#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemControl.h>

#include <QScreen>

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
		Property* p = nullptr;

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::styleSheet,
									 PropertyNames::appearanceCategory,
									 true,
									 SchemaItemControl::styleSheet,
									 SchemaItemControl::setStyleSheet);
		p->setDescription(PropertyNames::widgetPropStyleSheet);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::toolTip,
									 PropertyNames::commonCategory,
									 true,
									 SchemaItemControl::toolTip,
									 SchemaItemControl::setToolTip);
		p->setDescription(PropertyNames::widgetPropToolTip);

		// --
		//
		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemControl::~SchemaItemControl(void) {}

	// Serialization
	//
	bool SchemaItemControl::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemControl* controlMessage = message->MutableExtension(Proto::schemaitem)->mutable_control();

		controlMessage->set_stylesheet(m_styleSheet.toStdString());
		controlMessage->set_tooltip(m_toolTip.toStdString());

		return true;
	}

	bool SchemaItemControl::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
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
		if (message.GetExtension(Proto::schemaitem).has_control() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_control());
			return false;
		}

		const Proto::SchemaItemControl& controlMessage = message.GetExtension(Proto::schemaitem).control();

		setStyleSheet(
			QString::fromStdString(controlMessage.stylesheet()));     // Text setters can have some string optimization for default values
		setToolTip(QString::fromStdString(controlMessage.tooltip())); // Text setters can have some string optimization for default values

		return true;
	}

	double SchemaItemControl::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemControl::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	QWidget* SchemaItemControl::createWidget(QWidget* parent, bool editMode, double zoom)
	{
		QWidget* widget = createWidgetImpl(parent, editMode, zoom);
		associateWidget(widget);

		// Run script after create
		//
		if (editMode == false)
		{
			afterCreate(widget);
		}

		return widget;
	}

	void SchemaItemControl::afterCreate(QWidget* control)
	{
		return afterCreateImpl(control);
	}

	void SchemaItemControl::updateWidgetProperties(QWidget* widget, bool editMode) const
	{
		Q_UNUSED(editMode);

		if (widget == nullptr)
		{
			assert(widget);
			return;
		}

		bool updateRequired = false;

		if (widget->isEnabled() == isCommented() || widget->styleSheet() != styleSheet() || widget->toolTip() != toolTip())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			widget->setUpdatesEnabled(false);

			widget->setDisabled(isCommented());
			widget->setStyleSheet(styleSheet());
			widget->setToolTip(toolTip());

			widget->setUpdatesEnabled(true);
		}

		return;
	}

	QWidget* SchemaItemControl::createWidgetImpl(QWidget* /*parent*/, bool /*editMode*/, double /*zoom*/)
	{
		// Must be implemented in the derived class.
		//
		Q_ASSERT(false);
		return nullptr;
	}

	void SchemaItemControl::afterCreateImpl(QWidget* /*control*/)
	{
		// Must be implemented in the derived class.
		//
		Q_ASSERT(false);
		return;
	}

	void SchemaItemControl::associateWidget(QWidget* widget)
	{
		// Disconnect currently associated widget
		//
		if (m_widget != nullptr)
		{
			disconnect(m_widget, nullptr, this, nullptr);
		}

		// Connect the new one if possible
		//
		m_widget = widget;

		if (m_widget != nullptr)
		{
			connect(m_widget,
					&QObject::destroyed,
					this,
					[this](QObject* object)
					{
						if (m_widget == object)
						{
							associateWidget(nullptr);
						}
					});
		}

		return;
	}

	void SchemaItemControl::updateWidgetPosAndSize(QWidget* widget, double zoom)
	{
		if (widget == nullptr)
		{
			assert(widget);
			return;
		}

		bool updateRequired = false;

		QPoint displayPos;
		QSize displaySize;

		switch (itemUnit())
		{
		case SchemaUnit::Display:
			{
				double devicePixelRatio = widget->devicePixelRatioF();

				displayPos = {static_cast<int>(leftDocPt() * zoom / 100.0 / devicePixelRatio),
							  static_cast<int>(topDocPt() * zoom / 100.0 / devicePixelRatio)};

				displaySize = {static_cast<int>(widthDocPt() * zoom / 100.0 / devicePixelRatio),
							   static_cast<int>(heightDocPt() * zoom / 100.0 / devicePixelRatio)};
			}
			break;
		case SchemaUnit::Inch:
			{
				const auto widgetScreen = widget->screen();

				double dpiX = widgetScreen ? widgetScreen->physicalDotsPerInchX() : widget->physicalDpiX();
				double dpiY = widgetScreen ? widgetScreen->physicalDotsPerInchY() : widget->physicalDpiY();

				displayPos = {static_cast<int>(leftDocPt() * zoom / 100.0 * dpiX), static_cast<int>(topDocPt() * zoom / 100.0 * dpiY)};

				displaySize = {static_cast<int>(widthDocPt() * zoom / 100.0 * dpiX), static_cast<int>(heightDocPt() * zoom / 100.0 * dpiY)};
			}
			break;
		default:
			assert(false);
		}

		if (widget->pos() != displayPos || widget->size() != displaySize)
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			widget->setUpdatesEnabled(false);

			if (widget->pos() != displayPos)
			{
				widget->move(displayPos);
			}

			if (widget->size() != displaySize)
			{
				widget->resize(displaySize);
			}

			widget->setUpdatesEnabled(true);
		}
	}

	QJSValue SchemaItemControl::evaluateScript(QString scriptName, QWidget* controlWidget, QString script)
	{
		if (controlWidget == nullptr)
		{
			assert(controlWidget);
			return false;
		}

		// Suppose that parent of sender is SchemaView
		//
		ClientSchemaView* schemaView = dynamic_cast<ClientSchemaView*>(controlWidget->parentWidget());
		if (schemaView == nullptr)
		{
			assert(schemaView);
			return {};
		}

		QJSEngine* engine = schemaView->jsEngine();
		assert(engine);

		QJSValue result = SchemaItem::evaluateScript(scriptName, script, engine, schemaView);
		return result;
	}

	// Properties and Data
	//
	const QString& SchemaItemControl::styleSheet() const
	{
		return m_styleSheet;
	}

	void SchemaItemControl::setStyleSheet(QString value)
	{
		m_styleSheet = std::move(value);
	}

	const QString& SchemaItemControl::toolTip() const
	{
		return m_toolTip;
	}

	void SchemaItemControl::setToolTip(QString value)
	{
		m_toolTip = value;
	}

} // namespace VFrame30
