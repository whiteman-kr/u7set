#pragma once
#include "SchemaItemControl.h"
#include "PropertyNames.h"

namespace VFrame30
{
	/*! \class SchemaItemPushButton
		\ingroup controlSchemaItems
		\brief This class is used to create a push button

		SchemaItemPushButton class implement buttons on schemas. Buttons can be checkable (like a checkbox) and non-checkable (simple).
		Buttons generate events when user interacts with them.

		<b>Handling button events</b>

		Push button can generate following events:
		- <b>"created"</b> event, handler in <b>AfterCreate</b> property;
		- <b>"clicked"</b> event, handler in <b>Clicked</b> property;
		- <b>"pressed"</b> event, handler in <b>Pressed</b> property;
		- <b>"released"</b> event, handler in <b>Released</b> property;
		- <b>"toggled"</b> event, handler in <b>Toggled</b> property.

		To implement an event handler, project designer should write a function in corresponding property. Each event handler has the same protype:

		\code
		function(schemaItem, pushButtonWidget, checked)
		\endcode

		Function parameters:

		<b>schemaItem</b> - a handle to schema item, type: SchemaItemPushButton;<br>
		<b>pushButtonWidget</b> - a handle to button widget, type: QPushButton;<br>
		<b>checked</b> - button check state, type: boolean.<br>

		<b>Accessing button widget</b>

		Button widget is used to read or modify button control properties. For example, script can modify Enabled property to enable/disable the button
		or Text property to modify button text.

		Button widget can be accessed by pushButtonWidget parameter or requested by findWidget function of ScriptSchemaView class.

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

			// Increment the value
			//
			var value = state.Value;
			value += step;

			// Check signal limits
			//
			var lowLimit = param.TuningLowBound;
			var highLimit = param.TuningHighBound;

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

		/// \brief Script called when schema item is created
		Q_PROPERTY(QString AfterCreate READ scriptAfterCreate)

		/// \brief Script called when button is clicked (pressed and released)
		Q_PROPERTY(QString Clicked READ scriptClicked)

		/// \brief Script called when button is pressed
		Q_PROPERTY(QString Pressed READ scriptPressed)

		/// \brief Script called when button is released
		Q_PROPERTY(QString Released READ scriptReleased)

		/// \brief Script called when button is toggled (for checkable buttons)
		Q_PROPERTY(QString Toggled READ scriptToggled)


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
		virtual QWidget* createWidget(QWidget* parent, bool editMode) final;
		virtual void updateWidgetProperties(QWidget* widget) const final;

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
}
