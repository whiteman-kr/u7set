#pragma once
#include "SchemaItemControl.h"
#include "PropertyNames.h"

class QLineEdit;

namespace VFrame30
{
	/*! \class SchemaItemLineEdit
		\ingroup controlSchemaItems
		\brief This class is used to create input fields

		SchemaItemLineEdit class implements input fields on schemas.

		<b>Event handlers</b>

		To customize item's apperance and behaviour, event handler code is placed to following properties of the schema item using RPCT:

		- <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn;<br>
		- <b>AfterCreate</b> contains creation event handler code. This event is generated when schema item is created;<br>
		- <b>EditingFinished</b> contains editing finished event handler code. This event is generated when text editing is finished and focus is switched to another item;<br>
		- <b>ReturnPressed</b> contains return pressing event handler code. This event is generated when Enter key is pressed;<br>
		- <b>TextChanged</b> contains text changing event handler code. This event is generated when text is modified.<br>

		<b>PreDrawScript</b> event handler function protype:

		\code
		function(schemaItem)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemLineEdit.<br>

		<b>AfterCreate</b>, <b>EditingFinished</b>, <b>ReturnPressed</b>, <b>TextChanged</b> event handlers function protypes:

		\code
		function(schemaItem, lineEditWidget, text)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemLineEdit;<br>
		<i>lineEditWidget</i> - a handle to line edit widget, type: LineEditWidget;<br>
		<i>text</i> - text in the control, type: QString.<br>

		<b>Accessing line edit widget</b>

		Line edit widget is used to read or modify edit control properties. It is implemented by LineEditWidget class.

		Widget is can be accessed by <i>lineEditWidget</i> parameter or requested by <i>findWidget</i> function of ScriptSchemaView class.

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
		virtual QWidget* createWidget(QWidget* parent, bool editMode, double zoom) override final;
		virtual void updateWidgetProperties(QWidget* widget) const  override final;

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

	//
	// Warning! LineEditWidget class is used only for documentation generated by Doxygen and should not be used in code!
	//

	/*! \class LineEditWidget
		\ingroup widgets
		\brief This class is used to control appearance and behaviour of a line edit widget

		LineEditWidget class is used to control appearance and behaviour of a line edit widget
	*/
	class VFRAME30LIBSHARED_EXPORT LineEditWidget : public QLineEdit
	{
		Q_OBJECT

		/*! \brief This property holds the alignment of the line edit
		Both horizontal and vertical alignment is allowed here.
		By default, this property contains a combination of AlignLeft and AlignVCenter.

		<b>Example</b>
		\code
		// Adjust alignment to left and bottom
		//
		var AlignLeft = 0x01;
		var AlignBottom = 0x40;
		...
		lineEditWidget.alignment = AlignLeft | AlignBottom;
		\endcode
		*/
		Q_PROPERTY(Alignment alignment READ someAlignmentProperty WRITE setSomeAlignmentProperty)

		/// \brief This property holds the maximum permitted length of the text
		///
		/// If the text is too long, it is truncated at the limit.
		/// By default, this property contains a value of 32767.
		Q_PROPERTY(int maxLength READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds whether the line edit is read only
		Q_PROPERTY(bool readOnly READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds the line edit's placeholder text
		///
		/// Setting this property makes the line edit display a grayed-out placeholder text as long as the line edit is empty.
		/// Normally, an empty line edit shows the placeholder text even when it has focus.
		/// By default, this property contains an empty string.
		Q_PROPERTY(QString placeholderText READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds the line edit's text
		Q_PROPERTY(QString text READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds whether the widget is enabled
		Q_PROPERTY(bool enabled READ someBoolProperty WRITE setSomeBoolProperty)

		/*! \brief This property holds the widget's style sheet

		The style sheet contains a textual description of customizations to the widget's style, as described in the <a href="https://doc.qt.io/qt-5/stylesheet.html">Qt Style Sheets</a> document.

		<b>Example</b>

		\code
		// Event handler sets new styleSheet for a line edit with custom border style, padding, background color and background selection color
		//
		(function(schemaItem, lineEditWidget, checked)
		{

			lineEditWidget.styleSheet = "QLineEdit {\
					border: 2px solid gray;\
					border-radius: 10px;\
					padding: 0 8px;\
					background: yellow;\
					selection-background-color: darkgray;\
					}";
		})
		\endcode
		*/
		Q_PROPERTY(QString styleSheet READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds the widget's tooltip
		Q_PROPERTY(QString toolTip READ someStringProperty WRITE setSomeStringProperty)

		/// \brief Specifies how long time the tooltip will be displayed, in milliseconds.
		Q_PROPERTY(int toolTipDuration READ someIntProperty WRITE setSomeIntProperty)

		// Enumerations
		//

	public:
		/// \brief This enum describes text alignment
		enum class Alignment
		{
			AlignLeft = 0x01,		/**< AlignLeft = 0x01*/
			AlignRight = 0x02,		/**< AlignRight = 0x02*/
			AlignHCenter = 0x04,	/**< AlignHCenter = 0x04*/
			AlignJustify = 0x08,	/**< AlignJustify = 0x08*/
			AlignAbsolute = 0x10,	/**< AlignAbsolute = 0x10*/

			AlignTop = 0x20,		/**< AlignTop = 0x20*/
			AlignBottom = 0x40,		/**< AlignBottom = 0x40*/
			AlignVCenter = 0x80,	/**< AlignVCenter = 0x80*/
			AlignBaseline = 0x100	/**< AlignBaseline = 0x100*/
		};

		// Empty property getters and setters

		bool someBoolProperty() const {return false;}
		void setSomeBoolProperty(bool value) {Q_UNUSED(value);}

		int someIntProperty() const {return 0;}
		void setSomeIntProperty(int value) {Q_UNUSED(value);}

		QString someStringProperty() const {return QString();}
		void setSomeStringProperty(QString value) {Q_UNUSED(value);}

		Alignment someAlignmentProperty() const {return Alignment::AlignLeft;}
		void setSomeAlignmentProperty(Alignment value) {Q_UNUSED(value);}
	};

}
