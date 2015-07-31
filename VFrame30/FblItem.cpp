#include "Stable.h"
#include "FblItem.h"

namespace VFrame30
{
	//
	// CFblConnectionPoint
	//
	CFblConnectionPoint::CFblConnectionPoint()
	{
	}

	CFblConnectionPoint::CFblConnectionPoint(
			ConnectionDirrection dirrection,
			const QUuid& guid,
			int operandIndex,
			QString caption) :
		m_guid(guid),
		m_point(0, 0),
		m_dirrection(dirrection),
		m_afbOperandIndex(operandIndex),
		m_caption(caption)
	{
	}

	CFblConnectionPoint::CFblConnectionPoint(ConnectionDirrection dirrection,
			const QUuid& guid,
			const Afb::AfbSignal& afbSignal) :
		m_guid(guid),
		m_point(0, 0),
		m_dirrection(dirrection),
		m_afbOperandIndex(afbSignal.operandIndex()),
		m_caption(afbSignal.caption())
	{
	}

	CFblConnectionPoint::CFblConnectionPoint(const Proto::FblConnectionPoint& cpm)
	{
		LoadData(cpm);
	}

	bool CFblConnectionPoint::SaveData(Proto::FblConnectionPoint* cpm) const
	{
		//m_point.SaveData(cpm->mutable_point());	// Pin pos will be calculated before drawing or compilation

		cpm->set_dirrection(static_cast<Proto::ConnectionDirrection>(dirrection()));
		Proto::Write(cpm->mutable_uuid(), m_guid);
		cpm->set_operandindex(m_afbOperandIndex);
		cpm->set_caption(m_caption.toStdString());

		return true;
	}

	bool CFblConnectionPoint::LoadData(const Proto::FblConnectionPoint& cpm)
	{
		//m_point.LoadData(cpm.point());	// Pin pos will be calculated before drawing or compilation

		m_dirrection = static_cast<ConnectionDirrection>(cpm.dirrection());
		m_guid = Proto::Read(cpm.uuid());
		m_afbOperandIndex = cpm.operandindex();
		m_caption = QString::fromStdString(cpm.caption());

		return true;
	}

	const SchemePoint& CFblConnectionPoint::point() const
	{
		return m_point;
	}

	void CFblConnectionPoint::setPoint(const SchemePoint& value)
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

	QString CFblConnectionPoint::caption() const
	{
		return m_caption;
	}

	void CFblConnectionPoint::setCaption(QString caption)
	{
		m_caption = caption;
	}


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
		Proto::FblItem* fblItemMessage = message->mutable_schemeitem()->mutable_fblitem();

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
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		if (message.schemeitem().has_fblitem() == false)
		{
			assert(message.schemeitem().has_fblitem());
			return false;
		}
		
		// --
		//
		const Proto::FblItem& fblItemMessage = message.schemeitem().fblitem();

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
		double crossSize = pinWidth / 4.0;

		QPointF cross_pt1(x - crossSize, y - crossSize);
		QPointF cross_pt2(x + crossSize, y - crossSize);
		QPointF cross_pt3(x + crossSize, y + crossSize);
		QPointF cross_pt4(x - crossSize, y + crossSize);

		p->drawLine(cross_pt1, cross_pt3);
		p->drawLine(cross_pt2, cross_pt4);
	}

	void FblItem::DrawPinJoint(QPainter* p, double x, double y, double pinWidth) const
	{
		double radius = static_cast<double>(pinWidth) / 9.0;

		p->drawEllipse(QPointF(x, y), radius, radius);
	}

	double FblItem::GetPinWidth(SchemeUnit unit, int dpi) const
	{
		double pinWidth = static_cast<float>(mm2in(3));	// 3 μμ!

		if (unit == SchemeUnit::Display)
		{
			pinWidth = pinWidth * dpi;
		}

		return pinWidth;
	}

	// Connections
	//
	const std::list<VFrame30::CFblConnectionPoint>& FblItem::inputs() const
	{
		return m_inputPoints;
	}

	const std::list<VFrame30::CFblConnectionPoint>& FblItem::outputs() const
	{
		return m_outputPoints;
	}

	std::list<VFrame30::CFblConnectionPoint>* FblItem::mutableInputs()
	{
		return &m_inputPoints;
	}

	std::list<VFrame30::CFblConnectionPoint>* FblItem::mutableOutputs()
	{
		return &m_outputPoints;
	}

	bool FblItem::GetConnectionPoint(const QUuid& guid, VFrame30::CFblConnectionPoint* pResult) const
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
		CFblConnectionPoint cp(ConnectionDirrection::Input, QUuid::createUuid(), -1, "");
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(const Afb::AfbSignal& s)
	{
		CFblConnectionPoint cp(ConnectionDirrection::Input, QUuid::createUuid(), s);
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(int opIndex, QString caption)
	{
		CFblConnectionPoint cp(ConnectionDirrection::Input, QUuid::createUuid(), opIndex, caption);
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
		CFblConnectionPoint cp(ConnectionDirrection::Output, QUuid::createUuid(), -1, "");
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(const Afb::AfbSignal& s)
	{
		CFblConnectionPoint cp(ConnectionDirrection::Output, QUuid::createUuid(), s);
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(int opIndex, QString caption)
	{
		CFblConnectionPoint cp(ConnectionDirrection::Output, QUuid::createUuid(), opIndex, caption);
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

	void FblItem::SetConnectionsPos(double /*gridSize*/, int /*pinGridStep*/)
	{
		assert(false);	// Must be implemented in derived classes CFblItemLine, CFblItemRect...
		return;
	}

	bool FblItem::GetConnectionPointPos(const QUuid&, SchemePoint*, double /*gridSize*/, int /*pinGridStep*/) const
	{
		assert(false);	// Must be implemented in derived classes CFblItemLine, CFblItemRect...
		return false;
	}

	QString FblItem::buildName() const
	{
		return "FblItem";
	}


	// Properties and Data
	//
}
