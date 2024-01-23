#include "Schema.h"
#include "SchemaLayer.h"
#include "LogicSchema.h"
#include "UfbSchema.h"
#include "MonitorSchema.h"
#include "TuningSchema.h"
#include "DiagSchema.h"
#include "ClientSchemaView.h"
#include "FblItem.h"
#include "SchemaItemAfb.h"
#include "SchemaItemUfb.h"
#include "SchemaItemBus.h"
#include "SchemaItemLink.h"
#include "SchemaItemConnection.h"
#include "SchemaItemLoopback.h"
#include "SchemaItemIndicator.h"
#include "HorzVertLinks.h"
#include "DrawParam.h"
#include "PropertyNames.h"


namespace VFrame30
{
	::Factory<VFrame30::Schema> SchemaFactory;

	//
	// ScriptSchema
	//
	ScriptSchema::ScriptSchema(std::shared_ptr<Schema> schema) :
		m_schema(std::move(schema))
	{
		Q_ASSERT(m_schema);
	}

	bool ScriptSchema::isLogicSchema() const
	{
		return m_schema ? m_schema->isLogicSchema() : false;
	}

	bool ScriptSchema::isUfbSchema() const
	{
		return m_schema ? m_schema->isUfbSchema() : false;
	}

	bool ScriptSchema::isMonitorSchema() const
	{
		return m_schema ? m_schema->isMonitorSchema() : false;
	}


	bool ScriptSchema::isTuningSchema() const
	{
		return m_schema ? m_schema->isTuningSchema() : false;
	}

	bool ScriptSchema::isDiagSchema() const
	{
		return m_schema ? m_schema->isDiagSchema() : false;
	}

	QJSValue ScriptSchema::layer(int index)
	{
		QJSValue result;

		if (m_schema == nullptr || index < 0 || index >= layerCount())
		{
			return result;
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return result;
		}

		auto layer = m_schema->layers()[index];
		Q_ASSERT(layer->parentSchema() == m_schema.get());

		result = engine->newQObject(new ScriptSchemaLayer{layer});

		return result;
	}

	QJSValue ScriptSchema::layer(QString caption)
	{
		QJSValue result;

		if (m_schema == nullptr)
		{
			return result;
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return result;
		}

		for (const auto& l : m_schema->layers())
		{
			if (l->name() == caption)
			{
				result = engine->newQObject(new ScriptSchemaLayer(l));
				break;
			}
		}

		return result;
	}

	QVariantList ScriptSchema::itemsByTag(QString tag)
	{
		QVariantList result;
		result.reserve(8);

		if (m_schema == nullptr)
		{
			return result;
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return result;
		}

		// --
		//
		for (const auto& layer : m_schema->layers())
		{
			for (const auto& item : layer->items())
			{
				if (item->hasTag(tag) == true)
				{
					result.push_back(QVariant::fromValue<VFrame30::SchemaItem*>(item.get()));
					QQmlEngine::setObjectOwnership(item.get(), QQmlEngine::CppOwnership);
				}
			}
		}

		return result;
	}

	QJSValue ScriptSchema::findSchemaItem(QString objectName)
	{
		if (m_schema == nullptr)
		{
			return QJSValue::NullValue;
		}

		QJSEngine* engine = qjsEngine(this);
		if (engine == nullptr)
		{
			Q_ASSERT(engine);
			return QJSValue::NullValue;
		}

		for (const auto& layer : m_schema->layers())
		{
			for (const auto& item : layer->items())
			{
				if (item->objectName() == objectName)
				{
					QJSValue v = engine->newQObject(item.get());
					QQmlEngine::setObjectOwnership(item.get(), QQmlEngine::ObjectOwnership::CppOwnership);

					return v;
				}
			}
		}

		return QJSValue::NullValue;;
	}

	QString ScriptSchema::schemaId() const
	{
		return m_schema ? m_schema->schemaId() : QString{};
	}

	QString ScriptSchema::caption() const
	{
		return m_schema ? m_schema->caption() : QString{};
	}

	QColor ScriptSchema::backgroundColor() const
	{
		return m_schema ? m_schema->backgroundColor() : QColor{};
	}

	void ScriptSchema::setBackgroundColor(QColor value)
	{
		if (m_schema != nullptr)
		{
			m_schema->setBackgroundColor(value);
		}

		return;
	}

	int ScriptSchema::layerCount() const
	{
		return m_schema ?
			static_cast<int>(m_schema->layers().size()) :
			0;
	}

	//
	// Schema
	//
	Schema::Schema(void)
	{
		Init();
	}

	Schema::~Schema(void)
	{
		clearLayers();	// It sets no parent to layers.
	}

	void Schema::Init(void)
	{
		auto schemaIdProp = ADD_PROPERTY_GETTER_SETTER(QString, "SchemaID", true, Schema::schemaId, Schema::setSchemaId);
		schemaIdProp->setValidator("^[A-Za-z\\d_]{1,64}$");

		ADD_PROPERTY_GETTER(int, "Changeset", true, Schema::changeset);
		ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, Schema::caption, Schema::setCaption);

		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::tags, true, Schema::tagsAsString, Schema::setTags)
			->setSpecificEditor(E::PropertySpecificEditor::Tags);

		ADD_PROPERTY_GETTER_SETTER(bool, "ExcludeFromBuild", true, Schema::excludeFromBuild, Schema::setExcludeFromBuild);
		ADD_PROPERTY_GETTER_SETTER(double, "SchemaWidth", true, Schema::docWidthRegional, Schema::setDocWidthRegional);
		ADD_PROPERTY_GETTER_SETTER(double, "SchemaHeight", true, Schema::docHeightRegional, Schema::setDocHeightRegional);
		ADD_PROPERTY_GETTER_SETTER(QColor, "BackgroundColor", true, Schema::backgroundColor, Schema::setBackgroundColor);

		// Monitor properties
		//
		ADD_PROPERTY_GET_SET_CAT(bool, "JoinHorzPriority", "Monitor", true, Schema::joinHorzPriority, Schema::setJoinHorzPriority)
			->setViewOrder(100);
		ADD_PROPERTY_GET_SET_CAT(QString, "JoinLeftSchemaID", "Monitor", true, Schema::joinLeftSchemaId, Schema::setJoinLeftSchemaId)
			->setViewOrder(101);
		ADD_PROPERTY_GET_SET_CAT(QString, "JoinTopSchemaID", "Monitor", true, Schema::joinTopSchemaId, Schema::setJoinTopSchemaId)
			->setViewOrder(102);
		ADD_PROPERTY_GET_SET_CAT(QString, "JoinRightSchemaID", "Monitor", true, Schema::joinRightSchemaId, Schema::setJoinRightSchemaId)
			->setViewOrder(103);
		ADD_PROPERTY_GET_SET_CAT(QString, "JoinBottomSchemaID", "Monitor", true, Schema::joinBottomSchemaId, Schema::setJoinBottomSchemaId)
			->setViewOrder(104);

		addProperty<QString, Schema, &Schema::preDrawScript, &Schema::setPreDrawScript>(PropertyNames::preDrawScript, PropertyNames::scriptsCategory, true)
			->setIsScript(true);

		addProperty<QString, Schema, &Schema::onShowScript, &Schema::setOnShowScript>(PropertyNames::onShowScript, PropertyNames::scriptsCategory, true)
			->setIsScript(true);

		m_guid = QUuid();  // GUID_NULL

		m_width = 0;
		m_height = 0;

		return;
	}

	// Serialization
	//
	bool Schema::SaveData(Proto::Envelope* message) const
	{
		//		// Set new uuids and labels to the schema
		//		//
		//		Schema* sss = const_cast<Schema*>(this);
		//		sss->setGuid(QUuid::createUuid());

		//		for (auto layer : sss->Layers)
		//		{
		//			layer->setGuid(QUuid::createUuid());

		//			for (SchemaItemPtr item : layer->Items)
		//			{
		//				item->setNewGuid();

		//				if (item->isFblItemRect() == true)
		//				{
		//static int counterValue = 18000;
		//					//int counterValue = //m_db->nextCounterValue();
		//					item->toFblItemRect()->setLabel(schemaId() + "_" + QString::number(counterValue++));
		//				}
		//			}
		//		}

		std::string className = this->metaObject()->className();
		quint32 classnamehash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnamehash);	// Required field, class name hash code, by it instance is created

		Proto::Schema* mutableSchema = message->mutable_schema();

		Proto::Write(mutableSchema->mutable_uuid(), m_guid);
		Proto::Write(mutableSchema->mutable_schemaid(), m_schemaID);
		Proto::Write(mutableSchema->mutable_caption(), m_caption);

		mutableSchema->set_tags(tagsAsString().toStdString());

		mutableSchema->set_joinhorzpriority(m_joinHorzPriority);
		mutableSchema->set_joinleftschemaid(m_joinLeftSchemaId.toStdString());
		mutableSchema->set_jointopschemaid(m_joinTopSchemaId.toStdString());
		mutableSchema->set_joinrightschemaid(m_joinRightSchemaId.toStdString());
		mutableSchema->set_joinbottomschemaid(m_joinBottomSchemaId.toStdString());

		mutableSchema->set_width(m_width);
		mutableSchema->set_height(m_height);
		mutableSchema->set_unit(static_cast<Proto::SchemaUnit>(m_unit));
		mutableSchema->set_excludefrombuild(m_excludeFromBuild);
		mutableSchema->set_backgroundcolor(m_backgroundColor.rgba());

		mutableSchema->set_predrawscript(m_preDrawScript.toStdString());
		mutableSchema->set_onshowscript(m_onShowScript.toStdString());

		// Save Layers
		//
		bool saveLayersResult = true;

		for (const auto& layer : layers())
		{
			Proto::Envelope* pLayerMessage = mutableSchema->add_layers();
			saveLayersResult &= layer->Save(pLayerMessage);
		}

		// Save fake empty Afb Collection, keep for compatibility
		//
		Afb::AfbElementCollection fakeAfbCollection;
		fakeAfbCollection.SaveData(mutableSchema->mutable_afbs());

		return saveLayersResult;
	}

	bool Schema::LoadData(const Proto::Envelope& message)
	{
		//		qDebug() << Q_FUNC_INFO;
		//		qDebug() << "        Start loading Schema....";

		//		QTime t;
		//		t.start();

		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return false;
		}

		const Proto::Schema& schema = message.schema();

		m_guid = Proto::Read(schema.uuid());
		Proto::Read(schema.schemaid(), &m_schemaID);
		Proto::Read(schema.caption(), &m_caption);

		if (schema.has_tags() == true)	// if schema does not have saved tags, then default values are taken from the each schema type constructor
		{
			setTags(QString::fromStdString(schema.tags()));
		}

		m_joinHorzPriority = schema.joinhorzpriority();
		m_joinLeftSchemaId = QString::fromStdString(schema.joinleftschemaid());
		m_joinTopSchemaId = QString::fromStdString(schema.jointopschemaid());
		m_joinRightSchemaId = QString::fromStdString(schema.joinrightschemaid());
		m_joinBottomSchemaId = QString::fromStdString(schema.joinbottomschemaid());

		m_width = schema.width();
		m_height = schema.height();
		setUnit(static_cast<SchemaUnit>(schema.unit()));
		m_excludeFromBuild = schema.excludefrombuild();

		if (schema.has_backgroundcolor() == true)
		{
			m_backgroundColor = schema.backgroundcolor();
		}

		m_preDrawScript = QString::fromStdString(schema.predrawscript());
		m_onShowScript = QString::fromStdString(schema.onshowscript());

		// Layers
		//
		clearLayers();

		for (int i = 0; i < schema.layers().size(); i++)
		{
			std::shared_ptr<SchemaLayer> layer = SchemaLayer::Create(schema.layers(i));

			if (layer == nullptr)
			{
				assert(layer);
				continue;
			}

			addLayer(layer);

			if (layer->compile() == true)
			{
				m_activeLayer = i;
			}
		}

		if (schema.layers().size() != std::ssize(m_layers))
		{
			assert(schema.layers().size() == std::ssize(m_layers));
			clearLayers();
			return false;
		}

		//		int elapsed = t.elapsed();
		//		qDebug() << "        Schema " << schemaId() << " is loaded for " << elapsed << " ms";

		return true;
	}

	std::shared_ptr<Schema> Schema::CreateObject(const Proto::Envelope& message)
	{
		// This function can create only one instance
		//
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<Schema> schema = SchemaFactory.Create(classNameHash);

		if (schema == nullptr)
		{
			assert(schema);
			return nullptr;
		}

		schema->LoadData(message);

		return schema;
	}

	void Schema::Draw(CDrawParam* drawParam, const QRectF& clipRect)
	{
		if (drawParam == nullptr ||
			context() == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(context());
			return;
		}

		// Start stats
		//
		if (drawParam->timeStats() != nullptr)
		{
			drawParam->timeStats()->clear("Schema", schemaId());
		}
		auto startTime = std::chrono::system_clock::now();

		// --
		//
		ClientSchemaView* clientView = drawParam->drawMode() == DrawMode::Editor ?
			nullptr :
			drawParam->clientSchemaView();

		ILogFile* log = context()->log();

		if (clientView != nullptr)
		{
			// Start stats
			//
			auto startTimePreDraw = std::chrono::system_clock::now();

			// Monitor or Simulator
			//
			bool mbe = clientView->setScriptMessageBoxAllowed(false);

			this->preDrawEvent(clientView->jsEngine());

			clientView->setScriptMessageBoxAllowed(mbe);

			// Collect stats
			//
			if (drawParam->timeStats() != nullptr)
			{
				using namespace std::chrono;
				auto now = system_clock::now();
				auto ellapsed = duration_cast<microseconds>(now - startTimePreDraw);
				drawParam->timeStats()->addRecord("Schema", schemaId(), "preDrawEvent", ellapsed);
			}
		}

		// Clear client area by "grey" color.
		//
		QPainter* p = drawParam->painter();

		// ---
		//
		if (drawParam->pdfMode() == false)
		{
			QRectF pageRect(-0.1, -0.1, docWidth() + 0.1, docHeight() + 0.1); // +/- some space to avoid single line not filled.
			p->fillRect(pageRect, backgroundColor());
		}
		else
		{
			QRectF pageRect(0, 0, docWidth(), docHeight());
			p->fillRect(pageRect, backgroundColor());
		}

		// Draw items by layers which has Show flag
		//
		double clipX = clipRect.left();
		double clipY = clipRect.top();
		double clipWidth = clipRect.width();
		double clipHeight = clipRect.height();

		QElapsedTimer timer;
		timer.start();

		bool isClientMode = clientView != nullptr;

		// DrawParam requires for preDrawScript and for draw itself
		// Set it for all items in schema, it allows to cross use of items.
		//
		auto setDrawParam = [this](CDrawParam* drawParam)
		{
			for (const SchemaLayerPtr& layer : layers())
			{
				std::ranges::for_each(layer->items(), [drawParam](auto& item)
				{
					item->setDrawParam(drawParam);
				});
			}
		};

		setDrawParam(drawParam);

		std::shared_ptr<void> finalizer(nullptr,
			[&setDrawParam](void*)
			{
				setDrawParam(nullptr);
			});

		for (const SchemaLayerPtr& layer : layers())
		{
			Q_ASSERT(layer);

			if (layer->show() == false)
			{
				continue;
			}

			if (drawParam->drawNotesLayer() == false &&
				layer->name().compare(QLatin1String("Notes"), Qt::CaseInsensitive) == 0)
			{
				continue;
			}

			for (const auto& item : layer->items())
			{
				Q_ASSERT(item);

				// Start stats
				//
				if (drawParam->timeStats() != nullptr)
				{
					drawParam->timeStats()->clear(schemaId(), item->label());
				}

				if (isClientMode == true && item->visible() == false)
				{
					continue;
				}

				// --
				//
				if (isClientMode == true && item->isCommented() == false)
				{
					auto startTimePreDraw = std::chrono::system_clock::now();

					// Call preDrawEvent for all items, even if they out of screen
					// Some items preDrawEvents may have script for caching reasons
					//
					bool mbe = clientView->setScriptMessageBoxAllowed(false);

					item->preDrawEvent(clientView->jsEngine());

					clientView->setScriptMessageBoxAllowed(mbe);

					if (item->lastScriptError().isEmpty() == false &&
						log != nullptr)
					{
						// Report script error to Monitor or TuningClient log
						//
						log->writeWarning(tr("SchemaItem %1, PreDrawEvent script error: %2")
										  .arg(item->label())
										  .arg(item->lastScriptError()));
					}

					// Collect stats
					//
					if (drawParam->timeStats() != nullptr)
					{
						using namespace std::chrono;
						auto now = system_clock::now();
						auto elapsed = duration_cast<microseconds>(now - startTimePreDraw);
						drawParam->timeStats()->addRecord(schemaId(), item->label(), "preDrawEvent", elapsed);
					}
				}

				if (item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					// Start stats
					//
					auto startTimeDraw = std::chrono::system_clock::now();

					// Drawing item is here.
					//
					item->draw(drawParam);

					if (item->isCommented() == true)
					{
						item->drawCommentDim(drawParam);
					}

					if (drawParam->infoMode() == true)
					{
						item->drawLabel(drawParam);
					}

					// Draw lastScriptError after drawing item
					//
					if (item->lastScriptError().isEmpty() == false)
					{
						item->drawScriptError(drawParam);
					}

					// Collect stats
					//
					if (drawParam->timeStats() != nullptr)
					{
						using namespace std::chrono;
						auto now = system_clock::now();
						auto elapsed = duration_cast<microseconds>(now - startTimeDraw);
						drawParam->timeStats()->addRecord(schemaId(), item->label(), "draw", elapsed);
					}
				}
			}

			// Draw highlighted items after drawing the layer.
			//
			for (const auto& item : layer->items())
			{
				Q_ASSERT(item);

				if (item->isType<PosRectImpl>() == false || (isClientMode == true && item->visible() == false))
				{
					continue;
				}

				if (item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					item->toType<PosRectImpl>()->drawHighlight(drawParam);
				}
			}
		}

		if (isClientMode == true)
		{
			drawScriptError(drawParam);
		}

		// Collect stats
		//
		if (drawParam->timeStats() != nullptr)
		{
			using namespace std::chrono;

			auto now = system_clock::now();
			auto elapsed = duration_cast<microseconds>(now - startTime);

			drawParam->timeStats()->addRecord("Schema", schemaId(), "Draw", elapsed);
		}

#if 0
		thread_local std::list<qint64> elapsedAverage;

		elapsedAverage.push_back(timer.elapsed());
		while (elapsedAverage.size() > 20)
		{
			elapsedAverage.pop_front();
		}

		qDebug() << "Schema::Draw " << elapsedAverage.back() <<
			", average " << std::accumulate(elapsedAverage.begin(), elapsedAverage.end(), 0) / elapsedAverage.size();
#endif

		return;
	}

	int Schema::GetDocumentWidth(double DpiX, double zoom) const
	{
		if (unit() == SchemaUnit::Display)
		{
			return static_cast<int>(docWidth() * (zoom / 100.0));
		}
		else
		{
			return static_cast<int>(docWidth() * DpiX * (zoom / 100.0));
		}
	}

	int Schema::GetDocumentHeight(double DpiY, double zoom) const
	{
		if (unit() == SchemaUnit::Display)
		{
			return static_cast<int>(docHeight() * (zoom / 100.0));
		}
		else
		{
			return static_cast<int>(docHeight() * DpiY * (zoom / 100.0));
		}
	}

	void Schema::BuildFblConnectionMap() const
	{
		CHorzVertLinks horzVertLinks;

		for (const auto& layer : layers())
		{
			for (const auto& item : layer->items())
			{
				if (item->isFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item.get());
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

				SchemaItemLink* schemaItemLink = dynamic_cast<SchemaItemLink*>(item.get());
				if (schemaItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemaItemLink->GetPointList();

					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}

					horzVertLinks.AddLinks(pointList, schemaItemLink->guid());
				}
			}
		}

		// --
		//
		for (auto layer : layers())
		{
			layer->connectionMap.clear();

			for (const auto& item : layer->items())
			{
				if (item->isFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item.get());

				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

				SchemaItemLink* schemaItemLink = dynamic_cast<SchemaItemLink*>(item.get());

				if (schemaItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemaItemLink->GetPointList();

					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}

					layer->ConnectionMapPosInc(pointList.front());
					layer->ConnectionMapPosInc(pointList.back());

					if (horzVertLinks.IsPointOnLink(pointList.front(), schemaItemLink->guid()) == true)
					{
						layer->ConnectionMapPosInc(pointList.front());
					}

					if (horzVertLinks.IsPointOnLink(pointList.back(), schemaItemLink->guid()) == true)
					{
						layer->ConnectionMapPosInc(pointList.back());
					}

					continue;
				}

				pFblItem->SetConnectionsPos(gridSize(), pinGridStep());

				const std::vector<AfbPin>& inputs = pFblItem->inputs();
				for (auto pin = inputs.begin(); pin != inputs.end(); ++pin)
				{
					SchemaPoint pinPos = pin->point();

					layer->ConnectionMapPosInc(pinPos);

					if (horzVertLinks.IsPinOnLink(pinPos, item->guid()) == true)
					{
						layer->ConnectionMapPosInc(pinPos);
					}
				}

				const std::vector<AfbPin>& outputs = pFblItem->outputs();
				for (auto pin = outputs.begin(); pin != outputs.end(); ++pin)
				{
					SchemaPoint pinPos = pin->point();

					layer->ConnectionMapPosInc(pinPos);

					if (horzVertLinks.IsPinOnLink(pinPos, item->guid()) == true)
					{
						layer->ConnectionMapPosInc(pinPos);
					}
				}
			}
		}

		return;
	}

	bool Schema::updateAllSchemaItemFbs(const std::vector<std::shared_ptr<Afb::AfbElement>>& afbs, int* updatedItemCount, QString* errorMessage)
	{
		if (updatedItemCount == nullptr ||
			errorMessage == nullptr)
		{
			assert(updatedItemCount);
			assert(errorMessage);
			return false;
		}

		*updatedItemCount = 0;

		// Find all VFrame30::SchemaItemAfb items
		//
		std::list<std::shared_ptr<VFrame30::SchemaItemAfb>> schemaAfbItems;

		for (const std::shared_ptr<SchemaLayer>& l : layers())
		{
			for (const auto& si : l->items())
			{
				std::shared_ptr<VFrame30::SchemaItemAfb> schemaAfbItem = std::dynamic_pointer_cast<VFrame30::SchemaItemAfb>(si);

				if (schemaAfbItem != nullptr && schemaAfbItem->isCommented() == false)
				{
					schemaAfbItems.push_back(schemaAfbItem);
				}
			}
		}

		// Update found items
		//
		for (std::shared_ptr<VFrame30::SchemaItemAfb> si : schemaAfbItems)
		{
			auto foundIt = std::find_if(afbs.begin(), afbs.end(),
				[&si](const std::shared_ptr<Afb::AfbElement>& afb)
				{
					return si->afbStrID() == afb->strID();
				});

			if (foundIt == afbs.end())
			{
				*errorMessage += tr("Cant find AFB description file for %1.\n").arg(si->afbStrID());
				continue;
			}

			const Afb::AfbElement& afb = *foundIt->get();

			if (si->afbElement().version() != afb.version())
			{
				bool ok = si->updateAfbElement(afb, errorMessage);

				if (ok == true)
				{
					(*updatedItemCount) ++;
				}
			}
		}

		return errorMessage->isEmpty();
	}

	bool Schema::updateAllSchemaItemUfb(const std::vector<std::shared_ptr<UfbSchema>>& ufbs, int* updatedItemCount, QString* errorMessage)
	{
		if (updatedItemCount == nullptr ||
			errorMessage == nullptr)
		{
			assert(updatedItemCount);
			assert(errorMessage);
			return false;
		}

		*updatedItemCount = 0;

		// Find all VFrame30::SchemaItemAfb items
		//
		std::list<std::shared_ptr<VFrame30::SchemaItemUfb>> schemaUfbItems;

		for (const std::shared_ptr<SchemaLayer>& l : layers())
		{
			for (const auto& si : l->items())
			{
				std::shared_ptr<VFrame30::SchemaItemUfb> schemaUfbItem = std::dynamic_pointer_cast<VFrame30::SchemaItemUfb>(si);

				if (schemaUfbItem != nullptr && schemaUfbItem->isCommented() == false)
				{
					schemaUfbItems.push_back(schemaUfbItem);
				}
			}
		}

		// Update found items
		//
		for (std::shared_ptr<VFrame30::SchemaItemUfb> si : schemaUfbItems)
		{
			auto foundIt = std::find_if(ufbs.begin(), ufbs.end(),
				[&si](const std::shared_ptr<UfbSchema>& ufb)
				{
					return si->ufbSchemaId() == ufb->schemaId();
				});

			if (foundIt == ufbs.end())
			{
				*errorMessage += tr("Cant find UFB schema for %1.\n").arg(si->ufbSchemaId());
				continue;
			}

			const UfbSchema* ufb = foundIt->get();

			if (si->ufbSchemaVersion() != ufb->version())
			{
				bool ok = si->updateUfbElement(ufb, errorMessage);

				if (ok == true)
				{
					(*updatedItemCount) ++;
				}
			}
		}

		return errorMessage->isEmpty();
	}

	bool Schema::updateAllSchemaItemBusses(const std::vector<Bus>& busses, int* updatedItemCount, QString* errorMessage)
	{
		if (updatedItemCount == nullptr ||
			errorMessage == nullptr)
		{
			assert(updatedItemCount);
			assert(errorMessage);
			return false;
		}

		*updatedItemCount = 0;

		// Find all VFrame30::SchemaItemBus items
		//
		std::vector<std::shared_ptr<VFrame30::SchemaItemBus>> schemaItemBusses;

		for (const std::shared_ptr<SchemaLayer>& l : layers())
		{
			for (const auto& si : l->items())
			{
				std::shared_ptr<VFrame30::SchemaItemBus> schemaItemBus = std::dynamic_pointer_cast<VFrame30::SchemaItemBus>(si);

				if (schemaItemBus != nullptr && schemaItemBus->isCommented() == false)
				{
					schemaItemBusses.push_back(schemaItemBus);
				}
			}
		}

		// Update found items
		//
		for (std::shared_ptr<VFrame30::SchemaItemBus> si : schemaItemBusses)
		{
			auto foundIt = std::find_if(busses.begin(), busses.end(),
				[&si](const Bus& bus)
				{
					return si->busTypeId() == bus.busTypeId();
				});

			if (foundIt == busses.end())
			{
				*errorMessage += tr("Cant find BusType %1. Schema %2.\n").arg(si->busTypeId()).arg(si->parentSchema()->schemaId());
				continue;
			}

			const Bus& bus = *foundIt;

			if (si->busTypeHash() != bus.calcHash())
			{
				si->setBusType(bus);
				si->adjustHeight(gridSize(), pinGridStep());

				(*updatedItemCount) ++;
			}
		}

		return errorMessage->isEmpty();
	}

	QStringList Schema::getSignalList() const
	{
		std::set<QString> signalMap;	// signal ids can be duplicated, std::set removes duplicates.

		for (const auto& layer : layers())
		{
			// Get all signals
			//
			for (const auto& item : layer->items())
			{
				IMatsSchemaItemAssociations* itemAssociations = item->toType<IMatsSchemaItemAssociations>();
				if (itemAssociations == nullptr)
				{
					continue;
				}

				for (auto appSignals = itemAssociations->associatedAppSignalIds();
					 const QString & id : appSignals)
				{
					signalMap.insert(id);
				}

				for (auto appSignals = itemAssociations->associatedImpactAppSignalIds();
					 const QString & id : appSignals)
				{
					signalMap.insert(id);
				}
			}
		}

		// Move set to list
		//
		QStringList result;
		result.reserve(static_cast<qsizetype>(signalMap.size()));

		for (const QString& id : signalMap)
		{
			result.append(id);
		}

		return result;
	}

	QStringList Schema::getLabels() const
	{
		QStringList labels;
		labels.reserve(1024);

		for (const std::shared_ptr<SchemaLayer>& layer : layers())
		{
			for (const auto& item : layer->items())
			{
				QString itemLabel = item->label();

				if (itemLabel.isEmpty() == false)
				{
					labels.append(itemLabel);
				}
			}
		}

		return labels;
	}

	std::vector<QUuid> Schema::getGuids() const
	{
		std::vector<QUuid> result;
		result.reserve(2048);

		for (const std::shared_ptr<SchemaLayer>& layer : layers())
		{
			for (const auto& item : layer->items())
			{
				result.push_back(item->guid());
			}
		}

		return result;
	}

	QStringList Schema::itemTags() const
	{
		QSet<QString> tags;

		for (const std::shared_ptr<SchemaLayer>& layer : layers())
		{
			for (const auto& item : layer->items())
			{
				for (QStringList itemTags = item->tagsAsList();
					 const auto& tag : itemTags)
				{
					tags.insert(tag);
				}
			}
		}

		return tags.values();
	}

	QString Schema::details(const QString& path) const
	{
		return SchemaDetails::getDetailsString(this, path);
	}

	SchemaDetails Schema::parseDetails(const QString& details)
	{
		return SchemaDetails{details};
	}

	std::shared_ptr<SchemaItem> Schema::getItemById(const QUuid& id) const
	{
		for (const std::shared_ptr<VFrame30::SchemaLayer>& layer : layers())
		{
			std::shared_ptr<SchemaItem> result = layer->getItemById(id);

			if (result != nullptr)
			{
				return result;
			}
		}

		return {};
	}

	template<typename SchemaItemType>
	bool Schema::hasSchemaItemType() const
	{
		for (const std::shared_ptr<VFrame30::SchemaLayer>& layer : layers())
		{
			for (const SchemaItemPtr& item : layer->items())
			{
				if (dynamic_cast<SchemaItemType>(item) != nullptr)
				{
					return true;
				}
			}
		}

		return false;
	}

	// Scripting
	//
	bool Schema::preDrawEvent(QJSEngine* engine)
	{
		if (engine == nullptr ||
			context() == nullptr)
		{
			Q_ASSERT(engine);
			Q_ASSERT(context());
			return false;
		}

		if (m_preDrawScript.trimmed().isEmpty() == true)
		{
			return true;
		}

		// Evaluate script
		//
		if (m_jsPreDrawScript.isUndefined() == true || qHash(m_preDrawScript) != m_evaluatedPreDrawScript)
		{
			m_jsPreDrawScript = evaluateScript(m_preDrawScript, engine, nullptr);
			m_evaluatedPreDrawScript = qHash(m_preDrawScript);
		}

		if (m_jsPreDrawScript.isError() == true ||
			m_jsPreDrawScript.isNull() == true)
		{
			return false;
		}

		bool result = runScript(m_jsPreDrawScript, engine);

		if (m_lastScriptError.isEmpty() == false && context()->log() != nullptr)
		{
			context()->log()->writeWarning(tr("Schema %1, preDrawEvent script error: %2")
							  .arg(schemaId())
							  .arg(m_lastScriptError));
		}

		return result;
	}

	bool Schema::onShowEvent(QJSEngine* engine, ILogFile* log)
	{
		if (engine == nullptr ||
			context() == nullptr)
		{
			Q_ASSERT(engine);
			Q_ASSERT(context());
			return false;
		}

		if (m_onShowScript.trimmed().isEmpty() == true)
		{
			return true;
		}

		// Evaluate script
		//
		if (m_jsOnShowScript.isUndefined() == true || qHash(m_onShowScript) != m_evaluatedOnShowScript)
		{
			m_jsOnShowScript = evaluateScript(m_onShowScript, engine, nullptr);
			m_evaluatedOnShowScript = qHash(m_preDrawScript);
		}

		if (m_jsOnShowScript.isError() == true ||
			m_jsOnShowScript.isNull() == true)
		{
			return false;
		}

		bool result = runScript(m_jsOnShowScript, engine);

		if (m_lastScriptError.isEmpty() == false && log != nullptr)
		{
			log->writeWarning(tr("Schema %1, ShowEvent script error: %2")
							  .arg(schemaId())
							  .arg(m_lastScriptError));
		}

		return result;
	}

	bool Schema::runScript(QJSValue& evaluatedJs, QJSEngine* engine)
	{
		if (evaluatedJs.isUndefined() == true ||
			evaluatedJs.isError() == true ||
			engine == nullptr ||
			context() == nullptr)
		{
			Q_ASSERT(engine);
			Q_ASSERT(context());
			return false;
		}

		// Set argument list
		//
		QJSValueList args;
		args << engine->newQObject(new ScriptSchema(this->shared_from_this()));

		// Run script
		//
		m_lastScriptError.clear();

		try
		{
			QJSValue jsResult = evaluatedJs.call(args);

			if (jsResult.isError() == true)
			{
				m_lastScriptError = formatSqriptError(jsResult);
			}
		}
		catch (std::exception& e)
		{
			m_lastScriptError = QString::fromStdString(e.what());
		}

		return m_lastScriptError.isEmpty();
	}

	QJSValue Schema::evaluateScript(QString script, QJSEngine* engine, QWidget* parentWidget) const
	{
		if (engine == nullptr)
		{
			assert(engine);
			assert(parentWidget);
			return {};
		}

		QJSValue result = engine->evaluate(script);

		if (result.isError() == true)
		{
			m_lastScriptError = formatSqriptError(result);

			if (parentWidget != nullptr)
			{
				QMessageBox::critical(parentWidget, qAppName(), m_lastScriptError);
			}
		}

		return result;
	}

	QString Schema::formatSqriptError(const QJSValue& scriptValue) const
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tItem: " << guid().toString() << " " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QString str = QString("Script running uncaught exception at line %1\n"
							  "\tItem: %2 %3\n"
							  "\tStack: %4\n"
							  "\tMessage: %5")
			.arg(scriptValue.property("lineNumber").toInt())
			.arg(guid().toString()).arg(metaObject()->className())
			.arg(scriptValue.property("stack").toString())
			.arg(scriptValue.toString());

		return str;
	}

	void Schema::reportSqriptError(const QJSValue& scriptValue, QWidget* parent) const
	{
		qDebug() << "Script running uncaught exception at line " << scriptValue.property("lineNumber").toInt();
		qDebug() << "\tItem: " << guid().toString() << " " << metaObject()->className();
		qDebug() << "\tStack: " << scriptValue.property("stack").toString();
		qDebug() << "\tMessage: " << scriptValue.toString();

		QMessageBox::critical(parent, QApplication::applicationDisplayName(),
							  tr("Script uncaught exception at line %1:\n%2")
								  .arg(scriptValue.property("lineNumber").toInt())
								  .arg(scriptValue.toString()));

		return;
	}

	void Schema::drawScriptError(CDrawParam* drawParam) const
	{
		if (m_lastScriptError.isEmpty() == true)
		{
			return;
		}

		QRectF pageRect{0, 0, 1, 1};
		static FontParam font(QStringLiteral("Arial"), drawParam->gridSize() * 1.75, false, false);

		QPainter* p = drawParam->painter();
		p->setPen(Qt::red);

		DrawHelper::drawText(p,
							 font,
							 unit(),
							 m_lastScriptError,
							 pageRect,
							 Qt::TextDontClip | Qt::AlignTop | Qt::AlignLeft);

		return;
	}

	// Properties and Data
	//

	const std::vector<std::shared_ptr<SchemaLayer>>& Schema::layers() const
	{
#ifdef QT_DEBUG
		for (const auto& layer : m_layers)
		{
			Q_ASSERT(layer->parentSchema() == this);
		}
#endif
		return m_layers;
	}

	int Schema::activeLayerIndex() const
	{
		return m_activeLayer;
	}

	QUuid Schema::activeLayerGuid() const
	{
		Q_ASSERT(m_layers.at(m_activeLayer)->parentSchema() == this);

		try
		{
			return m_layers.at(m_activeLayer)->guid();
		}
		catch (...)
		{
			Q_ASSERT(false);
			return {};
		}
	}

	std::shared_ptr<VFrame30::SchemaLayer> Schema::activeLayer() const
	{
		Q_ASSERT(m_layers.at(m_activeLayer)->parentSchema() == this);

		try
		{
			return m_layers.at(m_activeLayer);
		}
		catch (...)
		{
			assert(false);
			return {};
		}
	}

	void Schema::setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		Q_ASSERT(layer->parentSchema() == this);

		for (int index = 0;
			 const auto& l : m_layers)
		{
			Q_ASSERT(l->parentSchema() == this);

			if (l == layer)
			{
				m_activeLayer = index;
				return;
			}

			index ++;
		}

		// Layer was not found
		//
		assert(false);
		return;
	}

	void Schema::clearLayers()
	{
		std::ranges::for_each(m_layers, [](const auto& l)
		{
			l->setParentSchema({});
		});
		m_layers.clear();
	}

	void Schema::addLayer(std::shared_ptr<SchemaLayer> layer)
	{
		layer->setParentSchema(this);
		m_layers.push_back(layer);

		return;
	}

	// Guid
	//
	QUuid Schema::guid() const
	{
		return m_guid;
	}

	void Schema::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// SchemaID
	//
	QString Schema::schemaId() const
	{
		return m_schemaID;
	}

	void Schema::setSchemaId(const QString& id)
	{
		if (id.trimmed().isEmpty() == true)
		{
			return;
		}

		if (m_schemaID != id)
		{
			// Update item labels.
			//
			QString startWith = QString{"%1_"}.arg(m_schemaID);
			QString newPrefix = QString{"%1_"}.arg(id);

			for (auto& layer : layers())
			{
				for (auto& item : layer->items())
				{
					if (item->label().startsWith(startWith, Qt::CaseSensitive) == true)
					{
						QString newItemLabel = item->label();
						newItemLabel.replace(0, startWith.size(), newPrefix);

						item->setLabel(newItemLabel);
					}
				}
			}
		}

		m_schemaID = id;
	}

	// Caption
	//
	QString Schema::caption() const
	{
		return m_caption;
	}

	void Schema::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Tags
	//
	QString Schema::tagsAsString() const
	{
		QString result;

		for (QString t : m_tags)
		{
			t = t.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(t);
		}

		return result;
	}

	QStringList Schema::tagsAsList() const
	{
		QStringList result;
		result.reserve(m_tags.size());

		for (const QString& t : m_tags)
		{
			result.push_back(t.trimmed());
		}

		return result;
	}

	void Schema::setTags(QString tags)
	{
		//tags.replace(';', QChar::LineFeed);
		//tags.replace(',', QChar::LineFeed);	QChar::LineFeed

		m_tags = tags.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);

		for (QString& t : m_tags)
		{
			t = t.trimmed();
		}

		return;
	}

	void Schema::setTagsList(const QStringList& tags)
	{
		m_tags.clear();
		m_tags.reserve(tags.size());

		for (QString t : tags)
		{
			QString trimmed = t.trimmed();

			if (trimmed.isEmpty() == false)
			{
				m_tags.append(trimmed);
			}
		}

		return;
	}

	bool Schema::joinHorzPriority() const
	{
		return m_joinHorzPriority;
	}

	void Schema::setJoinHorzPriority(bool value)
	{
		m_joinHorzPriority = value;
	}

	QString Schema::joinLeftSchemaId() const
	{
		return m_joinLeftSchemaId;
	}

	void Schema::setJoinLeftSchemaId(const QString& value)
	{
		m_joinLeftSchemaId = value;
	}

	QString Schema::joinTopSchemaId() const
	{
		return m_joinTopSchemaId;
	}

	void Schema::setJoinTopSchemaId(const QString& value)
	{
		m_joinTopSchemaId = value;
	}

	QString Schema::joinRightSchemaId() const
	{
		return m_joinRightSchemaId;
	}

	void Schema::setJoinRightSchemaId(const QString& value)
	{
		m_joinRightSchemaId = value;
	}

	QString Schema::joinBottomSchemaId() const
	{
		return m_joinBottomSchemaId;
	}

	void Schema::setJoinBottomSchemaId(const QString& value)
	{
		m_joinBottomSchemaId = value;
	}


	// Width
	//
	double Schema::docWidth() const
	{
		return m_width;
	}

	void Schema::setDocWidth(double width)
	{
		m_width = width;
	}

	double Schema::docWidthRegional() const
	{
		switch (unit())
		{
		case SchemaUnit::Display:
			return m_width;

		case SchemaUnit::Inch:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				return m_width;
			}
			else
			{
				return in2mm(m_width);
			}

		case SchemaUnit::Millimeter:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				return mm2in(m_width);
			}
			else
			{
				return m_width;
			}

		default:
			assert(false);
			return 0.0;
		}
	}

	void Schema::setDocWidthRegional(double width)
	{
		switch (unit())
		{
		case SchemaUnit::Display:
			m_width = width;
			return;

		case SchemaUnit::Inch:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				m_width = width;
			}
			else
			{
				m_width = mm2in(width);
			}
			return;

		case SchemaUnit::Millimeter:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				m_width = in2mm(width);
			}
			else
			{
				m_width = width;
			}
			return;

		default:
			assert(false);
			return;
		}
	}

	// Height
	//
	double Schema::docHeight() const
	{
		return m_height;
	}

	void Schema::setDocHeight(double height)
	{
		m_height = height;
	}

	double Schema::docHeightRegional() const
	{
		switch (unit())
		{
		case SchemaUnit::Display:
			return m_height;

		case SchemaUnit::Inch:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				return m_height;
			}
			else
			{
				return in2mm(m_height);
			}

		case SchemaUnit::Millimeter:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				return mm2in(m_height);
			}
			else
			{
				return m_height;
			}

		default:
			assert(false);
			return 0.0;
		}
	}

	void Schema::setDocHeightRegional(double height)
	{
		switch (unit())
		{
		case SchemaUnit::Display:
			m_height = height;
			return;

		case SchemaUnit::Inch:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				m_height = height;
			}
			else
			{
				m_height = mm2in(height);
			}
			return;

		case SchemaUnit::Millimeter:
			if (Settings::regionalUnit() == SchemaUnit::Inch)
			{
				m_height = in2mm(height);
			}
			else
			{
				m_height = height;
			}
			return;

		default:
			assert(false);
			return;
		}
	}

	// Unit
	//
	SchemaUnit Schema::unit() const
	{
		return m_unit;
	}

	void Schema::setUnit(SchemaUnit value)
	{
		Q_ASSERT(value == SchemaUnit::Display || value == SchemaUnit::Inch);

		m_unit = value;
		setGridSize(Settings::defaultGridSize(value));

		if (value == SchemaUnit::Display)
		{
			setPinGridStep(20);
		}
		else
		{
			setPinGridStep(4);
		}
	}

	double Schema::gridSize() const
	{
		return m_gridSize;
	}

	void Schema::setGridSize(double value)
	{
		m_gridSize = value;
	}

	int Schema::pinGridStep() const
	{
		return m_pinGridStep;
	}

	void Schema::setPinGridStep(int value)
	{
		m_pinGridStep = value;
	}

	bool Schema::excludeFromBuild() const
	{
		return m_excludeFromBuild;
	}

	void Schema::setExcludeFromBuild(bool value)
	{
		m_excludeFromBuild = value;
	}

	QColor Schema::backgroundColor() const
	{
		return m_backgroundColor;
	}

	void Schema::setBackgroundColor(const QColor& value)
	{
		m_backgroundColor = value;
	}

	bool Schema::isLogicSchema() const
	{
		return dynamic_cast<const VFrame30::LogicSchema*>(this) != nullptr;
	}

	bool Schema::isUfbSchema() const
	{
		return dynamic_cast<const VFrame30::UfbSchema*>(this) != nullptr;
	}

	bool Schema::isMonitorSchema() const
	{
		return dynamic_cast<const VFrame30::MonitorSchema*>(this) != nullptr;
	}

	bool Schema::isTuningSchema() const
	{
		return dynamic_cast<const VFrame30::TuningSchema*>(this) != nullptr;
	}

	bool Schema::isDiagSchema() const
	{
		return dynamic_cast<const VFrame30::DiagSchema*>(this) != nullptr;
	}

	LogicSchema* Schema::toLogicSchema()
	{
		return dynamic_cast<VFrame30::LogicSchema*>(this);
	}

	const LogicSchema* Schema::toLogicSchema() const
	{
		return dynamic_cast<const VFrame30::LogicSchema*>(this);
	}

	UfbSchema* Schema::toUfbSchema()
	{
		return dynamic_cast<VFrame30::UfbSchema*>(this);
	}

	const UfbSchema* Schema::toUfbSchema() const
	{
		return dynamic_cast<const VFrame30::UfbSchema*>(this);
	}

	int Schema::changeset() const
	{
		return m_changeset;
	}

	void Schema::setChangeset(int value)
	{
		m_changeset = value;
	}

	QString Schema::preDrawScript() const
	{
		return m_preDrawScript;
	}

	void Schema::setPreDrawScript(QString value)
	{
		m_preDrawScript = std::move(value);
	}

	QString Schema::onShowScript() const
	{
		return m_onShowScript;
	}

	void Schema::setOnShowScript(QString value)
	{
		m_onShowScript = std::move(value);
	}


	const std::shared_ptr<VFrame30::Context>& Schema::context() const
	{
		return m_context;
	}

	void Schema::setContext(std::shared_ptr<VFrame30::Context> context)
	{
		m_context = std::move(context);
	}

	//
	//
	//				SchemaDetails
	//
	//
	SchemaDetails::SchemaDetails(const QString& details) noexcept
	{
		parseDetails(details);
	}

	bool SchemaDetails::operator< (const SchemaDetails& b) const noexcept
	{
		return this->m_schemaId < b.m_schemaId;
	}

	QString SchemaDetails::getDetailsString(const Schema* schema, const QString& path)
	{
		if (schema == nullptr)
		{
			assert(schema);
			return QString();
		}

		// form details JSON object (signal list)
		//
		QJsonObject jsonObject;

		// Get signalIds
		//
		QStringList signalIdsList = schema->getSignalList();
		QSet<QString> signalIds{signalIdsList.begin(), signalIdsList.end()};

		// Get labels for AFBs
		//
		QStringList labels = schema->getLabels();

		// Get list of receivers/transmitters and loopbacks
		//
		QSet<QString> connections;
		QSet<QString> loopbacks;
		QSet<QString> packedLogicIds;
		std::vector<TrendIndicatorSchemaItems> realTimeTrends;

		for (const std::shared_ptr<SchemaLayer>& layer : schema->layers())
		{
			if (layer->compile() == true)
			{
				for (const auto& item : layer->items())
				{
					// Items on LogicSchemas
					//
					if (schema->isLogicSchema() == true)
					{
						if (const SchemaItemConnection* connItem = item->toType<SchemaItemConnection>();
							connItem != nullptr)
						{
							for (const auto& connectionIds = connItem->connectionIdsAsList();
								 const QString& connectionId : connectionIds)
							{
								connections.insert(connectionId);
							}
						}

						if (const SchemaItemReceiver* receiver = item->toType<SchemaItemReceiver>();
							receiver != nullptr)
						{
							for (const QString& appSignalId : receiver->appSignalIdsAsList())
							{
								signalIds << appSignalId;
							}
						}

						if (const SchemaItemLoopback* lb = item->toType<SchemaItemLoopback>();
							lb != nullptr)
						{
							loopbacks << lb->loopbackId();
						}

						if (const SchemaItemAfb* afb = item->toType<SchemaItemAfb>();
							afb != nullptr && afb->isPackedLogic() == true)
						{
							packedLogicIds << afb->packedLogicId();
						}
					}

					// Items on all other schemas
					//
					if (const SchemaItemIndicator* ii = item->toType<SchemaItemIndicator>();
						ii != nullptr &&
						ii->isTrend() == true)
					{
						realTimeTrends.emplace_back(ii->guid(),
													ii->trendSamplePeriod(),
													ii->trendTimeType(),
													ii->trendDurationSeconds(),
													ii->signalIds());
					}
				}

				break;
			}
		}

		// Get schema tags, kept in lowercase
		//
		QStringList schemaTags = schema->tagsAsList();
		for (QString& tag : schemaTags)
		{
			tag = tag.toLower();
		}

		// Get item tags, kept in lowercase
		//
		QStringList itemTags = schema->itemTags().toList();
		for (QString& tag : itemTags)
		{
			tag = tag.toLower();
		}

		// Get a list of guids
		//
		std::vector<QUuid> guids = schema->getGuids();

		QStringList guidsStringList;
		guidsStringList.reserve(static_cast<int>(guids.size()));

		for (const QUuid uuid : guids)
		{
			guidsStringList.push_back(uuid.toString());
		}

		// Form JSon object
		//
		QVariant signaListVariant(signalIds.values());
		QVariant labelsVariant(labels);
		QVariant connectionsVariant(connections.values());
		QVariant loopbacksVariant(loopbacks.values());
		QVariant tagsVariant(schemaTags);
		QVariant packedLogicIdsVariant(packedLogicIds.values());
		QVariant itemTagsVariant(itemTags);
		QVariant guidsVariant(guidsStringList);

		jsonObject.insert("Version", QJsonValue(1));
		jsonObject.insert("SchemaID", QJsonValue(schema->schemaId()));
		jsonObject.insert("Caption", QJsonValue(schema->caption()));
		jsonObject.insert("ExcludedFromBuild", QJsonValue(schema->excludeFromBuild()));

		if (schema->isLogicSchema() == true)
		{
			const LogicSchema* logicSchema = schema->toLogicSchema();
			assert(logicSchema);

			QString equipIds = logicSchema->equipmentIds();
			equipIds = equipIds.replace('\n', ' ');
			jsonObject.insert("EquipmentID", QJsonValue(equipIds));

			jsonObject.insert(PropertyNames::lmDescriptionFile, QJsonValue(logicSchema->lmDescriptionFile()));
		}

		if (schema->isUfbSchema() == true)
		{
			const UfbSchema* ufbSchema = schema->toUfbSchema();
			assert(ufbSchema);

			jsonObject.insert(PropertyNames::lmDescriptionFile, QJsonValue(ufbSchema->lmDescriptionFile()));
		}

		jsonObject.insert("Path", QJsonValue(path));
		jsonObject.insert("Signals", QJsonValue::fromVariant(signaListVariant));
		jsonObject.insert("Labels", QJsonValue::fromVariant(labelsVariant));
		jsonObject.insert("Connections", QJsonValue::fromVariant(connectionsVariant));
		jsonObject.insert("Loopbacks", QJsonValue::fromVariant(loopbacksVariant));
		jsonObject.insert("Tags", QJsonValue::fromVariant(tagsVariant));
		jsonObject.insert("PackedLogicIds", QJsonValue::fromVariant(packedLogicIdsVariant));
		jsonObject.insert("ItemTags", QJsonValue::fromVariant(itemTagsVariant));
		jsonObject.insert("ItemGuids", QJsonValue::fromVariant(guidsVariant));

		QJsonArray jsontrendsIndicators;
		for (const TrendIndicatorSchemaItems& trendItem : realTimeTrends)
		{
			jsontrendsIndicators.push_back(trendItem.toJsonObject());
		}
		jsonObject["RealTimeTrendItems"] = jsontrendsIndicators;

		// Convert json to string and return it
		//
		QJsonDocument jsonDoc(jsonObject);

		QByteArray data = jsonDoc.toJson(QJsonDocument::JsonFormat::Indented);
		QString result = QString::fromUtf8(data);

		return result;
	}

	bool SchemaDetails::parseDetails(const QString& details)
	{
		if (details.trimmed().isEmpty() == true)
		{
			*this = {};
			return true;
		}

		// parse details section (from DB), result is signal list
		//
		QByteArray data = details.toUtf8();

		QJsonParseError parseError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

		if (parseError.error != QJsonParseError::NoError)
		{
			qDebug() << "Schema details parsing error: " << parseError.errorString();
			qDebug() << "JSON document: " << details;
			return false;
		}

		if (jsonDoc.isObject() == false)
		{
			assert(jsonDoc.isObject());		// have a look at json doc, it is supposed to be an object
			qDebug() << Q_FUNC_INFO << " json is supposed to be object";
			return false;
		}

		QJsonObject jsonObject = jsonDoc.object();

		QJsonValue version = jsonObject.value(QLatin1String("Version"));
		int versionInt = version.toInt(-1);

		if (versionInt == -1 ||
			version.type() != QJsonValue::Double)
		{
			return false;
		}

		switch (versionInt)
		{
		case 1:
			{
				// SchemaID
				//
				m_schemaId = jsonObject.value(QLatin1String("SchemaID")).toString();

				// Caption
				//
				m_caption = jsonObject.value(QLatin1String("Caption")).toString();

				// ExcludedFromBuild
				//
				QJsonValue excFromBuild = jsonObject.value(QLatin1String("ExcludedFromBuild"));
				if (excFromBuild.isUndefined() == false && excFromBuild.isBool() == true)
				{
					m_excludedFromBuild = excFromBuild.toBool();
				}
				else
				{
					m_excludedFromBuild = false;
				}

				// EquipmentID
				//
				QJsonValue eqidValue = jsonObject.value(QLatin1String("EquipmentID")).toString();
				if (eqidValue.isUndefined() == false)
				{
					m_equipmentId = eqidValue.toString();
				}
				else
				{
					m_equipmentId.clear();
				}

				// LmDescriptionFile
				//
				QJsonValue lmdescrValue = jsonObject.value(PropertyNames::lmDescriptionFile).toString();
				if (lmdescrValue.isUndefined() == false)
				{
					m_lmDescriptionFile = lmdescrValue.toString();
				}
				else
				{
					m_lmDescriptionFile.clear();
				}

				// Path
				//
				m_path = jsonObject.value(QLatin1String("Path")).toString();

				// Signals
				//
				m_signals.clear();
				QStringList signalsStrings = jsonObject.value(QLatin1String("Signals")).toVariant().toStringList();

				for (const QString& str : signalsStrings)
				{
					m_signals.insert(str);
				};

				// Labels
				//
				m_labels.clear();
				QStringList labelList = jsonObject.value(QLatin1String("Labels")).toVariant().toStringList();

				for (const QString& str : labelList)
				{
					m_labels.insert(str);
				}

				// Connections
				//
				m_connections.clear();
				QStringList connList = jsonObject.value(QLatin1String("Connections")).toVariant().toStringList();

				for (const QString& str : connList)
				{
					m_connections.insert(str);
				}

				// Loopbacks
				//
				m_loopbacks.clear();
				QStringList loopbackList = jsonObject.value(QLatin1String("Loopbacks")).toVariant().toStringList();

				for (const QString& str : loopbackList)
				{
					m_loopbacks.insert(str);
				}

				// Schema Tags
				//
				{
					m_schemaTags.clear();
					QStringList tagsList = jsonObject.value(QLatin1String("Tags")).toVariant().toStringList();

					for (const QString& str : tagsList)
					{
						m_schemaTags.insert(str);
					}
				}

				// SchemaItems Tags
				//
				{
					m_itemTags.clear();
					QStringList itemTagsList = jsonObject.value(QLatin1String("ItemTags")).toVariant().toStringList();

					for (const QString& str : itemTagsList)
					{
						m_itemTags.insert(str.toLower());
					}
				}

				// m_packedLogicIds
				//
				{
					m_packedLogicIds.clear();
					QStringList packedIdList = jsonObject.value(QLatin1String("PackedLogicIds")).toVariant().toStringList();

					for (const QString& str : packedIdList)
					{
						m_packedLogicIds.insert(str);
					}
				}

				// ItemGuids
				//
				{
					m_guids.clear();

					QStringList guidList = jsonObject.value(QLatin1String("ItemGuids")).toVariant().toStringList();
					std::for_each(guidList.begin(), guidList.end(), [this](const QString& str){	m_guids.insert(QUuid(str));});
				}

				// m_trendsIndicators
				//
				{
					m_trendsIndicators.clear();
					QJsonArray jsa = jsonObject.value(QLatin1String("RealTimeTrendItems")).toArray();

					for (QJsonValueRef v : jsa)
					{
						Q_ASSERT(v.isObject() == true);
						m_trendsIndicators.emplace_back().fromJsonObject(v.toObject());
					}
				}
			}
			break;
		default:
			assert(false);
			return false;
		}
		return true;
	}

	bool SchemaDetails::saveData(Proto::SchemaDetails* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_version(m_version);
		message->set_schemaid(m_schemaId.toStdString());
		message->set_caption(m_caption.toStdString());
		message->set_excludedfrombuild(m_excludedFromBuild);
		message->set_equipmentid(m_equipmentId.toStdString());
		message->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());
		message->set_path(m_path.toStdString());

		for (const QString& s : m_signals)
		{
			message->add_signalids(s.toStdString());
		}

		for (const QString& l : m_labels)
		{
			message->add_labels(l.toStdString());
		}

		for (const QString& c : m_connections)
		{
			message->add_connections(c.toStdString());
		}

		for (const QString& l : m_loopbacks)
		{
			message->add_loopbacks(l.toStdString());
		}

		for (const QString& t : m_schemaTags)
		{
			message->add_schematags(t.toStdString());
		}

		for (const QString& t : m_itemTags)
		{
			message->add_itemtags(t.toStdString());
		}

		for (const QString& t : m_packedLogicIds)
		{
			message->add_packedlogicids(t.toStdString());
		}

		for (const QUuid& u : m_guids)
		{
			::Proto::Uuid* uuidMesage = message->add_guids();
			assert(uuidMesage);

			uuidMesage->set_uuid(&u, sizeof(u));
		}

		for (const auto& ti : m_trendsIndicators)
		{
			::Proto::SchemaDetails::TrendIndicatorSchemaItems* trendsIndicators = message->add_trendindicators();
			ti.saveData(trendsIndicators);
		}

		return true;
	}

	bool SchemaDetails::loadData(const Proto::SchemaDetails& message)
	{
		m_version = message.version();
		m_schemaId = QString::fromStdString(message.schemaid());
		m_caption = QString::fromStdString(message.caption());
		m_excludedFromBuild = message.excludedfrombuild();
		m_equipmentId = QString::fromStdString(message.equipmentid());
		m_lmDescriptionFile = QString::fromStdString(message.lmdescriptionfile());
		m_path = QString::fromStdString(message.path());

		m_signals.clear();
		int signalsCount = message.signalids_size();
		for (int i = 0; i < signalsCount; i++)
		{
			QString signalId = QString::fromStdString(message.signalids(i));
			m_signals.insert(signalId);
		}

		m_labels.clear();
		int labelCount = message.labels_size();
		for (int i = 0; i < labelCount; i++)
		{
			QString label = QString::fromStdString(message.labels(i));
			m_labels.insert(label);
		}

		m_connections.clear();
		int connectionCount = message.connections_size();
		for (int i = 0; i < connectionCount; i++)
		{
			QString conn = QString::fromStdString(message.connections(i));
			m_connections.insert(conn);
		}

		m_loopbacks.clear();
		int loopbackCount = message.loopbacks_size();
		for (int i = 0; i < loopbackCount; i++)
		{
			QString lb = QString::fromStdString(message.loopbacks(i));
			m_loopbacks.insert(lb);
		}

		m_schemaTags.clear();
		for (int i = 0, tagCount = message.schematags_size(); i < tagCount; i++)
		{
			QString tag = QString::fromStdString(message.schematags(i));
			m_schemaTags.insert(tag);
		}

		m_itemTags.clear();
		for (int i = 0, tagCount = message.itemtags_size(); i < tagCount; i++)
		{
			QString tag = QString::fromStdString(message.itemtags(i)).toLower();
			m_itemTags.insert(tag);
		}

		m_packedLogicIds.clear();
		for (int i = 0, size = message.packedlogicids_size(); i < size; i++)
		{
			m_packedLogicIds.insert(QString::fromStdString(message.packedlogicids(i)));
		}

		m_guids.clear();
		int guidCount = message.guids_size();
		for (int i = 0; i < guidCount; i++)
		{
			QUuid guid = Proto::Read(message.guids(i));
			m_guids.insert(guid);
		}

		{
			m_trendsIndicators.clear();
			int tiCount = message.trendindicators_size();
			for (int i = 0; i < tiCount; i++)
			{
				const auto& tiMessage = message.trendindicators(i);

				TrendIndicatorSchemaItems& trendSchemaItem = m_trendsIndicators.emplace_back();
				trendSchemaItem.loadData(tiMessage);
			}
		}

		return true;
	}

	bool SchemaDetails::searchForString(const QString& searchText) const
	{
		if (m_schemaId.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_caption.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_equipmentId.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_signals.find(searchText) != m_signals.end())
		{
			return true;
		}

		if (m_labels.find(searchText) != m_labels.end())
		{
			return true;
		}

		if (m_connections.find(searchText) != m_connections.end())
		{
			return true;
		}

		if (m_loopbacks.find(searchText) != m_loopbacks.end())
		{
			return true;
		}

		if (m_itemTags.contains(searchText.toLower()) == true)
		{
			return true;
		}

		if (m_packedLogicIds.contains(searchText) == true)
		{
			return true;
		}

		QUuid textAsUuid(searchText);

		if (textAsUuid.isNull() == false &&
			m_guids.find(textAsUuid) != m_guids.end())
		{
			return true;
		}

		return false;
	}

	bool SchemaDetails::hasSchemaTag(const QString& tag) const
	{
		return m_schemaTags.find(tag.trimmed().toLower()) != m_schemaTags.end();
	}

	bool SchemaDetails::hasSchemaTag(const QStringList& tags) const
	{
		for (const QString& tag : tags)
		{
			if (m_schemaTags.find(tag.trimmed().toLower()) != m_schemaTags.end())
			{
				return true;
			}
		}

		return false;
	}

	const std::set<QString>& SchemaDetails::schemaTags() const
	{
		return m_schemaTags;
	}

	bool SchemaDetails::hasEquipmentId(const QString& equipmentId) const
	{
		QStringList eqs = m_equipmentId.split(' ', Qt::SkipEmptyParts);

		bool result = eqs.contains(equipmentId, Qt::CaseInsensitive);
		return result;
	}

	bool SchemaDetails::hasSignal(const QString& signalId) const
	{
		return m_signals.contains(signalId);
	}

	bool SchemaDetails::hasLoopback(const QString& loopbackId) const
	{
		return m_loopbacks.contains(loopbackId);
	}

	bool SchemaDetails::hasConnection(const QString& connectionId) const
	{
		return m_connections.contains(connectionId);
	}

	bool SchemaDetails::hasLabel(const QString& label) const
	{
		return m_labels.contains(label);
	}

	bool SchemaDetails::hasGuid(QUuid guid) const
	{
		return m_guids.contains(guid);
	}

	SchemaDetails::TrendIndicatorSchemaItems::TrendIndicatorSchemaItems(QUuid _itemUuid,
																		E::RtTrendsSamplePeriod _samplePeriod,
																		E::TimeType _timeType,
																		int _durationSeconds,
																		const QStringList& _appSignalIds) :
		itemUuid(_itemUuid),
		samplePeriod(_samplePeriod),
		timeType(_timeType),
		durationSeconds(_durationSeconds),
		appSignalIds(_appSignalIds)
	{
	}

	QJsonObject SchemaDetails::TrendIndicatorSchemaItems::toJsonObject() const
	{
		QJsonObject jsonObject;

		jsonObject.insert(QLatin1String{"itemUuid"}, QJsonValue{itemUuid.toString()});
		jsonObject.insert(QLatin1String{"samplePeriod"}, QJsonValue{static_cast<int>(samplePeriod)});
		jsonObject.insert(QLatin1String{"timeType"}, QJsonValue{static_cast<int>(timeType)});
		jsonObject.insert(QLatin1String{"appSignalIds"}, QJsonValue{appSignalIds.join(", ")});
		jsonObject.insert(QLatin1String{"durationSeconds"}, QJsonValue{durationSeconds});

		return jsonObject;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::fromJsonObject(const QJsonObject& jsonObject)
	{
		itemUuid = QUuid::fromString(jsonObject.value(QLatin1String{"itemUuid"}).toString());
		samplePeriod = static_cast<E::RtTrendsSamplePeriod>(jsonObject.value(QLatin1String{"samplePeriod"}).toInt());
		timeType = static_cast<E::TimeType>(jsonObject.value(QLatin1String{"timeType"}).toInt());
		durationSeconds = jsonObject.value(QLatin1String{"durationSeconds"}).toInt();

		appSignalIds = jsonObject.value(QLatin1String{"appSignalIds"}).toString().split(',', Qt::SkipEmptyParts);
		for (QString& appSignal : appSignalIds)
		{
			appSignal = appSignal.trimmed();
		}

		return true;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::saveData(::Proto::SchemaDetails::TrendIndicatorSchemaItems* message) const
	{
		Proto::Write(message->mutable_itemuuid(), itemUuid);
		message->set_sampleperiod(static_cast<int>(samplePeriod));
		message->set_timetype(static_cast<int>(timeType));
		message->set_durationseconds(durationSeconds);

		for (const QString& s : appSignalIds)
		{
			message->add_appsignalids(s.toStdString());
		}

		return true;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::loadData(const ::Proto::SchemaDetails::TrendIndicatorSchemaItems& message)
	{
		itemUuid = Proto::Read(message.itemuuid());
		samplePeriod = static_cast<E::RtTrendsSamplePeriod>(message.sampleperiod());
		timeType = static_cast<E::TimeType>(message.timetype());
		durationSeconds = message.durationseconds();

		appSignalIds.clear();
		for (const auto& sm : message.appsignalids())
		{
			appSignalIds.push_back(QString::fromStdString(sm));
		}

		return true;
	}

	SchemaDetailsSet::SchemaDetailsSet() :
		Proto::ObjectSerialization<SchemaDetailsSet>(Proto::ProtoCompress::Auto)
	{
	}

	bool SchemaDetailsSet::SaveData(Proto::Envelope* envelopeMessage) const
	{
		std::string className = {"SchemaDetailsSet"};
		quint32 classnamehash = ::ClassNameHashCode(className);
		envelopeMessage->set_classnamehash(classnamehash);

		::Proto::SchemaDetailsSet* setMessage = envelopeMessage->mutable_schemadetailsset();

		for (auto detailsPair : m_details)
		{
			::Proto::SchemaDetails* detailsMessage = setMessage->add_schemasdetails();
			detailsPair.second->saveData(detailsMessage);
		}

		return true;
	}

	bool SchemaDetailsSet::LoadData(const Proto::Envelope& message)
	{
		clear();

		if (message.has_schemadetailsset() == false)
		{
			assert(message.has_schemadetailsset());
			return false;
		}

		const Proto::SchemaDetailsSet& setMessage = message.schemadetailsset();

		int detailsCount = setMessage.schemasdetails_size();
		for (int i = 0; i < detailsCount; i++)
		{
			std::shared_ptr<SchemaDetails> details = std::make_shared<SchemaDetails>();
			bool loadOk = details->loadData(setMessage.schemasdetails(i));

			if (loadOk == true)
			{
				add(details);
			}
		}

		return true;
	}

	std::shared_ptr<SchemaDetailsSet> SchemaDetailsSet::CreateObject(const Proto::Envelope& message)
	{
		std::shared_ptr<SchemaDetailsSet> object = std::make_shared<SchemaDetailsSet>();
		object->LoadData(message);

		return object;
	}

	void SchemaDetailsSet::clear()
	{
		m_details.clear();
	}

	void SchemaDetailsSet::add(const QString& details)
	{
		return add(std::make_shared<SchemaDetails>(details));
	}

	void SchemaDetailsSet::add(const SchemaDetails& details)
	{
		return add(std::make_shared<SchemaDetails>(details));
	}

	void SchemaDetailsSet::add(SchemaDetails&& details)
	{
		return add(std::make_shared<SchemaDetails>(std::move(details)));
	}

	void SchemaDetailsSet::add(std::shared_ptr<SchemaDetails> details)
	{
		assert(details);

		m_details[details->m_schemaId] = std::move(details);

		return;
	}

	std::vector<SchemaDetails> SchemaDetailsSet::schemasDetails() const
	{
		std::vector<SchemaDetails> result;
		result.reserve(m_details.size());

		for (const auto& schemaPair : m_details)
		{
			result.push_back(*schemaPair.second);
		}

		return result;
	}

	std::vector<SchemaDetails> SchemaDetailsSet::schemasDetails(QString equipmentId) const
	{
		std::vector<SchemaDetails> result;
		result.reserve(m_details.size());

		for (const auto& schemaPair : m_details)
		{
			const SchemaDetails* ptr = schemaPair.second.get();

			if (ptr->hasEquipmentId(equipmentId) == true)
			{
				result.push_back(*ptr);
			}
		}

		return result;
	}

	std::shared_ptr<SchemaDetails> SchemaDetailsSet::schemaDetails(QString schemaId) const
	{
		std::shared_ptr<SchemaDetails> result;

		if (auto it = m_details.find(schemaId);
			it != m_details.end())
		{
			result = it->second;
		}

		return result;
	}

	std::shared_ptr<SchemaDetails> SchemaDetailsSet::schemaDetails(int index) const
	{
		std::shared_ptr<SchemaDetails> result;

		if (index >= 0 && index < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, index);

			if (it != m_details.end())
			{
				result = it->second;
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByAppSignalId(const QString& appSignalId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto&[schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasSignal(appSignalId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByConnectionId(const QString& connectionId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto&[schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasConnection(connectionId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByLoopbackId(const QString& loopbackId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto&[schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasLoopback(loopbackId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	int SchemaDetailsSet::schemaCount() const
	{
		return static_cast<int>(m_details.size());
	}

	QString SchemaDetailsSet::schemaCaptionById(const QString& schemaId) const
	{
		auto it = m_details.find(schemaId);
		return (it == m_details.end()) ? QString{} : it->second->m_caption;
	}

	QString SchemaDetailsSet::schemaCaptionByIndex(int schemaIndex) const
	{
		if (schemaIndex >= 0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_caption;
		}

		return {};
	}

	QString SchemaDetailsSet::schemaIdByIndex(int schemaIndex) const
	{
		if (schemaIndex >= 0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_schemaId;
		}

		return {};
	}

	std::vector<SchemaDetails::TrendIndicatorSchemaItems> SchemaDetailsSet::trendIndicators() const
	{
		std::vector<SchemaDetails::TrendIndicatorSchemaItems> result;
		result.reserve(128);

		for (const auto& [schemaId, schemaDetails] : m_details)
		{
			result.insert(result.end(), schemaDetails->m_trendsIndicators.begin(), schemaDetails->m_trendsIndicators.end());
		}

		return result;
	}

}

