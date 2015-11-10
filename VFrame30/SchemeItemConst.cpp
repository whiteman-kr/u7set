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

		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		Proto::SchemeItemConst* constitem = message->mutable_schemeitem()->mutable_constitem();

		constitem->set_type(m_type);
		constitem->set_intvalue(m_intValue);
		constitem->set_floatvalue(m_floatValue);
		constitem->set_precision(m_precision);

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
		if (message.schemeitem().has_constitem() == false)
		{
			assert(message.schemeitem().has_constitem() == true);
			return false;
		}

		const Proto::SchemeItemConst& constitem = message.schemeitem().constitem();

		m_type = static_cast<ConstType>(constitem.type());
		m_intValue = constitem.intvalue();
		m_floatValue = constitem.floatvalue();
		m_precision = constitem.precision();

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
			case ConstType::FloatType:
				text = QString::number(floatValue(), 'f', precision());
				break;
			default:
				assert(false);
				break;
		}

		return text;
	}

	QString SchemeItemConst::buildName() const
	{
		return QString("Const (%1)").arg(valueToString());
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

	bool SchemeItemConst::isFloat() const
	{
		return m_type == SchemeItemConst::ConstType::FloatType;
	}

	int SchemeItemConst::intValue() const
	{
		return m_intValue;
	}

	void SchemeItemConst::setIntValue(int intValue)
	{
		m_intValue = intValue;
	}
	double SchemeItemConst::floatValue() const
	{
		return m_floatValue;
	}

	void SchemeItemConst::setFloatValue(double doubleValue)
	{
		m_floatValue = doubleValue;
	}

	int SchemeItemConst::precision() const
	{
		return m_precision;
	}

	void SchemeItemConst::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 24)
		{
			value = 24;
		}

		m_precision = value;
	}

}
