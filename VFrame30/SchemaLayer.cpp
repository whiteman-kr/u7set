#include "SchemaLayer.h"
#include "SchemaItem.h"
#include "FblItemRect.h"
#include "PosConnectionImpl.h"


namespace VFrame30
{
	::Factory<VFrame30::SchemaLayer> VideoLayerFactory;

	ScriptSchemaLayer::ScriptSchemaLayer(SchemaLayerPtr schemaLayer) :
		m_schemaLayer(std::move(schemaLayer))
	{
		Q_ASSERT(m_schemaLayer);
	}

	QString ScriptSchemaLayer::caption() const
	{
		return m_schemaLayer ?
					m_schemaLayer->name() :
					QString{};
	}

	bool ScriptSchemaLayer::visible() const
	{
		return m_schemaLayer ?
					m_schemaLayer->show() :
					false;
	}

	void ScriptSchemaLayer::setVisible(bool value)
	{
		if (m_schemaLayer != nullptr)
		{
			m_schemaLayer->setShow(value);
		}

		return;
	}

	SchemaLayer::SchemaLayer(void) :
		Proto::ObjectSerialization<SchemaLayer>(Proto::ProtoCompress::Never)
	{
		Init("Undifined Layer", false);
	}

	SchemaLayer::SchemaLayer(Schema* parentSchema, const QString& name, bool compile) :
		Proto::ObjectSerialization<SchemaLayer>(Proto::ProtoCompress::Never),
		m_parentSchema(parentSchema)
	{
		Init(name, compile);
	}

	SchemaLayer::~SchemaLayer(void)
	{
		clearItems();	// It will reset parents for items.
	}

	void SchemaLayer::Init(const QString& name, bool compile)
	{
		m_items.reserve(64);
		m_guid = QUuid::createUuid();
		m_name = name;
		m_compile = compile;
		return;
	}

	// Serialization
	//
	bool SchemaLayer::SaveData(Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnamehash);	// Required field, hash of the class name, it restores the class.

		auto layer = message->mutable_schemalayer();

		Proto::Write(layer->mutable_uuid(), m_guid);
		Proto::Write(layer->mutable_name(), m_name);

		layer->set_compile(m_compile);
		layer->set_show(m_show);
		layer->set_print(m_print);

		// Save Items
		//
		for (const auto& item : m_items)
		{
			Proto::Envelope* pItemMessage = layer->add_items();

			if (item->Save(pItemMessage) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool SchemaLayer::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemalayer() == false)
		{
			assert(message.has_schemalayer());
			return false;
		}

		const Proto::SchemaLayer& layer = message.schemalayer();

		m_guid = Proto::Read(layer.uuid());
		Proto::Read(layer.name(), &m_name);

		m_compile = layer.compile();
		m_show = layer.show();
		m_print = layer.print();

		// Read schema items
		//
		m_items.clear();

		for (int i = 0; i < layer.items().size(); i++)
		{
			std::shared_ptr<SchemaItem> item = SchemaItem::Create(layer.items(i));
			
			if (item == nullptr)
			{
				assert(item != nullptr);
				continue;
			}

			if (item->isType<PosConnectionImpl>() == true)
			{
				// Check if it has only one point then drop this item
				//
				if (item->toType<PosConnectionImpl>()->GetPointList().size() < 2)
				{
					qDebug() << "Warning, PosConnectionImpl is skipped while loading schema layer. it has only " << item->toType<PosConnectionImpl>()->GetPointList().size() << " points.";
					continue;
				}
			}
			
			pushBackItem(item);
		}

		return true;
	}

	std::shared_ptr<SchemaLayer> SchemaLayer::CreateObject(const Proto::Envelope& message)
	{
		// This func can create only one instance
		//
		if (message.has_schemalayer() == false)
		{
			assert(message.has_schemalayer());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		std::shared_ptr<SchemaLayer> layer = VideoLayerFactory.Create(classNameHash);

		if (layer == nullptr)
		{
			assert(layer != nullptr);
			return nullptr;
		}

		layer->LoadData(message);

		return layer;
	}

	// Methods
	//
	std::shared_ptr<SchemaItem> SchemaLayer::getItemById(const QUuid id) const
	{
		auto foundItem = std::find_if(m_items.begin(), m_items.end(),
			[&](const std::shared_ptr<SchemaItem>& vi)
			{
				return vi->guid() == id;
			});

		if (foundItem != m_items.end())
		{
			return *foundItem;
		}
		else
		{
			return {};
		}
	}

	void SchemaLayer::ConnectionMapPosInc(SchemaPoint pinPos)
	{
		auto mapitem = connectionMap.find(pinPos);

		if (mapitem == connectionMap.end())
		{
			connectionMap[pinPos] = 1;
		}
		else
		{
			connectionMap[pinPos] = mapitem->second + 1;
		}
	}

	int SchemaLayer::GetPinPosConnectinCount(SchemaPoint pinPos) const
	{
		auto mapitem = connectionMap.find(pinPos);

		if (mapitem != connectionMap.end())
		{
			return mapitem->second;
		}
		else
		{
			return 0;
		}
	}

	template<typename SchemaItemType>
	std::shared_ptr<SchemaItemType> SchemaLayer::getItemUnderPointByType(QPointF point) const
	{
		std::shared_ptr<SchemaItemType> result;
		double x = point.x();
		double y = point.y();

		for (const auto& item : m_items | std::views::reverse)
		{
			if (qobject_cast<SchemaItemType>(item.get()) != nullptr &&
				item->isIntersectPoint(x, y) == true)
			{
				result = std::dynamic_pointer_cast<SchemaItemType>(item);
				break;
			}
		}

		return result;
	}

	std::shared_ptr<SchemaItem> SchemaLayer::getItemUnderPoint(QPointF point, const QString& className /*= QString{}*/) const
	{
		std::shared_ptr<SchemaItem> result;
		double x = point.x();
		double y = point.y();

		for (const auto& item : m_items | std::views::reverse)
		{
			if (item->isIntersectPoint(x, y) == true)
			{
				if ((className.isEmpty() == true) ||
					(className == item->metaObject()->className()))
				{
					result = item;
					break;
				}
			}
		}

		return result;
	}

	std::list<std::shared_ptr<SchemaItem>> SchemaLayer::getItemListUnderPoint(QPointF point, const QString& className /*= QString{}*/) const
	{
		double x = point.x();
		double y = point.y();

		std::list<std::shared_ptr<SchemaItem>> out;

		for (const auto& item : m_items | std::views::reverse)
		{
			if (item->isIntersectPoint(x, y) == true)
			{
				if ((className.isEmpty() == true) ||
					(className == item->metaObject()->className()))
				{
					out.push_back(item);
				}
			}
		}

		return out;
	}

	std::list<std::shared_ptr<SchemaItem>> SchemaLayer::getItemListInRectangle(const QRectF& rect) const
	{
		std::list<std::shared_ptr<SchemaItem>> out;

		std::copy_if(m_items.begin(), m_items.end(), std::back_inserter(out),
				[&rect](std::shared_ptr<SchemaItem> item)
				{
			        return item->isIntersectRect(rect.x(), rect.y(), rect.width(), rect.height());
				}
			);

		return out;
	}

	std::shared_ptr<SchemaItem> SchemaLayer::findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const
	{
		std::shared_ptr<SchemaItem> result;
		double x = point.x();
		double y = point.y();

		for (const auto& item : m_items | std::views::reverse)
		{
			if (dynamic_cast<VFrame30::FblItemRect*>(item.get()) != nullptr &&
			    item->isIntersectPoint(x, y) == true)
			{
				VFrame30::FblItemRect* fbl = dynamic_cast<VFrame30::FblItemRect*>(item.get());

				fbl->SetConnectionsPos(gridSize, pinGridStep);

				for (const VFrame30::AfbPin& pin : fbl->inputs())
				{
					if (pin.point() == point)
					{
						result = item;
						return result;
					}
				}

				for (const VFrame30::AfbPin& pin : fbl->outputs())
				{
					if (pin.point() == point)
					{
						result = item;
						return result;
					}
				}
			}
		}

		return result;
	}

	void SchemaLayer::clearItems()
	{
		std::ranges::for_each(m_items, [](auto& item)
		{
			item->setParentLayer({});
		});
		m_items.clear();
		return;
	}

	bool SchemaLayer::removeItem(const SchemaItemPtr& item)
	{
		auto removed = std::erase(m_items, item);
		if (removed != 0)
		{
			item->setParentLayer({});
		}

		return removed != 0;
	}

	void SchemaLayer::pushBackItem(SchemaItemPtr item)
	{
		Q_ASSERT(item);
		Q_ASSERT(shared_from_this());

		item->setParentLayer(shared_from_this());
		m_items.push_back(std::move(item));

		return;
	}

	const std::vector<std::shared_ptr<SchemaItem>>& SchemaLayer::items() const
	{
		return m_items;
	}

	// Properties
	//
	Schema* SchemaLayer::parentSchema()
	{
		return m_parentSchema;
	}

	const Schema* SchemaLayer::parentSchema() const
	{
		return m_parentSchema;
	}

	void SchemaLayer::setParentSchema(Schema* parentSchema)
	{
		m_parentSchema = parentSchema;
	}

	QUuid SchemaLayer::guid() const
	{
		return m_guid;
	}
	void SchemaLayer::setGuid(QUuid value)
	{
		m_guid = value;
	}

	QString SchemaLayer::name() const
	{
		return m_name;
	}
	void SchemaLayer::setName(const QString& value)
	{
		m_name = value;
	}

	bool SchemaLayer::compile() const
	{
		return m_compile;
	}
	void SchemaLayer::setCompile(bool value)
	{
		m_compile = value;
	}

	bool SchemaLayer::show() const
	{
		return m_show;
	}
	void SchemaLayer::setShow(bool value)
	{
		m_show = value;
	}

	bool SchemaLayer::print() const
	{
		return m_print;
	}

	void SchemaLayer::setPrint(bool value)
	{
		m_print = value;
	}
}
