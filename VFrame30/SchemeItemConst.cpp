#include "SchemeItemConst.h"

namespace VFrame30
{

	SchemeItemConst::SchemeItemConst()
	{
		// This constructor can be called while serialization
		//
	}

	SchemeItemConst::SchemeItemConst(SchemeUnit unit) :
		FblItemRect(unit)
	{
		addOutput();
	}

	SchemeItemConst::~SchemeItemConst()
	{
	}

	bool SchemeItemConst::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		Proto::SchemeItemConst* constitem = message->mutable_videoitem()->mutable_constitem();

		constitem->set_type(m_type);
		constitem->set_intvalue(m_intValue);
		constitem->set_doublevalue(m_doubleValue);

		return true;
	}

	bool SchemeItemConst::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_constitem() == false)
		{
			assert(message.videoitem().has_constitem() == true);
			return false;
		}

		const Proto::SchemeItemConst& constitem = message.videoitem().constitem();

		m_type = static_cast<ConstType>(constitem.type());
		m_intValue = constitem.intvalue();
		m_doubleValue = constitem.doublevalue();

		return true;
	}

	void SchemeItemConst::Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* layer) const
	{
		FblItemRect::Draw(drawParam, scheme, layer);

		//--
		//
		QPainter* p = drawParam->painter();

		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}

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

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw Signals StrIDs
		//
		QString text = valueToString();

		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignTop);

		return;
	}

	QString SchemeItemConst::valueToString() const
	{
		QString text;

		switch (type())
		{
			case ConstType::IntegralType:
				text = QString::number(intValue());
				break;
			case ConstType::DoubleType:
				text = QString::number(doubleValue(), 'g', 4);
				break;
			default:
				assert(false);
				break;
		}

		return text;
	}

	SchemeItemConst::ConstType SchemeItemConst::type() const
	{
		return m_type;
	}

	void SchemeItemConst::setType(SchemeItemConst::ConstType value)
	{
		m_type = value;
	}

		bool SchemeItemConst::isIntegral() const
	{
		return m_type == SchemeItemConst::ConstType::IntegralType;
	}

	bool SchemeItemConst::isDouble() const
	{
		return m_type == SchemeItemConst::ConstType::DoubleType;
	}

	int SchemeItemConst::intValue() const
	{
		return m_intValue;
	}

	void SchemeItemConst::setIntValue(int intValue)
	{
		m_intValue = intValue;
	}
	double SchemeItemConst::doubleValue() const
	{
		return m_doubleValue;
	}

	void SchemeItemConst::setDoubleValue(double doubleValue)
	{
		m_doubleValue = doubleValue;
	}

}
