#include "Stable.h"
#include "VideoLayer.h"

namespace VFrame30
{
	Factory<VFrame30::CVideoLayer> VideoLayerFactory;

	CVideoLayer::CVideoLayer(void)
	{
		Init("Undifined Layer", false);
	}

	CVideoLayer::CVideoLayer(const QString& name, bool compile)
	{
		Init(name, compile);
	}

	CVideoLayer::~CVideoLayer(void)
	{
	}

	void CVideoLayer::Init(const QString& name, bool compile)
	{
		m_guid = QUuid::createUuid();
		m_name = name;
		m_compile = compile;
		return;
	}

	// Serialization
	//
	bool CVideoLayer::SaveData(Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = CVFrameUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хш имени класса, по нему восстанавливается класс.

		auto pMutableVideoLayer = message->mutable_videolayer();

		Proto::Write(pMutableVideoLayer->mutable_uuid(), m_guid);
		Proto::Write(pMutableVideoLayer->mutable_name(), m_name);

		pMutableVideoLayer->set_compile(m_compile);
		pMutableVideoLayer->set_show(m_show);
		pMutableVideoLayer->set_print(m_print);

		// Сохранить Items
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

	bool CVideoLayer::LoadData(const Proto::Envelope& message)
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

		// Прочитать элементы
		//
		Items.clear();

		for (int i = 0; i < videoLayer.items().size(); i++)
		{
			CVideoItem* pItem = CVideoItem::Create(videoLayer.items(i));
			
			if (pItem == nullptr)
			{
				assert(pItem != nullptr);
				continue;
			}
			
			Items.push_back(std::shared_ptr<CVideoItem>(pItem));
		}

		if (videoLayer.items().size() != (int)Items.size())
		{
			assert(videoLayer.items().size() == (int)Items.size());
			return false;
		}

		return true;
	}

	CVideoLayer* CVideoLayer::CreateObject(const Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_videolayer() == false)
		{
			assert(message.has_videolayer());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		CVideoLayer* pLayer = VideoLayerFactory.Create(classNameHash);

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
	void CVideoLayer::ConnectionMapPosInc(VideoItemPoint pinPos)
	{
		pinPos.X = CVFrameUtils::Round(pinPos.X, 5);
		pinPos.Y = CVFrameUtils::Round(pinPos.Y, 5);

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

	int CVideoLayer::GetPinPosConnectinCount(VideoItemPoint pinPos) const
	{
		pinPos.X = CVFrameUtils::Round(pinPos.X, 5);
		pinPos.Y = CVFrameUtils::Round(pinPos.Y, 5);

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

	std::shared_ptr<CVideoItem> CVideoLayer::getItemUnderPoint(QPointF point) const
	{
		double x = point.x();
		double y = point.y();

		for (auto it = Items.rbegin(); it != Items.rend(); it ++)
		{
			const std::shared_ptr<VFrame30::CVideoItem>& item = *it;

			if (item->IsIntersectPoint(x, y) == true)
			{
				return item;
			}
		}

		return std::shared_ptr<CVideoItem>();
	}

	std::list<std::shared_ptr<CVideoItem>> CVideoLayer::getItemListInRectangle(const QRectF& rect) const
	{
		std::list<std::shared_ptr<CVideoItem>> out;

		std::copy_if(Items.begin(), Items.end(), std::back_inserter(out),
				[&rect](std::shared_ptr<CVideoItem> item)
				{
					return item->IsIntersectRect(rect.x(), rect.y(), rect.width(), rect.height());
				}
			);

		return out;
	}

	// Properties
	//
	QUuid CVideoLayer::guid() const
	{
		return m_guid;
	}
	void CVideoLayer::setGuid(const QUuid& guid)
	{
		m_guid = guid;
	}

	QString CVideoLayer::name() const
	{
		return m_name;
	}
	void CVideoLayer::setName(const QString& value)
	{
		m_name = value;
	}

	bool CVideoLayer::compile() const
	{
		return m_compile;
	}
	void CVideoLayer::setCompile(bool value)
	{
		m_compile = value;
	}

	bool CVideoLayer::show() const
	{
		return m_show;
	}
	void CVideoLayer::setShow(bool value)
	{
		m_show = value;
	}

	bool CVideoLayer::print() const
	{
		return m_print;
	}

	void CVideoLayer::setPrint(bool value)
	{
		m_print = value;
	}
}
