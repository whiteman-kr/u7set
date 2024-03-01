#include "Schema.h"
#include "./SchemaItems/FblItem.h"
#include "./SchemaItems/SchemaItemAfb.h"
#include "./SchemaItems/SchemaItemBus.h"
#include "./SchemaItems/SchemaItemLink.h"
#include "./SchemaItems/SchemaItemUfb.h"
#include "ClientSchemaView.h"
#include "Context.h"
#include "DiagSchema.h"
#include "DrawParam.h"
#include "HorzVertLinks.h"
#include "LogicSchema.h"
#include "MonitorSchema.h"
#include "PropertyNames.h"
#include "SchemaDetails.h"
#include "SchemaLayer.h"
#include "TuningSchema.h"
#include "UfbSchema.h"


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
					QJSEngine::setObjectOwnership(item.get(), QJSEngine::CppOwnership);
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
					QJSEngine::setObjectOwnership(item.get(), QJSEngine::ObjectOwnership::CppOwnership);

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

	bool Schema::updateAllSchemaItemBusses(const std::vector<AppSignalLib::Bus>& busses, int* updatedItemCount, QString* errorMessage)
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
				[&si](const AppSignalLib::Bus& bus)
				{
					return si->busTypeId() == bus.busTypeId();
				});

			if (foundIt == busses.end())
			{
				*errorMessage += tr("Cant find BusType %1. Schema %2.\n").arg(si->busTypeId()).arg(si->parentSchema()->schemaId());
				continue;
			}

			const AppSignalLib::Bus& bus = *foundIt;

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
}

