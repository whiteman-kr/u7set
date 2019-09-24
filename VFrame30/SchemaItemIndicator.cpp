#include "SchemaItemIndicator.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "Schema.h"
#include "FblItemRect.h"

namespace VFrame30
{
	SchemaItemIndicator::SchemaItemIndicator(void) :
		SchemaItemIndicator(SchemaUnit::Inch)
	{
		// This cinstructor can be called during serialization, then all variables
		// are initialized in loading process
		//
	}

	SchemaItemIndicator::SchemaItemIndicator(SchemaUnit unit)
	{
		//ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::schemaId, true, SchemaItemIndicator::schemaId, SchemaItemIndicator::setSchemaId);

		//ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::allowScale, true, SchemaItemIndicator::allowScale, SchemaItemIndicator::setAllowScale);
		//ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::keepAspectRatio, true, SchemaItemIndicator::keepAspectRatio, SchemaItemIndicator::setKeepAspectRatio);

		m_static = false;
		setItemUnit(unit);

		return;
	}

	SchemaItemIndicator::~SchemaItemIndicator(void)
	{
	}

	// Serialization
	//
	bool SchemaItemIndicator::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemIndicator* indicatorMessage = message->mutable_schemaitem()->mutable_indicator();

		indicatorMessage->set_type(static_cast<int>(m_type));

		return true;
	}

	bool SchemaItemIndicator::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			Q_ASSERT(message.has_schemaitem());
			return false;
		}

		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_indicator() == false)
		{
			Q_ASSERT(message.schemaitem().has_indicator());
			return false;
		}

		const Proto::SchemaItemIndicator& indicatorMessage = message.schemaitem().indicator();

		m_type = static_cast<E::IndicatorType>(indicatorMessage.type());

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemIndicator::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//

		// Calculate rectangle
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		// Filling rect 
		//
//		QPainter::RenderHints oldrenderhints = p->renderHints();
//		p->setRenderHint(QPainter::Antialiasing, false);

//		QBrush fillBrush{QColor{0xFF, 0xFF, 0xFF, 0xC0}};
//		p->fillRect(r, fillBrush);

//		p->setRenderHints(oldrenderhints);

//		// Drawing rect
//		//
//		QPen pen(Qt::black);
//		pen.setWidthF(drawParam->cosmeticPenWidth());

//		p->setPen(pen);
//		p->drawRect(r);

//		// Drawing Text
//		//
//		QString text = schemaId();

//		p->setPen(Qt::black);

//		QFont f;		// Default application font
//		p->setFont(f);

//		DrawHelper::drawText(p, drawParam->schema()->unit(), text, r, Qt::AlignCenter | Qt::AlignVCenter);

		return;
	}

	// Properties and Data
	//
	E::IndicatorType SchemaItemIndicator::type() const
	{
		return m_type;
	}

	void SchemaItemIndicator::setType(E::IndicatorType value)
	{
		m_type = value;
	}

}

