#include "SchemaItemLineEdit.h"
#include "SchemaView.h"
#include "../lib/Tuning/TuningController.h"
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

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::afterCreate, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptAfterCreate, SchemaItemLineEdit::setScriptAfterCreate);
		p->setDescription(PropertyNames::widgetPropAfterCreate);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::editingFinished, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptEditingFinished, SchemaItemLineEdit::setScriptEditingFinished);
		p->setDescription(PropertyNames::lineEditPropEditingFinished);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::returnPressed, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptReturnPressed, SchemaItemLineEdit::setScriptReturnPressed);
		p->setDescription(PropertyNames::lineEditPropReturnPressed);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::textChanged, PropertyNames::scriptsCategory, true, SchemaItemLineEdit::scriptTextChanged, SchemaItemLineEdit::setScriptTextChanged);
		p->setDescription(PropertyNames::lineEditPropTextChanged);
		p->setIsScript(true);

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

		lineEditMessage->set_scriptaftercreate(m_scriptAfterCreate.toStdString());
		lineEditMessage->set_scripteditingfinished(m_scriptEditingFinished.toStdString());
		lineEditMessage->set_scriptreturnpressed(m_scriptReturnPressed.toStdString());
		lineEditMessage->set_scripttextchanged(m_scriptTextChanged.toStdString());

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

		setScriptAfterCreate(QString::fromStdString(lineEditMessagemessage.scriptaftercreate()));			// Text setters can have some string optimization for default values
		setScriptEditingFinished(QString::fromStdString(lineEditMessagemessage.scripteditingfinished()));	// Text setters can have some string optimization for default values
		setScriptReturnPressed(QString::fromStdString(lineEditMessagemessage.scriptreturnpressed()));		// Text setters can have some string optimization for default values
		setScriptTextChanged(QString::fromStdString(lineEditMessagemessage.scripttextchanged()));			// Text setters can have some string optimization for default values

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
			afterCreate(control);

			// Connect slots only if it has any sense
			//
			if (scriptEditingFinished().isEmpty() == false &&
				scriptEditingFinished() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::editingFinished, this, &SchemaItemLineEdit::editingFinished);
			}

			if (scriptReturnPressed().isEmpty() == false &&
				scriptReturnPressed() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::returnPressed, this, &SchemaItemLineEdit::returnPressed);
			}

			if (scriptTextChanged().isEmpty() == false &&
				scriptTextChanged() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::textChanged, this, &SchemaItemLineEdit::textChanged);
			}
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

	void SchemaItemLineEdit::editingFinished()
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptEditingFinished.isEmpty() == true ||
			m_scriptEditingFinished == PropertyNames::lineEditDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		runEventScript(m_scriptEditingFinished, senderWidget);

		return;
	}

	void SchemaItemLineEdit::returnPressed()
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptReturnPressed.isEmpty() == true ||
			m_scriptReturnPressed == PropertyNames::lineEditDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		runEventScript(m_scriptReturnPressed, senderWidget);

		return;
	}

	void SchemaItemLineEdit::textChanged(const QString& /*text*/)
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptTextChanged.isEmpty() == true ||
			m_scriptTextChanged== PropertyNames::lineEditDefaultEventScript)		// Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		runEventScript(m_scriptTextChanged, senderWidget);

		return;
	}

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

		QJSEngine* engine = schemaView->jsEngine();
		assert(engine);

		QJSValue jsEval = engine->evaluate(script);
		if (jsEval.isError() == true)
		{
			QMessageBox::critical(schemaView, qAppName(), "Script evaluating error: " + jsEval.toString());
			return;
		}

		// Create JS params
		//
		QJSValue jsSchemaItem = engine->newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsWidget = engine->newQObject(widget);
		QQmlEngine::setObjectOwnership(widget, QQmlEngine::CppOwnership);

		QJSValue jsWidgetText = {widget->text()};

		// Set argument list
		//
		QJSValueList args;

		args << jsSchemaItem;
		args << jsWidget;
		args << jsWidgetText;

		// Global Objects
		//
		QJSValue jsSchemaView = engine->newQObject(schemaView);
		QQmlEngine::setObjectOwnership(schemaView, QQmlEngine::CppOwnership);
		engine->globalObject().setProperty(PropertyNames::scriptGlobalVariableView, jsSchemaView);

		TuningController* tuningController = &schemaView->tuningController();

		if (tuningController != nullptr)
		{
			assert(tuningController);
			engine->globalObject().setProperty(PropertyNames::scriptGlobalVariableTuning, QJSValue());
		}
		else
		{
			QJSValue jsTuning = engine->newQObject(tuningController);
			QQmlEngine::setObjectOwnership(tuningController, QQmlEngine::CppOwnership);
			engine->globalObject().setProperty(PropertyNames::scriptGlobalVariableTuning, jsTuning);
		}

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

		engine->collectGarbage();

		return;
	}

	bool SchemaItemLineEdit::searchText(const QString& text) const
	{
		return	SchemaItem::searchText(text) ||
				m_text.contains(text, Qt::CaseInsensitive) ||
				m_scriptAfterCreate.contains(text, Qt::CaseInsensitive) ||
				m_scriptEditingFinished.contains(text, Qt::CaseInsensitive) ||
				m_scriptReturnPressed.contains(text, Qt::CaseInsensitive) ||
				m_scriptTextChanged.contains(text, Qt::CaseInsensitive);
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

	QString SchemaItemLineEdit::scriptEditingFinished() const
	{
		return m_scriptEditingFinished;
	}

	void SchemaItemLineEdit::setScriptEditingFinished(const QString& value)
	{
		if (value == PropertyNames::lineEditDefaultEventScript)
		{
			m_scriptEditingFinished = PropertyNames::lineEditDefaultEventScript;
		}
		else
		{
			m_scriptEditingFinished = value;
		}
	}

	QString SchemaItemLineEdit::scriptReturnPressed() const
	{
		return m_scriptReturnPressed;
	}

	void SchemaItemLineEdit::setScriptReturnPressed(const QString& value)
	{
		if (value == PropertyNames::lineEditDefaultEventScript)
		{
			m_scriptReturnPressed = PropertyNames::lineEditDefaultEventScript;
		}
		else
		{
			m_scriptReturnPressed = value;
		}
	}

	QString SchemaItemLineEdit::scriptTextChanged() const
	{
		return m_scriptTextChanged;
	}

	void SchemaItemLineEdit::setScriptTextChanged(const QString& value)
	{
		if (value == PropertyNames::lineEditDefaultEventScript)
		{
			m_scriptTextChanged = PropertyNames::lineEditDefaultEventScript;
		}
		else
		{
			m_scriptTextChanged = value;
		}
	}

}

