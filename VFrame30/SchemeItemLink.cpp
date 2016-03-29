#include "Stable.h"
#include "SchemeItemLink.h"
#include "SchemaLayer.h"

namespace VFrame30
{
	SchemeItemLink::SchemeItemLink(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	SchemeItemLink::SchemeItemLink(SchemaUnit unit) :
		FblItemLine(unit)
	{
		//AddInput();
		//AddOutput();
	}


	SchemeItemLink::~SchemeItemLink(void)
	{
	}

	// Serialization
	//
	bool SchemeItemLink::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemLine::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}
		
		// --
		//
		/*Proto::VideoItemLink* linkMessage = */message->mutable_schemeitem()->mutable_link();

		//linkMessage->set_weight(weight);
		//linkMessage->set_linecolor(lineColor);

		return true;
	}

	bool SchemeItemLink::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
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
		if (message.schemeitem().has_link() == false)
		{
			assert(message.schemeitem().has_link());
		}

		//const Proto::VideoItemLink& linkMessage = message.videoitem(0).link();

		//weight = linkMessage.weight();
		//lineColor = linkMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
	//
	void SchemeItemLink::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer* pLayer) const
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
		const std::list<SchemePoint>& poinlist = GetPointList();
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

		// ���� ������/�������
		//

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPen redPen(QColor(0xE0D00000));
		redPen.setWidthF(m_weight);			// Don't use getter!

		// ����/����� - ��������� �������� ������ 
		//
		auto drawPin = [&](SchemePoint pt)
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
					// ��������� �������� ������
					//
					p->setPen(redPen);
					DrawPinCross(p, pt.X, pt.Y, pinWidth);
				}
			};

		drawPin(poinlist.front());
		drawPin(poinlist.back());

		return;
	}

	// ��������� ���������� �����
	//
	void SchemeItemLink::SetConnectionsPos(double /*gridSize*/, int /*pinGridStep*/)
	{
		return;
	}

	bool SchemeItemLink::GetConnectionPointPos(const QUuid&, SchemePoint*, double, int) const
	{
		return false;
	}


	// Properties and Data
	//
}
