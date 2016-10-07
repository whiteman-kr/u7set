#include "Stable.h"
#include "SchemaItemLink.h"
#include "SchemaLayer.h"

namespace VFrame30
{
	SchemaItemLink::SchemaItemLink(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemLink::SchemaItemLink(SchemaUnit unit) :
		FblItemLine(unit)
	{
		//AddInput();
		//AddOutput();
	}


	SchemaItemLink::~SchemaItemLink(void)
	{
	}

	// Serialization
	//
	bool SchemaItemLink::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemLine::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		/*Proto::VideoItemLink* linkMessage = */message->mutable_schemaitem()->mutable_link();

		//linkMessage->set_weight(weight);
		//linkMessage->set_linecolor(lineColor);

		return true;
	}

	bool SchemaItemLink::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
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
		if (message.schemaitem().has_link() == false)
		{
			assert(message.schemaitem().has_link());
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
	void SchemaItemLink::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer* pLayer) const
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
		const std::list<SchemaPoint>& poinlist = GetPointList();
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
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(pen);

		p->drawPolyline(polyline);

		// Пины входов/выходов
		//

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPen redPen(QColor(0xE0D00000));
		redPen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);			// Don't use getter!

		// вход/выход - рисование красного креста 
		//
		auto drawPin = [&](SchemaPoint pt)
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
	void SchemaItemLink::SetConnectionsPos(double /*gridSize*/, int /*pinGridStep*/)
	{
		return;
	}

	bool SchemaItemLink::GetConnectionPointPos(const QUuid&, SchemaPoint*, double, int) const
	{
		return false;
	}


	// Properties and Data
	//
}
