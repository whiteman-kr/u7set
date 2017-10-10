#include "SchemaItemLoopback.h"
#include "SchemaLayer.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{

	SchemaItemLoopback::SchemaItemLoopback() :
		SchemaItemLoopback(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemLoopback::SchemaItemLoopback(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::loopbackId, PropertyNames::functionalCategory, true, SchemaItemLoopback::loopbackId, SchemaItemLoopback::setLoopbackId);
	}

	SchemaItemLoopback::~SchemaItemLoopback()
	{
	}

	bool SchemaItemLoopback::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false ||
			message->schemaitem().has_fblitemrect() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			assert(message->schemaitem().has_fblitemrect() == true);
			return false;
		}

		// --
		//
		Proto::SchemaItemLoopback* loopbackitem = message->mutable_schemaitem()->mutable_loopbackitem();

		loopbackitem->set_loopbackid(m_loobackId.toStdString());

		return result;
	}

	bool SchemaItemLoopback::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_loopbackitem() == false)
		{
			assert(message.schemaitem().has_loopbackitem() == true);
			return false;
		}

		const Proto::SchemaItemLoopback& loopbackitem = message.schemaitem().loopbackitem();

		m_loobackId = QString::fromStdString(loopbackitem.loopbackid());

		return true;
	}

	void SchemaItemLoopback::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(painter->device());

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		DrawHelper::drawText(painter, m_font, itemUnit(), loopbackId(), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	bool SchemaItemLoopback::searchText(const QString& text) const
	{
		return	FblItemRect::searchText(text) ||
				m_loobackId.contains(text, Qt::CaseInsensitive);
	}

	QString SchemaItemLoopback::loopbackId() const
	{
		return m_loobackId;
	}

	void SchemaItemLoopback::setLoopbackId(QString value)
	{
		m_loobackId = value.trimmed();
	}

	//
	//
	//			SchemaItemLoopbackSource
	//
	//
	SchemaItemLoopbackSource::SchemaItemLoopbackSource() :
		SchemaItemLoopbackSource(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemLoopbackSource::SchemaItemLoopbackSource(SchemaUnit unit) :
		SchemaItemLoopback(unit)
	{
		addInput();
	}

	SchemaItemLoopbackSource::~SchemaItemLoopbackSource()
	{
	}

	bool SchemaItemLoopbackSource::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemLoopback::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false ||
			message->schemaitem().has_loopbackitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			assert(message->schemaitem().has_loopbackitem() == true);
			return false;
		}

		// --
		//
		Proto::SchemaItemLoopbackSource* source = message->mutable_schemaitem()->mutable_loopbacksource();
		Q_UNUSED(source);

		return true;
	}

	bool SchemaItemLoopbackSource::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemLoopback::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_loopbacksource() == false)
		{
			assert(message.schemaitem().has_loopbacksource() == true);
			return false;
		}

		const Proto::SchemaItemLoopbackSource& source = message.schemaitem().loopbacksource();
		Q_UNUSED(source);

		return true;
	}

	void SchemaItemLoopbackSource::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemLoopback::Draw(drawParam, schema, layer);

//		QPainter* painter = drawParam->painter();

//		QRectF r = itemRectPinIndent(painter->device());
//		r.setTopRight(drawParam->gridToDpi(r.topRight()));
//		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

//		// Draw bold left line
//		//
//		QPen pen(lineColor());
//		//pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
//		pen.setWidthF(LoopbackSideLineWidth);
//		pen.setCapStyle(Qt::FlatCap);
//		painter->setPen(pen);

//		QLineF leftLine = {QPointF(r.right() - pen.widthF() / 2.0, r.top()),
//						   QPointF(r.right() - pen.widthF() / 2.0 , r.bottom())};

//		painter->drawLine(leftLine);

//		// Draw bold output pin
//		//
//		if (outputsCount() != 1)
//		{
//			assert(outputsCount() == 1);
//			return;
//		}

//		const std::vector<AfbPin>& outputPins = outputs();
//		const AfbPin& output = outputPins[0];

//		// Get pin position
//		//
//		SchemaPoint vip;
//		GetConnectionPointPos(output.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

//		// Draw pin
//		//
//		int dpiX = drawParam->dpiX();
//		double pinWidth = GetPinWidth(itemUnit(), dpiX);

//		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
//		QPointF pt2(drawParam->gridToDpi(vip.X - pinWidth, vip.Y));

//		painter->setPen(pen);
//		painter->drawLine(pt1, pt2);

//		int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

//		if (connectionCount > 1)
//		{
//			painter->setBrush(pen.color());
//			painter->setPen(pen);
//			DrawPinJoint(painter, pt1.x(), pt1.y(), pinWidth);
//			painter->setBrush(Qt::NoBrush);
//		}
//		else
//		{
//			// Draw red cross error mark
//			//
//			QPen redPen(QColor(0xE0B00000));
//			redPen.setWidthF(pen.widthF());

//			painter->setPen(redPen);
//			DrawPinCross(painter, pt1.x(), pt1.y(), pinWidth);
//		}

		return;
	}

	QString SchemaItemLoopbackSource::buildName() const
	{
		return QString("LoopbackSource %1").arg(loopbackId());
	}

	//
	//
	//			SchemaItemLoopbackTarget
	//
	//
	SchemaItemLoopbackTarget::SchemaItemLoopbackTarget() :
		SchemaItemLoopbackTarget(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemLoopbackTarget::SchemaItemLoopbackTarget(SchemaUnit unit) :
		SchemaItemLoopback(unit)
	{
		addOutput();
	}

	SchemaItemLoopbackTarget::~SchemaItemLoopbackTarget()
	{
	}

	bool SchemaItemLoopbackTarget::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemLoopback::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false ||
			message->schemaitem().has_loopbackitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			assert(message->schemaitem().has_loopbackitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemLoopbackTarget* target = message->mutable_schemaitem()->mutable_loopbacktarget();
		Q_UNUSED(target);

		return true;
	}

	bool SchemaItemLoopbackTarget::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemLoopback::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_loopbacktarget() == false)
		{
			assert(message.schemaitem().has_loopbacktarget() == true);
			return false;
		}

		const Proto::SchemaItemLoopbackTarget& target = message.schemaitem().loopbacktarget();
		Q_UNUSED(target);

		return true;
	}

	void SchemaItemLoopbackTarget::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemLoopback::Draw(drawParam, schema, layer);

//		QPainter* painter = drawParam->painter();

//		// Custom draw
//		//
//		QRectF r = itemRectPinIndent(painter->device());
//		r.setTopRight(drawParam->gridToDpi(r.topRight()));
//		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

//		// Draw bold left line
//		//
//		QPen pen(lineColor());
//		//pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
//		pen.setWidthF(LoopbackSideLineWidth);
//		pen.setCapStyle(Qt::FlatCap);
//		painter->setPen(pen);

//		QLineF leftLine = {QPointF(r.left() + pen.widthF() / 2.0, r.top()),
//						   QPointF(r.left() + pen.widthF() / 2.0 , r.bottom())};

//		painter->drawLine(leftLine);

//		// Draw bold input pin
//		//
//		if (inputsCount() != 1)
//		{
//			assert(inputsCount() == 1);
//			return;
//		}

//		const std::vector<AfbPin>& inputPins = inputs();
//		const AfbPin& input = inputPins[0];

//		// Get pin position
//		//
//		SchemaPoint vip;
//		GetConnectionPointPos(input.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

//		// Draw pin
//		//
//		int dpiX = drawParam->dpiX();
//		double pinWidth = GetPinWidth(itemUnit(), dpiX);

//		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
//		QPointF pt2(drawParam->gridToDpi(vip.X + pinWidth, vip.Y));

//		painter->setPen(pen);
//		painter->drawLine(pt1, pt2);

//		int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

//		if (connectionCount > 1)
//		{
//			painter->setBrush(pen.color());
//			painter->setPen(pen);
//			DrawPinJoint(painter, pt1.x(), pt1.y(), pinWidth);
//			painter->setBrush(Qt::NoBrush);
//		}
//		else
//		{
//			// Draw red cross error mark
//			//
//			QPen redPen(QColor(0xE0B00000));
//			redPen.setWidthF(pen.widthF());

//			painter->setPen(redPen);
//			DrawPinCross(painter, pt1.x(), pt1.y(), pinWidth);
//		}

		return;
	}

	QString SchemaItemLoopbackTarget::buildName() const
	{
		return QString("LoopbackTarget %1").arg(loopbackId());
	}
}
