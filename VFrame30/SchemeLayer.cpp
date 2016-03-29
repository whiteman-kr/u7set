#include "Stable.h"
#include "SchemeLayer.h"
#include "FblItemRect.h"


namespace VFrame30
{

	Factory<VFrame30::SchemeLayer> VideoLayerFactory;

	SchemeLayer::SchemeLayer(void)
	{
		Init("Undifined Layer", false);
	}

	SchemeLayer::SchemeLayer(const QString& name, bool compile)
	{
		Init(name, compile);
	}

	SchemeLayer::~SchemeLayer(void)
	{
	}

	void SchemeLayer::Init(const QString& name, bool compile)
	{
		m_guid = QUuid::createUuid();
		m_name = name;
		m_compile = compile;
		return;
	}

	// Serialization
	//
	bool SchemeLayer::SaveData(Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		auto layer = message->mutable_schemelayer();

		Proto::Write(layer->mutable_uuid(), m_guid);
		Proto::Write(layer->mutable_name(), m_name);

		layer->set_compile(m_compile);
		layer->set_show(m_show);
		layer->set_print(m_print);

		// Сохранить Items
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

	bool SchemeLayer::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemelayer() == false)
		{
			assert(message.has_schemelayer());
			return false;
		}

		const Proto::SchemeLayer& layer = message.schemelayer();

		m_guid = Proto::Read(layer.uuid());
		Proto::Read(layer.name(), &m_name);

		m_compile = layer.compile();
		m_show = layer.show();
		m_print = layer.print();

		// Прочитать элементы
		//
		Items.clear();

		for (int i = 0; i < layer.items().size(); i++)
		{
			SchemeItem* pItem = SchemeItem::Create(layer.items(i));
			
			if (pItem == nullptr)
			{
				assert(pItem != nullptr);
				continue;
			}
			
			Items.push_back(std::shared_ptr<SchemeItem>(pItem));
		}

		if (layer.items().size() != (int)Items.size())
		{
			assert(layer.items().size() == (int)Items.size());
			return false;
		}

		return true;
	}

	SchemeLayer* SchemeLayer::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_schemelayer() == false)
		{
			assert(message.has_schemelayer());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		SchemeLayer* pLayer = VideoLayerFactory.Create(classNameHash);

		if (pLayer == nullptr)
		{
			assert(pLayer != nullptr);
			return nullptr;
		}

		pLayer->LoadData(message);

		return pLayer;
	}

	// Methods
	//

	std::shared_ptr<SchemeItem> SchemeLayer::getItemById(const QUuid& id) const
	{
		auto foundItem = std::find_if(Items.begin(), Items.end(),
			[&](const std::shared_ptr<SchemeItem>& vi)
			{
				return vi->guid() == id;
			});

		if (foundItem != Items.end())
		{
			return *foundItem;
		}
		else
		{
			return std::shared_ptr<SchemeItem>();
		}
	}

	void SchemeLayer::ConnectionMapPosInc(SchemePoint pinPos)
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

	int SchemeLayer::GetPinPosConnectinCount(SchemePoint pinPos, SchemaUnit /*unit*/) const
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

	std::shared_ptr<SchemeItem> SchemeLayer::getItemUnderPoint(QPointF point, QString className) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const std::shared_ptr<VFrame30::SchemeItem>& item = *it;

			if (item->IsIntersectPoint(x, y) == true)
			{
				if ((className.isEmpty() == true) ||
					(className == item->metaObject()->className()))
				{
					return item;
				}
			}
		}

		return std::shared_ptr<SchemeItem>();
	}

	std::list<std::shared_ptr<SchemeItem>> SchemeLayer::getItemListInRectangle(const QRectF& rect) const
	{
		std::list<std::shared_ptr<SchemeItem>> out;

		std::copy_if(Items.begin(), Items.end(), std::back_inserter(out),
				[&rect](std::shared_ptr<SchemeItem> item)
				{
					return item->IsIntersectRect(rect.x(), rect.y(), rect.width(), rect.height());
				}
			);

		return out;
	}

	std::shared_ptr<SchemeItem> SchemeLayer::findPinUnderPoint(QPointF point, double gridSize, int pinGridStep) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const std::shared_ptr<VFrame30::SchemeItem>& item = *it;

			if (dynamic_cast<VFrame30::FblItemRect*>(item.get()) != nullptr &&
				item->IsIntersectPoint(x, y) == true)
			{
				VFrame30::FblItemRect* fbl = dynamic_cast<VFrame30::FblItemRect*>(item.get());

				fbl->SetConnectionsPos(gridSize, pinGridStep);

				const std::list<VFrame30::AfbPin>& inputs = fbl->inputs();
				const std::list<VFrame30::AfbPin>& outputs = fbl->outputs();

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

		return std::shared_ptr<SchemeItem>();
	}

	// Properties
	//
	QUuid SchemeLayer::guid() const
	{
		return m_guid;
	}
	void SchemeLayer::setGuid(const QUuid& guid)
	{
		m_guid = guid;
	}

	QString SchemeLayer::name() const
	{
		return m_name;
	}
	void SchemeLayer::setName(const QString& value)
	{
		m_name = value;
	}

	bool SchemeLayer::compile() const
	{
		return m_compile;
	}
	void SchemeLayer::setCompile(bool value)
	{
		m_compile = value;
	}

	bool SchemeLayer::show() const
	{
		return m_show;
	}
	void SchemeLayer::setShow(bool value)
	{
		m_show = value;
	}

	bool SchemeLayer::print() const
	{
		return m_print;
	}

	void SchemeLayer::setPrint(bool value)
	{
		m_print = value;
	}
}
