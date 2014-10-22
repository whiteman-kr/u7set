#include "Stable.h"
#include "SchemeLayer.h"

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

		message->set_classnamehash(classnamehash);	// ������������ ����, �� ����� ������, �� ���� ����������������� �����.

		auto pMutableVideoLayer = message->mutable_videolayer();

		Proto::Write(pMutableVideoLayer->mutable_uuid(), m_guid);
		Proto::Write(pMutableVideoLayer->mutable_name(), m_name);

		pMutableVideoLayer->set_compile(m_compile);
		pMutableVideoLayer->set_show(m_show);
		pMutableVideoLayer->set_print(m_print);

		// ��������� Items
		//
		for (auto item = Items.begin(); item != Items.end(); ++item)
		{
			Proto::Envelope* pItemMessage = pMutableVideoLayer->add_items();

			if (item->get()->Save(pItemMessage) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool SchemeLayer::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videolayer() == false)
		{
			assert(message.has_videolayer());
			return false;
		}

		const Proto::VideoLayer& videoLayer = message.videolayer();

		m_guid = Proto::Read(videoLayer.uuid());
		m_name = Proto::Read(videoLayer.name());

		m_compile = videoLayer.compile();
		m_show = videoLayer.show();
		m_print = videoLayer.print();

		// ��������� ��������
		//
		Items.clear();

		for (int i = 0; i < videoLayer.items().size(); i++)
		{
			VideoItem* pItem = VideoItem::Create(videoLayer.items(i));
			
			if (pItem == nullptr)
			{
				assert(pItem != nullptr);
				continue;
			}
			
			Items.push_back(std::shared_ptr<VideoItem>(pItem));
		}

		if (videoLayer.items().size() != (int)Items.size())
		{
			assert(videoLayer.items().size() == (int)Items.size());
			return false;
		}

		return true;
	}

	SchemeLayer* SchemeLayer::CreateObject(const Proto::Envelope& message)
	{
		// ��� ������� ����� ��������� ������ ���� ���������
		//
		if (message.has_videolayer() == false)
		{
			assert(message.has_videolayer());
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
	void SchemeLayer::ConnectionMapPosInc(VideoItemPoint pinPos)
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

	int SchemeLayer::GetPinPosConnectinCount(VideoItemPoint pinPos, SchemeUnit /*unit*/) const
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

	std::shared_ptr<VideoItem> SchemeLayer::getItemUnderPoint(QPointF point) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const std::shared_ptr<VFrame30::VideoItem>& item = *it;

			if (item->IsIntersectPoint(x, y) == true)
			{
				return item;
			}
		}

		return std::shared_ptr<VideoItem>();
	}

	std::list<std::shared_ptr<VideoItem>> SchemeLayer::getItemListInRectangle(const QRectF& rect) const
	{
		std::list<std::shared_ptr<VideoItem>> out;

		std::copy_if(Items.begin(), Items.end(), std::back_inserter(out),
				[&rect](std::shared_ptr<VideoItem> item)
				{
					return item->IsIntersectRect(rect.x(), rect.y(), rect.width(), rect.height());
				}
			);

		return out;
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
