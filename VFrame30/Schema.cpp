#include "Schema.h"
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
#include "HorzVertLinks.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "../lib/ProtoSerialization.h"


namespace VFrame30
{
	Factory<VFrame30::Schema> SchemaFactory;

	//
	// ScriptSchema
	//
	ScriptSchema::ScriptSchema(std::shared_ptr<Schema> schema) :
		m_schema(schema)
	{
		Q_ASSERT(m_schema);
	}

	ScriptSchema::~ScriptSchema()
	{
		qDebug() << "ScriptSchema::~ScriptSchema " << schemaId();
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

	QString ScriptSchema::schemaId() const
	{
		return m_schema ? m_schema->schemaId() : QString{};
	}

	QString ScriptSchema::caption() const
	{
		return m_schema ? m_schema->caption() : QString{};
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
	}

	void Schema::Init(void)
	{
		auto schemaIdProp = ADD_PROPERTY_GETTER_SETTER(QString, "SchemaID", true, Schema::schemaId, Schema::setSchemaId);
		schemaIdProp->setValidator("^[A-Za-z\\d_]{1,64}$");

		ADD_PROPERTY_GETTER(int, "Changeset", true, Schema::changeset);
		ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, Schema::caption, Schema::setCaption);

		auto tagsProp = ADD_PROPERTY_GETTER_SETTER(QString, "Tags", true, Schema::tagsAsString, Schema::setTags);
		tagsProp->setSpecificEditor(E::PropertySpecificEditor::Tags);

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
		quint32 classnamehash = CUtils::GetClassHashCode(className);

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

		// Save Layers
		//
		bool saveLayersResult = true;

		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			Proto::Envelope* pLayerMessage = mutableSchema->add_layers();
			saveLayersResult &= layer->get()->Save(pLayerMessage);
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

		if (schema.has_tags() == true)	// if schema does not have saved tags, then default values are teaken from the each schema type constructor
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

		// Layers
		//
		Layers.clear();

		for (int i = 0; i < schema.layers().size(); i++)
		{
			std::shared_ptr<SchemaLayer> layer = SchemaLayer::Create(schema.layers(i));

			if (layer == nullptr)
			{
				assert(layer);
				continue;
			}

			Layers.push_back(layer);

			if (layer->compile() == true)
			{
				m_activeLayer = i;
			}
		}

		if (schema.layers().size() != (int)Layers.size())
		{
			assert(schema.layers().size() == (int)Layers.size());
			Layers.clear();
			return false;
		}

		// Load fake empty Afb Collection,
		//
		//m_afbCollection.LoadData(schema.afbs());

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

	void Schema::Draw(CDrawParam* drawParam, const QRectF& clipRect) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		// Cleare client area by "grey" color
		//
		QPainter* p = drawParam->painter();
		//p->fill(0xB0, 0xB0, 0xB0);	-- Done in? CDrawParam::BeginPaint

		// ---
		//
		QRectF pageRect(0.0, 0.0, static_cast<qreal>(docWidth()), static_cast<qreal>(docHeight()));
		p->fillRect(pageRect, backgroundColor());

		// Draw items by layers which has Show flag
		//
		double clipX = clipRect.left();
		double clipY = clipRect.top();
		double clipWidth = clipRect.width();
		double clipHeight = clipRect.height();

		for (auto layer : Layers)
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

			for (const SchemaItemPtr& item : layer->Items)
			{
				Q_ASSERT(item);

				item->setDrawParam(drawParam);

				if (item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					if (drawParam->isMonitorMode() == true)
					{
						ClientSchemaView* view = drawParam->clientSchemaView();
						Q_ASSERT(view);

						item->preDrawEvent(view->jsEngine());
					}

					item->draw(drawParam, this, layer.get());	// Drawing item is here

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
				}

				item->setDrawParam(nullptr);
			}
		}

		return;
	}

	void Schema::Print()
	{
		assert(false);
	}

	void Schema::MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const
	{
		if (pVideoFrameWidgetAgent == nullptr)
		{
			assert(pVideoFrameWidgetAgent != nullptr);
			return;
		}

		double x = docPoint.x();
		double y = docPoint.y();

		bool stop = false;

		for (auto layer = Layers.crbegin(); layer != Layers.crend(); layer++)
		{
			const SchemaLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<SchemaItem>& item = *vi;

				if (item->acceptClick() == true && item->isIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
				{
					RunClickScript(item/*, pVideoFrameWidgetAgent*/);
					stop = true;
					break;
				}
			}

			if (stop == true)
			{
				break;
			}
		}

		return;
	}

	void Schema::RunClickScript(const std::shared_ptr<SchemaItem>& schemaItem/*, VideoFrameWidgetAgent* pVideoFrameWidgetAgent*/) const
	{
		assert(false);
		Q_UNUSED(schemaItem);

/*		if (pVideoFrameWidgetAgent == nullptr || schemaItem->acceptClick() == false || schemaItem->clickScript().isEmpty() == true)
		{
			assert(pVideoFrameWidgetAgent != nullptr);
			return;
		}

		// Extract script text from SchemaItem
		//
		QString script = schemaItem->clickScript();
		QScriptEngine scriptEngine;
		QScriptValue globalValue = scriptEngine.globalObject();

		// Adust script enviroment
		//

		// Set proprties
		//
		QScriptValue vfWidgetAgentValue = scriptEngine.newQObject(pVideoFrameWidgetAgent);
		globalValue.setProperty("videoFrameWidget", vfWidgetAgentValue);

		// Run script
		//
		QScriptValue result = scriptEngine.evaluate(script);

		// Process script running result
		//
#ifdef _DEBUG
		qDebug() << strID() << "RunClickScript result:" << result.toString();
#endif*/
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

	int Schema::GetLayerCount() const
	{
		return (int)Layers.size();
	}

	void Schema::BuildFblConnectionMap() const
	{
		// --
		//
		CHorzVertLinks horzVertLinks;

		// ?????? ?? ???? ???????????, ????????? horzlinks ? vertlinks
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			SchemaLayer* pLayer = layer->get();

			for (auto item = pLayer->Items.begin(); item != pLayer->Items.end(); ++item)
			{
				if (item->get()->isFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item->get());
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

				SchemaItemLink* schemaItemLink = dynamic_cast<SchemaItemLink*>(item->get());
				if (schemaItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemaItemLink->GetPointList();

					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}

					// ????????? ?????? ?? ????????? ??????? ? ??????? ?? ? horzlinks ? vertlinks
					//
					horzVertLinks.AddLinks(pointList, schemaItemLink->guid());
				}
			}
		}

		// --
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			SchemaLayer* pLayer = layer->get();

			pLayer->connectionMap.clear();

			for (auto item = pLayer->Items.begin(); item != pLayer->Items.end(); ++item)
			{
				if (item->get()->isFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item->get());

				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}


				// ???? ??????? SchemaItemLink, ?? ? ???????? ????????? ????? ????? ??????? ?????
				//
				SchemaItemLink* schemaItemLink = dynamic_cast<SchemaItemLink*>(item->get());

				if (schemaItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemaItemLink->GetPointList();

					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}

					pLayer->ConnectionMapPosInc(pointList.front());
					pLayer->ConnectionMapPosInc(pointList.back());

					// ?????????, ?? ????? ?? ??? ?? ?????????????? ?????
					//
					if (horzVertLinks.IsPointOnLink(pointList.front(), schemaItemLink->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pointList.front());
					}

					if (horzVertLinks.IsPointOnLink(pointList.back(), schemaItemLink->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pointList.back());
					}

					continue;
				}

				// ?????????? ? ?????????? ????????? ????? ??? ???????? Fbl ????????
				//
				pFblItem->SetConnectionsPos(gridSize(), pinGridStep());

				// ????? ? connectionMap ????? ?????????? ? ???? ??? ???? ?? ????????? ??????? ?????,
				// ???? ???, ?? ??????? ?????? ? ??????
				//
				const std::vector<AfbPin>& inputs = pFblItem->inputs();
				for (auto pin = inputs.begin(); pin != inputs.end(); ++pin)
				{
					SchemaPoint pinPos = pin->point();

					pLayer->ConnectionMapPosInc(pinPos);

					// ?????????, ?? ????? ?? ??? ?? ?????????????? ?????
					//
					if (horzVertLinks.IsPinOnLink(pinPos, item->get()->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pinPos);
					}
				}

				const std::vector<AfbPin>& outputs = pFblItem->outputs();
				for (auto pin = outputs.begin(); pin != outputs.end(); ++pin)
				{
					SchemaPoint pinPos = pin->point();

					pLayer->ConnectionMapPosInc(pinPos);

					// ?????????, ?? ????? ?? ??? ?? ?????????????? ?????
					//
					if (horzVertLinks.IsPinOnLink(pinPos, item->get()->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pinPos);
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

		for (std::shared_ptr<SchemaLayer> l : Layers)
		{
			for (std::shared_ptr<SchemaItem> si : l->Items)
			{
				std::shared_ptr<VFrame30::SchemaItemAfb> schemaAfbItem = std::dynamic_pointer_cast<VFrame30::SchemaItemAfb>(si);

				if (schemaAfbItem != nullptr)
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
				[&si](std::shared_ptr<Afb::AfbElement> afb)
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

		for (std::shared_ptr<SchemaLayer> l : Layers)
		{
			for (std::shared_ptr<SchemaItem> si : l->Items)
			{
				std::shared_ptr<VFrame30::SchemaItemUfb> schemaUfbItem = std::dynamic_pointer_cast<VFrame30::SchemaItemUfb>(si);

				if (schemaUfbItem != nullptr)
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
				[&si](std::shared_ptr<UfbSchema> ufb)
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

		for (std::shared_ptr<SchemaLayer> l : Layers)
		{
			for (std::shared_ptr<SchemaItem> si : l->Items)
			{
				std::shared_ptr<VFrame30::SchemaItemBus> schemaItemBus = std::dynamic_pointer_cast<VFrame30::SchemaItemBus>(si);

				if (schemaItemBus != nullptr)
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
				*errorMessage += tr("Cant find BusType %1.\n").arg(si->busTypeId());
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
		// Do nothing, override to get signal list
		//
		QStringList result;
		return result;
	}

	QStringList Schema::getLabels() const
	{
		QStringList labels;
		labels.reserve(1024);

		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			for (const SchemaItemPtr& item : layer->Items)
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

		for (std::shared_ptr<SchemaLayer> layer : Layers)
		{
			for (std::shared_ptr<SchemaItem> item : layer->Items)
			{
				result.push_back(item->guid());
			}
		}

		return result;
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
		for (std::shared_ptr<VFrame30::SchemaLayer> layer : Layers)
		{
			std::shared_ptr<SchemaItem> result = layer->getItemById(id);

			if (result != nullptr)
			{
				return result;
			}
		}

		return std::shared_ptr<SchemaItem>();
	}

	template<typename SchemaItemType>
	bool Schema::hasSchemaItemType() const
	{
		for (std::shared_ptr<VFrame30::SchemaLayer> layer : Layers)
		{
			for (const SchemaItemPtr& item : layer->Items)
			{
				if (dynamic_cast<SchemaItemType>(item) != nullptr)
				{
					return true;
				}
			}
		}

		return false;
	}

	// Properties and Data
	//

	// Guid
	//
	QUuid Schema::guid() const noexcept
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
	QString Schema::schemaId() const noexcept
	{
		return m_schemaID;
	}

	void Schema::setSchemaId(const QString& id)
	{
		m_schemaID = id;
	}

	// Caption
	//
	QString Schema::caption() const noexcept
	{
		return m_caption;
	}

	void Schema::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Tags
	//
	QString Schema::tagsAsString() const noexcept
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

	QStringList Schema::tagsAsList() const noexcept
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

		m_tags = tags.split(QRegExp("\\W+"), Qt::SkipEmptyParts);

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
	SchemaUnit Schema::unit() const noexcept
	{
		return m_unit;
	}

	void Schema::setUnit(SchemaUnit value) noexcept
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

	int Schema::activeLayerIndex() const
	{
		return m_activeLayer;
	}

	QUuid Schema::activeLayerGuid() const
	{
		try
		{
			return Layers.at(m_activeLayer)->guid();
		}
		catch (...)
		{
			assert(false);
			return QUuid();
		}
	}

	std::shared_ptr<VFrame30::SchemaLayer> Schema::activeLayer() const
	{
		try
		{
			return Layers.at(m_activeLayer);
		}
		catch (...)
		{
			assert(false);
			return std::shared_ptr<VFrame30::SchemaLayer>();
		}
	}

	void Schema::setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		for (int i = 0; i < static_cast<int>(Layers.size()); i++)
		{
			if (Layers[i] == layer)
			{
				m_activeLayer = i;
				return;
			}
		}

		// Layer was not found
		//
		assert(false);
		return;
	}

	double Schema::gridSize() const noexcept
	{
		return m_gridSize;
	}

	void Schema::setGridSize(double value)
	{
		m_gridSize = value;
	}

	int Schema::pinGridStep() const noexcept
	{
		return m_pinGridStep;
	}

	void Schema::setPinGridStep(int value)
	{
		m_pinGridStep = value;
	}

	bool Schema::excludeFromBuild() const noexcept
	{
		return m_excludeFromBuild;
	}

	void Schema::setExcludeFromBuild(bool value)
	{
		m_excludeFromBuild = value;
	}

	QColor Schema::backgroundColor() const noexcept
	{
		return m_backgroundColor;
	}

	void Schema::setBackgroundColor(const QColor& value)
	{
		m_backgroundColor = value;
	}

	bool Schema::isLogicSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::LogicSchema*>(this) != nullptr;
	}

	bool Schema::isUfbSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::UfbSchema*>(this) != nullptr;
	}

	bool Schema::isMonitorSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::MonitorSchema*>(this) != nullptr;
	}

	bool Schema::isTuningSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::TuningSchema*>(this) != nullptr;
	}

	bool Schema::isDiagSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::DiagSchema*>(this) != nullptr;
	}

	LogicSchema* Schema::toLogicSchema() noexcept
	{
		return dynamic_cast<VFrame30::LogicSchema*>(this);
	}

	const LogicSchema* Schema::toLogicSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::LogicSchema*>(this);
	}

	UfbSchema* Schema::toUfbSchema() noexcept
	{
		return dynamic_cast<VFrame30::UfbSchema*>(this);
	}

	const UfbSchema* Schema::toUfbSchema() const noexcept
	{
		return dynamic_cast<const VFrame30::UfbSchema*>(this);
	}

	int Schema::changeset() const noexcept
	{
		return m_changeset;
	}

	void Schema::setChangeset(int value)
	{
		m_changeset = value;
	}

	//
	//
	//				SchemaDetails
	//
	//

	SchemaDetails::SchemaDetails() noexcept
	{
	}

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

		// Get list of receivers/transmitters
		//
		QSet<QString> connections;
		QSet<QString> loopbacks;

		if (schema->isLogicSchema() == true)
		{
			for (std::shared_ptr<SchemaLayer> layer : schema->Layers)
			{
				if (layer->compile() == true)
				{
					for (std::shared_ptr<SchemaItem> item : layer->Items)
					{
						if (item->isType<SchemaItemConnection>() == true)
						{
							const SchemaItemConnection* connItem = item->toType<SchemaItemConnection>();
							assert(connItem);

							connections.insert(connItem->connectionIds());
						}

						if (item->isType<SchemaItemReceiver>() == true)
						{
							const SchemaItemReceiver* receiver = item->toType<SchemaItemReceiver>();
							Q_ASSERT(receiver);

							for (const QString& appSignalId : receiver->appSignalIdsAsList())
							{
								signalIds << appSignalId;
							}
						}

						if (item->isType<SchemaItemLoopback>() == true)
						{
							const SchemaItemLoopback* lb = item->toType<SchemaItemLoopback>();
							Q_ASSERT(lb);

							loopbacks << lb->loopbackId();
						}
					}

					break;
				}
			}
		}

		// Get tags, kept in lowercase
		//
		QStringList tags = schema->tagsAsList();

		for (QString& tag : tags)
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
		QVariant tagsVariant(tags);
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
		jsonObject.insert("ItemGuids", QJsonValue::fromVariant(guidsVariant));

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
			qDebug() << "Schema details pasing error: " << parseError.errorString();
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

				// ExcludedeFromBuild
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

				// Tags
				//
				m_tags.clear();
				QStringList tagsList = jsonObject.value(QLatin1String("Tags")).toVariant().toStringList();

				for (const QString& str : tagsList)
				{
					m_tags.insert(str);
				}

				// ItemGuids
				//
				m_guids.clear();

				QStringList guidList = jsonObject.value(QLatin1String("ItemGuids")).toVariant().toStringList();

				std::for_each(guidList.begin(), guidList.end(), [this](const QString& str){	m_guids.insert(QUuid(str));});
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

		for (const QString& t : m_tags)
		{
			message->add_tags(t.toStdString());
		}

		for (const QUuid& u : m_guids)
		{
			::Proto::Uuid* uuidMesage = message->add_guids();
			assert(uuidMesage);

			uuidMesage->set_uuid(&u, sizeof(u));
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

		m_tags.clear();
		int tagCount = message.tags_size();
		for (int i = 0; i < tagCount; i++)
		{
			QString tag = QString::fromStdString(message.tags(i));
			m_tags.insert(tag);
		}

		m_guids.clear();
		int guidCount = message.guids_size();
		for (int i = 0; i < guidCount; i++)
		{
			QUuid guid = Proto::Read(message.guids(i));
			m_guids.insert(guid);
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

		QUuid textAsUuid(searchText);

		if (textAsUuid.isNull() == false &&
			m_guids.find(textAsUuid) != m_guids.end())
		{
			return true;
		}

		return false;
	}

	bool SchemaDetails::hasTag(const QString& tag) const
	{
		return m_tags.find(tag.trimmed().toLower()) != m_tags.end();
	}

	bool SchemaDetails::hasTag(const QStringList& tags) const
	{
		for (const QString& tag : tags)
		{
			if (m_tags.find(tag.trimmed().toLower()) != m_tags.end())
			{
				return true;
			}
		}

		return false;
	}

	const std::set<QString>& SchemaDetails::tags() const
	{
		return m_tags;
	}

	bool SchemaDetails::hasEquipmentId(const QString& equipmentId) const
	{
		QStringList eqs = m_equipmentId.split(' ', Qt::SkipEmptyParts);

		bool result = eqs.contains(equipmentId, Qt::CaseInsensitive);
		return result;
	}

	bool SchemaDetails::hasSignal(const QString& signalId) const
	{
		return m_signals.find(signalId) != std::cend(m_signals);
	}

	SchemaDetailsSet::SchemaDetailsSet() :
		Proto::ObjectSerialization<SchemaDetailsSet>(Proto::ProtoCompress::Never)
	{
	}

	bool SchemaDetailsSet::SaveData(Proto::Envelope* envelopeMessage) const
	{
		std::string className = {"SchemaDetailsSet"};
		quint32 classnamehash = CUtils::GetClassHashCode(className);
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

		m_details[details->m_schemaId] = details;

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

		if (index >=0 && index < static_cast<int>(m_details.size()))
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
		if (schemaIndex >=0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_caption;
		}

		return {};
	}

	QString SchemaDetailsSet::schemaIdByIndex(int schemaIndex) const
	{
		if (schemaIndex >=0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_schemaId;
		}

		return {};
	}

}

