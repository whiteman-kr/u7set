#pragma once

#include "SchemaView.h"
#include "SchemaManager.h"
#include "TuningController.h"
#include "AppSignalController.h"
#include "SchemaItem.h"

class QPaintEvent;
class QTimerEvent;
class QMouseEvent;

namespace VFrame30
{
	class ClientSchemaView;

	// Proxy class for using in scripts
	//
	class ScriptSchemaView : public QObject
	{
		Q_OBJECT

	public:
		explicit ScriptSchemaView(ClientSchemaView* clientSchemaView, QObject* parent = nullptr);
		~ScriptSchemaView();

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str);					// Debug output to qDebug

		void setSchema(QString schemaId);				// Set schema by SchemaID

		QObject* findSchemaItem(QString objectName);	// Find SchemaItem by ObjectName
		QObject* findWidget(QString objectName);		// Find Widget associated with SchemaItem

		void update();									// Update (redraw) schema view

		// Message Box functions
		//
		void warningMessageBox(QString text);
		void errorMessageBox(QString text);
		void infoMessageBox(QString text);
		bool questionMessageBox(QString text);

		// Variables functions
		//
		bool variableExists(QString name) const;

		QVariant variable(QString name);
		void setVariable(QString name, const QVariant& value);

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

		bool runScript(QJSValue& evaluatedJs, bool reportError);
		QJSValue evaluateScript(QString script, bool reportError);
		QString formatSqriptError(const QJSValue& scriptValue) const;
		void reportSqriptError(const QJSValue& scriptValue);

		// Variables
		//
		bool variableExists(QString name) const;

		QVariant variable(QString name) const;
		void setVariable(QString name, const QVariant& value);

		const QVariantHash& variables() const;
		void setVariables(const QVariantHash& values);

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
	};
}

