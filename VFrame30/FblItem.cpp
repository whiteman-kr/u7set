#include "Stable.h"
#include "FblItem.h"

namespace VFrame30
{
	//
	// CFblConnectionPoint
	//
	AfbPin::AfbPin()
	{
		m_associatedIOs.reserve(32);
	}

	AfbPin::AfbPin(
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
		m_associatedIOs.reserve(32);
	}

	AfbPin::AfbPin(ConnectionDirrection dirrection,
			const QUuid& guid,
			const Afb::AfbSignal& afbSignal) :
		m_guid(guid),
		m_point(0, 0),
		m_dirrection(dirrection),
		m_afbOperandIndex(afbSignal.operandIndex()),
		m_caption(afbSignal.caption())
	{
		m_associatedIOs.reserve(32);
	}

	AfbPin::AfbPin(const Proto::FblConnectionPoint& cpm)
	{
		m_associatedIOs.reserve(32);
		LoadData(cpm);
	}

	bool AfbPin::SaveData(Proto::FblConnectionPoint* cpm) const
	{
		//m_point.SaveData(cpm->mutable_point());	// Pin pos will be calculated before drawing or compilation

		cpm->set_dirrection(static_cast<Proto::ConnectionDirrection>(dirrection()));
		Proto::Write(cpm->mutable_uuid(), m_guid);
		cpm->set_operandindex(m_afbOperandIndex);
		cpm->set_caption(m_caption.toStdString());

		for (const QUuid au : m_associatedIOs)
		{
			::Proto::Uuid* p = cpm->add_associatedios();
			Proto::Write(p, au);
		}

		return true;
	}

	bool AfbPin::LoadData(const Proto::FblConnectionPoint& cpm)
	{
		//m_point.LoadData(cpm.point());	// Pin pos will be calculated before drawing or compilation

		m_dirrection = static_cast<ConnectionDirrection>(cpm.dirrection());
		m_guid = Proto::Read(cpm.uuid());
		m_afbOperandIndex = cpm.operandindex();
		m_caption = QString::fromStdString(cpm.caption());

		m_associatedIOs.clear();
		m_associatedIOs.reserve(cpm.associatedios_size());
		for (int i = 0; i < cpm.associatedios_size(); i++)
		{
			QUuid au = Proto::Read(cpm.associatedios(i));
			m_associatedIOs.push_back(au);
		}

		return true;
	}

	const SchemaPoint& AfbPin::point() const
	{
		return m_point;
	}

	void AfbPin::setPoint(const SchemaPoint& value)
	{
		m_point = value;
	}

	double AfbPin::x() const
	{
		return m_point.X;
	}

	void AfbPin::setX(double val)
	{
		m_point.X = val;
	}

	double AfbPin::y() const
	{
		return m_point.Y;
	}

	void AfbPin::setY(double val)
	{
		m_point.Y = val;
	}

	ConnectionDirrection AfbPin::dirrection() const
	{
		return m_dirrection;
	}

	bool AfbPin::IsInput() const
	{
		return m_dirrection == ConnectionDirrection::Input;
	}

	bool AfbPin::IsOutput() const
	{
		return m_dirrection == ConnectionDirrection::Output;
	}

	const QUuid& AfbPin::guid() const
	{
		return m_guid;
	}

	void AfbPin::setGuid(const QUuid& guid)
	{
		m_guid = guid;
	}

	const std::vector<QUuid>& AfbPin::associatedIOs() const
	{
		return m_associatedIOs;
	}

	std::vector<QUuid>& AfbPin::associatedIOs()
	{
		return m_associatedIOs;
	}

	void AfbPin::ClearAssociattdIOs()
	{
		m_associatedIOs.clear();
	}

	void AfbPin::AddAssociattedIOs(const QUuid& guid)
	{
		m_associatedIOs.push_back(guid);
	}

	bool AfbPin::removeFromAssociatedIo(const QUuid& uuid)
	{
		auto oldSize = m_associatedIOs.size();

		m_associatedIOs.erase(std::remove(m_associatedIOs.begin(),
										  m_associatedIOs.end(),
										  uuid),
							  m_associatedIOs.end());

		return m_associatedIOs.size() < oldSize;
	}

	bool AfbPin::hasAssociatedIo(const QUuid& guid) const
	{
		auto it = std::find(m_associatedIOs.begin(), m_associatedIOs.end(), guid);
		return it != m_associatedIOs.end();
	}

	bool AfbPin::HasConnection() const
	{
		assert(!(IsInput() && m_associatedIOs.size() > 1));
		return !m_associatedIOs.empty();
	}

	int AfbPin::afbOperandIndex() const
	{
		return m_afbOperandIndex;
	}

	void AfbPin::setAfbOperandIndex(int value)
	{
		m_afbOperandIndex = value;
	}

	QString AfbPin::caption() const
	{
		return m_caption;
	}

	void AfbPin::setCaption(QString caption)
	{
		m_caption = caption;
	}


	//
	// CFblItem
	//

	FblItem::FblItem(void)
	{
		m_inputPoints.reserve(8);
		m_outputPoints.reserve(8);
	}

	FblItem::~FblItem(void)
	{
	}
		
	// Serialization
	//
	bool FblItem::SaveData(Proto::Envelope* message) const
	{
		Proto::FblItem* fblItemMessage = message->mutable_schemaitem()->mutable_fblitem();

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
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		if (message.schemaitem().has_fblitem() == false)
		{
			assert(message.schemaitem().has_fblitem());
			return false;
		}
		
		// --
		//
		const Proto::FblItem& fblItemMessage = message.schemaitem().fblitem();

		m_inputPoints.clear();
		m_outputPoints.clear();
		for (int i = 0; i < fblItemMessage.points().size(); i++)
		{
			const Proto::FblConnectionPoint& cpm = fblItemMessage.points(i);
			AfbPin cp(cpm);

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
		double radius = pinWidth / 9.0;

		//p->drawEllipse(QPointF(x, y), radius, radius);
		drawEllipse(QRectF(x - radius, y - radius, 2 * radius, 2 * radius));
	}

	double FblItem::GetPinWidth(SchemaUnit unit, int dpi) const
	{
		double pinWidth = static_cast<float>(mm2in(3));	// 3 μμ!

		if (unit == SchemaUnit::Display)
		{
			pinWidth = pinWidth * dpi;
		}

		return pinWidth;
	}

	double FblItem::GetPinWidth(SchemaUnit unit, QPaintDevice* device) const
	{
		int dpi = 96;

		if (device == nullptr)
		{
			assert(device);
		}
		else
		{
			dpi = device->physicalDpiX();
		}

		double pinWidth = static_cast<float>(mm2in(3));	// 3 μμ!

		if (unit == SchemaUnit::Display)
		{
			pinWidth = pinWidth * dpi;
		}

		return pinWidth;
	}

	// Connections
	//
	const std::vector<AfbPin>& FblItem::inputs() const
	{
		return m_inputPoints;
	}

	std::vector<AfbPin>& FblItem::inputs()
	{
		return m_inputPoints;
	}

	const std::vector<AfbPin>& FblItem::outputs() const
	{
		return m_outputPoints;
	}

	std::vector<AfbPin>& FblItem::outputs()
	{
		return m_outputPoints;
	}

	std::vector<VFrame30::AfbPin>* FblItem::mutableInputs()
	{
		return &m_inputPoints;
	}

	std::vector<VFrame30::AfbPin>* FblItem::mutableOutputs()
	{
		return &m_outputPoints;
	}

	bool FblItem::GetConnectionPoint(const QUuid& guid, VFrame30::AfbPin* pResult) const
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
		AfbPin cp(ConnectionDirrection::Input, QUuid::createUuid(), -1, "");
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(const Afb::AfbSignal& s)
	{
		AfbPin cp(ConnectionDirrection::Input, QUuid::createUuid(), s);
		m_inputPoints.push_back(cp);
	}

	void FblItem::addInput(int opIndex, QString caption)
	{
		AfbPin cp(ConnectionDirrection::Input, QUuid::createUuid(), opIndex, caption);
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
		AfbPin cp(ConnectionDirrection::Output, QUuid::createUuid(), -1, "");
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(const Afb::AfbSignal& s)
	{
		AfbPin cp(ConnectionDirrection::Output, QUuid::createUuid(), s);
		m_outputPoints.push_back(cp);
	}

	void FblItem::addOutput(int opIndex, QString caption)
	{
		AfbPin cp(ConnectionDirrection::Output, QUuid::createUuid(), opIndex, caption);
		m_outputPoints.push_back(cp);
	}

	void FblItem::ClearAssociatedConnections()
	{
		std::for_each(m_inputPoints.begin(), m_inputPoints.end(),
			[](AfbPin& pin)
			{
				pin.ClearAssociattdIOs();
			});

		std::for_each(m_outputPoints.begin(), m_outputPoints.end(),
			[](AfbPin& pin)
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

	bool FblItem::GetConnectionPointPos(const QUuid&, SchemaPoint*, double /*gridSize*/, int /*pinGridStep*/) const
	{
		assert(false);	// Must be implemented in derived classes CFblItemLine, CFblItemRect...
		return false;
	}

	bool FblItem::hasInput(const QUuid& guid) const
	{
		auto foundIt = std::find_if(m_inputPoints.begin(), m_inputPoints.end(),
									[&guid](const VFrame30::AfbPin& pin)
									{
										return pin.guid() == guid;
									});

		return foundIt != m_inputPoints.end();
	}

	bool FblItem::hasOutput(const QUuid& guid) const
	{
		auto foundIt = std::find_if(m_outputPoints.begin(), m_outputPoints.end(),
									[&guid](const VFrame30::AfbPin& pin)
									{
										return pin.guid() == guid;
									});

		return foundIt != m_outputPoints.end();
	}

	const VFrame30::AfbPin& FblItem::input(const QUuid& guid) const
	{
		for (const VFrame30::AfbPin& pin : m_inputPoints)
		{
			if (pin.guid() == guid)
			{
				return pin;
			}
		}

		assert(false);

		static const VFrame30::AfbPin dummy;
		return dummy;
	}

	VFrame30::AfbPin& FblItem::input(const QUuid& guid)
	{
		for (VFrame30::AfbPin& pin : m_inputPoints)
		{
			if (pin.guid() == guid)
			{
				return pin;
			}
		}

		assert(false);

		static VFrame30::AfbPin dummy;
		return dummy;
	}

	const VFrame30::AfbPin& FblItem::output(const QUuid& guid) const
	{
		for (const VFrame30::AfbPin& pin : m_outputPoints)
		{
			if (pin.guid() == guid)
			{
				return pin;
			}
		}

		assert(false);

		static const VFrame30::AfbPin dummy;
		return dummy;
	}

	VFrame30::AfbPin& FblItem::output(const QUuid& guid)
	{
		for (VFrame30::AfbPin& pin : m_outputPoints)
		{
			if (pin.guid() == guid)
			{
				return pin;
			}
		}

		assert(false);

		static VFrame30::AfbPin dummy;
		return dummy;
	}

	QString FblItem::buildName() const
	{
		return "FblItem";
	}


	// Properties and Data
	//
}
