#include "Schema.h"
#include "LogicSchema.h"
#include "UfbSchema.h"
#include "MonitorSchema.h"
#include "DiagSchema.h"
#include "SchemaView.h"
#include "FblItem.h"
#include "SchemaItemAfb.h"
#include "SchemaItemUfb.h"
#include "SchemaItemLink.h"
#include "SchemaItemConnection.h"
#include "HorzVertLinks.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "../lib/ProtoSerialization.h"


namespace VFrame30
{
	Factory<VFrame30::Schema> SchemaFactory;

	Schema::Schema(void)
	{
		Init();
	}

	Schema::~Schema(void)
	{
	}

	void Schema::Init(void)
	{
		ADD_PROPERTY_GETTER(QString, "SchemaID", true, Schema::schemaId);
		ADD_PROPERTY_GETTER(int, "Changeset", true, Schema::changeset);
		ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, Schema::caption, Schema::setCaption);
		ADD_PROPERTY_GETTER_SETTER(bool, "ExcludeFromBuild", true, Schema::excludeFromBuild, Schema::setExcludeFromBuild);
		ADD_PROPERTY_GETTER_SETTER(double, "SchemaWidth", true, Schema::docWidthRegional, Schema::setDocWidthRegional);
		ADD_PROPERTY_GETTER_SETTER(double, "SchemaHeight", true, Schema::docHeightRegional, Schema::setDocHeightRegional);
		ADD_PROPERTY_GETTER_SETTER(QColor, "BackgroundColor", true, Schema::backgroundColor, Schema::setBackgroundColor);

		m_guid = QUuid();  // GUID_NULL

		m_width = 0;
		m_height = 0;
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

//			for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
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
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return false;
		}

		const Proto::Schema& schema = message.schema();

		m_guid = Proto::Read(schema.uuid());
		Proto::Read(schema.schemaid(), &m_schemaID);
		Proto::Read(schema.caption(), &m_caption);
		m_width = schema.width();
		m_height = schema.height();
		m_unit = static_cast<SchemaUnit>(schema.unit());
		m_excludeFromBuild = schema.excludefrombuild();

		if (schema.has_backgroundcolor() == true)
		{
			m_backgroundColor = schema.backgroundcolor();
		}

		// ѕрочитать Layers
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
		//p->fill(0xB0, 0xB0, 0xB0);	-- ќчистка происходит в CDrawParam::BeginPaint

		// Ќарисовать лист
		//
		QRectF pageRect(0.0, 0.0, static_cast<qreal>(docWidth()), static_cast<qreal>(docHeight()));
		p->fillRect(pageRect, backgroundColor());

		// Draw items by layers which has Show flag
		//
		double clipX = static_cast<double>(clipRect.left());
		double clipY = static_cast<double>(clipRect.top());
		double clipWidth = static_cast<double>(clipRect.width());
		double clipHeight = static_cast<double>(clipRect.height());

		for (auto layer = Layers.cbegin(); layer != Layers.cend(); ++layer)
		{
			const SchemaLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
			{
				const std::shared_ptr<SchemaItem>& item = *vi;

				if (item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					if (drawParam->isMonitorMode() == true)
					{
						SchemaView* view = drawParam->schemaView();
						assert(view);

						item->preDrawEvent(view->globalScript(), view->jsEngine());
					}

					item->Draw(drawParam, this, pLayer);

					if (item->isCommented() == true)
					{
						item->drawCommentDim(drawParam);
					}

					// Draw lastScriptError after drawing item
					//
					if (item->lastScriptError().isEmpty() == false)
					{
						item->DrawScriptError(drawParam);
					}
				}
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

				if (item->acceptClick() == true && item->IsIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
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

		// ѕройти по всем соединени€м, заполнить horzlinks и vertlinks
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
					
					// разложить кривую на отдельные отрезки и занести их в horzlinks и vertlinks
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

			
				// ≈сли элемент SchemaItemLink, то в качестве координат пинов будут крайние точки
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

					// проверить, не лежит ли пин на соединительной линии
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

				// ¬ычисление и сохранение координат пинов дл€ обычного Fbl элементы
				//
				pFblItem->SetConnectionsPos(gridSize(), pinGridStep());

				// Ќайти в connectionMap такую координату и если она есть до увеличить счетчик пинов,
				// если нет, то создать запись в списке
				//
				const std::vector<AfbPin>& inputs = pFblItem->inputs();
				for (auto pin = inputs.begin(); pin != inputs.end(); ++pin)
				{
					SchemaPoint pinPos = pin->point();
					
					pLayer->ConnectionMapPosInc(pinPos);

					// проверить, не лежит ли пин на соединительной линии
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

					// проверить, не лежит ли пин на соединительной линии
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

	QStringList Schema::getSignalList() const
	{
		// Do nothing, override to get signal list
		//
		QStringList result;
		return result;
	}

	QStringList Schema::getLabels() const
	{
		QStringList result;
		return result;
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

	QString Schema::details() const
	{
		return SchemaDetails::getDetailsString(this);
	}

	SchemaDetails Schema::parseDetails(const QString& details)
	{
		SchemaDetails d;
		d.parseDetails(details);

		return d;
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

	// Properties and Data
	//

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
		m_unit = value;
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

	//
	//
	//				SchemaDetails
	//
	//

	SchemaDetails::SchemaDetails()
	{
	}

	SchemaDetails::SchemaDetails(const QString& details)
	{
		parseDetails(details);
	}

//	SchemaDetails::SchemaDetails(SchemaDetails&& src)
//	{
//		m_version = src.m_version;
//		m_schemaId = src.m_schemaId;
//		m_caption = src.m_caption;
//		m_equipmentId = src.m_equipmentId;
//		m_signals = std::move(src.m_signals);
//		m_labels = std::move(src.m_labels);
//		m_guids = std::move(src.m_guids);
//	}

	QString SchemaDetails::getDetailsString(const Schema* schema)
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
		QSet<QString> signalIds = schema->getSignalList().toSet();

		// Get labels for AFBs
		//
		QStringList labels = schema->getLabels();

		// Get list of receivers/transmitters
		//
		QSet<QString> connections;

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

							connections.insert(connItem->connectionId());
						}

						if (item->isType<SchemaItemReceiver>() == true)
						{
							const SchemaItemReceiver* receiver = item->toType<SchemaItemReceiver>();
							assert(receiver);

							signalIds << receiver->appSignalId();
						}
					}

					break;
				}
			}
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
		QVariant signaListVariant(signalIds.toList());
		QVariant labelsVariant(labels);

		QVariant connectionsVariant(connections.toList());
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

		jsonObject.insert("Signals", QJsonValue::fromVariant(signaListVariant));
		jsonObject.insert("Labels", QJsonValue::fromVariant(labelsVariant));
		jsonObject.insert("Connections", QJsonValue::fromVariant(connectionsVariant));
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

		QUuid textAsUuid(searchText);

		if (textAsUuid.isNull() == false &&
			m_guids.find(textAsUuid) != m_guids.end())
		{
			return true;
		}

		return false;
	}
}
