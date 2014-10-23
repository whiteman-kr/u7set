#include "Stable.h"
#include "Scheme.h"
#include "FblItem.h"
#include "VideoItemLink.h"
#include "HorzVertLinks.h"
#include "VideoFrameWidgetAgent.h"
#include "../include/ProtoSerialization.h"

namespace VFrame30
{
	Factory<VFrame30::Scheme> VideoFrameFactory;

	Scheme::Scheme(void)
	{
		Init();
	}

	Scheme::~Scheme(void)
	{
	}

	void Scheme::Init(void)
	{
		m_guid = QUuid();  // GUID_NULL

		m_width = 0;
		m_height = 0;
	}

	// Serialization
	//
	bool Scheme::SaveData(Proto::Envelope* message) const
	{
		std::string className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);	// ������������ ����, ��� ����� ������, �� ���� ����������������� �����.
		
		Proto::VideoFrame* pMutableVideoFrame = message->mutable_videoframe();

		Proto::Write(pMutableVideoFrame->mutable_uuid(), m_guid);
		Proto::Write(pMutableVideoFrame->mutable_strid(), m_strID);
		Proto::Write(pMutableVideoFrame->mutable_caption(), m_caption);

		pMutableVideoFrame->set_width(m_width);
		pMutableVideoFrame->set_height(m_height);
		pMutableVideoFrame->set_unit(static_cast<Proto::SchemeUnit>(m_unit));

		// ��������� Layers
		//
		bool saveLayersResult = true;

		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			Proto::Envelope* pLayerMessage = pMutableVideoFrame->add_layers();
			saveLayersResult &= layer->get()->Save(pLayerMessage);
		}

		// Save Afb Collection
		//
		m_afbCollection.SaveData(pMutableVideoFrame->mutable_afbs());

		return saveLayersResult;
	}

	bool Scheme::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoframe() == false)
		{
			assert(message.has_videoframe());
			return false;
		}

		const Proto::VideoFrame& videoframe = message.videoframe();

		m_guid = Proto::Read(videoframe.uuid());
		m_strID = Proto::Read(videoframe.strid());
		m_caption = Proto::Read(videoframe.caption());
		m_width = videoframe.width();
		m_height = videoframe.height();
		m_unit = static_cast<SchemeUnit>(videoframe.unit());

		// ��������� Layers
		//
		Layers.clear();

		for (int i = 0; i < videoframe.layers().size(); i++)
		{
			SchemeLayer* pLayer = SchemeLayer::Create(videoframe.layers(i));
			
			if (pLayer == nullptr)
			{
				assert(pLayer);
				continue;
			}
			
			Layers.push_back(std::shared_ptr<SchemeLayer>(pLayer));
		}

		if (videoframe.layers().size() != (int)Layers.size())
		{
			assert(videoframe.layers().size() == (int)Layers.size());
			Layers.clear();
			return false;
		}

		// Load Afb Collection
		//
		m_afbCollection.LoadData(videoframe.afbs());

		return true;
	}

	Scheme* Scheme::CreateObject(const Proto::Envelope& message)
	{
		// ��� ������� ����� ��������� ������ ���� ���������
		//
		if (message.has_videoframe() == false)
		{
			assert(message.has_videoframe());
			return nullptr;
		}

		quint32 classNameHash = message.classnamehash();
		Scheme* pVideoFrame = VideoFrameFactory.Create(classNameHash);

		if (pVideoFrame == nullptr)
		{
			assert(pVideoFrame);
			return nullptr;
		}
		
		pVideoFrame->LoadData(message);

		return pVideoFrame;
	}

	void Scheme::Draw(CDrawParam* drawParam, const QRectF& clipRect) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		// �������� ���������� �������, ������� (�����) ������
		//
		QPainter* p = drawParam->painter();
		//p->fill(0xB0, 0xB0, 0xB0);	-- ������� ���������� � CDrawParam::BeginPaint

		// ���������� ����
		//
		QRectF pageRect(0.0, 0.0, static_cast<qreal>(docWidth()), static_cast<qreal>(docHeight()));
		p->fillRect(pageRect, Qt::white);

		// ��������� ��������� �� ������� �����
		//

		// ���� �� ����� � �� ���������
		//
		double clipX = static_cast<double>(clipRect.left());
		double clipY = static_cast<double>(clipRect.top());
		double clipWidth = static_cast<double>(clipRect.width());
		double clipHeight = static_cast<double>(clipRect.height());

		for (auto layer = Layers.cbegin(); layer != Layers.cend(); ++layer)
		{
			const SchemeLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
			{
				const std::shared_ptr<VideoItem>& item = *vi;

				if (item->IsIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
				{
					item->Draw(drawParam, this, pLayer);
				}
			}
		}
	}

	void Scheme::Print()
	{
		assert(false);
		//::MessageBox(NULL, GetStrID().c_str(), _T("Print"), MB_OK);
	}

	void Scheme::MouseClick(const QPointF& docPoint, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const
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
			const SchemeLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<VideoItem>& item = *vi;

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

	void Scheme::RunClickScript(const std::shared_ptr<VideoItem>& videoItem, VideoFrameWidgetAgent* pVideoFrameWidgetAgent) const
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

	int Scheme::GetDocumentWidth(double DpiX, double zoom) const
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

	int Scheme::GetDocumentHeight(double DpiY, double zoom) const
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

	int Scheme::GetLayerCount() const
	{
		return (int)Layers.size();
	}

	void Scheme::BuildFblConnectionMap() const
	{
		// --
		//
		CHorzVertLinks horzVertLinks;

		// ������ �� ���� �����������, ��������� horzlinks � vertlinks
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			SchemeLayer* pLayer = layer->get();

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

				VideoItemLink* pVideoItemLink = dynamic_cast<VideoItemLink*>(item->get());
				if (pVideoItemLink != nullptr)
				{
					const std::list<VideoItemPoint>& pointList = pVideoItemLink->GetPointList();
					
					if (pointList.size() < 2)
					{
						assert(pointList.size() >= 2);
						continue;
					}
					
					// ��������� ������ �� ��������� ������� � ������� �� � horzlinks � vertlinks
					//
					horzVertLinks.AddLinks(pointList, pVideoItemLink->guid());
				}
			}
		}

		// --
		//
		for (auto layer = Layers.begin(); layer != Layers.end(); ++layer)
		{
			SchemeLayer* pLayer = layer->get();

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

			
				// ���� ������� CVideoItemLink, �� � �������� ��������� ����� ����� ������� �����
				//
				VideoItemLink* pVideoItemLink = dynamic_cast<VideoItemLink*>(item->get());

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

					// ���������, �� ����� �� ��� �� �������������� �����
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

				// ���������� � ���������� ��������� ����� ��� �������� Fbl ��������
				//
				pFblItem->SetConnectionsPos();

				// ����� � connectionMap ����� ���������� � ���� ��� ���� �� ��������� ������� �����,
				// ���� ���, �� ������� ������ � ������
				//
				const std::list<CFblConnectionPoint>& inputs = pFblItem->inputs();
				for (auto pin = inputs.begin(); pin != inputs.end(); ++pin)
				{
					VideoItemPoint pinPos = pin->point();
					
					pLayer->ConnectionMapPosInc(pinPos);

					// ���������, �� ����� �� ��� �� �������������� �����
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

					// ���������, �� ����� �� ��� �� �������������� �����
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
	QUuid Scheme::guid() const
	{
		return m_guid;
	}

	void Scheme::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	// StrID
	//
	QString Scheme::strID() const
	{
		return m_strID;
	}

	void Scheme::setStrID(const QString& strID)
	{
		m_strID = strID;
	}

	// Caption
	//
	QString Scheme::caption() const
	{
		return m_caption;
	}

	void Scheme::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	// Width
	//
	double Scheme::docWidth() const
	{
		return m_width;
	}

	void Scheme::setDocWidth(double width)
	{
		m_width = width;
	}

	// Height
	//
	double Scheme::docHeight() const
	{
		return m_height;
	}

	void Scheme::setDocHeight(double height)
	{
		m_height = height;
	}

	// Unit
	//
	SchemeUnit Scheme::unit() const
	{
		return m_unit;
	}

	void Scheme::setUnit(SchemeUnit value)
	{
		m_unit = value;
	}

	const Afbl::AfbElementCollection& Scheme::afbCollection() const
	{
		return m_afbCollection;
	}

	void Scheme::setAfbCollection(const std::vector<std::shared_ptr<Afbl::AfbElement>>& elements)
	{
		// set new collection
		//
		m_afbCollection.setElements(elements);

		// update videoframe items
		//
	}


}
