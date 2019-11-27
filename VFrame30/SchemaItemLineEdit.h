#pragma once
#include "SchemaItemControl.h"
#include "PropertyNames.h"

class QLineEdit;

namespace VFrame30
{
	/*! \class SchemaItemLineEdit
		\ingroup controlSchemaItems
		\brief This class is used to create input fields

		SchemaItemLineEdit class implement input fields on schemas.
		Line edits generate events when user interacts with them.

		<b>Handling line edit events</b>

		Push button can generate following events:
		- <b>"created"</b> event, handler in <b>AfterCreate</b> property;
		- <b>"editingFinished"</b> event, handler in <b>EditingFinished</b> property;
		- <b>"returnPressed"</b> event, handler in <b>ReturnPressed</b> property;
		- <b>"textChanged"</b> event, handler in <b>TextChanged</b> property.

		To implement an event handler, project designer should write a function in corresponding property. Each event handler has the same protype:

		\code
		function(schemaItem, lineEditWidget, text)
		\endcode

		Function parameters:

		<b>schemaItem</b> - a handle to schema item, type: SchemaItemLineEdit;<br>
		<b>lineEditWidget</b> - a handle to line edit widget, type: QLineEdit;<br>
		<b>text</b> - text in the control, type: QString.<br>

		<b>Accessing line edit widget</b>

		Line edit widget is used to read or modify edit control properties. For example, script can modify Enabled property to enable/disable input
		or Text property to modify text.

		Line edit widget can be accessed by lineEditWidget parameter or requested by findWidget function of ScriptSchemaView class.

		<b>Example:</b>

		Tunable signal value should be taken from line edit and written to logic module after pressing Enter key.
		If signal does not exist or its value is not valid, line edit should be disabled.

		\warning <b>ObjectName</b> property of line edit schema item should have unique name.

		<b>GlobalScript</b> functions:

		\code
		// Function writes new value to tunable signal <b>signalId</b> specified in <b>text</b>
		//
		function setValueByText(signalId, text)
		{
			// Get current signal parameters and state
			//
			var param = tuning.signalParam(signalId);
			var state = tuning.signalState(signalId);

			// Check if signal exists
			//
			if (param == undefined || state == undefined)
			{
				view.errorMessageBox("Signal does not exist!");
				return;
			}

			// Check signal validity
			//
			if (state.Valid == false)
			{
				view.errorMessageBox("Signal is not valid!");
				return;
			}

			// Convert text value to number
			//
			var x = Number(text);

			if (isNaN(x) == true)
			{
				view.errorMessageBox("Invalid number format - '" + text + "'!");
				return;
			}

			// Check if new value is out of range
			//
			if (param.IsAnalog == true)
			{
				var lowLimit = param.TuningLowBound;
				var highLimit = param.TuningHighBound;

				if (x < lowLimit || x > highLimit)
				{
					view.errorMessageBox("Input value is out of range! Valid range is " + lowLimit + "..." + highLimit + "!");
					return;
				}
			}

			// Show warning message
			//
			if (view.questionMessageBox("Are you sure you want to set value '" + x + "' ?") == false)
			{
				return;
			}

			// Write new value
			//
			if (tuning.writeValue(signalId, x) == false)
			{
				view.errorMessageBox("Value set error!");
				return;
			}
		}

		// Function enables or disables widget with <b>ObjectName</b> specified in <b>objectName</b> parameter depending on signal with <b>signalId</b> identifier
		//
		function enableControlBySignal(objectName, signalId)
		{
			// Find widget by its ObjectName
			//
			var widget = view.findWidget(objectName);
			if (widget == null)
			{
				return;
			}

			var enabled = true;

			// Get signal state and set enabled to false if signal does not exist or is not valid
			//
			var state = tuning.signalState(signalId);
			if (state == undefined || state.Valid == false)
			{
				enabled = false;
			}

			// Enable/disable the widget
			//
			if (widget.enabled != enabled)
			{
				widget.enabled = enabled;
			}
		}
		\endcode

		<b>PreDrawScript</b> event handler:

		\code
		(function(schemaItem)
		{
			// Enable this widget depending on signal "APPSIGNALID002" state
			//
			enableControlBySignal(schemaItem.ObjectName, "#APPSIGNALID002");
		})
		\endcode

		<b>ReturnPressed</b> event handler:

		\code
		(function(schemaItem, lineEditWidget, text)
		{
			// Set application signal "#APPSIGNALID002" to value from text parameter
			//
			setValueByText("#APPSIGNALID002", text);
		})
		\endcode
	*/

	class VFRAME30LIBSHARED_EXPORT SchemaItemLineEdit : public SchemaItemControl
	{
		Q_OBJECT

		/// \brief Script called when schema item is created
		Q_PROPERTY(QString AfterCreate READ scriptAfterCreate)

		/// \brief Script called when editing is finished, e.g. focus is moved to other control
		Q_PROPERTY(QString EditingFinished READ scriptEditingFinished)

		/// \brief Script called when Enter key is pressed
		Q_PROPERTY(QString ReturnPressed READ scriptReturnPressed)

		/// \brief Script called when text is modified
		Q_PROPERTY(QString TextChanged READ scriptTextChanged)

	public:
		SchemaItemLineEdit(void);
		explicit SchemaItemLineEdit(SchemaUnit unit);
		virtual ~SchemaItemLineEdit(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Methods
	public:
		virtual QWidget* createWidget(QWidget* parent, bool editMode) final;
		virtual void updateWidgetProperties(QWidget* widget) const final;

	protected slots:
		void afterCreate(QLineEdit* control);
		void editingFinished();
		void returnPressed();
		void textChanged(const QString& text);

		void runEventScript(QJSValue& evaluatedJs, QLineEdit* controlWidget);

		// Properties and Data
		//
	public:
		const QString& text() const;
		void setText(QString value);

		const QString& placeholderText() const;
		void setPlaceholderText(QString value);

		int maxLength() const;
		void setMaxLength(int value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign value);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign value);

		bool readOnly() const;
		void setReadOnly(bool value);

		virtual void setStyleSheet(QString value) override;

		QString scriptAfterCreate() const;
		void setScriptAfterCreate(const QString& value);

		QString scriptEditingFinished() const;
		void setScriptEditingFinished(const QString& value);

		QString scriptReturnPressed() const;
		void setScriptReturnPressed(const QString& value);

		QString scriptTextChanged() const;
		void setScriptTextChanged(const QString& value);

	private:
		QString m_text = {"0.0"};
		QString m_placeholderText;
		int m_maxLength = 32;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;

		bool m_readOnly = false;

		QString m_scriptAfterCreate = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptEditingFinished = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptReturnPressed = PropertyNames::lineEditDefaultEventScript;
		QString m_scriptTextChanged = PropertyNames::lineEditDefaultEventScript;

		// Evaluated scripts
		//
		QJSValue m_jsAfterCreate;
		QJSValue m_jsEditingFinished;
		QJSValue m_jsReturnPressed;
		QJSValue m_jsTextChanged;
	};
}
