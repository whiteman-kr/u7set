#include "ClientSchemaView.h"
#include "SchemaLayer.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "../CommonLib/Times.h"

namespace VFrame30
{
	//
	// ScriptSchemaView
	//
	ScriptSchemaView::ScriptSchemaView(ClientSchemaView* clientSchemaView,
									   ISchemaViewHistory* schemaViewHistory,
									   QObject* parent) :
		QObject(parent),
		m_clientSchemaView(clientSchemaView),
		m_schemaViewHistory(schemaViewHistory)
	{
		assert(m_clientSchemaView);

		// m_schemaViewHistory can be nullptr if widget does not support history (like edit widget)
		//
		return;
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

		for (const auto& layer : schema->layers())
		{
			for (const auto& item : layer->items())
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

	bool ScriptSchemaView::canBackHistory() const
	{
		return m_schemaViewHistory ? m_schemaViewHistory->canBackHistory() : false;
	}

	bool ScriptSchemaView::canForwardHistory() const
	{
		return m_schemaViewHistory ? m_schemaViewHistory->canForwardHistory() : false;
	}

	void ScriptSchemaView::historyBack()
	{
		if (m_schemaViewHistory != nullptr)
		{
			m_schemaViewHistory->historyBack();
		}

		return;
	}

	void ScriptSchemaView::historyForward()
	{
		if (m_schemaViewHistory != nullptr)
		{
			m_schemaViewHistory->historyForward();
		}

		return;
	}

	void ScriptSchemaView::warningMessageBox(QString text)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox::warning(m_clientSchemaView, qAppName(), text);
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	void ScriptSchemaView::errorMessageBox(QString text)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox::critical(m_clientSchemaView, qAppName(), text);
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	void ScriptSchemaView::infoMessageBox(QString text)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox::information(m_clientSchemaView, qAppName(), text);
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	bool ScriptSchemaView::questionMessageBox(QString text)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			int result = QMessageBox::question(m_clientSchemaView, qAppName(), text,  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
			return result == QMessageBox::Yes;
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
			return false;
		}
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

		auto context = Context::create(m_clientSchemaView);
		auto schema = m_clientSchemaView->schemaManager()->schemaByIndex(schemaIndex, std::move(context));

		return schema ? new ScriptSchema(schema) : nullptr;
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

		auto context = VFrame30::Context::create(m_clientSchemaView);

		return new ScriptSchema(m_clientSchemaView->schemaSharedPtr());
	}

	int ScriptSchemaView::schemaCount() const
	{
		return m_clientSchemaView->schemaManager()->schemaCount();
	}


	//
	// ClientSchemaView
	//
	ClientSchemaView::ClientSchemaView(VFrame30::SchemaManager* schemaManager,
									   ISchemaViewHistory* schemaViewHistory,
									   QWidget* parent) :
		VFrame30::SchemaViewWidget(parent),
		m_schemaManager(schemaManager),
		m_schemaViewHistory(schemaViewHistory)
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

	void ClientSchemaView::paintEvent(QPaintEvent* paintEvent)
	{
		// Draw schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		if (schema() != nullptr && m_infoMode  == false)
		{
			QRect updateRect = paintEvent->rect();
			updateRect.adjust(-logicalDpiX(), -logicalDpiY() / 4, logicalDpiX(), logicalDpiY() / 4);	// some space to draw pin names

			QPointF cls;
			QPointF clf;

			bool mouseOk = true;
			mouseOk &= MousePosToDocPoint(updateRect.topLeft(), &cls);
			mouseOk &= MousePosToDocPoint(updateRect.bottomRight(), &clf);

			if (mouseOk == true)
			{
				clipRect.setTopLeft(cls);
				clipRect.setSize({clf.x() - cls.x(), clf.y() - cls.y()});
			}
		}

		// Draw Schema
		//
		QPainter p;
		p.begin(this);

		VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

		drawParam.setControlBarSize(CONTROL_BAR(schema()->unit(), p.device()->devicePixelRatioF(), zoom()));		// Is required for drawing highlights on items
		drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));	// 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
		drawParam.setInfoMode(m_infoMode);

		drawParam.setHightlightIds(hightlightIds());

		// Draw schema
		//
		SchemaViewWidget::draw(drawParam, clipRect);

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

	void ClientSchemaView::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::LeftButton) == true &&
			m_leftClickOverItem != nullptr)
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

			if (m_leftClickOverItem->isIntersectPoint(x, y) == true)
			{
				setCursor(Qt::PointingHandCursor);
			}
			else
			{
				unsetCursor();
			}
		}
		else
		{
			VFrame30::SchemaViewWidget::mouseMoveEvent(event);	// This will set mouse cursor
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

		if (event->buttons().testFlag(Qt::MiddleButton) == true)
		{
			// It is scrolling by midbutton, let scroll view process it
			//
			VFrame30::SchemaViewWidget::mouseMoveEvent(event);	// This will set mouse cursor
			VFrame30::SchemaViewWidget::mousePressEvent(event);
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

		for (const auto& layer : schema()->layers() | std::views::reverse)
		{
			if (layer->show() == false)
			{
				continue;
			}

			for (const auto& item: layer->items() | std::views::reverse)
			{
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
		if (event->button() == Qt::MiddleButton)
		{
			// It is scrolling by midbutton, let scroll view process it
			//
			VFrame30::SchemaViewWidget::mouseMoveEvent(event);	// This will set mouse cursor
			VFrame30::SchemaViewWidget::mouseReleaseEvent(event);
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

			for (const auto& layer : schema()->layers() | std::views::reverse)
			{
				if (layer->show() == false)
				{
					continue;
				}

				for (const auto& item : layer->items() | std::views::reverse)
				{
					if (item == m_leftClickOverItem &&
						item->acceptClick() == true &&
					    item->isIntersectPoint(x, y) == true &&
						item->clickScript().isEmpty() == false)
					{
						// Run script
						//
						bool prev = setScriptMessageBoxAllowed(true);

						item->clickEvent(jsEngine(), this);

						setScriptMessageBoxAllowed(prev);

						if (item->lastScriptError().isEmpty() == false &&
							logController() != nullptr)
						{
							// Report script error to Monitor or TuningClient log
							//
							auto l = logController();
							l->writeWarning(tr("SchemaItem %1, ClickEvent script error: %2")
												.arg(item->label())
												.arg(item->lastScriptError()));
						}

						// --
						//
						update();		// Repaint screen
						m_leftClickOverItem.reset();
						event->accept();

						VFrame30::SchemaViewWidget::mouseMoveEvent(event);	// This will set mouse cursor
						return;
					}
				}
			}

			m_leftClickOverItem.reset();
		}

		VFrame30::SchemaViewWidget::mouseMoveEvent(event);	// This will set mouse cursor

		return;
	}

	void ClientSchemaView::updateScriptGlobalVars(QJSEngine& engine)
	{
		// create global variable "view"
		//
		{
			m_scriptSchemaView = std::make_unique<ScriptSchemaView>(this, m_schemaViewHistory);

			QQmlEngine::setObjectOwnership(m_scriptSchemaView.get(), QQmlEngine::CppOwnership);
			QJSValue jsSchemaView = engine.newQObject(m_scriptSchemaView.get());

			engine.globalObject().setProperty(PropertyNames::scriptGlobalVariableView, jsSchemaView);
		}

		// Create global variable "log"
		//
		{
			QJSValue jsLog = engine.newQObject(m_logController);
			QQmlEngine::setObjectOwnership(m_logController, QQmlEngine::CppOwnership);

			engine.globalObject().setProperty(PropertyNames::scriptGlobalVariableLog, jsLog);
		}

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

	ILogFile* ClientSchemaView::logFile()
	{
		if (m_logController != nullptr)
		{
			return m_logController->logFile();
		}
		else
		{
			return nullptr;
		}
	}

	const ILogFile* ClientSchemaView::logFile() const
	{
		if (m_logController != nullptr)
		{
			return m_logController->logFile();
		}
		else
		{
			return nullptr;
		}
	}

	void ClientSchemaView::setLogController(LogController* value)
	{
		m_logController = value;
		m_jsEngineGlobalsWereCreated = false;	// it will make jsEngine() to initialize global script vars again
	}

	void ClientSchemaView::setGlobalScript(QString value)
	{
		m_globalScript = value + QChar::LineFeed;
		m_jsEngineGlobalsWereCreated = false;
	}

	void ClientSchemaView::setOnConfigurationArrivedScript(QString value)
	{
		m_onConfigurationArrivedScript = std::move(value);
		m_jsEngineGlobalsWereCreated = false;
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
			updateScriptGlobalVars(m_jsEngine);

			// Evaluate global script
			//
			reEvaluateGlobalScript();
			execOnConfigurationArrived();

			// --
			//
			m_jsEngineGlobalsWereCreated = true;
		}

		return &m_jsEngine;
	}

	bool ClientSchemaView::runScript(QJSValue& evaluatedJs, QString where, bool reportError)
	{
		if (evaluatedJs.isUndefined() == true ||
			evaluatedJs.isError() == true)
		{
			return false;
		}

		if (schema() != nullptr)
		{
			// Context must have already been set, it sould be done after creation of the schema.
			//
			Q_ASSERT(schema()->context());
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
		QJSValue result = m_jsEngine.evaluate(m_globalScript);

		if (result.isError())
		{
			QString err = formatScriptError(result);	// it will trace error, must not use any messageboxes here, it lead to exception on paint device
			logController()->writeError(tr("Evaluating GlobalScript error:") + err);
		}

		return result.isError() == false;
	}

	bool ClientSchemaView::execOnConfigurationArrived()
	{
		QJSValue scriptValue = m_jsEngine.evaluate(m_onConfigurationArrivedScript);

		if (scriptValue.isError() == true)
		{
			reportScriptError(scriptValue, "ClientSchemaView::execOnConfigurationArrived()");
			return false;
		}

		if (scriptValue.isUndefined() == true)
		{
			return false;
		}

		return runScript(scriptValue, "run onConfigurationArrivedScript", true);
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

		QString stack = scriptValue.property("stack").toString();

		QString str = QString("Script running uncaught exception at line [%1], Class: [%2], Stack: [%3], Message: [%4]")
					  .arg(scriptValue.property("lineNumber").toInt())
					  .arg(metaObject()->className())
					  .arg(stack)
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

		if (logController() != nullptr)
		{
			logController()->writeError(message);
		}

		QMessageBox::critical(this, QApplication::applicationDisplayName(), message);

		return;
	}

	bool ClientSchemaView::scriptMessageBoxAllowed() const
	{
		return m_alloScriptMessageBox;
	}

	bool ClientSchemaView::setScriptMessageBoxAllowed(bool enable)
	{
		bool prevState = m_alloScriptMessageBox;
		m_alloScriptMessageBox = enable;

		return prevState;
	}

	bool ClientSchemaView::variableExists(const QString& name) const
	{
		return m_variables.contains(name);
	}

	QVariant ClientSchemaView::variable(const QString& name) const
	{
		return m_variables.value(name);
	}

	void ClientSchemaView::setVariable(const QString& name, const QVariant& value)
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

	const MonitorBehavior& ClientSchemaView::monitorBehavor() const noexcept
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

	const TuningClientBehavior& ClientSchemaView::tuningClientBehavior() const noexcept
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
