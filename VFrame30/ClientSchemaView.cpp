#include "ClientSchemaView.h"
#include "AppSignalController.h"
#include "Context.h"
#include "DrawParam.h"
#include "PropertyNames.h"


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
					QJSEngine::setObjectOwnership(item.get(), QJSEngine::ObjectOwnership::CppOwnership);
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
			// It can happen if the schema was changed in scripts, but this is still the same cycle of draw items.
			//
			qDebug() << "Can't find widget with objectName " << objectName << ", UUID = " << schemaItem->guid().toString();
			return nullptr;
		}

		QJSEngine::setObjectOwnership(widget, QJSEngine::ObjectOwnership::CppOwnership);

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

	void ScriptSchemaView::startTimer(int intervalMs, QString timerId)
	{
		if (m_clientSchemaView == nullptr)
		{
			return;
		}

		return m_clientSchemaView->scriptStartTimer(intervalMs, timerId);
	}

	void ScriptSchemaView::killTimer(QString timerId)
	{
		if (m_clientSchemaView == nullptr)
		{
			return;
		}

		return m_clientSchemaView->scriptKillTimer(timerId);
	}

	void ScriptSchemaView::killAllTimers()
	{
		if (m_clientSchemaView == nullptr)
		{
			return;
		}

		return m_clientSchemaView->scriptKillAllTimers();
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

	void ScriptSchemaView::warningMessageBox(QString text, QString details)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox msgBox{m_clientSchemaView};
			msgBox.setText(text);
			if (details.isEmpty() == false)
			{
				msgBox.setDetailedText(details);
			}
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	void ScriptSchemaView::errorMessageBox(QString text, QString details)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox msgBox{m_clientSchemaView};
			msgBox.setText(text);
			if (details.isEmpty() == false)
			{
				msgBox.setDetailedText(details);
			}
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	void ScriptSchemaView::infoMessageBox(QString text, QString details)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox msgBox{m_clientSchemaView};
			msgBox.setText(text);
			if (details.isEmpty() == false)
			{
				msgBox.setDetailedText(details);
			}
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Information);
			msgBox.exec();
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
		}
		return;
	}

	bool ScriptSchemaView::questionMessageBox(QString text, QString details)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox msgBox{m_clientSchemaView};
			msgBox.setText(text);
			if (details.isEmpty() == false)
			{
				msgBox.setDetailedText(details);
			}
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			msgBox.setIcon(QMessageBox::Question);
			return msgBox.exec() == QMessageBox::Yes;
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
			return false;
		}
	}

	int ScriptSchemaView::messageBox(QString text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton, QMessageBox::Icon icon, QString details)
	{
		if (m_clientSchemaView->scriptMessageBoxAllowed() == true)
		{
			QMessageBox msgBox{m_clientSchemaView};
			msgBox.setText(text);
			if (details.isEmpty() == false)
			{
				msgBox.setDetailedText(details);
			}
			msgBox.setStandardButtons(buttons);
			msgBox.setDefaultButton(defaultButton);
			msgBox.setIcon(icon);
			return msgBox.exec();
		}
		else
		{
			auto l = m_clientSchemaView->logController();
			l->writeWarning(tr("MessageBox is not allowed at current script. Text: ") + text);
			return QMessageBox::No;
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

	double ScriptSchemaView::zoomFactor() const
	{
		return m_clientSchemaView->zoom() / 100.0;
	}

	Behavior::ScriptMonitorBehavior ScriptSchemaView::monitorBehavior() const
	{
		return Behavior::ScriptMonitorBehavior{m_clientSchemaView->monitorBehavior()};
	}


	//
	// ClientSchemaView
	//
	ClientSchemaView::ClientSchemaView(VFrame30::SchemaManager* schemaManager,
									   ISchemaViewHistory* schemaViewHistory,
									   ITimeStats* timeStats,
									   QWidget* parent) :
		VFrame30::SchemaViewWidget(parent),
		m_schemaManager(schemaManager),
		m_schemaViewHistory(schemaViewHistory),
		m_timeStats(timeStats)
	{
		assert(schemaManager);

		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		startRepaintTimer(); // This is a main repaint timer, it fires on the edge of 250ms
		startTimer(1000);    // This is a guard timer

		return;
	}

	ClientSchemaView::~ClientSchemaView()
	{
	}

	void ClientSchemaView::paintEvent(QPaintEvent* paintEvent)
	{
		Q_ASSERT(schema());

		if (m_timeStats != nullptr)
		{
			m_timeStats->clear("ClientSchemaView");
		}
		auto startTime = std::chrono::system_clock::now();

		// Draw schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		if (schema() != nullptr && m_infoMode == false)
		{
			QRect updateRect = paintEvent->rect();
			updateRect.adjust(-logicalDpiX(), -logicalDpiY() / 4, logicalDpiX(), logicalDpiY() / 4); // some space to draw pin names

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

		drawParam.setControlBarSize(CONTROL_BAR(schema()->unit(), p.device()->devicePixelRatioF(), zoom())); // Is required for drawing highlights on items
		drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));                 // 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
		drawParam.setInfoMode(m_infoMode);

		drawParam.setHighlightIds(highlightIds());

		// Draw schema
		//
		SchemaViewWidget::draw(drawParam, clipRect);

		// --
		//
		p.end();

		if (m_timeStats != nullptr)
		{
			using namespace std::chrono;

			auto now = system_clock::now();
			auto ellapsed = duration_cast<microseconds>(now - startTime);

			m_timeStats->addRecord("ClientSchemaView", schema()->schemaId(), "paintEvent", ellapsed);
		}

		return;
	}

	void ClientSchemaView::timerEvent(QTimerEvent* event)
	{
		// Is this a script timer event?
		//
		if (auto timerIt = m_scriptTimers.find(event->timerId());
			timerIt != m_scriptTimers.end())
		{
			const auto& [qtTimerId, scriptTimerId] = *timerIt;

			runGlobalScriptEvent(GlobalScriptEvents::OnTimerEvent, QJSValueList{} << scriptTimerId, false);
			return;
		}

		// Guard timer in case if the main repaint timer has stopped
		//
		if (QDateTime::currentMSecsSinceEpoch() - m_lastRepaintEventFired.toMSecsSinceEpoch() > 500_ms)
		{
			// Something wrong with timer, start it again
			//
			startRepaintTimer();
			return;
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
			VFrame30::SchemaViewWidget::mouseMoveEvent(event); // This will set mouse cursor
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
			// It is scrolling by middle button, let scroll view process it
			//
			VFrame30::SchemaViewWidget::mouseMoveEvent(event); // This will set mouse cursor
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

			for (const auto& item : layer->items() | std::views::reverse)
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
			// It is scrolling by middle button, let scroll view process it
			//
			VFrame30::SchemaViewWidget::mouseMoveEvent(event); // This will set mouse cursor
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
						update();                                          // Repaint screen
						m_leftClickOverItem.reset();
						event->accept();

						VFrame30::SchemaViewWidget::mouseMoveEvent(event); // This will set mouse cursor
						return;
					}
				}
			}

			m_leftClickOverItem.reset();
		}

		VFrame30::SchemaViewWidget::mouseMoveEvent(event); // This will set mouse cursor

		return;
	}

	void ClientSchemaView::updateScriptGlobalVars(QJSEngine& engine)
	{
		// create global variable "view"
		//
		{
			m_scriptSchemaView = std::make_unique<ScriptSchemaView>(this, m_schemaViewHistory);

			QJSEngine::setObjectOwnership(m_scriptSchemaView.get(), QJSEngine::CppOwnership);
			QJSValue jsSchemaView = engine.newQObject(m_scriptSchemaView.get());

			engine.globalObject().setProperty(PropertyNames::scriptGlobalVariableView, jsSchemaView);
		}

		// Create global variable "log"
		//
		if (m_logController != nullptr)
		{
			QJSValue jsLog = engine.newQObject(m_logController);
			QJSEngine::setObjectOwnership(m_logController, QJSEngine::CppOwnership);

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

	bool ClientSchemaView::saveSchemaToPdf(const QString& fileName)
	{
		if (schema() == nullptr)
		{
			Q_ASSERT(schema());
			return false;
		}

		// --
		//
		QPdfWriter pdfWriter(fileName);

		pdfWriter.setTitle(schema()->caption());

		QPageSize pageSize;
		double pageWidth = schema()->docWidth();
		double pageHeight = schema()->docHeight();

		if (schema()->unit() == SchemaUnit::Inch)
		{
			pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
		}
		else
		{
			assert(schema()->unit() == SchemaUnit::Display);
			pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

			pdfWriter.setResolution(72);	// 72 is from enum QPageLayout::Unit help,
			// QPageLayout::Point	1	1/!!! 72th !!!! of an inch
		}

		pdfWriter.setPageSize(pageSize);
		pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

		// --
		//
		QPainter p(&pdfWriter);
		VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

		drawParam.setInfoMode(false);
		drawParam.setPdfMode(true);

		// Calc size
		//
		int widthInPixel = schema()->GetDocumentWidth(pdfWriter.resolution(), 100.0);		// Export 100% zoom
		int heightInPixel = schema()->GetDocumentHeight(pdfWriter.resolution(), 100.0);		// Export 100% zoom

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, schema()->unit(), 0, 0, 100.0);			// Export 100% zoom

		// Draw Schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		schema()->Draw(&drawParam, clipRect);

		// Ending
		//

		return true;
	}

	bool ClientSchemaView::saveSchemaToPng(const QString& fileName)
	{
		if (schema() == nullptr)
		{
			Q_ASSERT(schema());
			return false;
		}

		QPageSize pageSize;
		double pageWidth = schema()->docWidth();
		double pageHeight = schema()->docHeight();

		if (schema()->unit() == SchemaUnit::Inch)
		{
			pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
		}
		else
		{
			assert(schema()->unit() == SchemaUnit::Display);
			pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));
		}

		// Calc size
		//
		const int resolution = 300;	// Image resolution is 300 dpi

		int widthInPixel = schema()->GetDocumentWidth(resolution, 100.0);		// Export 100% zoom
		int heightInPixel = schema()->GetDocumentHeight(resolution, 100.0);		// Export 100% zoom

		// --
		//
		QImage image(QSize(widthInPixel, heightInPixel), QImage::Format_RGB32);

		QPainter p(&image);
		VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

		drawParam.setInfoMode(false);
		drawParam.setPdfMode(true);

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, schema()->unit(), 0, 0, (double)resolution / image.logicalDpiX() * 100.0);			// Export 100% zoom

		// Draw Schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		schema()->Draw(&drawParam, clipRect);

		// Saving
		//
		if (image.save(fileName) == false)
		{
			return false;
		}
		return true;
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

	const QStringList& ClientSchemaView::highlightIds() const
	{
		return m_highlightIds;
	}

	void ClientSchemaView::setHighlightIds(const QStringList& value)
	{
		m_highlightIds = value;
	}

	TuningController* ClientSchemaView::tuningController()
	{
		return m_tuningController.get();
	}

	const TuningController* ClientSchemaView::tuningController() const
	{
		return m_tuningController.get();
	}

	void ClientSchemaView::setTuningController(ITuningSignalManager& signalManager,
											   ITuningConnection& tuningConnection,
											   ITuningAuthorization& tuningAuthorization)
	{
		m_tuningController = std::make_unique<TuningController>(signalManager, tuningConnection, tuningAuthorization, this);
		m_jsEngineGlobalsWereCreated = false; // it will make jsEngine() to initialize global script vars again

		return;
	}

	DiagStateController* ClientSchemaView::diagStateController()
	{
		return m_diagStateController;
	}

	const DiagStateController* ClientSchemaView::diagStateController() const
	{
		return m_diagStateController;
	}

	void ClientSchemaView::setDiagStateController(DiagStateController* value)
	{
		m_diagStateController = value;
		m_scriptDiagStateController = std::make_unique<ScriptDiagStateController>(m_diagStateController->diagStateManager());
		m_jsEngineGlobalsWereCreated = false; // it will make jsEngine() to initialize global script vars again

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
		m_jsEngineGlobalsWereCreated = false; // it will make jsEngine() to initialize global script vars again

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
		m_jsEngineGlobalsWereCreated = false; // it will make jsEngine() to initialize global script vars again
	}

	void ClientSchemaView::setGlobalScript(QString value)
	{
		m_globalScript = value + QChar::LineFeed;
		m_jsEngineGlobalsWereCreated = false; // it will make jsEngine() to initialize global script vars again.

		std::ignore = jsEngine(); // jsEngine() prepares global script and calls execOnConfigurationArrived.

		return;
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
			// Context must have already been set, it should be done after creation of the schema.
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

	bool ClientSchemaView::runGlobalScriptEvent(const QString& functionName, const QJSValueList& arguments, bool funcIsOptional)
	{
		// functionName - Function and event name
		// arguments - arguments to pass to that script function
		// funcIsOptional - function may not be present, do not emit error if it was not found in script.
		//

		if (schema() != nullptr)
		{
			// Context must have already been set, it should be done after creation of the schema.
			//
			Q_ASSERT(schema()->context());
		}

		QJSEngine* engine = jsEngine(); // jsEngine() prepares global script.

		if (engine == nullptr)
		{
			LogController* log = logController();
			if (log != nullptr)
			{
				if (funcIsOptional == true)
				{
					log->writeMessage(tr("Script event %1 occurred but QJSEngine is not available").arg(functionName));
				}
				else
				{
					log->writeWarning(tr("Script event %1 occurred but QJSEngine is not available").arg(functionName));
				}
			}

			return false;
		}

		return privateRunGlobalScriptEvent(*engine, functionName, arguments, funcIsOptional);
	}

	bool ClientSchemaView::privateRunGlobalScriptEvent(QJSEngine& engine, const QString& functionName, const QJSValueList& arguments, bool funcIsOptional)
	{
		LogController* log = logController();

		QJSValue globalObject = engine.globalObject();
		if (globalObject.isNull() == true || globalObject.isUndefined() == true)
		{
			if (log != nullptr)
			{
				if (funcIsOptional == true)
				{
					log->writeMessage(tr("Script event %1 occurred but engine->globalObject() is not available").arg(functionName));
				}
				else
				{
					log->writeWarning(tr("Script event %1 occurred but engine->globalObject() is not available").arg(functionName));
				}
			}

			return false;
		}

		QJSValue onTimerEvent = globalObject.property(functionName);
		if (onTimerEvent.isUndefined() == true)
		{
			if (log != nullptr)
			{
				if (funcIsOptional == true)
				{
					log->writeMessage(tr("Script event %1 occurred but global script function %1 is not defined").arg(functionName));
				}
				else
				{
					log->writeWarning(tr("Script event %1 occurred but global script function %1 is not defined").arg(functionName));
				}
			}

			return false;
		}

		if (onTimerEvent.isCallable() == false)
		{
			if (log != nullptr)
			{
				log->writeError(tr("Script event %1 occurred but %1 is not callable").arg(functionName));
			}

			return false;
		}

		// Run script function
		//
		QJSValue result = onTimerEvent.call(arguments);
		if (engine.isInterrupted())
		{
			if (log != nullptr)
			{
				log->writeWarning(tr("Script event %1 was interrupted").arg(functionName));
			}

			return false;
		}

		// Log errors and exit
		//
		if (result.isError() == true)
		{
			bool prev = setScriptMessageBoxAllowed(false);

			reportScriptError(result, QString("GlobalScript.") + functionName);

			setScriptMessageBoxAllowed(prev);
			return false;
		}

		return true;
	}

	bool ClientSchemaView::reEvaluateGlobalScript()
	{
		qDebug() << "ClientSchemaView::reEvaluateGlobalScript()";

		QJSValue result = m_jsEngine.evaluate(m_globalScript);

		if (result.isError())
		{
			QString err = formatScriptError(result); // it will trace error, must not use any message boxes here, it lead to exception on paint device
			logController()->writeError(tr("Evaluating GlobalScript error:") + err);
		}

		return result.isError() == false;
	}

	bool ClientSchemaView::execOnConfigurationArrived()
	{
		return privateRunGlobalScriptEvent(m_jsEngine, GlobalScriptEvents::OnConfigurationArrived, {}, true);
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

		if (m_allowScriptMessageBox == true)
		{
			QMessageBox::critical(this, QApplication::applicationDisplayName(), message);
		}

		return;
	}

	bool ClientSchemaView::scriptMessageBoxAllowed() const
	{
		return m_allowScriptMessageBox;
	}

	bool ClientSchemaView::setScriptMessageBoxAllowed(bool enable)
	{
		bool prevState = m_allowScriptMessageBox;
		m_allowScriptMessageBox = enable;

		return prevState;
	}

	void ClientSchemaView::scriptStartTimer(int intervalMs, QString timerId)
	{
		scriptKillTimer(timerId);

		int qtTimerId = startTimer(intervalMs);
		m_scriptTimers[qtTimerId] = timerId;
		return;
	}

	void ClientSchemaView::scriptKillTimer(QString timerId)
	{
		for (auto it = m_scriptTimers.begin(); it != m_scriptTimers.end(); ++it)
		{
			const auto& [qtTimerId, scriptTimerId] = *it;

			if (scriptTimerId == timerId)
			{
				killTimer(qtTimerId);
				m_scriptTimers.erase(it);
				return;
			}
		}

		return;
	}

	void ClientSchemaView::scriptKillAllTimers()
	{
		for (const auto& [qtTimerId, scriptTimerId] : m_scriptTimers)
		{
			Q_UNUSED(scriptTimerId);
			killTimer(qtTimerId);
		}

		m_scriptTimers.clear();
		return;
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

	ITimeStats* ClientSchemaView::timeStats()
	{
		return m_timeStats;
	}

	ITimeStats* ClientSchemaView::timeStats() const
	{
		return m_timeStats;
	}

	std::shared_ptr<const Behavior::MonitorBehavior> ClientSchemaView::monitorBehavior() const
	{
		return m_monitorBehavior;
	}

	void ClientSchemaView::setMonitorBehavior(Behavior::MonitorBehavior src)
	{
		m_monitorBehavior = std::make_shared<Behavior::MonitorBehavior>(std::move(src));
	}

	std::shared_ptr<const Behavior::TuningClientBehavior> ClientSchemaView::tuningClientBehavior() const
	{
		return m_tuningClientBehavior;
	}

	void ClientSchemaView::setTuningClientBehavior(Behavior::TuningClientBehavior src)
	{
		m_tuningClientBehavior = std::make_shared<Behavior::TuningClientBehavior>(std::move(src));
	}
} // namespace VFrame30
