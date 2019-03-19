#include "SchemaItemImage.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{
	SchemaItemImage::SchemaItemImage(void) :
		SchemaItemImage(SchemaUnit::Inch)
	{
		// ¬ызов этого конструктора возможен при сериализации объектов такого типа.
		// ѕосле этого вызова надо проинциализировать все, что и делаетс€ самой сериализацией.
		//
	}

	SchemaItemImage::SchemaItemImage(SchemaUnit unit)
	{
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::allowScale, PropertyNames::appearanceCategory, true, SchemaItemImage::allowScale, SchemaItemImage::setAllowScale);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::keepAspectRatio, PropertyNames::appearanceCategory, true, SchemaItemImage::keepAspectRatio, SchemaItemImage::setKeepAspectRatio);

		// --
		//
		m_static = true;
		setItemUnit(unit);
	}

	SchemaItemImage::~SchemaItemImage(void)
	{
	}

	// Serialization
	//
	bool SchemaItemImage::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemImage* imageMessage = message->mutable_schemaitem()->mutable_image();

		imageMessage->set_allowscale(m_allowScale);
		imageMessage->set_keepaspectratio(m_keepAspectRatio);

		return true;
	}

	bool SchemaItemImage::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_image() == false)
		{
			assert(message.schemaitem().has_image());
			return false;
		}

		const Proto::SchemaItemImage& imageMessage = message.schemaitem().image();

		m_allowScale = imageMessage.allowscale();
		m_keepAspectRatio = imageMessage.keepaspectratio();

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemImage::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
//		if (m_rectPen.get() == nullptr)
//		{
//			m_rectPen = std::make_shared<QPen>();
//		}

//		QColor qlinecolor(lineColor());
//		if (m_rectPen->color() !=qlinecolor )
//		{
//			m_rectPen->setColor(qlinecolor);
//		}

//		if (m_fillBrush.get() == nullptr)
//		{
//			m_fillBrush = std::make_shared<QBrush>(Qt::SolidPattern);
//		}
						
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

//		// Filling rect
//		//
//		if (fill() == true)
//		{
//			QPainter::RenderHints oldrenderhints = p->renderHints();
//			p->setRenderHint(QPainter::Antialiasing, false);

//			QColor qfillcolor(fillColor());

//			m_fillBrush->setColor(qfillcolor);
//			p->fillRect(r, *m_fillBrush);		// 22% если использовать Qcolor и намного меньше если использовать готовый Brush

//			p->setRenderHints(oldrenderhints);
//		}

		// Drawing rect 
		//
//		if (drawRect() == true)
//		{
//			m_rectPen->setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);

//			p->setPen(*m_rectPen);
//			p->drawRect(r);
//		}

		// Drawing Text
		//
//		QString text = MacrosExpander::parse(m_text, drawParam->session(), schema, this);

//		if (m_text.isEmpty() == false)
//		{
//			p->setPen(textColor());
//			DrawHelper::drawText(p, m_font, itemUnit(), text, r, horzAlign() | vertAlign());
//		}

		return;
	}

	double SchemaItemImage::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemImage::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// Properties and Data
	//

	// AllowScale property
	//
	bool SchemaItemImage::allowScale() const
	{
		return m_allowScale;
	}

	void SchemaItemImage::setAllowScale(bool value)
	{
		m_allowScale = value;
	}

	// KeepAspectRatio
	//
	bool SchemaItemImage::keepAspectRatio() const
	{
		return m_keepAspectRatio;
	}

	void SchemaItemImage::setKeepAspectRatio(bool value)
	{
		m_keepAspectRatio = value;
	}
}

