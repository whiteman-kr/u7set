#include "ClientSchemaView.h"
#include <QPainter>
#include "DrawParam.h"
#include "../lib/TimeStamp.h"

namespace VFrame30
{

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
bool what_to_do_with_theSignals_in_next_two_lines;
		//drawParam.setAppSignalManager(&theSignals);
		//drawParam.setInfoMode(theSettings.showItemsLabels());

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


	QString ClientSchemaView::globalScript() const
	{
		if (m_schemaManager == nullptr)
		{
			assert(m_schemaManager);
			return QString();
		}

		return m_schemaManager->globalScript();
	}

	QJSEngine* ClientSchemaView::jsEngine()
	{
		bool addSignalManager = false;
		if (m_jsEngineGlobalsWereCreated == false)
		{
			addSignalManager = true;
		}

		QJSEngine* result = VFrame30::SchemaView::jsEngine();

bool what_to_do_with_ScriptSignalManager;
bool i_do_not_like_that_jsEngine_is_virtual_here;
//		if (addSignalManager == true)
//		{
//			ScriptSignalManager* signalManager = new ScriptSignalManager(&theSignals);
//			QJSValue jsValue = m_jsEngine.newQObject(signalManager);

//			result->globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableSignals, jsValue);
//		}

		return result;
	}

}
