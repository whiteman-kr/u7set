#include "SchemaItemLineEdit.h"
#include "SchemaView.h"
#include <QLineEdit>

namespace VFrame30
{
	SchemaItemLineEdit::SchemaItemLineEdit(void) :
		SchemaItemLineEdit(SchemaUnit::Inch)
	{
		// This contructor can be call in case of loading this object
		//
	}

	SchemaItemLineEdit::SchemaItemLineEdit(SchemaUnit unit) :
		SchemaItemControl(unit)
	{
		setStyleSheet(PropertyNames::lineEditDefaultStyleSheet);

		Property* p = nullptr;

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::controlCategory, true, SchemaItemLineEdit::text, SchemaItemLineEdit::setText);
		p->setDescription(PropertyNames::lineEditPropText);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::placeholderText, PropertyNames::controlCategory, true, SchemaItemLineEdit::placeholderText, SchemaItemLineEdit::setPlaceholderText);
		p->setDescription(PropertyNames::lineEditPropPlaceholderText);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::appearanceCategory, true, SchemaItemLineEdit::horzAlign, SchemaItemLineEdit::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::appearanceCategory, true, SchemaItemLineEdit::vertAlign, SchemaItemLineEdit::setVertAlign);

		p = ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::maxLength, PropertyNames::controlCategory, true, SchemaItemLineEdit::maxLength, SchemaItemLineEdit::setMaxLength);
		p->setDescription(PropertyNames::lineEditPropMaxLength);

		p = ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::readOnly, PropertyNames::controlCategory, true, SchemaItemLineEdit::readOnly, SchemaItemLineEdit::setReadOnly);
		p->setDescription(PropertyNames::lineEditPropReadOnly);

//		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::afterCreate, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptAfterCreate, SchemaItemLineEdit::setScriptAfterCreate);
//		p->setDescription(PropertyNames::widgetPropAfterCreate);
//		p->setIsScript(true);

//		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::clicked, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptClicked, SchemaItemLineEdit::setScriptClicked);
//		p->setDescription(PropertyNames::LineEditPropClicked);
//		p->setIsScript(true);

//		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::pressed, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptPressed, SchemaItemLineEdit::setScriptPressed);
//		p->setDescription(PropertyNames::LineEditPropPressed);
//		p->setIsScript(true);

//		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::released, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptReleased, SchemaItemLineEdit::setScriptReleased);
//		p->setDescription(PropertyNames::LineEditPropReleased);
//		p->setIsScript(true);

//		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::toggled, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptToggled, SchemaItemLineEdit::setScriptToggled);
//		p->setDescription(PropertyNames::LineEditPropToggled);
//		p->setIsScript(true);

		return;
	}

	SchemaItemLineEdit::~SchemaItemLineEdit(void)
	{
	}

	// Serialization
	//
	bool SchemaItemLineEdit::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemLineEdit* lineEditMessage = message->mutable_schemaitem()->mutable_lineedit();

		lineEditMessage->set_text(m_text.toStdString());
		lineEditMessage->set_placeholdertext(m_placeholderText.toStdString());

		lineEditMessage->set_maxlength(m_maxLength);

		lineEditMessage->set_horzalign(m_horzAlign);
		lineEditMessage->set_vertalign(m_vertAlign);

		lineEditMessage->set_readonly(m_readOnly);

//		LineEditMessage->set_scriptaftercreate(m_scriptAfterCreate.toStdString());
//		LineEditMessage->set_scriptclicked(m_scriptClicked.toStdString());
//		LineEditMessage->set_scriptpressed(m_scriptPressed.toStdString());
//		LineEditMessage->set_scriptreleased(m_scriptReleased.toStdString());
//		LineEditMessage->set_scripttoggled(m_scriptToggled.toStdString());

		return true;
	}

	bool SchemaItemLineEdit::LoadData(const Proto::Envelope& message)
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

		const Proto::SchemaItemLineEdit& lineEditMessagemessage = message.schemaitem().lineedit();

		setText(QString::fromStdString(lineEditMessagemessage.text()));							// Text setters can have some string optimization for default values
		setPlaceholderText(QString::fromStdString(lineEditMessagemessage.placeholdertext()));

		m_maxLength = lineEditMessagemessage.maxlength();

		m_horzAlign = static_cast<E::HorzAlign>(lineEditMessagemessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(lineEditMessagemessage.vertalign());

		m_readOnly = lineEditMessagemessage.readonly();

//		setScriptAfterCreate(QString::fromStdString(LineEditMessagemessage.scriptaftercreate()));	// Text setters can have some string optimization for default values
//		setScriptClicked(QString::fromStdString(LineEditMessagemessage.scriptclicked()));			// Text setters can have some string optimization for default values
//		setScriptPressed(QString::fromStdString(LineEditMessagemessage.scriptpressed()));			// Text setters can have some string optimization for default values
//		setScriptReleased(QString::fromStdString(LineEditMessagemessage.scriptreleased()));		// Text setters can have some string optimization for default values
//		setScriptToggled(QString::fromStdString(LineEditMessagemessage.scripttoggled()));			// Text setters can have some string optimization for default values

		return true;
	}

	QWidget* SchemaItemLineEdit::createWidget(QWidget* parent, bool editMode) const
	{
		if (parent == nullptr)
		{
			assert(parent);
			return nullptr;
		}

		QLineEdit* control = new QLineEdit(m_text, parent);
		control->setObjectName(guid().toString());

		updateWidgetProperties(control);

		control->setText(text());

		if (editMode == false)
		{
//			afterCreate(control);

//			// Connect slots only if it has any sense
//			//
//			if (scriptClicked().isEmpty() == false &&
//				scriptClicked() != PropertyNames::LineEditDefaultEventScript)
//			{
//				connect(control, &QLineEdit::clicked, this, &SchemaItemLineEdit::clicked);
//			}

//			if (scriptPressed().isEmpty() == false &&
//				scriptPressed() != PropertyNames::LineEditDefaultEventScript)
//			{
//				connect(control, &QLineEdit::pressed, this, &SchemaItemLineEdit::pressed);
//			}

//			if (scriptReleased().isEmpty() == false &&
//				scriptReleased() != PropertyNames::LineEditDefaultEventScript)
//			{
//				connect(control, &QLineEdit::released, this, &SchemaItemLineEdit::released);
//			}

//			if (scriptToggled().isEmpty() == false &&
//				scriptToggled() != PropertyNames::LineEditDefaultEventScript)
//			{
//				connect(control, &QLineEdit::toggled, this, &SchemaItemLineEdit::toggled);
//			}
		}

		control->setVisible(true);
		control->update();

		return control;
	}

	// Update widget properties
	//
	void SchemaItemLineEdit::updateWidgetProperties(QWidget* widget) const
	{
		QLineEdit* control = dynamic_cast<QLineEdit*>(widget);

		if (control == nullptr)
		{
			assert(control);
			return;
		}

		SchemaItemControl::updateWidgetProperties(widget);

		bool updateRequired = false;

		if (control->text() != text() ||
			control->placeholderText() != placeholderText() ||
			static_cast<int>(control->alignment()) != (m_horzAlign | m_vertAlign) ||
			control->maxLength() != maxLength() ||
			control->isReadOnly() != readOnly())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			control->setUpdatesEnabled(false);

			control->setText(text());
			control->setPlaceholderText(placeholderText());
			control->setAlignment(static_cast<Qt::Alignment>(m_horzAlign | m_vertAlign));
			control->setMaxLength(maxLength());
			control->setReadOnly(readOnly());

			control->setUpdatesEnabled(true);
		}

		return;
	}


	void SchemaItemLineEdit::afterCreate(QLineEdit* control) const
	{
		if (control == nullptr)
		{
			assert(control);
			return;
		}

		if (m_scriptAfterCreate == PropertyNames::lineEditDefaultEventScript)	// Suppose Default script does nothing, just return
		{
			return;
		}

		// Shit happens
		//
		const_cast<SchemaItemLineEdit*>(this)->runEventScript(m_scriptAfterCreate, control);

		return;
	}

//	void SchemaItemLineEdit::clicked(bool)
//	{
//		if (m_scriptClicked.isEmpty() == true ||
//			m_scriptClicked == PropertyNames::LineEditDefaultEventScript)		// Suppose Default script does nothing, just return
//		{
//			return;
//		}

//		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
//		if (senderWidget == nullptr)
//		{
//			assert(senderWidget);
//			return;
//		}

//		runEventScript(m_scriptClicked, senderWidget);

//		return;
//	}

//	void SchemaItemLineEdit::pressed()
//	{
//		if (m_scriptPressed.isEmpty() == true ||
//			m_scriptPressed == PropertyNames::LineEditDefaultEventScript)		// Suppose Default script does nothing, just return
//		{
//			return;
//		}

//		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
//		if (senderWidget == nullptr)
//		{
//			assert(senderWidget);
//			return;
//		}

//		runEventScript(m_scriptPressed, senderWidget);

//		return;
//	}

//	void SchemaItemLineEdit::released()
//	{
//		if (m_scriptReleased.isEmpty() == true ||
//			m_scriptReleased == PropertyNames::LineEditDefaultEventScript)		// Suppose Default script does nothing, just return
//		{
//			return;
//		}

//		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
//		if (senderWidget == nullptr)
//		{
//			assert(senderWidget);
//			return;
//		}

//		runEventScript(m_scriptReleased, senderWidget);

//		return;
//	}

//	void SchemaItemLineEdit::toggled(bool /*checked*/)
//	{
//		if (m_scriptToggled.isEmpty() == true ||
//			m_scriptToggled == PropertyNames::LineEditDefaultEventScript)		// Suppose Default script does nothing, just return
//		{
//			return;
//		}

//		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
//		if (senderWidget == nullptr)
//		{
//			assert(senderWidget);
//			return;
//		}

//		runEventScript(m_scriptToggled, senderWidget);

//		return;
//	}

	void SchemaItemLineEdit::runEventScript(const QString& script, QLineEdit* widget)
	{
		if (script.trimmed().isEmpty() == true)
		{
			return;
		}

		// Suppose that parent of sender is SchemaView
		//
		QWidget* parentWidget = widget->parentWidget();
		if (parentWidget == nullptr)
		{
			assert(parentWidget);
			return;
		}

		SchemaView* schemaView = dynamic_cast<SchemaView*>(parentWidget);
		if (schemaView == nullptr)
		{
			assert(schemaView);
			return;
		}

		QJSValue jsEval = m_jsEngine.evaluate(script);
		if (jsEval.isError() == true)
		{
			QMessageBox::critical(schemaView, qAppName(), "Script evaluating error: " + jsEval.toString());
			return;
		}

		// Create JS params
		//
		QJSValue jsSchemaItem = m_jsEngine.newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsSchemaView = m_jsEngine.newQObject(schemaView);
		QQmlEngine::setObjectOwnership(schemaView, QQmlEngine::CppOwnership);

		QJSValue jsWidget = m_jsEngine.newQObject(widget);
		QQmlEngine::setObjectOwnership(widget, QQmlEngine::CppOwnership);

		//QJSValue jsChecked = {buttonWidget->isChecked()};

		// Set argument list
		//
		QJSValueList args;

		args << jsSchemaItem;
		args << jsSchemaView;
		args << jsWidget;
		args << widget->text();

		// Run script
		//
		QJSValue jsResult = jsEval.call(args);
		if (jsResult.isError() == true)
		{
			qDebug() << "Uncaught exception at line"
					 << jsResult.property("lineNumber").toInt()
					 << ":" << jsResult.toString();

			QMessageBox::critical(schemaView, qAppName(), "Script uncaught exception: " + jsEval.toString());
			return;
		}

		qDebug() << "runScript result:" <<  jsResult.toString();

		m_jsEngine.collectGarbage();
	}

	bool SchemaItemLineEdit::searchText(const QString& text) const
	{
		return	SchemaItem::searchText(text) ||
				m_text.contains(text, Qt::CaseInsensitive/*) ||
				m_scriptAfterCreate.contains(text, Qt::CaseInsensitive) ||
				m_scriptClicked.contains(text, Qt::CaseInsensitive) ||
				m_scriptPressed.contains(text, Qt::CaseInsensitive) ||
				m_scriptReleased.contains(text, Qt::CaseInsensitive) ||
				m_scriptToggled.contains(text, Qt::CaseInsensitive*/);
	}

	// Properties and Data
	//

	const QString& SchemaItemLineEdit::text() const
	{
		return m_text;
	}
	void SchemaItemLineEdit::setText(QString value)
	{
		m_text = value;
	}

	const QString& SchemaItemLineEdit::placeholderText() const
	{
		return m_placeholderText;
	}

	void SchemaItemLineEdit::setPlaceholderText(QString value)
	{
		m_placeholderText = value;
	}

	int SchemaItemLineEdit::maxLength() const
	{
		return m_maxLength;
	}

	void SchemaItemLineEdit::setMaxLength(int value)
	{
		m_maxLength = value;
	}

	E::HorzAlign SchemaItemLineEdit::horzAlign() const
	{
		return m_horzAlign;
	}

	void SchemaItemLineEdit::setHorzAlign(E::HorzAlign value)
	{
		m_horzAlign = value;
	}

	E::VertAlign SchemaItemLineEdit::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemLineEdit::setVertAlign(E::VertAlign value)
	{
		m_vertAlign = value;
	}

	bool SchemaItemLineEdit::readOnly() const
	{
		return m_readOnly;
	}

	void SchemaItemLineEdit::setReadOnly(bool value)
	{
		m_readOnly= value;
	}

	void SchemaItemLineEdit::setStyleSheet(QString value)
	{
		if (value == PropertyNames::lineEditDefaultStyleSheet)
		{
			SchemaItemControl::setStyleSheet(PropertyNames::lineEditDefaultStyleSheet);
		}
		else
		{
			SchemaItemControl::setStyleSheet(value);
		}
	}

	QString SchemaItemLineEdit::scriptAfterCreate() const
	{
		return m_scriptAfterCreate;
	}

	void SchemaItemLineEdit::setScriptAfterCreate(const QString& value)
	{
		if (value == PropertyNames::lineEditDefaultEventScript)
		{
			m_scriptAfterCreate = PropertyNames::lineEditDefaultEventScript;
		}
		else
		{
			m_scriptAfterCreate = value;
		}
	}

//	QString SchemaItemLineEdit::scriptClicked() const
//	{
//		return m_scriptClicked;
//	}

//	void SchemaItemLineEdit::setScriptClicked(const QString& value)
//	{
//		if (value == PropertyNames::LineEditDefaultEventScript)
//		{
//			m_scriptClicked = PropertyNames::LineEditDefaultEventScript;
//		}
//		else
//		{
//			m_scriptClicked = value;
//		}
//	}

//	QString SchemaItemLineEdit::scriptPressed() const
//	{
//		return m_scriptPressed;
//	}

//	void SchemaItemLineEdit::setScriptPressed(const QString& value)
//	{
//		if (value == PropertyNames::LineEditDefaultEventScript)
//		{
//			m_scriptPressed = PropertyNames::LineEditDefaultEventScript;
//		}
//		else
//		{
//			m_scriptPressed = value;
//		}
//	}

//	QString SchemaItemLineEdit::scriptReleased() const
//	{
//		return m_scriptReleased;
//	}

//	void SchemaItemLineEdit::setScriptReleased(const QString& value)
//	{
//		if (value == PropertyNames::LineEditDefaultEventScript)
//		{
//			m_scriptReleased = PropertyNames::LineEditDefaultEventScript;
//		}
//		else
//		{
//			m_scriptReleased = value;
//		}
//	}

//	QString SchemaItemLineEdit::scriptToggled() const
//	{
//		return m_scriptToggled;
//	}

//	void SchemaItemLineEdit::setScriptToggled(const QString& value)
//	{
//		if (value == PropertyNames::LineEditDefaultEventScript)
//		{
//			m_scriptToggled = PropertyNames::LineEditDefaultEventScript;
//		}
//		else
//		{
//			m_scriptToggled = value;
//		}
//	}
}

