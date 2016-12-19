#include "SchemaItemTerminator.h"

namespace VFrame30
{

	SchemaItemTerminator::SchemaItemTerminator() :
		SchemaItemTerminator(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemTerminator::SchemaItemTerminator(SchemaUnit unit) :
		FblItemRect(unit)
	{
		// --
		//
		addInput();
	}

	SchemaItemTerminator::~SchemaItemTerminator()
	{
	}

	bool SchemaItemTerminator::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::SchemaItemTerminator* termitem = */message->mutable_schemaitem()->mutable_terminator();

		//termitem->set_type(m_type);

		return true;
	}

	bool SchemaItemTerminator::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_terminator() == false)
		{
			assert(message.schemaitem().has_terminator() == true);
			return false;
		}

		//const Proto::SchemaItemTerminator& termitem = message.schemaitem().terminator();

		//m_type = static_cast<ConstType>(constitem.type());

		return true;
	}

	void SchemaItemTerminator::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

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
			dpiX = pPaintDevice->physicalDpiX();
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

		// Draw "T"
		//
		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), QLatin1String("T"), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	double SchemaItemTerminator::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		return m_cachedGridSize * 6;
	}

	QString SchemaItemTerminator::buildName() const
	{
		return QString("Terminator");
	}

}
