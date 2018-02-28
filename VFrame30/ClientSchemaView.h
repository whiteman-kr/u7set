#ifndef CLIENTSCHEMAVIEW_H
#define CLIENTSCHEMAVIEW_H

#include <memory>
#include <QJSEngine>
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

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str);					// Debug output to qDebug
		void setSchema(QString schemaId);				// Set schema by SchemaID

		QObject* findSchemaItem(QString objectName);	// Find SchemaItem by ObjectName
		QObject* findWidget(QString objectName);		// Find Widget associated with SchemaItem

		// Message Box functions
		//
		void warningMessageBox(QString text);
		void errorMessageBox(QString text);
		void infoMessageBox(QString text);
		bool questionMessageBox(QString text);

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

	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void timerEvent(QTimerEvent* event) override;

		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

	protected slots:
		void startRepaintTimer();

	signals:
		void signal_setSchema(QString schemaId);

		// Properties
		//
	public:
		bool infoMode() const;
		void setInfoMode(bool value);

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

	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;

		TuningController* m_tuningController = nullptr;
		AppSignalController* m_appSignalController = nullptr;

		QJSEngine m_jsEngine;

		bool m_infoMode = false;			// Show some aditional info like labels

		// --
		//
		bool m_jsEngineGlobalsWereCreated = false;

		std::shared_ptr<SchemaItem> m_leftClickOverItem;
		QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();
	};


}

#endif // CLIENTSCHEMAVIEW_H
