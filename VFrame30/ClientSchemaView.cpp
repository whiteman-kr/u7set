#include "ClientSchemaView.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "../lib/TimeStamp.h"

namespace VFrame30
{
	//
	// ScriptSchemaView
	//
	ScriptSchemaView::ScriptSchemaView(ClientSchemaView* clientSchemaView, QObject* parent) :
		QObject(parent),
		m_clientSchemaView(clientSchemaView)
	{
		assert(m_clientSchemaView);
		qDebug() << "ScriptSchemaView::ScriptSchemaView";
		return;
	}

	ScriptSchemaView::~ScriptSchemaView()
	{
		qDebug() << "ScriptSchemaView::~ScriptSchemaView";
	}

	void ScriptSchemaView::debugOutput(QString str)
	{
		qDebug() << str;
	}

	void ScriptSchemaView::setSchema(QString schemaId)
	{
		if (m_clientSchemaView == nullptr)
		{
			return;
		}

		// We can't change schema here, because we need to save history, so emit signal and change schema
		// in ClientSchemaWidget
		//
		m_clientSchemaView->setSchema(schemaId);
		return;
	}

	QObject* ScriptSchemaView::findSchemaItem(QString objectName)
	{
		if (m_clientSchemaView == nullptr)
		{
			return nullptr;
		}

		auto schema = m_clientSchemaView->schema();
		if (schema == nullptr)
		{
			return nullptr;
		}

		for (auto layer : schema->Layers)
		{
			for (auto item : layer->Items)
			{
				if (item->objectName() == objectName)
				{
					QQmlEngine::setObjectOwnership(item.get(), QQmlEngine::ObjectOwnership::CppOwnership);
					return item.get();
				}
			}
		}

		return nullptr;
	}

	QObject* ScriptSchemaView::findWidget(QString objectName)
	{
		if (objectName.trimmed().isEmpty() == true)
		{
			return nullptr;
		}

		QObject* itemObject = findSchemaItem(objectName);
		if (itemObject == nullptr)
		{
			return nullptr;
		}

		SchemaItem* schemaItem = dynamic_cast<SchemaItem*>(itemObject);
		if (schemaItem == nullptr)
		{
			assert(schemaItem);
			return nullptr;
		}

		QWidget* widget = m_clientSchemaView->findChild<QWidget*>(schemaItem->guid().toString());
		if (widget == nullptr)
		{
			qDebug() << "Can't find widget with UUID = " << schemaItem->guid().toString();
			assert(widget);
			return nullptr;
		}

		QQmlEngine::setObjectOwnership(widget, QQmlEngine::ObjectOwnership::CppOwnership);

		return widget;
	}

	void ScriptSchemaView::update()
	{
		if (m_clientSchemaView == nullptr)
		{
			return;
		}

		m_clientSchemaView->update();
		return;
	}

	void ScriptSchemaView::warningMessageBox(QString text)
	{
		QMessageBox::warning(m_clientSchemaView, qAppName(), text);
		return;
	}

	void ScriptSchemaView::errorMessageBox(QString text)
	{
		QMessageBox::critical(m_clientSchemaView, qAppName(), text);
		return;
	}

	void ScriptSchemaView::infoMessageBox(QString text)
	{
		QMessageBox::information(m_clientSchemaView, qAppName(), text);
		return;
	}

	bool ScriptSchemaView::questionMessageBox(QString text)
	{
		int result = QMessageBox::question(m_clientSchemaView, qAppName(), text);
		return result == QMessageBox::Yes;
	}

	bool ScriptSchemaView::variableExists(QString name) const
	{
		return m_clientSchemaView->variableExists(name);
	}

	QVariant ScriptSchemaView::variable(QString name)
	{
		return m_clientSchemaView->variable(name);
	}

	void ScriptSchemaView::setVariable(QString name, const QVariant& value)
	{
		m_clientSchemaView->setVariable(name, value);
	}

	QObject* ScriptSchemaView::schemaByIndex(int schemaIndex)
	{
		if (schemaIndex < 0)
		{
			return nullptr;
		}

		auto s = m_clientSchemaView->schemaManager()->schemaByIndex(schemaIndex);

		return s ? new ScriptSchema(s) : nullptr;
	}

	QString ScriptSchemaView::schemaCaptionById(const QString& schemaId) const
	{
		return m_clientSchemaView->schemaManager()->schemaCaptionById(schemaId);
	}

	QString ScriptSchemaView::schemaCaptionByIndex(int schemaIndex) const
	{
		return m_clientSchemaView->schemaManager()->schemaCaptionByIndex(schemaIndex);
	}

	QString ScriptSchemaView::schemaIdByIndex(int schemaIndex) const
	{
		return m_clientSchemaView->schemaManager()->schemaIdByIndex(schemaIndex);
	}

	QString ScriptSchemaView::schemaId() const
	{
		return m_clientSchemaView->schema()->schemaId();
	}

	QString ScriptSchemaView::schemaCaption() const
	{
		return m_clientSchemaView->schema()->caption();
	}

	QObject* ScriptSchemaView::schema()
	{
		if (m_clientSchemaView == nullptr)
		{
			return nullptr;
		}

		return new ScriptSchema(m_clientSchemaView->schema());
	}

	int ScriptSchemaView::schemaCount() const
	{
		return m_clientSchemaView->schemaManager()->schemaCount();
	}


	//
	// ClientSchemaView
	//
	ClientSchemaView::ClientSchemaView(VFrame30::SchemaManager* schemaManager, QWidget* parent) :
		VFrame30::SchemaView(parent),
		m_schemaManager(schemaManager)
	{
		assert(schemaManager);

		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		startRepaintTimer();	// This is a main repaint timer, it fires on the edge of 250ms
		startTimer(1000);		// This is a guard timer

		return;
	}

	ClientSchemaView::~ClientSchemaView()
	{
	}

	void ClientSchemaView::paintEvent(QPaintEvent*)
	{
		// Draw Schema
		//
		QPainter p;
		p.begin(this);

		p.save();

		VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());

		drawParam.setControlBarSize(schema()->unit() == VFrame30::SchemaUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));		// Is required for drawing highlights on items
		drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));	// 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
		drawParam.setEditMode(false);

		drawParam.setAppSignalController(m_appSignalController);
		drawParam.setTuningController(m_tuningController);
		drawParam.setInfoMode(m_infoMode);

		drawParam.setHightlightIds(hightlightIds());

		// Draw schema
		//
		SchemaView::draw(drawParam);

		// Calc size
		//
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, 0, 0, zoom());

		// Draw elements highlighted by its AppSignalId
		//
		//drawHighlights();

		// --
		//
		p.restore();

		// --
		//
		p.end();

		return;
	}

	void ClientSchemaView::timerEvent(QTimerEvent*)
	{
		// Guard timer in case if the main repaint timer has stopped
		//
		if (QDateTime::currentMSecsSinceEpoch() - m_lastRepaintEventFired.toMSecsSinceEpoch() > 500_ms)
		{
			// Something wrong with timer, start it again
			//
			startRepaintTimer();
		}

		return;
	}

	void ClientSchemaView::mousePressEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::RightButton) == true)
		{
			// Ignore event
			//
			event->ignore();
			return;
		}

		if (event->buttons().testFlag(Qt::MidButton) == true)
		{
			// It is scrolling by midbutton, let scroll view process it
			//
			VFrame30::SchemaView::mousePressEvent(event);
			return;
		}

		// Find is there any item under the cursor with AcceptClick
		//
		m_leftClickOverItem.reset();

		QPointF docPoint;

		bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
		if (convertResult == false)
		{
			event->ignore();
			return;
		}

		double x = docPoint.x();
		double y = docPoint.y();

		for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
		{
			const VFrame30::SchemaLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const SchemaItemPtr& item = *vi;

				if (item->acceptClick() == true &&
				    item->isIntersectPoint(x, y) == true &&
					item->clickScript().isEmpty() == false)
				{
					// Remember this item
					//
					m_leftClickOverItem = item;
					event->accept();
					return;
				}
			}
		}

		// Ignore event
		//
		event->ignore();
		return;
	}

	void ClientSchemaView::mouseReleaseEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MidButton) == true)
		{
			// It is scrolling by midbutton, let scroll view process it
			//
			VFrame30::SchemaView::mouseReleaseEvent(event);
			return;
		}

		// Find is there any item under the cursor with AcceptClick
		//
		if (m_leftClickOverItem != nullptr)
		{
			QPointF docPoint;

			bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
			if (convertResult == false)
			{
				event->ignore();
				return;
			}

			double x = docPoint.x();
			double y = docPoint.y();

			for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
			{
				const VFrame30::SchemaLayer* pLayer = layer->get();

				if (pLayer->show() == false)
				{
					continue;
				}

				for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
				{
					const SchemaItemPtr& item = *vi;

					if (item == m_leftClickOverItem &&
						item->acceptClick() == true &&
					    item->isIntersectPoint(x, y) == true &&
						item->clickScript().isEmpty() == false)
					{
						// Run script
						//
						item->clickEvent(jsEngine(), this);

						// --
						//
						update();		// Repaint screen
						unsetCursor();
						m_leftClickOverItem.reset();
						event->accept();
						return;
					}
				}
			}

			m_leftClickOverItem.reset();
		}

		// Ignore event
		//
		unsetCursor();
		event->ignore();
		return;
	}


	void ClientSchemaView::startRepaintTimer()
	{
		if (m_periodicUpdate == true)
		{
			update();
		}

		// Set this timer in the edge of 250ms
		//
		int64_t currentMs = QTime::currentTime().msec();
		int64_t ms = (currentMs / 250 + 1) * 250 - currentMs;

		QTimer::singleShot(ms, this, &ClientSchemaView::startRepaintTimer);
		m_lastRepaintEventFired = QDateTime::currentDateTime();

		return;
	}

	void ClientSchemaView::setSchema(QString schemaId)
	{
		return setSchema(schemaId, {});
	}

	void ClientSchemaView::setSchema(QString schemaId, const QStringList& highlightAppSignalIds)
	{
		// We can't change schema here, because we need to save history, so emit signal and change schema
		// in ClientSchemaWidget
		//
		emit signal_setSchema(schemaId, highlightAppSignalIds);
		return;
	}

	VFrame30::SchemaManager* ClientSchemaView::schemaManager()
	{
		return m_schemaManager;
	}

	const VFrame30::SchemaManager* ClientSchemaView::schemaManager() const
	{
		return m_schemaManager;
	}

	bool ClientSchemaView::periodicUpdate() const
	{
		return m_periodicUpdate;
	}

	void ClientSchemaView::setPeriodicUpdate(bool value)
	{
		m_periodicUpdate = value;
	}

	bool ClientSchemaView::infoMode() const
	{
		return m_infoMode;
	}

	void ClientSchemaView::setInfoMode(bool value)
	{
		m_infoMode = value;
	}

	const QStringList& ClientSchemaView::hightlightIds() const
	{
		return m_highlightIds;
	}

	void ClientSchemaView::setHighlightIds(const QStringList& value)
	{
		m_highlightIds = value;
	}

	TuningController* ClientSchemaView::tuningController()
	{
		return m_tuningController;
	}

	const TuningController* ClientSchemaView::tuningController() const
	{
		return m_tuningController;
	}

	void ClientSchemaView::setTuningController(TuningController* value)
	{
		m_tuningController = value;
		m_jsEngineGlobalsWereCreated = false;	// it will make jsEngine() to initialize global script vars again

		return;
	}

	AppSignalController* ClientSchemaView::appSignalController()
	{
		return m_appSignalController;
	}

	const AppSignalController* ClientSchemaView::appSignalController() const
	{
		return m_appSignalController;
	}

	void ClientSchemaView::setAppSignalController(AppSignalController* value)
	{
		m_appSignalController = value;
		m_scriptAppSignalController = std::make_unique<ScriptAppSignalController>(m_appSignalController->appSignalManager());
		m_jsEngineGlobalsWereCreated = false;	// it will make jsEngine() to initialize global script vars again

		return;
	}

	LogController* ClientSchemaView::logController()
	{
		return m_logController;
	}

	const LogController* ClientSchemaView::logController() const
	{
		return m_logController;
	}

	void ClientSchemaView::setLogController(LogController* value)
	{
		m_logController = value;
		m_jsEngineGlobalsWereCreated = false;	// it will make jsEngine() to initialize global script vars again
	}

	QJSEngine* ClientSchemaView::jsEngine()
	{
		if (m_schemaManager == nullptr)
		{
			Q_ASSERT(m_schemaManager);
			return nullptr;
		}

		if (m_jsEngineGlobalsWereCreated == false)
		{
			// create global variable "view"
			//
			m_scriptSchemaView = std::make_unique<ScriptSchemaView>(this);

			{
				QQmlEngine::setObjectOwnership(m_scriptSchemaView.get(), QQmlEngine::CppOwnership);
				QJSValue jsSchemaView = m_jsEngine.newQObject(m_scriptSchemaView.get());

				m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableView, jsSchemaView);
			}

			// create global variable "tuning"
			//
			{
				QJSValue jsTuning = m_jsEngine.newQObject(m_tuningController);
				QQmlEngine::setObjectOwnership(m_tuningController, QQmlEngine::CppOwnership);

				m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableTuning, jsTuning);
			}

			// Create global variable "signals"
			//
			{
				QJSValue jsSignals = m_jsEngine.newQObject(m_scriptAppSignalController.get());
				QQmlEngine::setObjectOwnership(m_scriptAppSignalController.get(), QQmlEngine::CppOwnership);

				m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableSignals, jsSignals);
			}

			// Create global variable "log"
			//
			{
				QJSValue jsLog = m_jsEngine.newQObject(m_logController);
				QQmlEngine::setObjectOwnership(m_logController, QQmlEngine::CppOwnership);

				m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableLog, jsLog);
			}

			// Evaluate global script
			//
			reEvaluateGlobalScript();

			// --
			//
			m_jsEngineGlobalsWereCreated = true;
		}

		return &m_jsEngine;
	}

	QString ClientSchemaView::globalScript() const
	{
		return m_schemaManager->globalScript();
	}

	bool ClientSchemaView::runScript(QJSValue& evaluatedJs, QString where, bool reportError)
	{
		if (evaluatedJs.isUndefined() == true ||
			evaluatedJs.isError() == true)
		{
			return false;
		}

		// Run script
		//
		QJSValue jsResult = evaluatedJs.call();
		if (jsResult.isError() == true)
		{
			if (reportError == true)
			{
				reportScriptError(jsResult, where);
			}

			return false;
		}

		return true;
	}

	bool ClientSchemaView::reEvaluateGlobalScript()
	{
		QJSValue result = m_jsEngine.evaluate(globalScript());

		if (result.isError())
		{
			formatScriptError(result);	// it will trace error, must not use any messageboxes here, it lead to exception on paint device
		}

		return result.isError() == false;
	}

	QJSValue ClientSchemaView::evaluateScript(QString script, QString where, bool reportError)
	{
		QJSValue result = jsEngine()->evaluate(script);

		if (result.isError() == true && reportError == true)
		{
			reportScriptError(result, where);
		}

		return result;
	}

	QString ClientSchemaView::formatScriptError(const QJSValue& scriptValue) const
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tClass: " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QString str = QString("Script running uncaught exception at line %1\n"
							  "\tClass: %2 %3\n"
							  "\tStack: %4\n"
							  "\tMessage: %5")
					  .arg(scriptValue.property("lineNumber").toInt())
					  .arg(metaObject()->className())
					  .arg(scriptValue.property("stack").toString())
					  .arg(scriptValue.toString());

		return str;
	}

	void ClientSchemaView::reportScriptError(const QJSValue& scriptValue, QString where)
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tClass: " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QString message = tr("Script (%1) uncaught exception at line %2:\n%3")
						  .arg(where)
						  .arg(scriptValue.property("lineNumber").toInt())
						  .arg(scriptValue.toString());

		QMessageBox::critical(this, QApplication::applicationDisplayName(), message);

		return;
	}

	bool ClientSchemaView::variableExists(QString name) const
	{
		return m_variables.contains(name);
	}

	QVariant ClientSchemaView::variable(QString name) const
	{
		return m_variables.value(name);
	}

	void ClientSchemaView::setVariable(QString name, const QVariant& value)
	{
		m_variables[name] = value;
	}

	const QVariantHash& ClientSchemaView::variables() const
	{
		return m_variables;
	}

	void ClientSchemaView::setVariables(const QVariantHash& values)
	{
		m_variables = values;
	}

	const MonitorBehavior& ClientSchemaView::monitorBehavor() const
	{
		return m_monitorBehavior;
	}

	void ClientSchemaView::setMonitorBehavior(const MonitorBehavior& src)
	{
		m_monitorBehavior = src;
	}

	void ClientSchemaView::setMonitorBehavior(MonitorBehavior&& src)
	{
		m_monitorBehavior = std::move(src);
	}

	const TuningClientBehavior& ClientSchemaView::tuningClientBehavior() const
	{
		return m_tuningClientBehavior;
	}

	void ClientSchemaView::setTuningClientBehavior(const TuningClientBehavior& src)
	{
		m_tuningClientBehavior = src;
	}

	void ClientSchemaView::setTuningClientBehavior(TuningClientBehavior&& src)
	{
		m_tuningClientBehavior = std::move(src);
	}
}
