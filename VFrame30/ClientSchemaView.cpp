#include "ClientSchemaView.h"
#include <QPainter>
#include <QQmlEngine>
#include <QMessageBox>
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

		QWidget* widget = findChild<QWidget*>(schemaItem->guid().toString());
		if (widget == nullptr)
		{
			qDebug() << "Can't find widget with UUID = " << schemaItem->guid().toString();
			//assert(widget);
			return nullptr;
		}

		QQmlEngine::setObjectOwnership(widget, QQmlEngine::ObjectOwnership::CppOwnership);

		return widget;
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

	//
	// ClientSchemaView
	//
	ClientSchemaView::ClientSchemaView(VFrame30::SchemaManager* schemaManager, QWidget* parent) :
		VFrame30::SchemaView(parent),
		m_schemaManager(schemaManager)
	{
		assert(schemaManager);

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

		drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));	// 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
		drawParam.setEditMode(false);

		drawParam.setAppSignalController(m_appSignalController);
		drawParam.setTuningController(m_tuningController);
		drawParam.setInfoMode(m_infoMode);

		// Draw schema
		//
		SchemaView::draw(drawParam);

		// Calc size
		//
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, 0, 0, zoom());

		// --
		//
		//drawMovingEdgesOrVertexConnectionLine(&drawParam);

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
				const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

				if (item->acceptClick() == true &&
					item->IsIntersectPoint(x, y) == true &&
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
					const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

					if (item == m_leftClickOverItem &&
						item->acceptClick() == true &&
						item->clickScript().trimmed().isEmpty() == false &&
						item->IsIntersectPoint(x, y) == true &&
						item->clickScript().isEmpty() == false)
					{
						// Run script
						//
						item->clickEvent(globalScript(), jsEngine(), this);

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
		update();

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
		// We can't change schema here, because we need to save history, so emit signal and change schema
		// in ClientSchemaWidget
		//
		emit signal_setSchema(schemaId);

		return;
	}

	bool ClientSchemaView::infoMode() const
	{
		return m_infoMode;
	}

	void ClientSchemaView::setInfoMode(bool value)
	{
		m_infoMode = value;
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
		m_jsEngineGlobalsWereCreated = false;	// it will make jsEngine() to initialize global script vars again

		return;
	}

	QJSEngine* ClientSchemaView::jsEngine()
	{
		if (m_jsEngineGlobalsWereCreated == false)
		{
			ScriptSchemaView* schemaViewObject = new ScriptSchemaView(this);	// Auto ownership, no delete
			QJSValue jsSchemaView = m_jsEngine.newQObject(schemaViewObject);
			m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableView, jsSchemaView);

			QJSValue jsTuning = m_jsEngine.newQObject(m_tuningController);
			QQmlEngine::setObjectOwnership(m_tuningController, QQmlEngine::CppOwnership);
			m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableTuning, jsTuning);

			QJSValue jsSignals = m_jsEngine.newQObject(m_appSignalController);
			QQmlEngine::setObjectOwnership(m_appSignalController, QQmlEngine::CppOwnership);
			m_jsEngine.globalObject().setProperty(PropertyNames::scriptGlobalVariableSignals, jsSignals);

			m_jsEngineGlobalsWereCreated = true;
		}

		return &m_jsEngine;
	}

	QString ClientSchemaView::globalScript() const
	{
		return m_schemaManager->globalScript();
	}

}
