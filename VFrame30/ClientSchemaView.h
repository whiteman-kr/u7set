#pragma once

#include "../lib/ClientBehavior.h"
#include "../lib/ITimeStats.h"

#include "AppSignalController.h"
#include "DiagStateController.h"
#include "ISchemaViewHistory.h"
#include "IViewVariables.h"
#include "LogController.h"
#include "SchemaItem.h"
#include "SchemaManager.h"
#include "SchemaView.h"
#include "TuningController.h"

class QPaintEvent;
class QTimerEvent;
class QMouseEvent;

namespace VFrame30
{
	class ClientSchemaView;

	struct GlobalScriptEvents
	{
		inline static const QString OnConfigurationArrived{"OnConfigurationArrived"};
		inline static const QString OnTimerEvent{"OnTimerEvent"};
	};

	// Proxy class for using in scripts
	//
	/*! \class ScriptSchemaView
		\ingroup view
		\brief Represents a class that contains schemas displayed on the screen. Used for switching schemas, searching items and displaying message boxes.

		sRepresents a class that contains schemas displayed on the screen. In scripts this object is accessed by global <b>view</b> object.

		Implements following functions:

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
		Q_PROPERTY(QString schemaID READ schemaId)
		Q_PROPERTY(QString SchemaID READ schemaId)

		/// \brief Current schema caption.
		Q_PROPERTY(QString schemaCaption READ schemaCaption)
		Q_PROPERTY(QString SchemaCaption READ schemaCaption)

		/// \brief Get current ScriptSchema object. To get SchemaID or SchemaCaption for performance reason use appropriate properties of view <b>view.schemaID</b> and <b>view.schemaCaption</b>.
		Q_PROPERTY(QObject* schema READ schema)
		Q_PROPERTY(QObject* Schema READ schema)

		/// \brief Get schema count.
		Q_PROPERTY(int schemaCount READ schemaCount)
		Q_PROPERTY(int SchemaCount READ schemaCount)

		/// \brief Get zoom factor for schema (100% zoom returns 1.0).
		Q_PROPERTY(double zoomFactor READ zoomFactor)
		Q_PROPERTY(double ZoomFactor READ zoomFactor)

	public:
		explicit ScriptSchemaView(ClientSchemaView* clientSchemaView,
								  ISchemaViewHistory* schemaViewHistory,
								  QObject* parent = nullptr);
		virtual ~ScriptSchemaView() = default;

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str); // Debug output to qDebug

		/// \brief Sets the active schema specified in schemaId parameter.
		void setSchema(QString schemaId); // Set schema by SchemaID

		/// \brief Finds schema item by its name (ObjectName property). Returned value has SchemaItem type or undefined if item is not found.
		QObject* findSchemaItem(QString objectName); // Find SchemaItem by ObjectName

		/// \brief Finds a schema control widget (edit control, button, etc...) by its name (ObjectName property).
		///
		/// Finds a schema control widget (edit control, button, etc...) by its name (ObjectName property).
		/// Return value type depends on an object type and can be one of following: PushButtonWidget, LineEditWidget, etc.
		/// Return value is set to <i>undefined</i> if item is not found.
		QObject* findWidget(QString objectName); // Find Widget associated with SchemaItem

		void update();                           // Update (redraw) schema view

		/// @brief Starts or restarts the timer with a timeout of duration intervalMs milliseconds.
		/// @param intervalMs Timer interval, milliseconds.
		/// @param timerId Timer identifier, string.
		///
		/// A timer event will occur every intervalMs milliseconds until killTimer() or killAllTimers() is called.
		/// The GlobalScript function named OnTimerEvent() is called with the timerId parameter when a timer event occurs.
		/// Each schema tab has its own set of timers.
		///
		/// @code {.js}
		/// ...
		/// view.startTimer(500, "My500MsTimer");
		/// ...
		/// // GlobalScript:
		/// //
		/// function OnTimerEvent(timerId)
		/// {
		///		if (timerId == "My500MsTimer")
		///		{
		///			// Timer event code
		///		}
		/// }
		///
		/// @endcode
		void startTimer(int intervalMs, QString timerId);

		/// @brief Kills the timer with timer identifier.
		/// @param timerId Timer identifier
		void killTimer(QString timerId);

		/// @brief Kills all active timers for the current schema tab.
		void killAllTimers();

		// History functions
		//

		/// \brief Returns <b>true</b> if back history sequence is not empty, call <b>historyBack</b> to set the previous schema view.
		///
		bool canBackHistory() const;

		/// \brief Returns <b>true</b> if forward history sequence is not empty, call <b>historyForward</b> to set the next schema view.
		///
		bool canForwardHistory() const;

		/// \brief Select the previous schema view from sequence of earlier opened schemas. Use <b>canBackHistory</b> to detect if operation is possible.
		///
		void historyBack();

		/// \brief Select the next schema view from sequence of earlier opened schemas. Use <b>canForwardHistory</b> to detect if operation is possible.
		///
		void historyForward();

		// Message Box functions
		//
		/// \brief Displays a warning message box with specified text. Optional details parameter specifies details text.
		///
		/// Displays a question message box with specified text.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void warningMessageBox(QString text, QString details = QString());

		/// \brief Displays an error message box with specified text. Optional details parameter specifies details text.
		///
		/// Displays a question message box with specified text. Optional details parameter specifies details text.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void errorMessageBox(QString text, QString details = QString());

		/// \brief Displays an information message box with specified text. Optional details parameter specifies details text.
		///
		/// Displays a question message box with specified text. Optional details parameter specifies details text.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		void infoMessageBox(QString text, QString details = QString());

		/// \brief Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// Displays a question message box with specified text. If user clicked "Yes", returns true, otherwise returns false.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		bool questionMessageBox(QString text, QString details = QString());

		/// \brief Displays a message box with specified text with specified buttons, default button and icon.
		///
		/// Displays a message box with specified text with specified buttons, default button and icon.
		/// Buttons are specified as QMessageBox::StandardButtons (mask of QMessageBox::StandardButtons enum values).
		/// Default button is specified as a one of QMessageBox::StandardButton enum values.
		/// Icon is specified as a one of QMessageBox::Icon enum values.
		/// Return value is QMessageBox::StandardButton enum value.
		///
		/// \warning Do not call this function from <b>PreDrawScript</b> and <b>AfterCreateScript</b> event handlers. This can cause user interface hang.
		int messageBox(QString text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton, QMessageBox::Icon icon, QString details = QString());

		// Variables functions
		//
		/// \brief Returns true if variable specified by name exists, otherwise returns false.
		bool variableExists(QString name) const;

		/// \brief Retrieves a value of the variable specified by name.
		QVariant variable(QString name);

		/// \brief Sets the value of the variable specified by name.
		void setVariable(QString name, const QVariant& value);

		/// \brief Get schema by index. Avoid using this function for performance reason. To get schemas' identifiers and captions use schemaCaptionById, schemaCaptionByIndex, schemaIdByIndex
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
		double zoomFactor() const;

		// Data
		//
	private:
		ClientSchemaView* m_clientSchemaView = nullptr;
		ISchemaViewHistory* m_schemaViewHistory = nullptr;
	};


	//
	// ClientSchemaView
	//
	class ClientSchemaView : public VFrame30::SchemaViewWidget,
							 public IViewVariables
	{
		Q_OBJECT

	public:
		explicit ClientSchemaView(VFrame30::SchemaManager* schemaManager,
								  ISchemaViewHistory* schemaViewHistory,
								  ITimeStats* timeStats,
								  QWidget* parent = nullptr);
		virtual ~ClientSchemaView();

	public:
		void setSchema(QString schemaId);
		void setSchema(QString schemaId, const QStringList& highlightAppSignalIds);

	public:
		bool saveSchemaToPdf(const QString& fileName); // Export schema to PDF or PNG
		bool saveSchemaToPng(const QString& fileName); // Export schema to PDF or PNG

	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void timerEvent(QTimerEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

		virtual void updateScriptGlobalVars(QJSEngine& engine);

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

		const QStringList& highlightIds() const;
		void setHighlightIds(const QStringList& value);

		// TuningController
		//
		TuningController* tuningController();
		const TuningController* tuningController() const;
		void setTuningController(ITuningSignalManager& signalManager,
								 ITuningConnection& tuningConnection,
								 ITuningAuthorization& tuningAuthorization);

		// DiagStateController
		//
		DiagStateController* diagStateController();
		const DiagStateController* diagStateController() const;
		void setDiagStateController(DiagStateController* value);

		//  AppSignalController
		//
		AppSignalController* appSignalController();
		const AppSignalController* appSignalController() const;
		void setAppSignalController(AppSignalController* value);

		//  LogController
		//
		[[nodiscard]] LogController* logController();
		[[nodiscard]] const LogController* logController() const;
		[[nodiscard]] ILogFile* logFile();
		[[nodiscard]] const ILogFile* logFile() const;

		void setLogController(LogController* value);

		// User must provide GlobalScript
		//
		void setGlobalScript(QString value);

		// --
		//
		[[nodiscard]] QJSEngine* jsEngine();

		bool runScript(QJSValue& evaluatedJs, QString where, bool reportError);

		bool runGlobalScriptEvent(const QString& functionName, const QJSValueList& arguments, bool funcIsOptional);

	private:
		bool privateRunGlobalScriptEvent(QJSEngine& engine, const QString& functionName, const QJSValueList& arguments, bool funcIsOptional);

	private:
		bool reEvaluateGlobalScript();
		bool execOnConfigurationArrived();

	public:
		QJSValue evaluateScript(QString script, QString where, bool reportError);
		[[nodiscard]] QString formatScriptError(const QJSValue& scriptValue) const;
		void reportScriptError(const QJSValue& scriptValue, QString where);

		bool scriptMessageBoxAllowed() const;
		bool setScriptMessageBoxAllowed(bool enable);

		// Script timers.
		//
		void scriptStartTimer(int intervalMs, QString timerId);
		void scriptKillTimer(QString timerId);
		void scriptKillAllTimers();

		// IViewVariables implementation
		//
		bool variableExists(const QString& name) const override;

		QVariant variable(const QString& name) const override;
		void setVariable(const QString& name, const QVariant& value) override;

		const QVariantHash& variables() const;
		void setVariables(const QVariantHash& values);

		// TimeStats
		//
		ITimeStats* timeStats();
		ITimeStats* timeStats() const;

		// ClientBehavior
		//
		const MonitorBehavior& monitorBehavior() const noexcept;
		void setMonitorBehavior(const MonitorBehavior& src);
		void setMonitorBehavior(MonitorBehavior&& src);

		const TuningClientBehavior& tuningClientBehavior() const noexcept;
		void setTuningClientBehavior(const TuningClientBehavior& src);
		void setTuningClientBehavior(TuningClientBehavior&& src);

	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;
		VFrame30::ISchemaViewHistory* m_schemaViewHistory = nullptr; // Can be nullptr if widget does not support history navigation

	protected:
		std::unique_ptr<TuningController> m_tuningController;
		
		DiagStateController* m_diagStateController = nullptr;
		std::unique_ptr<ScriptDiagStateController> m_scriptDiagStateController;

		AppSignalController* m_appSignalController = nullptr;
		std::unique_ptr<ScriptAppSignalController> m_scriptAppSignalController;
		
		LogController* m_logController = nullptr;

	private:
		bool m_periodicUpdate = true; // Update widget every 250 ms
		bool m_infoMode = false;      // Show some additional info like labels

		QStringList m_highlightIds;   // Highlighted IDs, can be any, like AppSignalID, ConnectionID... depends on item

		// --
		//
		bool m_jsEngineGlobalsWereCreated = false;
		std::unique_ptr<ScriptSchemaView> m_scriptSchemaView;

		QJSEngine m_jsEngine;

		bool m_allowScriptMessageBox = false;            // Allow or disable using message box in scripts

		QString m_globalScript;

		std::unordered_map<int, QString> m_scriptTimers; // Key is value returned by QObject::startTimer, value is script timerId.

		mutable ITimeStats* m_timeStats = nullptr;

		// --
		//
		std::shared_ptr<SchemaItem> m_leftClickOverItem;
		QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();

		// Variables
		//
		QVariantHash m_variables; // Key is variable name

		// Behaviors
		//
		MonitorBehavior m_monitorBehavior;
		TuningClientBehavior m_tuningClientBehavior;
	};
} // namespace VFrame30
