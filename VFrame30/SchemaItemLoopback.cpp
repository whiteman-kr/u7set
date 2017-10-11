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

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(painter->device());

		// Draw loopback logo
		//
		double pinVertGap =	CUtils::snapToGrid(drawParam->gridSize() * static_cast<double>(drawParam->pinGridStep()), drawParam->gridSize());

		QRectF logoRect = {r.right() - pinVertGap * 2.0, r.top(), pinVertGap * 2.0, r.height()};
		logoRect.setTopRight(drawParam->gridToDpi(logoRect.topRight()));
		logoRect.setBottomLeft(drawParam->gridToDpi(logoRect.bottomLeft()));

		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
		painter->setPen(pen);

		painter->drawLine(logoRect.topLeft(), logoRect.bottomLeft());

		DrawHelper::drawText(painter, m_font, itemUnit(), QLatin1String("LB"), logoRect, Qt::AlignHCenter | Qt::AlignVCenter);

		// Draw LoopbackID
		//
		QRectF textRect = {r.left(), r.top(), r.width() - logoRect.width(), r.height()};

		textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
		textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		DrawHelper::drawText(painter, m_font, itemUnit(), loopbackId(), textRect, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	QString SchemaItemLoopbackSource::buildName() const
	{
		return QString("LoopbackSource %1").arg(loopbackId());
	}

	QString SchemaItemLoopbackSource::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString str = QString("Loopback Source: "
							  "\n\tLoopbackID: %1"
							  "\n"
							  "\nHint: Press F2 to edit LoopbackID")
						.arg(loopbackId());

		return str;
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

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(painter->device());

		// Draw loopback logo
		//
		double pinVertGap =	CUtils::snapToGrid(drawParam->gridSize() * static_cast<double>(drawParam->pinGridStep()), drawParam->gridSize());

		QRectF logoRect = {r.left(), r.top(), pinVertGap * 2.0, r.height()};
		logoRect.setTopRight(drawParam->gridToDpi(logoRect.topRight()));
		logoRect.setBottomLeft(drawParam->gridToDpi(logoRect.bottomLeft()));

		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
		painter->setPen(pen);

		painter->drawLine(logoRect.topRight(), logoRect.bottomRight());

		DrawHelper::drawText(painter, m_font, itemUnit(), QLatin1String("LB"), logoRect, Qt::AlignHCenter | Qt::AlignVCenter);

		// Draw LoopbackID
		//
		QRectF textRect = {logoRect.right(), r.top(), r.width() - logoRect.width(), r.height()};

		textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
		textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		DrawHelper::drawText(painter, m_font, itemUnit(), loopbackId(), textRect, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	QString SchemaItemLoopbackTarget::buildName() const
	{
		return QString("LoopbackTarget %1").arg(loopbackId());
	}

	QString SchemaItemLoopbackTarget::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString str = QString("Loopback Target: "
							  "\n\tLoopbackID: %1"
							  "\n"
							  "\nHint: Press F2 to edit LoopbackID")
						.arg(loopbackId());

		return str;
	}
}
