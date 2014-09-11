#include "Stable.h"
#include "FblItem.h"

namespace VFrame30
{
	//
	// CFblConnectionPoint
	//
	CFblConnectionPoint::CFblConnectionPoint()
	{
		m_guid = QUuid::createUuid();

		m_point.X = 0;
		m_point.Y = 0;

		m_dirrection = ConnectionDirrection::Input;
		m_signalGuid = QUuid();
	}

	CFblConnectionPoint::CFblConnectionPoint(double x, double y, ConnectionDirrection dirrection, const QUuid& guid)
	{
		m_point.X = x;
		m_point.Y = y;
		m_dirrection = dirrection;
		m_guid = guid;

		m_signalGuid = QUuid();
	}

	CFblConnectionPoint::CFblConnectionPoint(const ::Proto::FblConnectionPoint& cpm)
	{
		LoadData(cpm);
	}

	bool CFblConnectionPoint::SaveData(::Proto::FblConnectionPoint* cpm) const
	{
		m_point.SaveData(cpm->mutable_point());
		cpm->set_dirrection(static_cast<::Proto::ConnectionDirrection>(dirrection()));
		VFrame30::Proto::Write(cpm->mutable_guid(), m_guid);

		if (m_signalGuid.isNull() == false)  // != GUI_NULL
		{
			VFrame30::Proto::Write(cpm->mutable_signalguid(), m_signalGuid);
		}

		if (m_signalStrID.isEmpty() == false)
		{
			VFrame30::Proto::Write(cpm->mutable_signalstrid(), m_signalStrID);
		}

		if (m_signalCaption.isEmpty() == false)
		{
			VFrame30::Proto::Write(cpm->mutable_signalcaption(), m_signalCaption);
		}

		return true;
	}

	bool CFblConnectionPoint::LoadData(const ::Proto::FblConnectionPoint& cpm)
	{
		m_point.LoadData(cpm.point());
		m_dirrection = static_cast<ConnectionDirrection>(cpm.dirrection());
		m_guid = VFrame30::Proto::Read(cpm.guid());

		if (cpm.has_signalguid() == true)
		{
			m_signalGuid = VFrame30::Proto::Read(cpm.signalguid());
		}
		else
		{
			m_signalGuid = QUuid();
		}

		if (cpm.has_signalstrid() == true)
		{
			m_signalStrID = VFrame30::Proto::Read(cpm.signalstrid());
		}
		else
		{
			m_signalStrID.clear();
		}

		if (cpm.has_signalcaption() == true)
		{
			m_signalCaption = VFrame30::Proto::Read(cpm.signalcaption());
		}
		else
		{
			m_signalCaption.clear();
		}

		return true;
	}


	//
	// CFblItem
	//

	CFblItem::CFblItem(void)
	{
	}

	CFblItem::~CFblItem(void)
	{
	}
		
	// Serialization
	//
	bool CFblItem::SaveData(::Proto::Envelope* message) const
	{
		::Proto::FblItem* fblItemMessage = message->mutable_videoitem()->mutable_fblitem();

		for (auto pt = m_inputPoints.cbegin(); pt != m_inputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Input);

			::Proto::FblConnectionPoint* pConnectionPointMessage = fblItemMessage->add_points();
			pt->SaveData(pConnectionPointMessage);
		}

		for (auto pt = m_outputPoints.cbegin(); pt != m_outputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Output);

			::Proto::FblConnectionPoint* pConnectionPointMessage = fblItemMessage->add_points();
			pt->SaveData(pConnectionPointMessage);
		}

		//itemMessage->set_weight(weight);

		return true;
	}

	bool CFblItem::LoadData(const ::Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		if (message.videoitem().has_fblitem() == false)
		{
			assert(message.videoitem().has_fblitem());
			return false;
		}
		
		// --
		//
		const ::Proto::FblItem& fblItemMessage = message.videoitem().fblitem();

		m_inputPoints.clear();
		m_outputPoints.clear();
		for (int i = 0; i < fblItemMessage.points().size(); i++)
		{
			const ::Proto::FblConnectionPoint& cpm = fblItemMessage.points(i);
			CFblConnectionPoint cp(cpm);

			if (cp.dirrection() == ConnectionDirrection::Input)
			{
				m_inputPoints.push_back(cp);
			}
			else
			{
				m_outputPoints.push_back(cp);
			}
		}

		//weight = itemMessage.weight();

		return true;
	}

	// Drawing stuff
	//
	void CFblItem::DrawPinCross(QPainter* p, double x, double y, double pinWidth) const
	{
		double crossSize = pinWidth / 3;

		QPointF cross_pt1(x - crossSize, y - crossSize);
		QPointF cross_pt2(x + crossSize, y - crossSize);
		QPointF cross_pt3(x + crossSize, y + crossSize);
		QPointF cross_pt4(x - crossSize, y + crossSize);

		p->drawLine(cross_pt1, cross_pt3);
		p->drawLine(cross_pt2, cross_pt4);
	}

	void CFblItem::DrawPinJoint(QPainter* p, double x, double y, double pinWidth) const
	{
		double radius = static_cast<double>(pinWidth) / 8.0;

		p->drawEllipse(QPointF(x, y), radius, radius);
	}

	double CFblItem::GetPinWidth(SchemeUnit unit, int dpi) const
	{
		double pinWidth = static_cast<float>(mm2in(3));	// 3 мм!

		if (unit == SchemeUnit::Display)
		{
			pinWidth = pinWidth * dpi;
		}

		return pinWidth;
	}

	// Connections
	//
	const std::list<CFblConnectionPoint>& CFblItem::inputs() const
	{
		return m_inputPoints;
	}

	const std::list<CFblConnectionPoint>& CFblItem::outputs() const
	{
		return m_outputPoints;
	}

	std::list<CFblConnectionPoint>* CFblItem::mutableInputs()
	{
		return &m_inputPoints;
	}

	std::list<CFblConnectionPoint>* CFblItem::mutableOutputs()
	{
		return &m_outputPoints;
	}

	bool CFblItem::GetConnectionPoint(const QUuid& guid, CFblConnectionPoint* pResult) const
	{
		if (pResult == nullptr)
		{
			assert(pResult);
			return false;
		}

		for (auto pt = m_inputPoints.cbegin(); pt != m_inputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Input);
			if (pt->guid() == guid)
			{
				*pResult = *pt;
				return true;
			}
		}

		for (auto pt = m_outputPoints.cbegin(); pt != m_outputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Output);
			if (pt->guid() == guid)
			{
				*pResult = *pt;
				return true;
			}
		}

		return false;
	}

	int CFblItem::inputsCount() const
	{
		return static_cast<int>(m_inputPoints.size());
	}

	int CFblItem::outputsCount() const
	{
		return static_cast<int>(m_outputPoints.size());
	}

	void CFblItem::AddInput()
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Input, QUuid::createUuid());
		m_inputPoints.push_back(cp);
	}

	void CFblItem::AddOutput()
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Output, QUuid::createUuid());
		m_outputPoints.push_back(cp);
	}

	void CFblItem::ClearAssociatedConnections()
	{
		std::for_each(m_inputPoints.begin(), m_inputPoints.end(), [](CFblConnectionPoint& pin){pin.ClearAssociattdIOs();});
		std::for_each(m_outputPoints.begin(), m_outputPoints.end(), [](CFblConnectionPoint& pin){pin.ClearAssociattdIOs();});
	}

	void CFblItem::SetConnectionsPos()
	{
		assert(false);	// должно быть реализовано у наследников, CFblItemLine, CFblItemRect
		return;
	}

	bool CFblItem::GetConnectionPointPos(const QUuid&, VideoItemPoint*) const
	{
		assert(false);	// должно быть реализовано у наследников, CFblItemLine, CFblItemRect
		return false;
	}


	// Properties and Data
	//
}
