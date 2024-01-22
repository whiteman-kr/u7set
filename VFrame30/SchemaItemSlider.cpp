#include "SchemaItemSlider.h"
#include "ClientSchemaView.h"
#include "DrawParam.h"
#include "TuningController.h"

namespace
{
	// Subclass control to filter mouse input events for schema editor.
	//
	class QLimitMouseWheelSlider : public QSlider
	{
	public:
		QLimitMouseWheelSlider(bool enableMouseWheel, bool editMode, QWidget* parent) :
			QSlider{parent},
			m_enableMouseWheel(enableMouseWheel),
			m_editMode(editMode)
		{
			//// Focus does not appear in the Monitor initial Tab key is pressed, I did not dig deep enough to understand
			//// the nature of this behavior. Just forbad focus.
			////
			//setFocusPolicy(Qt::FocusPolicy::NoFocus);

			if (m_editMode == true)
			{
				setTracking(false);
				setMouseTracking(false);
				setAttribute(Qt::WA_TransparentForMouseEvents);
			}
		}

	protected:
		virtual bool event(QEvent* event) override
		{
			// Ignore all input events in the edit mode, so click on the item will select it and so on.
			//
			if (m_editMode == true && event->isInputEvent() == true)
			{
				event->ignore();
				return true;
			}
			else
			{
				return QSlider::event(event);
			}
		}

		virtual void wheelEvent(QWheelEvent* event) override
		{
			if (m_editMode == true || m_enableMouseWheel == false)
			{
				event->ignore();
			}
			else
			{
				QSlider::wheelEvent(event);
				event->accept(); // Do not propagate event to the parent, so mouse wheel will not zoomIn/zoomOut to the view.
			}

			return;
		}

	public:
		bool m_enableMouseWheel = false;
		bool m_editMode = true;
	};
} // namespace

namespace VFrame30
{
	SchemaItemSlider::SchemaItemSlider(void) :
		SchemaItemSlider(SchemaUnit::Inch)
	{
		// This constructor can be call in case of loading this object
		//
	}

	SchemaItemSlider::SchemaItemSlider(SchemaUnit unit) :
		SchemaItemControl(unit)
	{
		Property* p = nullptr;

		p = ADD_PROPERTY_GET_SET_CAT(Qt::Orientation, PropertyNames::orientation, PropertyNames::appearanceCategory, true, SchemaItemSlider::orientation, SchemaItemSlider::setOrientation);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::sliderInvertedAppearance, PropertyNames::appearanceCategory, true, SchemaItemSlider::invertedAppearance, SchemaItemSlider::setInvertedAppearance);
		p->setDescription(PropertyNames::sliderInvertedAppearanceToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::sliderInvertedControls, PropertyNames::behaviourCategory, true, SchemaItemSlider::invertedControls, SchemaItemSlider::setInvertedControls);
		p->setDescription(PropertyNames::sliderInvertedControlsToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::sliderEnableMouseWheel, PropertyNames::behaviourCategory, true, SchemaItemSlider::enableMouseWheel, SchemaItemSlider::setEnableMouseWheel);
		p->setDescription(PropertyNames::sliderEnableMouseWheelToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderMaximum, PropertyNames::controlCategory, true, SchemaItemSlider::maximum, SchemaItemSlider::setMaximum);
		p->setDescription(PropertyNames::sliderMaximumToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderMinimum, PropertyNames::controlCategory, true, SchemaItemSlider::minimum, SchemaItemSlider::setMinimum);
		p->setDescription(PropertyNames::sliderMinimumToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderPageStep, PropertyNames::controlCategory, true, SchemaItemSlider::pageStep, SchemaItemSlider::setPageStep);
		p->setDescription(PropertyNames::sliderPageStepToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderSingleStep, PropertyNames::controlCategory, true, SchemaItemSlider::singleStep, SchemaItemSlider::setSingleStep);
		p->setDescription(PropertyNames::sliderSingleStepToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::sliderTracking, PropertyNames::behaviourCategory, true, SchemaItemSlider::tracking, SchemaItemSlider::setTracking);
		p->setDescription(PropertyNames::sliderTrackingToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderTickInterval, PropertyNames::appearanceCategory, true, SchemaItemSlider::tickInterval, SchemaItemSlider::setTickInterval);
		p->setDescription(PropertyNames::sliderTickIntervalToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(QSlider::TickPosition, PropertyNames::sliderTickPosition, PropertyNames::appearanceCategory, true, SchemaItemSlider::tickPosition, SchemaItemSlider::setTickPosition);
		p->setDescription(PropertyNames::sliderTickPositionToolTip);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::sliderDefaultValue, PropertyNames::controlCategory, true, SchemaItemSlider::defaultValue, SchemaItemSlider::setDefaultValue);

		// Script properties.
		//
		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::afterCreate, PropertyNames::scriptsCategory, true, SchemaItemSlider::scriptAfterCreate, SchemaItemSlider::setScriptAfterCreate);
		p->setDescription(PropertyNames::widgetPropAfterCreate);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::sliderMoved, PropertyNames::scriptsCategory, true, SchemaItemSlider::scriptSliderMoved, SchemaItemSlider::setScriptSliderMoved);
		p->setDescription(PropertyNames::sliderMovedToolTip);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::sliderPressed, PropertyNames::scriptsCategory, true, SchemaItemSlider::scriptSliderPressed, SchemaItemSlider::setScriptSliderPressed);
		p->setDescription(PropertyNames::sliderPressedToolTip);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::sliderReleased, PropertyNames::scriptsCategory, true, SchemaItemSlider::scriptSliderReleased, SchemaItemSlider::setScriptSliderReleased);
		p->setDescription(PropertyNames::sliderReleasedToolTip);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::sliderValueChanged, PropertyNames::scriptsCategory, true, SchemaItemSlider::scriptValueChanged, SchemaItemSlider::setScriptValueChanged);
		p->setDescription(PropertyNames::sliderValueChangedToolTip);
		p->setIsScript(true);

		return;
	}

	void SchemaItemSlider::draw(CDrawParam* drawParam) const
	{
		// Control is drawn only in PDF mode
		//
		if (drawParam->pdfMode() == true)
		{
			drawSliderControl(drawParam, parentSchema(), parentLayer().get());
		}

		return;
	}

	void SchemaItemSlider::drawSliderControl(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		Q_UNUSED(layer);

		QPainter* painter = drawParam->painter();
		const QStyle* style = QApplication::style();
		QRectF r = boundingRectInDocPt(drawParam);

		// Draw column.
		//
		painter->setBrush(style->standardPalette().button());

		QPen pen;
		pen.setColor(style->standardPalette().color(QPalette::Normal, QPalette::Dark));
		pen.setWidthF(1.0 / drawParam->realDpiX());
		painter->setPen(pen);

		QRectF columnRect{r.left() + r.width() / 2 - mm2in(1), r.top(), mm2in(2), r.height()};
		painter->drawRect(columnRect);

		// Draw handle.
		//
		const double handleHeight = (schema->unit() == SchemaUnit::Display) ? 12.0 : mm2in(3);

		QRectF handleRect{r.left(), r.top() + r.height() / 3 - handleHeight / 2, r.width(), handleHeight};
		painter->drawRect(handleRect);

		return;
	}

	// Serialization
	//
	bool SchemaItemSlider::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemControl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		assert(message->schemaitem().has_posrectimpl());
		assert(message->schemaitem().has_control());

		// --
		//
		::Proto::SchemaItemSlider* sliderMessage = message->mutable_schemaitem()->mutable_slider();

		sliderMessage->set_orientation(m_orientation);
		sliderMessage->set_invertedappearance(m_invertedAppearance);
		sliderMessage->set_invertedcontrols(m_invertedControls);
		sliderMessage->set_enablemousewheel(m_enableMouseWheel);

		sliderMessage->set_maximum(m_maximum);
		sliderMessage->set_minimum(m_minimum);

		sliderMessage->set_pagestep(m_pageStep);
		sliderMessage->set_singlestep(m_singleStep);
		sliderMessage->set_tracking(m_tracking);
		sliderMessage->set_tickinterval(m_tickInterval);
		sliderMessage->set_tickposition(m_tickPosition);

		sliderMessage->set_defaultvalue(m_defaultValue);

		// Scripts
		//
		sliderMessage->set_scriptaftercreate(m_scriptAfterCreate.toStdString());
		sliderMessage->set_scriptslidermoved(m_scriptSliderMoved.toStdString());
		sliderMessage->set_scriptsliderpressed(m_scriptSliderPressed.toStdString());
		sliderMessage->set_scriptsliderreleased(m_scriptSliderReleased.toStdString());
		sliderMessage->set_scriptvaluechanged(m_scriptValueChanged.toStdString());

		return true;
	}

	bool SchemaItemSlider::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemControl::LoadData(message);
		if (result == false || message.schemaitem().has_control() == false)
		{
			assert(message.schemaitem().has_control());
			return false;
		}

		const Proto::SchemaItemSlider& sliderMessage = message.schemaitem().slider();

		m_orientation = static_cast<Qt::Orientation>(sliderMessage.orientation());
		m_invertedAppearance = sliderMessage.invertedappearance();
		m_invertedControls = sliderMessage.invertedcontrols();
		m_enableMouseWheel = sliderMessage.enablemousewheel();

		m_maximum = sliderMessage.maximum();
		m_minimum = sliderMessage.minimum();

		m_pageStep = sliderMessage.pagestep();
		m_singleStep = sliderMessage.singlestep();
		m_tracking = sliderMessage.tracking();
		m_tickInterval = sliderMessage.tickinterval();
		m_tickPosition = static_cast<QSlider::TickPosition>(sliderMessage.tickposition());

		m_defaultValue = sliderMessage.defaultvalue();

		// Text setters can have some string optimization for default values
		//
		setScriptAfterCreate(QString::fromStdString(sliderMessage.scriptaftercreate()));
		setScriptSliderMoved(QString::fromStdString(sliderMessage.scriptslidermoved()));
		setScriptSliderPressed(QString::fromStdString(sliderMessage.scriptsliderpressed()));
		setScriptSliderReleased(QString::fromStdString(sliderMessage.scriptsliderreleased()));
		setScriptValueChanged(QString::fromStdString(sliderMessage.scriptvaluechanged()));

		return true;
	}

	QWidget* SchemaItemSlider::createWidgetImpl(QWidget* parent, bool editMode, double zoom)
	{
		if (parent == nullptr)
		{
			assert(parent);
			return nullptr;
		}

		QSlider* control = new QLimitMouseWheelSlider{enableMouseWheel(), editMode, parent};
		control->setObjectName(guid().toString());

		updateWidgetProperties(control, editMode);

		// Set default slider value
		//
		control->setValue(m_defaultValue);

		if (editMode == false)
		{
			// Connect slots only if it has any sense
			//
			if (scriptSliderMoved().isEmpty() == false &&
				scriptSliderMoved() != PropertyNames::sliderDefaultEventScript)
			{
				connect(control, &QSlider::sliderMoved, this, &SchemaItemSlider::sliderMoved);
			}

			if (scriptSliderPressed().isEmpty() == false &&
				scriptSliderPressed() != PropertyNames::sliderDefaultEventScript)
			{
				connect(control, &QSlider::sliderPressed, this, &SchemaItemSlider::sliderPressed);
			}

			if (scriptSliderReleased().isEmpty() == false &&
				scriptSliderReleased() != PropertyNames::sliderDefaultEventScript)
			{
				connect(control, &QSlider::sliderReleased, this, &SchemaItemSlider::sliderReleased);
			}

			if (scriptValueChanged().isEmpty() == false &&
				scriptValueChanged() != PropertyNames::sliderDefaultEventScript)
			{
				connect(control, &QSlider::valueChanged, this, &SchemaItemSlider::valueChanged);
			}
		}

		updateWidgetPosAndSize(control, zoom);

		control->setVisible(true);
		control->update();

		return control;
	}

	// Update widget properties
	//
	void SchemaItemSlider::updateWidgetProperties(QWidget* widget, bool editMode) const
	{
		QSlider* control = dynamic_cast<QSlider*>(widget);

		if (control == nullptr)
		{
			assert(control);
			return;
		}

		SchemaItemControl::updateWidgetProperties(widget, editMode);

		bool updateRequired = false;

		if (control->orientation() != orientation() ||
			control->invertedAppearance() != invertedAppearance() ||
			control->invertedControls() != invertedControls() ||
			control->maximum() != maximum() ||
			control->minimum() != minimum() ||
			control->pageStep() != pageStep() ||
			control->singleStep() != singleStep() ||
			control->hasTracking() != tracking() ||
			control->tickInterval() != tickInterval() ||
			control->tickPosition() != tickPosition())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			control->setUpdatesEnabled(false);

			control->setOrientation(orientation());
			control->setInvertedAppearance(invertedAppearance());
			control->setInvertedControls(invertedControls());

			control->setMaximum(maximum());
			control->setMinimum(minimum());

			control->setPageStep(pageStep());
			control->setSingleStep(singleStep());
			control->setTracking(tracking());
			control->setTickInterval(tickInterval());
			control->setTickPosition(tickPosition());

			control->setUpdatesEnabled(true);

			// When minimum/maximum are set, the minimum/maximum are adjusted if necessary to ensure that the range remains valid.
			// Also the slider's current value is adjusted to be within the new range.
			//
			const_cast<SchemaItemSlider*>(this)->setMaximum(control->maximum());
			const_cast<SchemaItemSlider*>(this)->setMinimum(control->minimum());
		}

		if (editMode == true &&
			control->value() != defaultValue())
		{
			control->setUpdatesEnabled(false);
			control->setValue(defaultValue());
			control->setUpdatesEnabled(true);
		}

		return;
	}


	void SchemaItemSlider::afterCreateImpl(QWidget* control)
	{
		QSlider* sliderWidget = dynamic_cast<QSlider*>(control);
		if (sliderWidget == nullptr)
		{
			assert(sliderWidget);
			return;
		}

		if (m_scriptAfterCreate.trimmed().isEmpty() == true ||
			m_scriptAfterCreate == PropertyNames::sliderDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		// Evaluate script
		//
		if (m_jsAfterCreate.isUndefined() == true)
		{
			m_jsAfterCreate = evaluateScript("AfterCreate", sliderWidget, m_scriptAfterCreate);

			if (m_jsAfterCreate.isError() == true ||
				m_jsAfterCreate.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript("AfterCreate", m_jsAfterCreate, sliderWidget, false);

		return;
	}

	void SchemaItemSlider::sliderMoved(int /*value*/)
	{
		return runEventScript("SliderMoved", m_scriptSliderMoved, m_jsSliderMoved, dynamic_cast<QSlider*>(sender()));
	}

	void SchemaItemSlider::sliderPressed()
	{
		return runEventScript("SliderPressed", m_scriptSliderPressed, m_jsSliderPressed, dynamic_cast<QSlider*>(sender()));
	}

	void SchemaItemSlider::sliderReleased()
	{
		return runEventScript("SliderReleased", m_scriptSliderReleased, m_jsSliderReleased, dynamic_cast<QSlider*>(sender()));
	}

	void SchemaItemSlider::valueChanged(int value)
	{
		Q_UNUSED(value);
		return runEventScript("ValueChanged", m_scriptValueChanged, m_jsValueChanged, dynamic_cast<QSlider*>(sender()));
	}

	void SchemaItemSlider::runEventScript(QString scriptName, const QString& script, QJSValue& js, QSlider* sliderWidget)
	{
		if (sliderWidget == nullptr)
		{
			Q_ASSERT(sliderWidget);
			return;
		}

		if (script.isEmpty() == true ||
			script == PropertyNames::sliderDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		// Evaluate script
		//
		if (js.isUndefined() == true)
		{
			js = evaluateScript(scriptName, sliderWidget, script);

			if (js.isError() == true ||
				js.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(scriptName, js, sliderWidget, true);
		return;
	}

	void SchemaItemSlider::runEventScript(QString scriptName, QJSValue& evaluatedJs, QSlider* sliderWidget, bool allowMessageBox)
	{
		return SchemaItemControl::runEventScript<QSlider>(evaluatedJs, allowMessageBox, scriptName, sliderWidget, sliderWidget->value());
	}

	// Properties and Data
	//
	Qt::Orientation SchemaItemSlider::orientation() const
	{
		return m_orientation;
	}

	void SchemaItemSlider::setOrientation(Qt::Orientation orientation)
	{
		m_orientation = orientation;
	}

	bool SchemaItemSlider::invertedAppearance() const
	{
		return m_invertedAppearance;
	}

	void SchemaItemSlider::setInvertedAppearance(bool inverted)
	{
		m_invertedAppearance = inverted;
	}

	bool SchemaItemSlider::invertedControls() const
	{
		return m_invertedControls;
	}

	void SchemaItemSlider::setInvertedControls(bool inverted)
	{
		m_invertedControls = inverted;
	}

	bool SchemaItemSlider::enableMouseWheel() const
	{
		return m_enableMouseWheel;
	}

	void SchemaItemSlider::setEnableMouseWheel(bool enable)
	{
		m_enableMouseWheel = enable;
	}

	int SchemaItemSlider::maximum() const
	{
		return m_maximum;
	}

	void SchemaItemSlider::setMaximum(int max)
	{
		m_maximum = max;

		if (m_maximum < m_minimum)
		{
			m_maximum = m_minimum;
		}

		return;
	}

	int SchemaItemSlider::minimum() const
	{
		return m_minimum;
	}

	void SchemaItemSlider::setMinimum(int min)
	{
		m_minimum = min;

		if (m_minimum > m_maximum)
		{
			m_minimum = m_maximum;
		}

		return;
	}

	int SchemaItemSlider::pageStep() const
	{
		return m_pageStep;
	}
	void SchemaItemSlider::setPageStep(int step)
	{
		m_pageStep = step;
	}

	int SchemaItemSlider::singleStep() const
	{
		return m_singleStep;
	}
	void SchemaItemSlider::setSingleStep(int step)
	{
		m_singleStep = step;
	}

	bool SchemaItemSlider::tracking() const
	{
		return m_tracking;
	}

	void SchemaItemSlider::setTracking(bool track)
	{
		m_tracking = track;
	}

	int SchemaItemSlider::tickInterval() const
	{
		return m_tickInterval;
	}

	void SchemaItemSlider::setTickInterval(int interval)
	{
		m_tickInterval = interval;
	}

	QSlider::TickPosition SchemaItemSlider::tickPosition() const
	{
		return m_tickPosition;
	}

	void SchemaItemSlider::setTickPosition(QSlider::TickPosition position)
	{
		m_tickPosition = position;
	}

	int SchemaItemSlider::defaultValue() const
	{
		return m_defaultValue;
	}

	void SchemaItemSlider::setDefaultValue(int value)
	{
		m_defaultValue = value;
	}

	QString SchemaItemSlider::scriptAfterCreate() const
	{
		return m_scriptAfterCreate;
	}

	void SchemaItemSlider::setScriptAfterCreate(const QString& value)
	{
		if (value == PropertyNames::sliderDefaultEventScript)
		{
			m_scriptAfterCreate = PropertyNames::sliderDefaultEventScript;
		}
		else
		{
			m_scriptAfterCreate = value;
		}
	}

	QString SchemaItemSlider::scriptSliderMoved() const
	{
		return m_scriptSliderMoved;
	}

	void SchemaItemSlider::setScriptSliderMoved(const QString& value)
	{
		if (value == PropertyNames::sliderDefaultEventScript)
		{
			m_scriptSliderMoved = PropertyNames::sliderDefaultEventScript;
		}
		else
		{
			m_scriptSliderMoved = value;
		}
	}

	QString SchemaItemSlider::scriptSliderPressed() const
	{
		return m_scriptSliderPressed;
	}

	void SchemaItemSlider::setScriptSliderPressed(const QString& value)
	{
		if (value == PropertyNames::sliderDefaultEventScript)
		{
			m_scriptSliderPressed = PropertyNames::sliderDefaultEventScript;
		}
		else
		{
			m_scriptSliderPressed = value;
		}
	}

	QString SchemaItemSlider::scriptSliderReleased() const
	{
		return m_scriptSliderReleased;
	}

	void SchemaItemSlider::setScriptSliderReleased(const QString& value)
	{
		if (value == PropertyNames::sliderDefaultEventScript)
		{
			m_scriptSliderReleased = PropertyNames::sliderDefaultEventScript;
		}
		else
		{
			m_scriptSliderReleased = value;
		}
	}

	QString SchemaItemSlider::scriptValueChanged() const
	{
		return m_scriptValueChanged;
	}

	void SchemaItemSlider::setScriptValueChanged(const QString& value)
	{
		if (value == PropertyNames::sliderDefaultEventScript)
		{
			m_scriptValueChanged = PropertyNames::sliderDefaultEventScript;
		}
		else
		{
			m_scriptValueChanged = value;
		}
	}
} // namespace VFrame30
