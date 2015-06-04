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
		//m_signalGuid = QUuid();
	}

	CFblConnectionPoint::CFblConnectionPoint(double x, double y, ConnectionDirrection dirrection, const QUuid& guid, int operandIndex)
	{
		m_point.X = x;
		m_point.Y = y;
		m_dirrection = dirrection;
		m_guid = guid;
		m_afbOperandIndex = operandIndex;

//		m_signalGuid = QUuid();
	}

	CFblConnectionPoint::CFblConnectionPoint(const Proto::FblConnectionPoint& cpm)
	{
		LoadData(cpm);
	}

	bool CFblConnectionPoint::SaveData(Proto::FblConnectionPoint* cpm) const
	{
		m_point.SaveData(cpm->mutable_point());

		cpm->set_dirrection(static_cast<Proto::ConnectionDirrection>(dirrection()));

		Proto::Write(cpm->mutable_uuid(), m_guid);

		cpm->set_operandindex(m_afbOperandIndex);

//		if (m_signalGuid.isNull() == false)  // != GUI_NULL
//		{
//			Proto::Write(cpm->mutable_signaluuid(), m_signalGuid);
//		}

//		if (m_signalStrID.isEmpty() == false)
//		{
//			Proto::Write(cpm->mutable_signalstrid(), m_signalStrID);
//		}

//		if (m_signalCaption.isEmpty() == false)
//		{
//			Proto::Write(cpm->mutable_signalcaption(), m_signalCaption);
//		}

		return true;
	}

	bool CFblConnectionPoint::LoadData(const Proto::FblConnectionPoint& cpm)
	{
		m_point.LoadData(cpm.point());
		m_dirrection = static_cast<ConnectionDirrection>(cpm.dirrection());
		m_guid = Proto::Read(cpm.uuid());
		m_afbOperandIndex = cpm.operandindex();

//		if (cpm.has_signaluuid() == true)
//		{
//			m_signalGuid = Proto::Read(cpm.signaluuid());
//		}
//		else
//		{
//			m_signalGuid = QUuid();
//		}

//		if (cpm.has_signalstrid() == true)
//		{
//			Proto::Read(cpm.signalstrid(), &m_signalStrID);
//		}
//		else
//		{
//			m_signalStrID.clear();
//		}

//		if (cpm.has_signalcaption() == true)
//		{
//			Proto::Read(cpm.signalcaption(), &m_signalCaption);
//		}
//		else
//		{
//			m_signalCaption.clear();
//		}

		return true;
	}

	const VideoItemPoint& CFblConnectionPoint::point() const
	{
		return m_point;
	}

	void CFblConnectionPoint::setPoint(const VideoItemPoint& value)
	{
		m_point = value;
	}

	double CFblConnectionPoint::x() const
	{
		return m_point.X;
	}

	void CFblConnectionPoint::setX(double val)
	{
		m_point.X = val;
	}

	double CFblConnectionPoint::y() const
	{
		return m_point.Y;
	}

	void CFblConnectionPoint::setY(double val)
	{
		m_point.Y = val;
	}

	ConnectionDirrection CFblConnectionPoint::dirrection() const
	{
		return m_dirrection;
	}

	bool CFblConnectionPoint::IsInput() const
	{
		return m_dirrection == ConnectionDirrection::Input;
	}

	bool CFblConnectionPoint::IsOutput() const
	{
		return m_dirrection == ConnectionDirrection::Output;
	}

	const QUuid& CFblConnectionPoint::guid() const
	{
		return m_guid;
	}

	void CFblConnectionPoint::setGuid(const QUuid& guid)
	{
		m_guid = guid;
	}

	const std::list<QUuid>& CFblConnectionPoint::associatedIOs() const
	{
		return m_associatedIOs;
	}

	void CFblConnectionPoint::ClearAssociattdIOs()
	{
		m_associatedIOs.clear();
	}

	void CFblConnectionPoint::AddAssociattedIOs(const QUuid& guid)
	{
		m_associatedIOs.push_back(guid);
	}

	bool CFblConnectionPoint::HasConnection() const
	{
		assert(!(IsInput() && m_associatedIOs.size() > 1));
		return !m_associatedIOs.empty();
	}

	int CFblConnectionPoint::afbOperandIndex() const
	{
		return m_afbOperandIndex;
	}

	void CFblConnectionPoint::setAfbOperandIndex(int value)
	{
		m_afbOperandIndex = value;
	}

//	const QUuid& CFblConnectionPoint::signalGuid() const
//	{
//		return m_signalGuid;
//	}

//	void CFblConnectionPoint::setSignalGuid(const QUuid& guid)
//	{
//		m_signalGuid = guid;
//	}

//	const QString& CFblConnectionPoint::signalStrID() const
//	{
//		return m_signalStrID;
//	}

//	void CFblConnectionPoint::setSignalStrID(const QString& strid)
//	{
//		m_signalStrID = strid;
//	}

//	const QString& CFblConnectionPoint::signalCaption() const
//	{
//		return m_signalCaption;
//	}

//	void CFblConnectionPoint::setSignalCaption(const QString& caption)
//	{
//		m_signalCaption = caption;
//	}


	//
	// CFblItem
	//

	FblItem::FblItem(void)
	{
	}

	FblItem::~FblItem(void)
	{
	}
		
	// Serialization
	//
	bool FblItem::SaveData(Proto::Envelope* message) const
	{
		Proto::FblItem* fblItemMessage = message->mutable_videoitem()->mutable_fblitem();

		for (auto pt = m_inputPoints.cbegin(); pt != m_inputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Input);

			Proto::FblConnectionPoint* pConnectionPointMessage = fblItemMessage->add_points();
			pt->SaveData(pConnectionPointMessage);
		}

		for (auto pt = m_outputPoints.cbegin(); pt != m_outputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Output);

			Proto::FblConnectionPoint* pConnectionPointMessage = fblItemMessage->add_points();
			pt->SaveData(pConnectionPointMessage);
		}

		//itemMessage->set_weight(weight);

		return true;
	}

	bool FblItem::LoadData(const Proto::Envelope& message)
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
		const Proto::FblItem& fblItemMessage = message.videoitem().fblitem();

		m_inputPoints.clear();
		m_outputPoints.clear();
		for (int i = 0; i < fblItemMessage.points().size(); i++)
		{
			const Proto::FblConnectionPoint& cpm = fblItemMessage.points(i);
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
	void FblItem::DrawPinCross(QPainter* p, double x, double y, double pinWidth) const
	{
		double crossSize = pinWidth / 3;

		QPointF cross_pt1(x - crossSize, y - crossSize);
		QPointF cross_pt2(x + crossSize, y - crossSize);
		QPointF cross_pt3(x + crossSize, y + crossSize);
		QPointF cross_pt4(x - crossSize, y + crossSize);

		p->drawLine(cross_pt1, cross_pt3);
		p->drawLine(cross_pt2, cross_pt4);
	}

	void FblItem::DrawPinJoint(QPainter* p, double x, double y, double pinWidth) const
	{
		double radius = static_cast<double>(pinWidth) / 8.0;

		p->drawEllipse(QPointF(x, y), radius, radius);
	}

	double FblItem::GetPinWidth(SchemeUnit unit, int dpi) const
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
	const std::list<CFblConnectionPoint>& FblItem::inputs() const
	{
		return m_inputPoints;
	}

	const std::list<CFblConnectionPoint>& FblItem::outputs() const
	{
		return m_outputPoints;
	}

	std::list<CFblConnectionPoint>* FblItem::mutableInputs()
	{
		return &m_inputPoints;
	}

	std::list<CFblConnectionPoint>* FblItem::mutableOutputs()
	{
		return &m_outputPoints;
	}

	bool FblItem::GetConnectionPoint(const QUuid& guid, CFblConnectionPoint* pResult) const
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

	int FblItem::inputsCount() const
	{
		return static_cast<int>(m_inputPoints.size());
	}

	int FblItem::outputsCount() const
	{
		return static_cast<int>(m_outputPoints.size());
	}

	void FblItem::addInput()
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Input, QUuid::createUuid(), -1);
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(const Afbl::AfbElementSignal& s)
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Input, QUuid::createUuid(), s.operandIndex());
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(int opIndex)
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Input, QUuid::createUuid(), opIndex);
		m_inputPoints.push_back(cp);
	}

	void FblItem::removeAllInputs()
	{
		m_inputPoints.clear();
	}

	void FblItem::removeAllOutputs()
	{
		m_outputPoints.clear();
	}

	void FblItem::addOutput()
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Output, QUuid::createUuid(), -1);
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(const Afbl::AfbElementSignal& s)
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Output, QUuid::createUuid(), s.operandIndex());
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(int opIndex)
	{
		CFblConnectionPoint cp(0, 0, ConnectionDirrection::Output, QUuid::createUuid(), opIndex);
		m_outputPoints.push_back(cp);
	}

	void FblItem::ClearAssociatedConnections()
	{
		std::for_each(m_inputPoints.begin(), m_inputPoints.end(),
			[](CFblConnectionPoint& pin)
			{
				pin.ClearAssociattdIOs();
			});

		std::for_each(m_outputPoints.begin(), m_outputPoints.end(),
			[](CFblConnectionPoint& pin)
			{
				pin.ClearAssociattdIOs();
			});

		return;
	}

	void FblItem::SetConnectionsPos()
	{
		assert(false);	// должно быть реализовано у наследников, CFblItemLine, CFblItemRect
		return;
	}

	bool FblItem::GetConnectionPointPos(const QUuid&, VideoItemPoint*) const
	{
		assert(false);	// должно быть реализовано у наследников, CFblItemLine, CFblItemRect
		return false;
	}


	// Properties and Data
	//
}
