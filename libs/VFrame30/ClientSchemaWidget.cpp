#include <VFrame30/ClientSchemaWidget.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/Context.h>
#include <AppSignal.pb.h>

namespace VFrame30
{

	SchemaHistoryItem::SchemaHistoryItem(std::shared_ptr<Schema> schema,
										 const QVariantHash& variables,
										 double zoom,
										 int horzScrollValue,
										 int vertScrollValue) :
		m_schema(schema),
		m_variables(variables),
		m_zoom(zoom),
		m_horzScrollValue(horzScrollValue),
		m_vertScrollValue(vertScrollValue)
	{
		Q_ASSERT(m_schema);
	}


	ClientSchemaWidget::ClientSchemaWidget(ClientSchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema, VFrame30::SchemaManager* schemaManager, QWidget* parent) :
		BaseSchemaWidget(schema, schemaView, parent),
		m_schemaManager(schemaManager)
	{
		Q_ASSERT(schemaView);
		Q_ASSERT(schemaManager);

		// --
		//
		connect(this->schemaView(), &VFrame30::SchemaViewWidget::signal_schemaChanged, this,
			[this](VFrame30::Schema* schema)
			{
				emit this->signal_schemaChanged(this, schema);
			});

		// --
		//
		connect(clientSchemaView(), &VFrame30::ClientSchemaView::signal_setSchema, this, [this](QString schemaId, QStringList highlightIds)
				{
					setSchema(schemaId, highlightIds, false);
				});

		// Init history
		//
		m_backHistory.push_back(currentHistoryState());
	}

	void ClientSchemaWidget::resizeEvent(QResizeEvent* event)
	{
		if (m_zoomMode == ZoomMode::Always100Percent || m_zoomMode == ZoomMode::FitToScreen)
		{
			// Argument zoom() can be any in this case, as set zoom will scale to 100% or to FitToScreen.
			//
			setZoom(zoom(), false, -1 , -1);
		}

		BaseSchemaWidget::resizeEvent(event);
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

		SchemaItemPtr schemaItem;

		for (const auto& layer : schema()->layers())
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
			if (schemaItem != nullptr)
			{
				break;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemValue"));
			if (schemaItem != nullptr)
			{
				break;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemImageValue"));
			if (schemaItem != nullptr)
			{
				break;
			}

			schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemIndicator"));
			if (schemaItem != nullptr)
			{
				break;
			}
		}

		if (schemaItem == nullptr)
		{
			return;
		}

		// Save signals to proto buffer
		//
		::Proto::AppSignalSet protoSetMessage;

		auto p = schemaItem->propertyByCaption(VFrame30::PropertyNames::appSignalIDs);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}

		QStringList appSignalIds = p->value().toString().split(QChar::LineFeed, Qt::SkipEmptyParts);
		if (appSignalIds.isEmpty() == true)
		{
			return;
		}

		AppSignalController* appSignalController = clientSchemaView()->appSignalController();
		if (appSignalController == nullptr)
		{
			Q_ASSERT(appSignalController);
			return;
		}

		for (QString id : appSignalIds)
		{
			bool ok = false;
			AppSignalParam signalParam = appSignalController->signalParam(id, &ok);
			if (ok == false)
			{
				continue;
			}

			Q_ASSERT(signalParam.appSignalId() == id);

			::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
			signalParam.save(protoSignalMessage);
		}

		if (protoSetMessage.appsignal_size() == 0)
		{
			return;
		}

		QByteArray data;
		data.resize(static_cast<int>(protoSetMessage.ByteSizeLong()));

		protoSetMessage.SerializeToArray(data.data(), static_cast<int>(protoSetMessage.ByteSizeLong()));

		// --
		//
		if (data.isEmpty() == false)
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;

			mimeData->setData(AppSignalParamMimeType::value, data);
			drag->setMimeData(mimeData);

			drag->exec(Qt::CopyAction);
		}

		return;
	}


	std::vector<SchemaItemPtr> ClientSchemaWidget::itemsUnderCursor(const QPoint& pos)
	{
		std::vector<SchemaItemPtr> result;
		result.reserve(8);

		QPointF docPoint = widgetPointToDocument(pos);

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
				if (item->isIntersectPoint(x, y) == true)
				{
					result.push_back(item);
				}
			}
		}

		return result;
	}

	bool ClientSchemaWidget::canBackHistory() const
	{
		bool enableBack = m_backHistory.size() > 1;		// m_backHistory has at least 2 items
		return enableBack;
	}

	bool ClientSchemaWidget::canForwardHistory() const
	{
		bool enableForward = m_forwardHistory.empty() == false;
		return enableForward;
	}

	void ClientSchemaWidget::historyBack()
	{
		if (canBackHistory() == false)
		{
			return;
		}

		SchemaHistoryItem currentView = m_backHistory.back();
		m_backHistory.pop_back();

		Q_ASSERT(currentView.m_schema->schemaId() == schemaId());	// Save current state
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
		if (canForwardHistory() == false)
		{
			return;
		}

		// Save current state
		//
		SchemaHistoryItem& currentView = m_backHistory.back();
		if (currentView.m_schema->schemaId() != schemaId())
		{
			Q_ASSERT(currentView.m_schema->schemaId() == schemaId());
			resetHistory();
			return;
		}

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
			Q_ASSERT(m_schemaManager);
			return;
		}

		std::shared_ptr<VFrame30::Schema> schema = historyState.m_schema;

		if (schema == nullptr)
		{
			Q_ASSERT(schema);
			return;
		}

		// --
		//
		BaseSchemaWidget::setSchema(schema, false);

		clientSchemaView()->setHighlightIds({});
		clientSchemaView()->setVariables(historyState.m_variables);

		setZoom(historyState.m_zoom, false);

		horizontalScrollBar()->setValue(historyState.m_horzScrollValue);
		verticalScrollBar()->setValue(historyState.m_vertScrollValue);

		schemaView()->update();

		return;
	}

	SchemaHistoryItem ClientSchemaWidget::currentHistoryState() const
	{
		SchemaHistoryItem hi{schemaSharedPtr(), clientSchemaView()->variables(), zoom(), horizontalScrollBar()->value(), verticalScrollBar()->value()};
		return hi;
	}

	void ClientSchemaWidget::emitHistoryChanged()
	{
		emit signal_historyChanged(canBackHistory(), canForwardHistory());
		return;
	}

	void ClientSchemaWidget::setSchema(QString schemaId, QStringList highlightIds, bool forceSchemaUpdate)
	{
		// forceSchemaUpdate is required only once, when the new configuration arrived and we need to update schema and call onShowScript.
		//
		if (schemaManager() == nullptr)
		{
			Q_ASSERT(schemaManager());
			return;
		}

		// --
		//
		clientSchemaView()->setHighlightIds(highlightIds);

		if (forceSchemaUpdate == false && schemaId == this->schemaId())
		{
			// Highlighted signals already set
			//
			return;
		}

		// Save current state to the history
		//
		resetForwardHistory();

		if (canBackHistory() == false)
		{
			VFrame30::SchemaHistoryItem& currentHistoryItem = m_backHistory.back();		// MUST BE REFERENCE!!!
			Q_ASSERT(currentHistoryItem.m_schema->schemaId() == this->schemaId());

			currentHistoryItem = currentHistoryState();
		}

		// --
		//
		auto context = VFrame30::Context::create(clientSchemaView());

		std::shared_ptr<VFrame30::Schema> schema = schemaManager()->schema(schemaId, context);

		// Run onShowScript
		//
		schema->onShowEvent(clientSchemaView()->jsEngine(), clientSchemaView()->logFile());

		// --
		//
		BaseSchemaWidget::setSchema(schema, false);
		
		if (zoomMode() == VFrame30::ZoomMode::Manual || zoomMode() == VFrame30::ZoomMode::FitToScreen)
		{
			// Initially set zoom to "fit to screen".
			//
			setZoom(0, true);  // Zoom value 0 means adjust schema zoom to fit screen
		}

		// --
		//
		VFrame30::SchemaHistoryItem hi = currentHistoryState();
		m_backHistory.push_back(hi);

		// Limit size of history
		//
		while (m_backHistory.size() > HistorySize)
		{
			m_backHistory.pop_front();
		}

		// --
		//
		emitHistoryChanged();

		return;
	}

	void ClientSchemaWidget::setZoom(double zoom, bool repaint, int horzScrollValue, int vertScrollValue)
	{
		switch (m_zoomMode)
		{
		case ZoomMode::Manual:
			BaseSchemaWidget::setZoom(zoom, repaint, horzScrollValue, vertScrollValue);
			break;
		case ZoomMode::Always100Percent:
			BaseSchemaWidget::setZoom(100.0, repaint, horzScrollValue, vertScrollValue);
			return;
		case ZoomMode::FitToScreen:
			BaseSchemaWidget::setZoom(0, repaint, -1, -1);
			return;
		}

		return;
	}

	QString ClientSchemaWidget::schemaId() const
	{
		if (schema() == nullptr)
		{
			return {};
		}

		return schema()->schemaId();
	}

	QString ClientSchemaWidget::caption() const
	{
		if (schema() == nullptr)
		{
			return {};
		}

		return schema()->caption();
	}

	VFrame30::SchemaManager* ClientSchemaWidget::schemaManager()
	{
		return m_schemaManager;
	}

	VFrame30::ZoomMode ClientSchemaWidget::zoomMode() const
	{
		return m_zoomMode;
	}
	
	void ClientSchemaWidget::setZoomMode(VFrame30::ZoomMode zoomMode, bool repaint)
	{
		Q_UNUSED(repaint);

		m_zoomMode = zoomMode;

		if (m_zoomMode != ZoomMode::Manual)
		{
			setZoom(zoom(), false, -1, -1);
		}

		return;
	}

	ClientSchemaView* ClientSchemaWidget::clientSchemaView()
	{
		ClientSchemaView* v = dynamic_cast<ClientSchemaView*>(schemaView());
		Q_ASSERT(v);
		return v;
	}

	const ClientSchemaView* ClientSchemaWidget::clientSchemaView() const
	{
		const ClientSchemaView* v = dynamic_cast<const ClientSchemaView*>(schemaView());
		Q_ASSERT(v);
		return v;
	}

}
