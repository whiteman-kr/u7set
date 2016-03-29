#include "Stable.h"
#include "Schema.h"
#include "FblItem.h"
#include "SchemeItemLink.h"
#include "HorzVertLinks.h"
#include "../include/ProtoSerialization.h"

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
		ADD_PROPERTY_GETTER_SETTER(bool, ExcludeFromBuild, true, Schema::excludeFromBuild, Schema::setExcludeFromBuild);
		ADD_PROPERTY_GETTER_SETTER(double, SchemaWidth, true, Schema::docWidth, Schema::setDocWidth);
		ADD_PROPERTY_GETTER_SETTER(double, SchemaHeight, true, Schema::docHeight, Schema::setDocHeight);

		m_guid = QUuid();  // GUID_NULL

		m_width = 0;
		m_height = 0;
	}

	// Serialization
	//
	bool Schema::SaveData(Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Required field, class name hash code, by it instance is created
		
		Proto::Schema* mutableSchema = message->mutable_schema();

		Proto::Write(mutableSchema->mutable_uuid(), m_guid);
		Proto::Write(mutableSchema->mutable_strid(), m_strID);
		Proto::Write(mutableSchema->mutable_caption(), m_caption);

		mutableSchema->set_width(m_width);
		mutableSchema->set_height(m_height);
		mutableSchema->set_unit(static_cast<Proto::SchemaUnit>(m_unit));
		mutableSchema->set_excludefrombuild(m_excludeFromBuild);

		// Save Layers
		//
		bool saveLayersResult = true;

		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			Proto::Envelope* pLayerMessage = mutableSchema->add_layers();
			saveLayersResult &= layer->get()->Save(pLayerMessage);
		}

		// Save Afb Collection
		//
		m_afbCollection.SaveData(mutableSchema->mutable_afbs());

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
		Proto::Read(schema.strid(), &m_strID);
		Proto::Read(schema.caption(), &m_caption);
		m_width = schema.width();
		m_height = schema.height();
		m_unit = static_cast<SchemaUnit>(schema.unit());
		m_excludeFromBuild = schema.excludefrombuild();

		// ѕрочитать Layers
		//
		Layers.clear();

		for (int i = 0; i < schema.layers().size(); i++)
		{
			SchemaLayer* pLayer = SchemaLayer::Create(schema.layers(i));
			
			if (pLayer == nullptr)
			{
				assert(pLayer);
				continue;
			}
			
			Layers.push_back(std::shared_ptr<SchemaLayer>(pLayer));
		}

		if (schema.layers().size() != (int)Layers.size())
		{
			assert(schema.layers().size() == (int)Layers.size());
			Layers.clear();
			return false;
		}

		// Load Afb Collection
		//
		m_afbCollection.LoadData(schema.afbs());

		return true;
	}

	Schema* Schema::CreateObject(const Proto::Envelope& message)
	{
		// This function can create only one instance
		//
		if (message.has_schema() == false)
		{
			assert(message.has_schema());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		Schema* schema = SchemaFactory.Create(classNameHash);

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
		p->fillRect(pageRect, Qt::white);

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
					item->Draw(drawParam, this, pLayer);
				}
			}
		}
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

	void Schema::RunClickScript(const std::shared_ptr<SchemaItem>& schemeItem/*, VideoFrameWidgetAgent* pVideoFrameWidgetAgent*/) const
	{
		assert(false);
		Q_UNUSED(schemeItem);

/*		if (pVideoFrameWidgetAgent == nullptr || schemeItem->acceptClick() == false || schemeItem->clickScript().isEmpty() == true)
		{
			assert(pVideoFrameWidgetAgent != nullptr);
			return;
		}

		// Extract script text from SchemeItem
		//
		QString script = schemeItem->clickScript();
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
				if (item->get()->IsFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item->get());
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

				SchemeItemLink* schemeItemLink = dynamic_cast<SchemeItemLink*>(item->get());
				if (schemeItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemeItemLink->GetPointList();
					
					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}
					
					// разложить кривую на отдельные отрезки и занести их в horzlinks и vertlinks
					//
					horzVertLinks.AddLinks(pointList, schemeItemLink->guid());
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
				if (item->get()->IsFblItem() == false)
				{
					continue;
				}

				FblItem* pFblItem = dynamic_cast<FblItem*>(item->get());
				
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

			
				// ≈сли элемент SchemeItemLink, то в качестве координат пинов будут крайние точки
				//
				SchemeItemLink* schemeItemLink = dynamic_cast<SchemeItemLink*>(item->get());

				if (schemeItemLink != nullptr)
				{
					const std::list<SchemaPoint>& pointList = schemeItemLink->GetPointList();
					
					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}
					
					pLayer->ConnectionMapPosInc(pointList.front());
					pLayer->ConnectionMapPosInc(pointList.back());

					// проверить, не лежит ли пин на соединительной линии
					//
					if (horzVertLinks.IsPointOnLink(pointList.front(), schemeItemLink->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pointList.front());
					}

					if (horzVertLinks.IsPointOnLink(pointList.back(), schemeItemLink->guid()) == true)
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
				const std::list<AfbPin>& inputs = pFblItem->inputs();
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

				const std::list<AfbPin>& outputs = pFblItem->outputs();
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

	// StrID
	//
	QString Schema::strID() const
	{
		return m_strID;
	}

	void Schema::setStrID(const QString& strID)
	{
		m_strID = strID;
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

	const Afb::AfbElementCollection& Schema::afbCollection() const
	{
		return m_afbCollection;
	}

	void Schema::setAfbCollection(const std::vector<std::shared_ptr<Afb::AfbElement>>& elements)
	{
		// set new collection
		//
		m_afbCollection.setElements(elements);

		// update schema items
		//
	}


}
