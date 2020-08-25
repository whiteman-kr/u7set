#pragma once

#include "SchemaView.h"
#include "SchemaManager.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "SchemaItem.h"
#include "../lib/ClientBehavior.h"

class QPaintEvent;
class QTimerEvent;
class QMouseEvent;


namespace VFrame30
{
	class ClientSchemaView;

	// Proxy class for using in scripts
	//
	/*! \class ScriptSchemaView
		\ingroup view
		\brief Represents a class that contains schemas displayed on the screen. Used for switching schemas, searching items and displaying message boxes.

		sRepresents a class that contains schemas displayed on the screen. In scripts this object is accessed by global <b>view</b> object.

		Implements following functons:

		- schemas switching;
		- items and widgets searching;
		- setting and reading variables;
		- displaying message boxes.

		\warning Items and widgets searching is performed by objects name. In order to find certain item, an item should have an unique <b>ObjectName</b> property value within the schema.

		\n
		\warning Do not call message box functions (<b>warningMessageBox, errorMessageBox, infoMessageBox and questionMessageBox</b>) from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers.
		This can cause user interface hang.

		<b>Schema Variables</b>

		Schema variables are used to change schema contents dynamically. For example, one schema can display different set of signals.
		Dynamic schema items (<b>SchemaItemValue</b>, <b>SchemaItemImageValue</b> etc.) can contain variable macros in signal properties
		(<b>AppSignalIDs</b>) and display different application signals when variable changes. Variable values can be changed by script code.

		<b>Example 1. Schema Switching</b>

		\code
		// Set another schema
		//
		view.setSchema("MYSCHEMA");
		\endcode

		<b>Example 2. Displaying Message Boxes</b>

		\code
		// Display information in the message box
		//
		view.infoMessageBox("Hello world!");

		...

		// Display a question
		//
		if (view.questionMessageBox("Are you sure?") == true)
		{
			// User pressed "Yes"
			....
		}
		\endcode

		<b>Example 3. Set current schema caption to rectangle text on click event (ClickScript)</b>

		\code
		// This is an example how to set current schema caption to rectangle text on click event (ClickScript)
		//
		(function(schemaItem)
		{
			schemaItem.Text = view.SchemaCaption;
		})
		\endcode

		<b>Example 4. Using schema variables</b>

		Assume schema has SchemaItemValue with <b>AppSignalIDs</b> property set to <i>"#SIGNAL_0_$(Var01)"</i>. Also schema contains two buttons (SchemaItemPushButton)
		with click event handlers.

		On first button click, <i>"Var01"</i> variable is set to <i>"SIG001"</i>, on second button click is set to <i>"SIG002"</i>.

		SchemaItemValue variable macro will be replaced to its value, so two different signal values (<i>SIGNAL_0_SIG001</i> or <i>SIGNAL_0_SIG002</i>) will be
		displayed depending on what button was clicked.

		\code
		// SchemaItemPushButton 1 ClickScript handler
		//
		(function(schemaItem, pushButtonWidget, checked)
		{
			view.setVariable("Var01", "SIG001");
			view.infoMessageBox("Variable is set to " + view.variable("Var01"));
		})

		// SchemaItemPushButton 2 ClickScript handler
		//
		(function(schemaItem, pushButtonWidget, checked)
		{
			view.setVariable("Var01", "SIG002");
			view.infoMessageBox("Variable is set to " + view.variable("Var01"));
		})
		\endcode
	*/
	class ScriptSchemaView : public QObject
	{
		Q_OBJECT

		/// \brief Current schema unique identifier (SchemaID).
		Q_PROPERTY(QString SchemaID READ schemaId)

		/// \brief Current schema caption.
		Q_PROPERTY(QString SchemaCaption READ schemaCaption)

		/// \brief Get current ScriptSchema object. To get SchemaID or SchemaCaption for perfomance reason use appropriate properties of view <b>view.SchemaID</b> and <b>view.SchemaCaption</b>.
		Q_PROPERTY(QObject* Schema READ schema)

		/// \brief Get schema count.
		Q_PROPERTY(int SchemaCount READ schemaCount)

	public:
		explicit ScriptSchemaView(ClientSchemaView* clientSchemaView, QObject* parent = nullptr);
		~ScriptSchemaView();

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str);					// Debug output to qDebug

		/// \brief Sets the active schema specified in schemaId parameter.
		void setSchema(QString schemaId);				// Set schema by SchemaID

		/// \brief Finds a schema item by its name (ObjectName property). Return value has SchemaItem type or undefined if item is not found.
		QObject* findSchemaItem(QString objectName);	// Find SchemaItem by ObjectName

		/// \brief Finds a schema control widget (edit control, button, etc...) by its name (ObjectName property).
		///
		/// Finds a schema control widget (edit control, button, etc...) by its name (ObjectName property).
		/// Return value type depends on an object type and can be one of following: PushButtonWidget, LineEditWidget, etc.
		/// Return value is set to <i>undefined</i> if item is not found.
		QObject* findWidget(QString objectName);		// Find Widget associated with SchemaItem

		void update();									// Update (redraw) schema view

		// Message Box functions
		//
		/// \brief Displays a warning message box with specified text.
		///
		/// Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void warningMessageBox(QString text);

		/// \brief Displays an error message box with specified text.
		///
		/// Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void errorMessageBox(QString text);

		/// \brief Displays an information message box with specified text.
		///
		/// Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void infoMessageBox(QString text);

		/// \brief Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		bool questionMessageBox(QString text);

		// Variables functions
		//
		/// \brief Returns true if variable specified by name exists, otherwise returns false.
		bool variableExists(QString name) const;

		/// \brief Retrieves a value of the variable specified by name.
		QVariant variable(QString name);

		/// \brief Sets the value of the variable specified by name.
		void setVariable(QString name, const QVariant& value);

		/// \brief Get schema by index. Avoid using this function for perfomance reason. To get schemas' identifiers and captions use schemaCaptionById, schemaCaptionByIndex, schemaIdByIndex
		QObject* schemaByIndex(int schemaIndex);

		/// \brief Get schema caption by schema identifier.
		QString schemaCaptionById(const QString& schemaId) const;

		// Not documented
		//
		QString schemaCaptionByIndex(int schemaIndex) const;

		// Not documented
		//
		QString schemaIdByIndex(int schemaIndex) const;

	private:
		QString schemaId() const;
		QString schemaCaption() const;

		QObject* schema();

		int schemaCount() const;

		// Data
		//
	private:
		ClientSchemaView* m_clientSchemaView = nullptr;
	};


	class VFRAME30LIBSHARED_EXPORT ClientSchemaView : public VFrame30::SchemaView
	{
		Q_OBJECT

	public:
		explicit ClientSchemaView(VFrame30::SchemaManager* schemaManager, QWidget* parent = nullptr);
		virtual ~ClientSchemaView();

	public:
		void setSchema(QString schemaId);
		void setSchema(QString schemaId, const QStringList& highlightAppSignalIds);

	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

	protected slots:
		void startRepaintTimer();

	signals:
		void signal_setSchema(QString schemaId, QStringList highlightIds);

		// Properties
		//
	public:
		VFrame30::SchemaManager* schemaManager();
		const VFrame30::SchemaManager* schemaManager() const;

		bool periodicUpdate() const;
		void setPeriodicUpdate(bool value);

		bool infoMode() const;
		void setInfoMode(bool value);

		const QStringList& hightlightIds() const;
		void setHighlightIds(const QStringList& value);

		// TuningController
		//
		TuningController* tuningController();
		const TuningController* tuningController() const;
		void setTuningController(TuningController* value);

		//  AppSignalController
		//
		AppSignalController* appSignalController();
		const AppSignalController* appSignalController() const;
		void setAppSignalController(AppSignalController* value);

		// --
		//
		QJSEngine* jsEngine();
		QString globalScript() const;

		bool runScript(QJSValue& evaluatedJs, QString where, bool reportError);
		bool reEvaluateGlobalScript();
		QJSValue evaluateScript(QString script, QString where, bool reportError);
		QString formatScriptError(const QJSValue& scriptValue) const;
		void reportScriptError(const QJSValue& scriptValue, QString where);

		// Variables
		//
		bool variableExists(QString name) const;

		QVariant variable(QString name) const;
		void setVariable(QString name, const QVariant& value);

		const QVariantHash& variables() const;
		void setVariables(const QVariantHash& values);

		// ClientBehavior
		//
		const MonitorBehavior& monitorBehavor() const;
		void setMonitorBehavior(const MonitorBehavior& src);
		void setMonitorBehavior(MonitorBehavior&& src);

		const TuningClientBehavior& tuningClientBehavior() const;
		void setTuningClientBehavior(const TuningClientBehavior& src);
		void setTuningClientBehavior(TuningClientBehavior&& src);

	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;

		TuningController* m_tuningController = nullptr;
		AppSignalController* m_appSignalController = nullptr;
		std::unique_ptr<ScriptAppSignalController> m_scriptAppSignalController;

		bool m_periodicUpdate = true;		// Update widget every 250 ms
		bool m_infoMode = false;			// Show some aditional info like labels

		QStringList m_highlightIds;			// Highligted IDs, can be any, like AppSignalID, ConnectiondID... depends on item

		// --
		//
		bool m_jsEngineGlobalsWereCreated = false;
		std::unique_ptr<ScriptSchemaView> m_scriptSchemaView;

		QJSEngine m_jsEngine;

		std::shared_ptr<SchemaItem> m_leftClickOverItem;
		QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();

		// Variables
		//
		QVariantHash m_variables;		// Key is variable name

		// Behaviors
		//
		MonitorBehavior m_monitorBehavior;
		TuningClientBehavior m_tuningClientBehavior;
	};
}

