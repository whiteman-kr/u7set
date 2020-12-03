#include "SchemaItemPushButton.h"
#include "ClientSchemaView.h"
#include "TuningController.h"

namespace VFrame30
{
	SchemaItemPushButton::SchemaItemPushButton(void) :
		SchemaItemPushButton(SchemaUnit::Inch)
	{
		// This contructor can be call in case of loading this object
		//
	}

	SchemaItemPushButton::SchemaItemPushButton(SchemaUnit unit) :
		SchemaItemControl(unit)
	{
		setStyleSheet(PropertyNames::pushButtonDefaultStyleSheet);

		Property* p = nullptr;

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::controlCategory, true, SchemaItemPushButton::text, SchemaItemPushButton::setText);
		p->setDescription(PropertyNames::pushButtonPropText);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::checkable, PropertyNames::controlCategory, true, SchemaItemPushButton::isCheckable, SchemaItemPushButton::setCheckable);
		p->setDescription(PropertyNames::pushButtonPropCheckable);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::checkedDefault, PropertyNames::controlCategory, true, SchemaItemPushButton::checkedDefault, SchemaItemPushButton::setCheckedDefault);
		p->setDescription(PropertyNames::pushButtonPropCheckedDefault);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::autoRepeat, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeat, SchemaItemPushButton::setAutoRepeat);
		p->setDescription(PropertyNames::pushButtonPropAutoRepeat);
		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::autoRepeatDelay, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeatDelay, SchemaItemPushButton::setAutoRepeatDelay);
		p->setDescription(PropertyNames::pushButtonPropAutoRepeatDelay);
		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::autoRepeatInterval, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeatInterval, SchemaItemPushButton::setAutoRepeatInterval);
		p->setDescription(PropertyNames::pushButtonPropAutoRepeatInterval);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::afterCreate, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptAfterCreate, SchemaItemPushButton::setScriptAfterCreate);
		p->setDescription(PropertyNames::widgetPropAfterCreate);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::clicked, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptClicked, SchemaItemPushButton::setScriptClicked);
		p->setDescription(PropertyNames::pushButtonPropClicked);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::pressed, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptPressed, SchemaItemPushButton::setScriptPressed);
		p->setDescription(PropertyNames::pushButtonPropPressed);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::released, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptReleased, SchemaItemPushButton::setScriptReleased);
		p->setDescription(PropertyNames::pushButtonPropReleased);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::toggled, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptToggled, SchemaItemPushButton::setScriptToggled);
		p->setDescription(PropertyNames::pushButtonPropToggled);
		p->setIsScript(true);

		return;
	}

	SchemaItemPushButton::~SchemaItemPushButton(void)
	{
	}

	// Serialization
	//
	bool SchemaItemPushButton::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemPushButton* pushButtonMessage = message->mutable_schemaitem()->mutable_pushbutton();

		pushButtonMessage->set_text(m_text.toStdString());

		pushButtonMessage->set_checkable(m_checkable);
		pushButtonMessage->set_checkeddefault(m_checkedDefault);

		pushButtonMessage->set_autorepeat(m_autoRepeat);
		pushButtonMessage->set_autorepeatdelay(m_autoRepeatDelay);
		pushButtonMessage->set_autorepeatinterval(m_autoRepeatInterval);

		pushButtonMessage->set_scriptaftercreate(m_scriptAfterCreate.toStdString());
		pushButtonMessage->set_scriptclicked(m_scriptClicked.toStdString());
		pushButtonMessage->set_scriptpressed(m_scriptPressed.toStdString());
		pushButtonMessage->set_scriptreleased(m_scriptReleased.toStdString());
		pushButtonMessage->set_scripttoggled(m_scriptToggled.toStdString());

		return true;
	}

	bool SchemaItemPushButton::LoadData(const Proto::Envelope& message)
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

		const Proto::SchemaItemPushButton& pushButtonMessagemessage = message.schemaitem().pushbutton();

		setText(QString::fromStdString(pushButtonMessagemessage.text()));							// Text setters can have some string optimization for default values

		m_checkable = pushButtonMessagemessage.checkable();
		m_checkedDefault = pushButtonMessagemessage.checkeddefault();

		m_autoRepeat = pushButtonMessagemessage.autorepeat();
		m_autoRepeatDelay = pushButtonMessagemessage.autorepeatdelay();
		m_autoRepeatInterval = pushButtonMessagemessage.autorepeatinterval();

		setScriptAfterCreate(QString::fromStdString(pushButtonMessagemessage.scriptaftercreate()));	// Text setters can have some string optimization for default values
		setScriptClicked(QString::fromStdString(pushButtonMessagemessage.scriptclicked()));			// Text setters can have some string optimization for default values
		setScriptPressed(QString::fromStdString(pushButtonMessagemessage.scriptpressed()));			// Text setters can have some string optimization for default values
		setScriptReleased(QString::fromStdString(pushButtonMessagemessage.scriptreleased()));		// Text setters can have some string optimization for default values
		setScriptToggled(QString::fromStdString(pushButtonMessagemessage.scripttoggled()));			// Text setters can have some string optimization for default values

		return true;
	}

	QWidget* SchemaItemPushButton::createWidget(QWidget* parent, bool editMode, double zoom)
	{
		if (parent == nullptr)
		{
			assert(parent);
			return nullptr;
		}

		QPushButton* control = new QPushButton(m_text, parent);
		control->setObjectName(guid().toString());

		updateWidgetProperties(control);

		control->setChecked(checkedDefault());

		if (editMode == false)
		{
			// Connect slots only if it has any sense
			//
			if (scriptClicked().isEmpty() == false &&
				scriptClicked() != PropertyNames::pushButtonDefaultEventScript)
			{
				connect(control, &QPushButton::clicked, this, &SchemaItemPushButton::clicked);
			}

			if (scriptPressed().isEmpty() == false &&
				scriptPressed() != PropertyNames::pushButtonDefaultEventScript)
			{
				connect(control, &QPushButton::pressed, this, &SchemaItemPushButton::pressed);
			}

			if (scriptReleased().isEmpty() == false &&
				scriptReleased() != PropertyNames::pushButtonDefaultEventScript)
			{
				connect(control, &QPushButton::released, this, &SchemaItemPushButton::released);
			}

			if (scriptToggled().isEmpty() == false &&
				scriptToggled() != PropertyNames::pushButtonDefaultEventScript)
			{
				connect(control, &QPushButton::toggled, this, &SchemaItemPushButton::toggled);
			}

			// Run script after create
			//
			afterCreate(control);
		}

		updateWdgetPosAndSize(control, zoom);

		control->setVisible(true);
		control->update();

		return control;
	}

	// Update widget properties
	//
	void SchemaItemPushButton::updateWidgetProperties(QWidget* widget) const
	{
		QPushButton* control = dynamic_cast<QPushButton*>(widget);

		if (control == nullptr)
		{
			assert(control);
			return;
		}

		SchemaItemControl::updateWidgetProperties(widget);

		bool updateRequired = false;

		if (control->text() != text() ||
			control->isCheckable() != isCheckable() ||
			control->autoRepeat() != autoRepeat() ||
			control->autoRepeatDelay() != autoRepeatDelay() ||
			control->autoRepeatInterval() != autoRepeatInterval())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			control->setUpdatesEnabled(false);

			control->setText(text());
			control->setCheckable(isCheckable());
			control->setAutoRepeat(autoRepeat());
			control->setAutoRepeatDelay(autoRepeatDelay());
			control->setAutoRepeatInterval(autoRepeatInterval());

			control->setUpdatesEnabled(true);
		}

		return;
	}


	void SchemaItemPushButton::afterCreate(QPushButton* control)
	{
		if (control == nullptr)
		{
			assert(control);
			return;
		}

		if (m_scriptAfterCreate.trimmed().isEmpty() == true ||
			m_scriptAfterCreate == PropertyNames::pushButtonDefaultEventScript)	// Suppose Default script does nothing, just return
		{
			return;
		}

		// Evaluate script
		//
		if (m_jsAfterCreate.isUndefined() == true)
		{
			m_jsAfterCreate = evaluateScript(control, m_scriptAfterCreate);

			if (m_jsAfterCreate.isError() == true ||
				m_jsAfterCreate.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(m_jsAfterCreate, control);

		return;
	}

	void SchemaItemPushButton::clicked(bool /*checked*/)
	{
		if (m_scriptClicked.isEmpty() == true ||
			m_scriptClicked == PropertyNames::pushButtonDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QPushButton* senderWidget = dynamic_cast<QPushButton*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsClicked.isUndefined() == true)
		{
			m_jsClicked = evaluateScript(senderWidget, m_scriptClicked);

			if (m_jsClicked.isError() == true ||
				m_jsClicked.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(m_jsClicked, senderWidget);

		return;
	}

	void SchemaItemPushButton::pressed()
	{
		if (m_scriptPressed.isEmpty() == true ||
			m_scriptPressed == PropertyNames::pushButtonDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QPushButton* senderWidget = dynamic_cast<QPushButton*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsPressed.isUndefined() == true)
		{
			m_jsPressed = evaluateScript(senderWidget, m_scriptPressed);

			if (m_jsPressed.isError() == true ||
				m_jsPressed.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(m_jsPressed, senderWidget);

		return;
	}

	void SchemaItemPushButton::released()
	{
		if (m_scriptReleased.isEmpty() == true ||
			m_scriptReleased == PropertyNames::pushButtonDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QPushButton* senderWidget = dynamic_cast<QPushButton*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsReleased.isUndefined() == true)
		{
			m_jsReleased = evaluateScript(senderWidget, m_scriptReleased);

			if (m_jsReleased.isError() == true ||
				m_jsReleased.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(m_jsReleased, senderWidget);

		return;
	}

	void SchemaItemPushButton::toggled(bool /*checked*/)
	{
		if (m_scriptToggled.isEmpty() == true ||
			m_scriptToggled == PropertyNames::pushButtonDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QPushButton* senderWidget = dynamic_cast<QPushButton*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsToggled.isUndefined() == true)
		{
			m_jsToggled = evaluateScript(senderWidget, m_scriptToggled);

			if (m_jsToggled.isError() == true ||
				m_jsToggled.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript(m_jsToggled, senderWidget);

		return;
	}

	void SchemaItemPushButton::runEventScript(QJSValue& evaluatedJs, QPushButton* buttonWidget)
	{
		if (evaluatedJs.isError() == true ||
			evaluatedJs.isNull() == true)
		{
			return;
		}

		// Suppose that parent of sender is ClientSchemaView, if not, then this is EditMode?
		//
		ClientSchemaView* schemaView = dynamic_cast<ClientSchemaView*>(buttonWidget->parentWidget());
		if (schemaView == nullptr)
		{
			assert(schemaView);
			return;
		}

		QJSEngine* engine = schemaView->jsEngine();
		assert(engine);

		// Set argument list
		//
		QJSValue jsSchemaItem = engine->newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsWidget = engine->newQObject(buttonWidget);
		QQmlEngine::setObjectOwnership(buttonWidget, QQmlEngine::CppOwnership);

		QJSValueList args;

		args << jsSchemaItem;
		args << jsWidget;
		args << buttonWidget->isChecked();

		// Run script
		//
		QJSValue jsResult = evaluatedJs.call(args);

		if (jsResult.isError() == true)
		{
			reportSqriptError(jsResult, schemaView);
			return;
		}

		engine->collectGarbage();
		return;
	}

	// Properties and Data
	//

	const QString& SchemaItemPushButton::text() const
	{
		return m_text;
	}
	void SchemaItemPushButton::setText(QString value)
	{
		m_text = value;
	}

	bool SchemaItemPushButton::isCheckable() const
	{
		return m_checkable;
	}

	void SchemaItemPushButton::setCheckable(bool value)
	{
		m_checkable = value;
	}

	bool SchemaItemPushButton::checkedDefault() const
	{
		return m_checkedDefault;
	}

	void SchemaItemPushButton::setCheckedDefault(bool value)
	{
		m_checkedDefault = value;
	}

	bool SchemaItemPushButton::autoRepeat() const
	{
		return m_autoRepeat;
	}

	void SchemaItemPushButton::setAutoRepeat(bool value)
	{
		m_autoRepeat = value;
	}

	int SchemaItemPushButton::autoRepeatDelay() const
	{
		return m_autoRepeatDelay;
	}

	void SchemaItemPushButton::setAutoRepeatDelay(int value)
	{
		m_autoRepeatDelay = value;
	}

	int SchemaItemPushButton::autoRepeatInterval() const
	{
		return m_autoRepeatInterval;
	}

	void SchemaItemPushButton::setAutoRepeatInterval(int value)
	{
		m_autoRepeatInterval = value;
	}

	void SchemaItemPushButton::setStyleSheet(QString value)
	{
		if (value == PropertyNames::pushButtonDefaultStyleSheet)
		{
			SchemaItemControl::setStyleSheet(PropertyNames::pushButtonDefaultStyleSheet);
		}
		else
		{
			SchemaItemControl::setStyleSheet(value);
		}
	}

	QString SchemaItemPushButton::scriptAfterCreate() const
	{
		return m_scriptAfterCreate;
	}

	void SchemaItemPushButton::setScriptAfterCreate(const QString& value)
	{
		if (value == PropertyNames::pushButtonDefaultEventScript)
		{
			m_scriptAfterCreate = PropertyNames::pushButtonDefaultEventScript;
		}
		else
		{
			m_scriptAfterCreate = value;
		}
	}

	QString SchemaItemPushButton::scriptClicked() const
	{
		return m_scriptClicked;
	}

	void SchemaItemPushButton::setScriptClicked(const QString& value)
	{
		if (value == PropertyNames::pushButtonDefaultEventScript)
		{
			m_scriptClicked = PropertyNames::pushButtonDefaultEventScript;
		}
		else
		{
			m_scriptClicked = value;
		}
	}

	QString SchemaItemPushButton::scriptPressed() const
	{
		return m_scriptPressed;
	}

	void SchemaItemPushButton::setScriptPressed(const QString& value)
	{
		if (value == PropertyNames::pushButtonDefaultEventScript)
		{
			m_scriptPressed = PropertyNames::pushButtonDefaultEventScript;
		}
		else
		{
			m_scriptPressed = value;
		}
	}

	QString SchemaItemPushButton::scriptReleased() const
	{
		return m_scriptReleased;
	}

	void SchemaItemPushButton::setScriptReleased(const QString& value)
	{
		if (value == PropertyNames::pushButtonDefaultEventScript)
		{
			m_scriptReleased = PropertyNames::pushButtonDefaultEventScript;
		}
		else
		{
			m_scriptReleased = value;
		}
	}

	QString SchemaItemPushButton::scriptToggled() const
	{
		return m_scriptToggled;
	}

	void SchemaItemPushButton::setScriptToggled(const QString& value)
	{
		if (value == PropertyNames::pushButtonDefaultEventScript)
		{
			m_scriptToggled = PropertyNames::pushButtonDefaultEventScript;
		}
		else
		{
			m_scriptToggled = value;
		}
	}
}

