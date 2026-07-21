#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/TuningController.h>

#include <QStyle>

namespace
{
	// Subclass control to filter mouse input events for schema editor.
	//
	class QEditorLineEdit : public QLineEdit
	{
	public:
		QEditorLineEdit(QString text, QWidget* parent, bool editMode) :
			QLineEdit{text, parent},
			m_editMode(editMode)
		{
			// Focus does not appear in the Monitor initial Tab key is pressed, I did not dig deep enough to understand
			// the nature of this behavior. Just forbad focus.
			//
			if (m_editMode == true)
			{
				setMouseTracking(false);
				setAttribute(Qt::WA_TransparentForMouseEvents);
			}
		}

	public:
		bool m_editMode = true;
	};
} // namespace


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

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::text,
									 PropertyNames::controlCategory,
									 true,
									 SchemaItemLineEdit::text,
									 SchemaItemLineEdit::setText);
		p->setDescription(PropertyNames::lineEditPropText);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::placeholderText,
									 PropertyNames::controlCategory,
									 true,
									 SchemaItemLineEdit::placeholderText,
									 SchemaItemLineEdit::setPlaceholderText);
		p->setDescription(PropertyNames::lineEditPropPlaceholderText);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign,
								 PropertyNames::alignHorz,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemLineEdit::horzAlign,
								 SchemaItemLineEdit::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign,
								 PropertyNames::alignVert,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemLineEdit::vertAlign,
								 SchemaItemLineEdit::setVertAlign);

		p = ADD_PROPERTY_GET_SET_CAT(int,
									 PropertyNames::maxLength,
									 PropertyNames::controlCategory,
									 true,
									 SchemaItemLineEdit::maxLength,
									 SchemaItemLineEdit::setMaxLength);
		p->setDescription(PropertyNames::lineEditPropMaxLength);

		p = ADD_PROPERTY_GET_SET_CAT(bool,
									 PropertyNames::readOnly,
									 PropertyNames::controlCategory,
									 true,
									 SchemaItemLineEdit::readOnly,
									 SchemaItemLineEdit::setReadOnly);
		p->setDescription(PropertyNames::lineEditPropReadOnly);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::afterCreate,
									 PropertyNames::scriptsCategory,
									 true,
									 SchemaItemLineEdit::scriptAfterCreate,
									 SchemaItemLineEdit::setScriptAfterCreate);
		p->setDescription(PropertyNames::widgetPropAfterCreate);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::editingFinished,
									 PropertyNames::scriptsCategory,
									 true,
									 SchemaItemLineEdit::scriptEditingFinished,
									 SchemaItemLineEdit::setScriptEditingFinished);
		p->setDescription(PropertyNames::lineEditPropEditingFinished);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::returnPressed,
									 PropertyNames::scriptsCategory,
									 true,
									 SchemaItemLineEdit::scriptReturnPressed,
									 SchemaItemLineEdit::setScriptReturnPressed);
		p->setDescription(PropertyNames::lineEditPropReturnPressed);
		p->setIsScript(true);

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::textChanged,
									 PropertyNames::scriptsCategory,
									 true,
									 SchemaItemLineEdit::scriptTextChanged,
									 SchemaItemLineEdit::setScriptTextChanged);
		p->setDescription(PropertyNames::lineEditPropTextChanged);
		p->setIsScript(true);

		return;
	}

	SchemaItemLineEdit::~SchemaItemLineEdit(void) {}

	void SchemaItemLineEdit::draw(CDrawParam* drawParam) const
	{
		// Control is drawn only in PDF mode
		//
		if (drawParam->pdfMode() == true)
		{
			drawLineEditControl(drawParam, parentSchema(), parentLayer().get());
		}

		return;
	}

	// Serialization
	//
	bool SchemaItemLineEdit::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemControl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		auto schemaItemMessage = message->MutableExtension(Proto::schemaitem);

		assert(schemaItemMessage->has_posrectimpl());
		assert(schemaItemMessage->has_control());

		// --
		//
		Proto::SchemaItemLineEdit* lineEditMessage = schemaItemMessage->mutable_lineedit();

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
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		const auto& schemaItemMessage = message.GetExtension(Proto::schemaitem);

		bool result = SchemaItemControl::LoadData(message);
		if (result == false || schemaItemMessage.has_control() == false)
		{
			assert(schemaItemMessage.has_control());
			return false;
		}

		const Proto::SchemaItemLineEdit& lineEditMessage = schemaItemMessage.lineedit();

		setText(QString::fromStdString(lineEditMessage.text())); // Text setters can have some string optimization for default values
		setPlaceholderText(QString::fromStdString(lineEditMessage.placeholdertext()));

		m_maxLength = lineEditMessage.maxlength();

		m_horzAlign = static_cast<E::HorzAlign>(lineEditMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(lineEditMessage.vertalign());

		m_readOnly = lineEditMessage.readonly();

		setScriptAfterCreate(QString::fromStdString(
			lineEditMessage.scriptaftercreate()));     // Text setters can have some string optimization for default values
		setScriptEditingFinished(QString::fromStdString(
			lineEditMessage.scripteditingfinished())); // Text setters can have some string optimization for default values
		setScriptReturnPressed(QString::fromStdString(
			lineEditMessage.scriptreturnpressed()));   // Text setters can have some string optimization for default values
		setScriptTextChanged(QString::fromStdString(
			lineEditMessage.scripttextchanged()));     // Text setters can have some string optimization for default values

		return true;
	}

	QWidget* SchemaItemLineEdit::createWidgetImpl(QWidget* parent, bool editMode, double zoom)
	{
		if (parent == nullptr)
		{
			assert(parent);
			return nullptr;
		}

		QLineEdit* control = new QEditorLineEdit(m_text, parent, editMode);
		control->setObjectName(guid().toString());

		updateWidgetProperties(control, editMode);

		control->setText(text());

		if (editMode == false)
		{
			// Connect slots only if it has any sense
			//
			if (scriptEditingFinished().isEmpty() == false && scriptEditingFinished() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::editingFinished, this, &SchemaItemLineEdit::editingFinished);
			}

			if (scriptReturnPressed().isEmpty() == false && scriptReturnPressed() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::returnPressed, this, &SchemaItemLineEdit::returnPressed);
			}

			if (scriptTextChanged().isEmpty() == false && scriptTextChanged() != PropertyNames::lineEditDefaultEventScript)
			{
				connect(control, &QLineEdit::textChanged, this, &SchemaItemLineEdit::textChanged);
			}
		}

		updateWidgetPosAndSize(control, zoom);

		control->setVisible(true);
		control->update();

		return control;
	}

	// Update widget properties
	//
	void SchemaItemLineEdit::updateWidgetProperties(QWidget* widget, bool editMode) const
	{
		QLineEdit* control = dynamic_cast<QLineEdit*>(widget);

		if (control == nullptr)
		{
			assert(control);
			return;
		}

		SchemaItemControl::updateWidgetProperties(widget, editMode);

		bool updateRequired = false;

		if (control->text() != text() || control->placeholderText() != placeholderText() ||
			static_cast<int>(control->alignment()) != (static_cast<int>(m_horzAlign) | static_cast<int>(m_vertAlign)) ||
			control->maxLength() != maxLength() || control->isReadOnly() != readOnly())
		{
			updateRequired = true;
		}

		if (updateRequired == true)
		{
			control->setUpdatesEnabled(false);

			control->setText(text());
			control->setPlaceholderText(placeholderText());
			control->setAlignment(static_cast<Qt::Alignment>(static_cast<int>(m_horzAlign) | static_cast<int>(m_vertAlign)));
			control->setMaxLength(maxLength());
			control->setReadOnly(readOnly());

			control->setUpdatesEnabled(true);
		}

		return;
	}

	void SchemaItemLineEdit::afterCreateImpl(QWidget* control)
	{
		QLineEdit* lineEditWidget = dynamic_cast<QLineEdit*>(control);

		if (lineEditWidget == nullptr)
		{
			assert(lineEditWidget);
			return;
		}

		if (m_scriptAfterCreate.trimmed().isEmpty() == true ||
			m_scriptAfterCreate == PropertyNames::lineEditDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		// Evaluate script
		//
		if (m_jsAfterCreate.isUndefined() == true)
		{
			m_jsAfterCreate = evaluateScript("AfterCreate", lineEditWidget, m_scriptAfterCreate);

			if (m_jsAfterCreate.isError() == true || m_jsAfterCreate.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript("AfterCreate", m_jsAfterCreate, lineEditWidget, false);

		return;
	}

	void SchemaItemLineEdit::editingFinished()
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptEditingFinished.isEmpty() == true ||
			m_scriptEditingFinished == PropertyNames::lineEditDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsEditingFinished.isUndefined() == true)
		{
			m_jsEditingFinished = evaluateScript("EditingFinished", senderWidget, m_scriptEditingFinished);

			if (m_jsEditingFinished.isError() == true || m_jsEditingFinished.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript("EditingFinished", m_jsEditingFinished, senderWidget, true);

		return;
	}

	void SchemaItemLineEdit::returnPressed()
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptReturnPressed.isEmpty() == true ||
			m_scriptReturnPressed == PropertyNames::lineEditDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsReturnPressed.isUndefined() == true)
		{
			m_jsReturnPressed = evaluateScript("ReturnPressed", senderWidget, m_scriptReturnPressed);

			if (m_jsReturnPressed.isError() == true || m_jsReturnPressed.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript("ReturnPressed", m_jsReturnPressed, senderWidget, true);

		return;
	}

	void SchemaItemLineEdit::textChanged(const QString& /*text*/)
	{
		qDebug() << Q_FUNC_INFO;

		if (m_scriptTextChanged.isEmpty() == true ||
			m_scriptTextChanged == PropertyNames::lineEditDefaultEventScript) // Suppose Default script does nothing, just return
		{
			return;
		}

		QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(sender());
		if (senderWidget == nullptr)
		{
			assert(senderWidget);
			return;
		}

		// Evaluate script
		//
		if (m_jsTextChanged.isUndefined() == true)
		{
			m_jsTextChanged = evaluateScript("TextChanged", senderWidget, m_scriptTextChanged);

			if (m_jsTextChanged.isError() == true || m_jsTextChanged.isNull() == true)
			{
				return;
			}
		}

		// Run script
		//
		runEventScript("TextChanged", m_jsTextChanged, senderWidget, true);

		return;
	}

	void SchemaItemLineEdit::runEventScript(QString scriptName, QJSValue& evaluatedJs, QLineEdit* controlWidget, bool allowMessageBox)
	{
		return SchemaItemControl::runEventScript<QLineEdit>(evaluatedJs, allowMessageBox, scriptName, controlWidget, controlWidget->text());
	}

	// Properties and Data
	//

	const QString& SchemaItemLineEdit::text() const
	{
		return m_text;
	}
	void SchemaItemLineEdit::setText(QString value)
	{
		m_text = std::move(value);
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
		m_readOnly = value;
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

	void SchemaItemLineEdit::drawLineEditControl(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Calculate rectangle
		//
		QRectF r = boundingRectInDocPt(drawParam);

		const QStyle* style = QApplication::style();

		// Draw control
		//
		p->setBrush(style->standardPalette().brush(QPalette::Normal, QPalette::Base));

		QPen pen;
		pen.setColor(style->standardPalette().color(QPalette::Normal, QPalette::Shadow));
		pen.setWidthF(1.0 / drawParam->realDpiX());
		p->setPen(pen);

		p->drawRect(r);

		// Draw text
		//
		p->setPen(style->standardPalette().color(QPalette::Normal, QPalette::WindowText));

		FontParam font;
		font.setName(QStringLiteral("Arial"));

		switch (schema->unit())
		{
		case SchemaUnit::Display:
			font.setSize(12.0, schema->unit());
			break;
		case SchemaUnit::Inch:
			font.setSize(1.0 / 8.0, schema->unit()); // 1/8"
			break;
		case SchemaUnit::Millimeter:
			font.setSize(mm2in(3), schema->unit());
			break;
		default:
			assert(false);
		}

		DrawHelper::drawText(p, font, schema->unit(), m_text, r, Qt::AlignCenter | Qt::AlignHCenter);
		return;
	}

} // namespace VFrame30
