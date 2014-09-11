#include "Stable.h"
#include "VideoFrame.h"
#include "FblItem.h"
#include "VideoItemLink.h"
#include "HorzVertLinks.h"
#include "VideoFrameWidgetAgent.h"
#include "../VFrame30/VFrame30.pb.h"

namespace VFrame30
{
	Factory<VFrame30::CVideoFrame> VideoFrameFactory;

	CVideoFrame::CVideoFrame(void)
	{
		Init();
	}

	CVideoFrame::~CVideoFrame(void)
	{
	}

	void CVideoFrame::Init(void)
	{
		m_guid = QUuid();  // GUID_NULL

		m_width = 0;
		m_height = 0;
	}

	// Serialization
	//
	bool CVideoFrame::SaveData(::Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = CVFrameUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// Обязательное поле, хэш имени класса, по нему восстанавливается класс.
		
		auto pMutableVideoFrame = message->mutable_videoframe();

		VFrame30::Proto::Write(pMutableVideoFrame->mutable_guid(), m_guid);
		VFrame30::Proto::Write(pMutableVideoFrame->mutable_strid(), m_strID);
		VFrame30::Proto::Write(pMutableVideoFrame->mutable_caption(), m_caption);
		pMutableVideoFrame->set_width(m_width);
		pMutableVideoFrame->set_height(m_height);
		pMutableVideoFrame->set_unit(static_cast<::Proto::SchemeUnit>(m_unit));

		// Сохранить Layers
		//
		bool saveLayersResult = true;

		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			::Proto::Envelope* pLayerMessage = pMutableVideoFrame->add_layers();
			saveLayersResult &= layer->get()->Save(pLayerMessage);
		}

		return saveLayersResult;
	}

	bool CVideoFrame::LoadData(const ::Proto::Envelope& message)
	{
		if (message.has_videoframe() == false)
		{
			assert(message.has_videoframe());
			return false;
		}

		const ::Proto::VideoFrame& videoframe = message.videoframe();

		m_guid = VFrame30::Proto::Read(videoframe.guid());
		m_strID = VFrame30::Proto::Read(videoframe.strid());
		m_caption = VFrame30::Proto::Read(videoframe.caption());
		m_width = videoframe.width();
		m_height = videoframe.height();
		m_unit = static_cast<SchemeUnit>(videoframe.unit());

		// Прочитать Layers
		//
		Layers.clear();

		for (int i = 0; i < videoframe.layers().size(); i++)
		{
			CVideoLayer* pLayer = CVideoLayer::Create(videoframe.layers(i));
			
			if (pLayer == nullptr)
			{
				assert(pLayer);
				continue;
			}
			
			Layers.push_back(std::shared_ptr<CVideoLayer>(pLayer));
		}

		if (videoframe.layers().size() != (int)Layers.size())
		{
			assert(videoframe.layers().size() == (int)Layers.size());
			Layers.clear();
			return false;
		}

		return true;
	}

	CVideoFrame* CVideoFrame::CreateObject(const ::Proto::Envelope& message)
	{
		// Эта функция может создавать только один экземпляр
		//
		if (message.has_videoframe() == false)
		{
			assert(message.has_videoframe());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		CVideoFrame* pVideoFrame = VideoFrameFactory.Create(classNameHash);

		if (pVideoFrame == nullptr)
		{
			assert(pVideoFrame);
			return nullptr;
		}
		
		pVideoFrame->LoadData(message);

		return pVideoFrame;
	}

	void CVideoFrame::Draw(CDrawParam* drawParam, const QRectF& clipRect) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		// очистить клиентскую область, фоновым (серым) цветом
		//
		QPainter* p = drawParam->painter();
		//p->fill(0xB0, 0xB0, 0xB0);	-- Очистка происходит в CDrawParam::BeginPaint

		// Нарисовать лист
		//
		QRectF pageRect(0.0, 0.0, static_cast<qreal>(docWidth()), static_cast<qreal>(docHeight()));
		p->fillRect(pageRect, Qt::white);

		// Рисование элементов по видимым слоям
		//

		// Цикл по слоям и их элементам
		//
		double clipX = static_cast<double>(clipRect.left());
		double clipY = static_cast<double>(clipRect.top());
		double clipWidth = static_cast<double>(clipRect.width());
		double clipHeight = static_cast<double>(clipRect.height());

		for (auto layer = Layers.cbegin(); layer != Layers.cend(); ++layer)
		{
			const CVideoLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
			{
				const std::shared_ptr<CVideoItem>& item = *vi;

				if (item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					item->Draw(drawParam, this, pLayer);
				}
			}
		}
	}

	void CVideoFrame::Print()
	{
		assert(false);
		//::MessageBox(NULL, GetStrID().c_str(), _T("Print"), MB_OK);
	}

	void CVideoFrame::MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const
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
			const CVideoLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<CVideoItem>& item = *vi;

				if (item->acceptClick() == true && item->IsIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
				{
					RunClickScript(item, pVideoFrameWidgetAgent);
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

	void CVideoFrame::RunClickScript(const std::shared_ptr<CVideoItem>& videoItem, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const
	{
		if (pVideoFrameWidgetAgent == nullptr || videoItem->acceptClick() == false || videoItem->clickScript().isEmpty() == true)
		{
			assert(pVideoFrameWidgetAgent != nullptr);
			return;
		}

		// Extract script text from VideoItem
		//
		QString script = videoItem->clickScript();
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
#endif
		return;
	}

	int CVideoFrame::GetDocumentWidth(double DpiX, double zoom) const
	{
		if (unit() == SchemeUnit::Display)
		{
			return static_cast<int>(docWidth() * (zoom / 100.0));
		}
		else
		{
			return static_cast<int>(docWidth() * DpiX * (zoom / 100.0));
		}
	}

	int CVideoFrame::GetDocumentHeight(double DpiY, double zoom) const
	{
		if (unit() == SchemeUnit::Display)
		{
			return static_cast<int>(docHeight() * (zoom / 100.0));
		}
		else
		{
			return static_cast<int>(docHeight() * DpiY * (zoom / 100.0));
		}
	}

	int CVideoFrame::GetLayerCount() const
	{
		return (int)Layers.size();
	}

	void CVideoFrame::BuildFblConnectionMap() const
	{
		// --
		//
		CHorzVertLinks horzVertLinks;

		// Пройти по всем соединениям, заполнить horzlinks и vertlinks
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			CVideoLayer* pLayer = layer->get();

			for (auto item = pLayer->Items.begin(); item != pLayer->Items.end(); ++item)
			{
				if (item->get()->IsFblItem() == false)
				{
					continue;
				}

				CFblItem* pFblItem = dynamic_cast<CFblItem*>(item->get());
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

				CVideoItemLink* pVideoItemLink = dynamic_cast<CVideoItemLink*>(item->get());
				if (pVideoItemLink != nullptr)
				{
					const std::list<VideoItemPoint>& pointList = pVideoItemLink->GetPointList();
					
					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}
					
					// разложить кривую на отдельные отрезки и занести их в horzlinks и vertlinks
					//
					horzVertLinks.AddLinks(pointList, pVideoItemLink->guid());
				}
			}
		}

		// --
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			CVideoLayer* pLayer = layer->get();

			pLayer->connectionMap.clear();

			for (auto item = pLayer->Items.begin(); item != pLayer->Items.end(); ++item)
			{
				if (item->get()->IsFblItem() == false)
				{
					continue;
				}

				CFblItem* pFblItem = dynamic_cast<CFblItem*>(item->get());
				
				if (pFblItem == nullptr)
				{
					assert(pFblItem);
					continue;
				}

			
				// Если элемент CVideoItemLink, то в качестве координат пинов будут крайние точки
				//
				CVideoItemLink* pVideoItemLink = dynamic_cast<CVideoItemLink*>(item->get());

				if (pVideoItemLink != nullptr)
				{
					const std::list<VideoItemPoint>& pointList = pVideoItemLink->GetPointList();
					
					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}
					
					pLayer->ConnectionMapPosInc(pointList.front());
					pLayer->ConnectionMapPosInc(pointList.back());

					// проверить, не лежит ли пин на соединительной линии
					//
					if (horzVertLinks.IsPointOnLink(pointList.front(), pVideoItemLink->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pointList.front());
					}

					if (horzVertLinks.IsPointOnLink(pointList.back(), pVideoItemLink->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pointList.back());
					}

					continue;
				}

				// Вычисление и сохранение координат пинов для обычного Fbl элементы
				//
				pFblItem->SetConnectionsPos();

				// Найти в connectionMap такую координату и если она есть до увеличить счетчик пинов,
				// если нет, то создать запись в списке
				//
				const std::list<CFblConnectionPoint>& inputs = pFblItem->inputs();
				for (auto pin = inputs.begin(); pin != inputs.end(); ++pin)
				{
					VideoItemPoint pinPos = pin->point();
					
					pLayer->ConnectionMapPosInc(pinPos);

					// проверить, не лежит ли пин на соединительной линии
					//
					if (horzVertLinks.IsPointOnLink(pinPos, item->get()->guid()) == true)
					{
						pLayer->ConnectionMapPosInc(pinPos);
					}
				}

				const std::list<CFblConnectionPoint>& outputs = pFblItem->outputs();
				for (auto pin = outputs.begin(); pin != outputs.end(); ++pin)
				{
					VideoItemPoint pinPos = pin->point();

					pLayer->ConnectionMapPosInc(pinPos);

					// проверить, не лежит ли пин на соединительной линии
					//
					if (horzVertLinks.IsPointOnLink(pinPos, item->get()->guid()) == true)
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
	QUuid CVideoFrame::guid() const
	{
		return m_guid;
	}

	void CVideoFrame::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// StrID
	//
	QString CVideoFrame::strID() const
	{
		return m_strID;
	}

	void CVideoFrame::setStrID(const QString& strID)
	{
		m_strID = strID;
	}

	// Caption
	//
	QString CVideoFrame::caption() const
	{
		return m_caption;
	}

	void CVideoFrame::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Width
	//
	double CVideoFrame::docWidth() const
	{
		return m_width;
	}

	void CVideoFrame::setDocWidth(double width)
	{
		m_width = width;
	}

	// Height
	//
	double CVideoFrame::docHeight() const
	{
		return m_height;
	}

	void CVideoFrame::setDocHeight(double height)
	{
		m_height = height;
	}

	// Unit
	//
	SchemeUnit CVideoFrame::unit() const
	{
		return m_unit;
	}

	void CVideoFrame::setUnit(SchemeUnit value)
	{
		m_unit = value;
	}
}
