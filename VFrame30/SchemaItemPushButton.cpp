#include "Stable.h"
#include "SchemaItemPushButton.h"
#include "MacrosExpander.h"
#include <QPushButton>

namespace VFrame30
{
	SchemaItemPushButton::SchemaItemPushButton(void) :
		SchemaItemPushButton(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemPushButton::SchemaItemPushButton(SchemaUnit unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::controlCategory, true, SchemaItemPushButton::text, SchemaItemPushButton::setText);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::checkable, PropertyNames::controlCategory, true, SchemaItemPushButton::isCheckable, SchemaItemPushButton::setCheckable);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::checkedDefault, PropertyNames::controlCategory, true, SchemaItemPushButton::checkedDefault, SchemaItemPushButton::setCheckedDefault);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::autoRepeat, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeat, SchemaItemPushButton::setAutoRepeat);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::autoRepeatDelay, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeatDelay, SchemaItemPushButton::setAutoRepeatDelay);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::autoRepeatInterval, PropertyNames::controlCategory, true, SchemaItemPushButton::autoRepeatInterval, SchemaItemPushButton::setAutoRepeatInterval);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::styleSheet, PropertyNames::controlCategory, true, SchemaItemPushButton::styleSheet, SchemaItemPushButton::setStyleSheet);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::afterCreate, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptAfterCreate, SchemaItemPushButton::setScriptAfterCreate);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::clicked, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptClicked, SchemaItemPushButton::setScriptClicked);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::pressed, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptPressed, SchemaItemPushButton::setScriptPressed);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::released, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptReleased, SchemaItemPushButton::setScriptReleased);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::toggled, PropertyNames::scriptsCategory, true, SchemaItemPushButton::scriptToggled, SchemaItemPushButton::setScriptToggled);

		// --
		//
		m_static = false;
		setItemUnit(unit);
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

		pushButtonMessage->set_stylesheet(m_styleSheet.toStdString());

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

		setStyleSheet(QString::fromStdString(pushButtonMessagemessage.stylesheet()));				// Text setters can have some string optimization for default values

		setScriptAfterCreate(QString::fromStdString(pushButtonMessagemessage.scriptaftercreate()));	// Text setters can have some string optimization for default values
		setScriptClicked(QString::fromStdString(pushButtonMessagemessage.scriptclicked()));			// Text setters can have some string optimization for default values
		setScriptPressed(QString::fromStdString(pushButtonMessagemessage.scriptpressed()));			// Text setters can have some string optimization for default values
		setScriptReleased(QString::fromStdString(pushButtonMessagemessage.scriptreleased()));		// Text setters can have some string optimization for default values
		setScriptToggled(QString::fromStdString(pushButtonMessagemessage.scripttoggled()));			// Text setters can have some string optimization for default values

		return true;
	}

	QWidget* SchemaItemPushButton::createWidget(QWidget* parent, double zoom) const
	{
		if (parent == nullptr)
		{
			assert(parent);
			return nullptr;
		}

		QPushButton* control = new QPushButton(m_text, parent);
		control->setObjectName(guid().toString());

		updateWidgetProperties(control, zoom);

		control->setChecked(checkedDefault());

		connect(control, &QPushButton::clicked, this, &SchemaItemPushButton::clicked);
		connect(control, &QPushButton::pressed, this, &SchemaItemPushButton::pressed);
		connect(control, &QPushButton::released, this, &SchemaItemPushButton::released);
		connect(control, &QPushButton::toggled, this, &SchemaItemPushButton::toggled);

		control->setVisible(true);

		return control;
	}

	// Update widget properties
	//
	void SchemaItemPushButton::updateWidgetProperties(QWidget* widget, double zoom) const
	{
		QPushButton* control = dynamic_cast<QPushButton*>(widget);

		if (control == nullptr)
		{
			assert(control);
			return;
		}

		control->setUpdatesEnabled(false);

		SchemaItemControl::updateWidgetProperties(widget, zoom);

		control->setText(text());
		control->setCheckable(isCheckable());
		control->setAutoRepeat(autoRepeat());
		control->setAutoRepeatDelay(autoRepeatDelay());
		control->setAutoRepeatInterval(autoRepeatInterval());

		QPoint pos = {static_cast<int>(leftDocPt() * zoom / 100.0),
					  static_cast<int>(topDocPt() * zoom / 100.0)};

		if (control->pos() != pos)
		{
			 control->move(pos);
		}

		QSize size = {static_cast<int>(widthDocPt() * zoom / 100.0),
					  static_cast<int>(heightDocPt() * zoom / 100.0)};

		if (control->size() != size)
		{
			control->resize(size);
		}

		if (control->styleSheet() != styleSheet())
		{
			control->setStyleSheet(styleSheet());
		}

		control->setUpdatesEnabled(true);

		return;
	}

	void SchemaItemPushButton::clicked(bool /*checked*/)
	{
		qDebug() << Q_FUNC_INFO;

		// Suppose Default script does nothing, just return
		//
	}

	void SchemaItemPushButton::pressed()
	{
		qDebug() << Q_FUNC_INFO;
	}

	void SchemaItemPushButton::released()
	{
		qDebug() << Q_FUNC_INFO;
	}

	void SchemaItemPushButton::toggled(bool /*checked*/)
	{
		qDebug() << Q_FUNC_INFO;
	}

//	bool SchemaItemPushButton::searchText(const QString& text) const
//	{
//		return SchemaItem::searchText(text) ||
//				m_text.contains(text, Qt::CaseInsensitive);
//	}

	// Properties and Data
	//

	// Text property
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

	const QString& SchemaItemPushButton::styleSheet() const
	{
		return m_styleSheet;
	}

	void SchemaItemPushButton::setStyleSheet(QString value)
	{
		if (value == PropertyNames::defaultPushButtonStyleSheet)
		{
			m_styleSheet = PropertyNames::defaultPushButtonStyleSheet;
		}
		else
		{
			m_styleSheet = value;
		}
	}

	QString SchemaItemPushButton::scriptAfterCreate() const
	{
		return m_scriptAfterCreate;
	}

	void SchemaItemPushButton::setScriptAfterCreate(const QString& value)
	{
		if (value == PropertyNames::defaultPushButtonEventScript)
		{
			m_scriptAfterCreate = PropertyNames::defaultPushButtonEventScript;
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
		if (value == PropertyNames::defaultPushButtonEventScript)
		{
			m_scriptClicked = PropertyNames::defaultPushButtonEventScript;
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
		if (value == PropertyNames::defaultPushButtonEventScript)
		{
			m_scriptPressed = PropertyNames::defaultPushButtonEventScript;
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
		if (value == PropertyNames::defaultPushButtonEventScript)
		{
			m_scriptReleased = PropertyNames::defaultPushButtonEventScript;
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
		if (value == PropertyNames::defaultPushButtonEventScript)
		{
			m_scriptToggled = PropertyNames::defaultPushButtonEventScript;
		}
		else
		{
			m_scriptToggled = value;
		}
	}
}

