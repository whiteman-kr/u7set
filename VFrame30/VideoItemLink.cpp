#include "Stable.h"
#include "VideoItemLink.h"
#include "SchemeLayer.h"

namespace VFrame30
{
	VideoItemLink::VideoItemLink(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemLink::VideoItemLink(SchemeUnit unit) : 
		FblItemLine(unit)
	{
		//AddInput();
		//AddOutput();
	}


	VideoItemLink::~VideoItemLink(void)
	{
	}

	// Serialization
	//
	bool VideoItemLink::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemLine::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}
		
		// --
		//
		/*Proto::VideoItemLink* linkMessage = */message->mutable_videoitem()->mutable_link();

		//linkMessage->set_weight(weight);
		//linkMessage->set_linecolor(lineColor);

		return true;
	}

	bool VideoItemLink::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = FblItemLine::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_link() == false)
		{
			assert(message.videoitem().has_link());
		}

		//const Proto::VideoItemLink& linkMessage = message.videoitem(0).link();

		//weight = linkMessage.weight();
		//lineColor = linkMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void VideoItemLink::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer* pLayer) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		int dpiX = 96;

		QPaintDevice* pPaintDevice = p->device();
		if (pPaintDevice == nullptr)
		{
			assert(pPaintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = pPaintDevice->logicalDpiX();
		}

		// Draw the main part
		//
		const std::list<VideoItemPoint>& poinlist = GetPointList();
		if (poinlist.size() < 2)
		{
			assert(poinlist.size() >= 2);
			return;
		}

		QPolygonF polyline(static_cast<int>(poinlist.size()));
		int index = 0;

		for (auto pt = poinlist.cbegin(); pt != poinlist.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight);			// Don't use getter!
		p->setPen(pen);

		p->drawPolyline(polyline);

		// Пины входов/выходов
		//

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPen redPen(QColor(0xE0D00000));
		redPen.setWidthF(m_weight);			// Don't use getter!

		// вход/выход - рисование красного креста 
		//
		auto drawPin = [&](VideoItemPoint pt)
			{
				int connectionCount = pLayer->GetPinPosConnectinCount(pt, itemUnit());

				if (connectionCount > 1)
				{
					p->setBrush(pen.color());
					p->setPen(pen);
					DrawPinJoint(p, pt.X, pt.Y, pinWidth);
					p->setBrush(Qt::NoBrush);
				}
				else
				{
					// рисование красного креста
					//
					p->setPen(redPen);
					DrawPinCross(p, pt.X, pt.Y, pinWidth);
				}
			};

		drawPin(poinlist.front());
		drawPin(poinlist.back());

		return;
	}

	// Вычислить координаты точки
	//
	void VideoItemLink::SetConnectionsPos()
	{
		return;
	}

	//bool CVideoItemLink::GetConnectionPointPos(const GUID& connectionPointGuid, VideoItemPoint* pResult) const
	bool VideoItemLink::GetConnectionPointPos(const QUuid&, VideoItemPoint*) const
	{
		return false;
	}


	// Properties and Data
	//
}
