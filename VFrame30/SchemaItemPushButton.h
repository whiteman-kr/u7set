#pragma once
#include "SchemaItemControl.h"
#include "PropertyNames.h"

namespace VFrame30
{
	/*! \class SchemaItemPushButton
		\ingroup controlSchemaItems
		\brief This class is used to create a push button schema item

		SchemaItemPushButton class implements buttons on schemas. Buttons can be checkable (like a checkbox) and non-checkable (simple).

		<b>Event handlers</b>

		To customize item's apperance and behaviour, event handler code is placed to following properties of the schema item using RPCT:

		- <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn;<br>
		- <b>AfterCreate</b> contains creation event handler code. This event is generated when schema item is created;<br>
		- <b>Clicked</b> contains clicked event handler code. This event is generated when button is clicked (pressed and released);<br>
		- <b>Pressed</b> contains press event handler code. This event is generated when button is pressed;<br>
		- <b>Released</b> contains release event handler code. This event is generated when button is released;<br>
		- <b>Toggled</b> contains toggle event handler code. This event is generated when button is toggled (for checkable buttons).<br>

		<b>PreDrawScript</b> event handler function protype:

		\code
		function(schemaItem)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemPushButton.<br>

		<b>AfterCreate</b>, <b>Clicked</b>, <b>Pressed</b>, <b>Toggled</b> event handlers function protypes:

		\code
		function(schemaItem, pushButtonWidget, checked)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemPushButton;<br>
		<i>pushButtonWidget</i> - a handle to button widget, type: PushButtonWidget;<br>
		<i>checked</i> - button check state, type: boolean.<br>

		<b>Accessing button widget</b>

		Button widget is used to read or modify button control properties. It is implemented by PushButtonWidget class.

		Widget can be accessed by <i>pushButtonWidget</i> parameter or requested by <i>findWidget</i> function of ScriptSchemaView class.

		<b>Example:</b>

		Push button increments tunable signal value on click. If signal does not exist or its value is not valid, button should be disabled.

		\warning <b>ObjectName</b> property of line edit schema item should have unique name.

		<b>GlobalScript</b> functions:

		\code
		// Function increments signal <b>signalId</b> for <b>step</b>
		//
		function incValue(signalId, step)
		{
			// Get current signal parameters and state
			//
			let param = tuning.signalParam(signalId);
			let state = tuning.signalState(signalId);

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

			// Increment the value
			//
			let value = state.Value;
			value += step;

			// Check signal limits
			//
			let lowLimit = param.TuningLowBound;
			let highLimit = param.TuningHighBound;

			if (value < lowLimit || value > highLimit)
			{
				view.errorMessageBox("Value is out of range!");
				return;
			}

			// Write new value
			//
			if (tuning.writeValue(signalId, value) == false)
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
			let widget = view.findWidget(objectName);
			if (widget == null)
			{
				return;
			}

			let enabled = true;

			// Get signal state and set enabled to false if signal does not exist or is not valid
			//
			let state = tuning.signalState(signalId);
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
			// Enable this widget depending on signal "APPSIGNALID001" state
			//
			enableControlBySignal(schemaItem.ObjectName, "#APPSIGNALID001");
		})
		\endcode

		<b>Clicked</b> event handler:

		\code
		(function(schemaItem, pushButtonWidget, checked)
		{
			// Increment application signal "#APPSIGNALID001" by 1
			//
			incValue("#APPSIGNALID001", 1);
		})
		\endcode
	*/
	class VFRAME30LIBSHARED_EXPORT SchemaItemPushButton : public SchemaItemControl
	{
		Q_OBJECT

	public:
		SchemaItemPushButton(void);
		explicit SchemaItemPushButton(SchemaUnit unit);
		virtual ~SchemaItemPushButton(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Methods
	public:
		virtual QWidget* createWidget(QWidget* parent, bool editMode, double zoom) override final;
		virtual void updateWidgetProperties(QWidget* widget) const override final;

	protected slots:
		void afterCreate(QPushButton* control);
		void clicked(bool checked);
		void pressed();
		void released();
		void toggled(bool checked);

		void runEventScript(QJSValue& evaluatedJs, QPushButton* buttonWidget);

		// Properties and Data
		//
	public:
		const QString& text() const;
		void setText(QString value);

		bool isCheckable() const;
		void setCheckable(bool value);

		bool checkedDefault() const;
		void setCheckedDefault(bool value);

		bool autoRepeat() const;
		void setAutoRepeat(bool value);

		int autoRepeatDelay() const;
		void setAutoRepeatDelay(int value);

		int autoRepeatInterval() const;
		void setAutoRepeatInterval(int value);

		virtual void setStyleSheet(QString value) override;

		QString scriptAfterCreate() const;
		void setScriptAfterCreate(const QString& value);

		QString scriptClicked() const;
		void setScriptClicked(const QString& value);

		QString scriptPressed() const;
		void setScriptPressed(const QString& value);

		QString scriptReleased() const;
		void setScriptReleased(const QString& value);

		QString scriptToggled() const;
		void setScriptToggled(const QString& value);

	private:
		QString m_text = {"Button"};

		bool m_checkable = false;
		bool m_checkedDefault = false;

		bool m_autoRepeat = false;
		int m_autoRepeatDelay = 300;
		int m_autoRepeatInterval = 100;

		QString m_scriptAfterCreate = PropertyNames::pushButtonDefaultEventScript;
		QString m_scriptClicked = PropertyNames::pushButtonDefaultEventScript;
		QString m_scriptPressed = PropertyNames::pushButtonDefaultEventScript;
		QString m_scriptReleased = PropertyNames::pushButtonDefaultEventScript;
		QString m_scriptToggled = PropertyNames::pushButtonDefaultEventScript;

		// Evaluated scripts
		//
		QJSValue m_jsAfterCreate;
		QJSValue m_jsClicked;
		QJSValue m_jsPressed;
		QJSValue m_jsReleased;
		QJSValue m_jsToggled;
	};

	//
	// Warning! PushButtonWidget class is used only for documentation generated by Doxygen and should not be used in code!
	//

	/*! \class PushButtonWidget
		\ingroup widgets
		\brief This class is used to control appearance and behaviour of a push button widget

		PushButtonWidget class is used to control push button widget's appearance and behaviour.
	*/
	class PushButtonWidget : public QPushButton
	{
		Q_OBJECT

		/// \brief This property holds whether autoRepeat is enabled
		///
		/// If autoRepeat is enabled, then the pressed(), released(), and clicked() signals are emitted at
		/// regular intervals when the button is down. autoRepeat is off by default. The initial delay and
		/// the repetition interval are defined in milliseconds by autoRepeatDelay and autoRepeatInterval properties.
		Q_PROPERTY(bool autoRepeat READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds the initial delay of auto-repetition
		///
		/// If autoRepeat is enabled, then autoRepeatDelay defines the initial delay in milliseconds before auto-repetition kicks in.
		Q_PROPERTY(int autoRepeatDelay READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds the interval of auto-repetition
		///
		/// If autoRepeat is enabled, then autoRepeatInterval defines the length of the auto-repetition interval in millisecons.
		Q_PROPERTY(int autoRepeatInterval READ someIntProperty WRITE setSomeIntProperty)

		/// \brief This property holds whether the button is checkable
		Q_PROPERTY(bool checkable READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds whether the button is checked
		Q_PROPERTY(bool checked READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds whether the button is pressed down
		///
		/// If this property is true, the button is pressed down. The signals pressed() and clicked() are not emitted if you set this property to true. The default is false.
		Q_PROPERTY(bool down READ someBoolProperty WRITE setSomeBoolProperty)

		/// \brief This property holds the text shown on the button
		Q_PROPERTY(QString text READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds whether the widget is enabled
		Q_PROPERTY(bool enabled READ someBoolProperty WRITE setSomeBoolProperty)

		/*! \brief This property holds the widget's style sheet

		The style sheet contains a textual description of customizations to the widget's style, as described in the <a href="https://doc.qt.io/qt-5/stylesheet.html">Qt Style Sheets</a> document.

		<b>Example</b>

		\code
		// Event handler sets new styleSheet for a button. Background color is red, border width is 2 pixels, border color is beige.
		//
		(function(schemaItem, pushButtonWidget, checked)
		{

			pushButtonWidget.styleSheet = "\
			QPushButton{\
			background-color: red;\
			border-width: 2px;\
			border-color: beige;\
			}";
		})
		\endcode
		*/
		Q_PROPERTY(QString styleSheet READ someStringProperty WRITE setSomeStringProperty)

		/// \brief This property holds the widget's tooltip
		Q_PROPERTY(QString toolTip READ someStringProperty WRITE setSomeStringProperty)

		/// \brief Specifies how long time the tooltip will be displayed, in milliseconds.
		Q_PROPERTY(int toolTipDuration READ someIntProperty WRITE setSomeIntProperty)

		// Empty property getters and setters

		bool someBoolProperty() const {return false;}
		void setSomeBoolProperty(bool value) {Q_UNUSED(value);}

		int someIntProperty() const {return 0;}
		void setSomeIntProperty(int value) {Q_UNUSED(value);}

		QString someStringProperty() const {return QString();}
		void setSomeStringProperty(QString value) {Q_UNUSED(value);}
	};
}
