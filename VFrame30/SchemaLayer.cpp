#include "SchemaLayer.h"
#include "FblItemRect.h"
#include "PosConnectionImpl.h"


namespace VFrame30
{

	Factory<VFrame30::SchemaLayer> VideoLayerFactory;


	SchemaLayer::SchemaLayer(void) :
		Proto::ObjectSerialization<SchemaLayer>(Proto::ProtoCompress::Never)
	{
		Init("Undifined Layer", false);
	}

	SchemaLayer::SchemaLayer(const QString& name, bool compile) :
		Proto::ObjectSerialization<SchemaLayer>(Proto::ProtoCompress::Never)
	{
		Init(name, compile);
	}

	SchemaLayer::~SchemaLayer(void)
	{
	}

	void SchemaLayer::Init(const QString& name, bool compile)
	{
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
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Required field, hash of the class name, it restores the class.

		auto layer = message->mutable_schemalayer();

		Proto::Write(layer->mutable_uuid(), m_guid);
		Proto::Write(layer->mutable_name(), m_name);

		layer->set_compile(m_compile);
		layer->set_show(m_show);
		layer->set_print(m_print);

		// ��������� Items
		//
		for (auto item = Items.begin(); item != Items.end(); ++item)
		{
			Proto::Envelope* pItemMessage = layer->add_items();

			if (item->get()->Save(pItemMessage) == false)
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
		Items.clear();

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
			
			Items.push_back(item);
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
	std::shared_ptr<SchemaItem> SchemaLayer::getItemById(const QUuid& id) const
	{
		auto foundItem = std::find_if(Items.begin(), Items.end(),
			[&](const std::shared_ptr<SchemaItem>& vi)
			{
				return vi->guid() == id;
			});

		if (foundItem != Items.end())
		{
			return *foundItem;
		}
		else
		{
			return std::shared_ptr<SchemaItem>();
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

	int SchemaLayer::GetPinPosConnectinCount(SchemaPoint pinPos, SchemaUnit /*unit*/) const
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
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const SchemaItemPtr& item = *it;

			if (qobject_cast<SchemaItemType>(item.get()) != nullptr &&
				item->isIntersectPoint(x, y) == true)
			{
				return std::dynamic_pointer_cast<SchemaItemType>(item);
			}
		}

		return std::shared_ptr<SchemaItem>();
	}

	std::shared_ptr<SchemaItem> SchemaLayer::getItemUnderPoint(QPointF point, QString className) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const SchemaItemPtr& item = *it;

			if (item->isIntersectPoint(x, y) == true)
			{
				if ((className.isEmpty() == true) ||
					(className == item->metaObject()->className()))
				{
					return item;
				}
			}
		}

		return std::shared_ptr<SchemaItem>();
	}

	std::list<std::shared_ptr<SchemaItem>> SchemaLayer::getItemListUnderPoint(QPointF point, QString className /*= ""*/) const
	{
		double x = point.x();
		double y = point.y();

		std::list<std::shared_ptr<SchemaItem>> out;

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const SchemaItemPtr& item = *it;

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

		std::copy_if(Items.begin(), Items.end(), std::back_inserter(out),
				[&rect](std::shared_ptr<SchemaItem> item)
				{
			        return item->isIntersectRect(rect.x(), rect.y(), rect.width(), rect.height());
				}
			);

		return out;
	}

	std::shared_ptr<SchemaItem> SchemaLayer::findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const SchemaItemPtr& item = *it;

			if (dynamic_cast<VFrame30::FblItemRect*>(item.get()) != nullptr &&
			    item->isIntersectPoint(x, y) == true)
			{
				VFrame30::FblItemRect* fbl = dynamic_cast<VFrame30::FblItemRect*>(item.get());

				fbl->SetConnectionsPos(gridSize, pinGridStep);

				const std::vector<VFrame30::AfbPin>& inputs = fbl->inputs();
				const std::vector<VFrame30::AfbPin>& outputs = fbl->outputs();

				for (const VFrame30::AfbPin& pin : inputs)
				{
					if (pin.point() == point)
					{
						return item;
					}
				}

				for (const VFrame30::AfbPin& pin : outputs)
				{
					if (pin.point() == point)
					{
						return item;
					}
				}
			}
		}

		return std::shared_ptr<SchemaItem>();
	}

	// Properties
	//
	QUuid SchemaLayer::guid() const
	{
		return m_guid;
	}
	void SchemaLayer::setGuid(const QUuid& guid)
	{
		m_guid = guid;
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
