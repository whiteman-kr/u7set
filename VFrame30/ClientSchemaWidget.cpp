#include "ClientSchemaWidget.h"
#include <QDrag>
#include "LogicSchema.h"
#include "../VFrame30/SchemaItemSignal.h"

namespace VFrame30
{

	SchemaHistoryItem::SchemaHistoryItem(QString schemaId, double zoom, int horzScrollValue, int vertScrollValue) :
		m_schemaId(schemaId),
		m_zoom(zoom),
		m_horzScrollValue(horzScrollValue),
		m_vertScrollValue(vertScrollValue)
	{
	}


	ClientSchemaWidget::ClientSchemaWidget(SchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema, VFrame30::SchemaManager* schemaManager) :
		BaseSchemaWidget(schema, schemaView),
		m_schemaManager(schemaManager)
	{
		assert(schemaView);
		assert(schemaManager);

		// --
		//
		connect(this->schemaView(), &VFrame30::SchemaView::signal_schemaChanged, this, [this](VFrame30::Schema* schema)
			{
				emit this->signal_schemaChanged(this, schema);
			});

		// --
		//
		//connect(monitorSchemaView(), &MonitorView::signal_setSchema, this, &MonitorSchemaWidget::slot_setSchema);

		// Init history
		//
		m_backHistory.push_back(currentHistoryState());
	}

	ClientSchemaWidget::~ClientSchemaWidget()
	{
	}

	void ClientSchemaWidget::mousePressEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::LeftButton) == true)
		{
			m_dragStartPosition = event->pos();
			VFrame30::BaseSchemaWidget::mousePressEvent(event);
		}
		else
		{
			VFrame30::BaseSchemaWidget::mousePressEvent(event);
		}
	}

	void ClientSchemaWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::LeftButton) == false)
		{
			VFrame30::BaseSchemaWidget::mouseMoveEvent(event);
			return;
		}

		// Left button is pressed, is this start of drag and drop
		//
		if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
		{
			VFrame30::BaseSchemaWidget::mouseMoveEvent(event);
			return;
		}

		// Try to start drag and drop
		//
		QPointF docPos = widgetPointToDocument(m_dragStartPosition);

		std::shared_ptr<VFrame30::SchemaItem> schemaItem;

		for (auto layer : schema()->Layers)
		{
			if (layer->compile() == false)
			{
				continue;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemInput"));
			if (schemaItem != nullptr)
			{
				break;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemOutput"));
			if (schemaItem != nullptr)
			{
				break;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemInOut"));
			break;
		}

		if (schemaItem == nullptr)
		{
			return;
		}

		std::shared_ptr<VFrame30::SchemaItemSignal> schemaItemSignal = std::dynamic_pointer_cast<VFrame30::SchemaItemSignal>(schemaItem);
		if (schemaItemSignal == nullptr)
		{
			assert(schemaItemSignal);
			return;
		}

		// Save signals to protobufer
		//
		::Proto::AppSignalSet protoSetMessage;
		QStringList appSignalIds = schemaItemSignal->appSignalIdList();

		for (QString id : appSignalIds)
		{
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			bool need_something_to_do_with_this_theSignals;

//			bool ok = false;
//			AppSignalParam signalParam = theSignals.signalParam(id, &ok);
//			if (ok == false)
//			{
//				continue;
//			}

//			assert(signalParam.appSignalId() == id);

//			::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
//			signalParam.save(protoSignalMessage);
		}

		if (protoSetMessage.appsignal_size() == 0)
		{
			return;
		}

		QByteArray data;
		data.resize(protoSetMessage.ByteSize());

		protoSetMessage.SerializeToArray(data.data(), protoSetMessage.ByteSize());

		// --
		//
		if (data.isEmpty() == false)
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;

			mimeData->setData(AppSignalParamMimeType::value, data);
			drag->setMimeData(mimeData);

			drag->exec(Qt::CopyAction);

			qDebug() << "Start drag for " << appSignalIds;
			qDebug() << "Drag and drop data buffer size " << data.size();
		}

		return;
	}


	std::vector<std::shared_ptr<VFrame30::SchemaItem>> ClientSchemaWidget::itemsUnderCursor(const QPoint& pos)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> result;
		result.reserve(8);

		QPointF docPoint;

		bool convertResult = MousePosToDocPoint(pos, &docPoint);
		if (convertResult == false)
		{
			return result;
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

				if (item->IsIntersectPoint(x, y) == true)
				{
					result.push_back(item);
				}
			}
		}

		return result;
	}



	bool ClientSchemaWidget::canBackHistory() const
	{
		bool enableBack = m_backHistory.empty() == false;
		return enableBack;
	}

	bool ClientSchemaWidget::canForwardHistory() const
	{
		bool enableForward = m_forwardHistory.empty() == false;
		return enableForward;
	}

	void ClientSchemaWidget::historyBack()
	{
		if (m_backHistory.empty() == true)
		{
			return;
		}

		SchemaHistoryItem currentView = m_backHistory.back();
		m_backHistory.pop_back();

		assert(currentView.m_schemaId == schemaId());	// Save current state
		currentView = currentHistoryState();

		m_forwardHistory.push_front(currentView);
		if (m_backHistory.empty() == true)
		{
			return;
		}

		SchemaHistoryItem& restoreItem = m_backHistory.back();
		restoreState(restoreItem);

		emitHistoryChanged();

		return;
	}

	void ClientSchemaWidget::historyForward()
	{
		if (m_forwardHistory.empty() == true)
		{
			return;
		}

		// Save current state
		//
		SchemaHistoryItem& currentView = m_backHistory.back();
		assert(currentView.m_schemaId == schemaId());
		currentView = currentHistoryState();

		// Switch history
		//
		SchemaHistoryItem hi = m_forwardHistory.front();
		m_forwardHistory.pop_front();

		m_backHistory.push_back(hi);

		restoreState(hi);

		emitHistoryChanged();

		return;
	}

	void ClientSchemaWidget::resetHistory()
	{
		m_backHistory.clear();
		m_backHistory.push_back(currentHistoryState());

		m_forwardHistory.clear();

		emitHistoryChanged();

		return;
	}

	void ClientSchemaWidget::resetForwardHistory()
	{
		m_forwardHistory.clear();
		emitHistoryChanged();
	}

	void ClientSchemaWidget::restoreState(const SchemaHistoryItem& historyState)
	{
		if (m_schemaManager == nullptr)
		{
			assert(m_schemaManager);
			return;
		}

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager->schema(historyState.m_schemaId);

		if (schema == nullptr)
		{
			// and there is no startSchemaId (((
			// Just create an empty schema
			//
			schema = std::make_shared<VFrame30::LogicSchema>();
			schema->setSchemaId(historyState.m_schemaId);
			schema->setCaption(historyState.m_schemaId + " not found");
		}

		// --
		//
		setSchema(schema, false);
		setZoom(historyState.m_zoom, false);

		horizontalScrollBar()->setValue(historyState.m_horzScrollValue);
		verticalScrollBar()->setValue(historyState.m_vertScrollValue);

		schemaView()->repaint();

		return;
	}

	SchemaHistoryItem ClientSchemaWidget::currentHistoryState() const
	{
		SchemaHistoryItem hi = {schemaId(), zoom(), horizontalScrollBar()->value(), verticalScrollBar()->value()};
		return hi;
	}

	void ClientSchemaWidget::emitHistoryChanged()
	{
		emit signal_historyChanged(canBackHistory(), canForwardHistory());
		return;
	}

	QString ClientSchemaWidget::schemaId() const
	{
		if (schema() == nullptr)
		{
			return QString();
		}

		return schema()->schemaId();
	}

	QString ClientSchemaWidget::caption() const
	{
		if (schema() == nullptr)
		{
			return QString();
		}

		return schema()->caption();
	}

	VFrame30::SchemaManager* ClientSchemaWidget::schemaManager()
	{
		return m_schemaManager;
	}

}
