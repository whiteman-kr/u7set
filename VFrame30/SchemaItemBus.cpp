#include "SchemaItemBus.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{

	SchemaItemBus::SchemaItemBus() :
		SchemaItemBus(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemBus::SchemaItemBus(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::busTypeId,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemBus::busTypeId,
								 SchemaItemBus::setBusTypeId);
	}

	SchemaItemBus::~SchemaItemBus()
	{
	}

	bool SchemaItemBus::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemBus* busitem = message->mutable_schemaitem()->mutable_busitem();

		busitem->set_bustypeid(m_busTypeId.toStdString());

		return true;
	}

	bool SchemaItemBus::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_busitem() == false)
		{
			assert(message.schemaitem().has_busitem() == true);
			return false;
		}

		const Proto::SchemaItemBus& busitem = message.schemaitem().busitem();

		m_busTypeId = QString::fromStdString(busitem.bustypeid());

		return true;
	}

	void SchemaItemBus::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);
		return;
	}

	QString SchemaItemBus::buildName() const
	{
		assert(false);
		return QString();
	}

	QString SchemaItemBus::busTypeId() const
	{
		return m_busTypeId;
	}

	void SchemaItemBus::setBusTypeId(const QString& value)
	{
		m_busTypeId = value.trimmed();
	}

	//
	//
	//			SchemaItemBusComposer
	//
	//
	SchemaItemBusComposer::SchemaItemBusComposer() :
		SchemaItemBusComposer(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemBusComposer::SchemaItemBusComposer(SchemaUnit unit) :
		SchemaItemBus(unit)
	{
		addOutput();
	}

	SchemaItemBusComposer::~SchemaItemBusComposer()
	{
	}

	bool SchemaItemBusComposer::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemBus::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemBusComposer* composer = message->mutable_schemaitem()->mutable_buscomposer();

		Q_UNUSED(composer);
//		composer->set_pincount(m_pinCount);

		return true;
	}

	bool SchemaItemBusComposer::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemBus::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_buscomposer() == false)
		{
			assert(message.schemaitem().has_buscomposer() == true);
			return false;
		}

		const Proto::SchemaItemBusComposer& composer = message.schemaitem().buscomposer();
		Q_UNUSED(composer);

		return true;
	}

	void SchemaItemBusComposer::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemBus::Draw(drawParam, schema, layer);

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(painter->device());

		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
		p->setPen(pen);


//		// Custom draw
//		//
//		QPainter* p = drawParam->painter();

//		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

//		if (std::abs(r.left() - r.right()) < 0.000001)
//		{
//			r.setRight(r.left() + 0.000001);
//		}

//		if (std::abs(r.bottom() - r.top()) < 0.000001)
//		{
//			r.setBottom(r.top() + 0.000001);
//		}

//		int dpiX = drawParam->dpiX();
//		double pinWidth = GetPinWidth(itemUnit(), dpiX);

//		// Draw line and symbol >>
//		//
//		QPen linePen(lineColor());
//		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
//		p->setPen(linePen);

//		p->drawLine(QPointF(r.right() - pinWidth, r.top()), QPointF(r.right() - pinWidth, r.bottom()));

//		// >>
//		//
//		QRectF arrowRect(r);
//		arrowRect.setLeft(r.right() - pinWidth);

//		p->setPen(textColor());
//		DrawHelper::drawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

//		// Draw ConnectionID
//		//
//		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
//		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

//		p->setPen(textColor());

//		DrawHelper::drawText(p, m_font, itemUnit(), connectionId(), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	bool SchemaItemBusComposer::searchText(const QString& text) const
	{
		return	FblItemRect::searchText(text) ||
				busTypeId().contains(text, Qt::CaseInsensitive);
	}

	QString SchemaItemBusComposer::buildName() const
	{
		return QString("BusComposer %1").arg(busTypeId());
	}

	//
	//
	//			SchemaItemBusExtractor
	//
	//

	SchemaItemBusExtractor::SchemaItemBusExtractor() :
		SchemaItemBusExtractor(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemBusExtractor::SchemaItemBusExtractor(SchemaUnit unit) :
		SchemaItemBus(unit)
	{
		addInput();
	}

	SchemaItemBusExtractor::~SchemaItemBusExtractor()
	{
	}

	bool SchemaItemBusExtractor::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemBus::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemBusExtractor* extractor = message->mutable_schemaitem()->mutable_busextractor();
		Q_UNUSED(extractor);

		return true;
	}

	bool SchemaItemBusExtractor::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemBus::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_busextractor() == false)
		{
			assert(message.schemaitem().has_busextractor() == true);
			return false;
		}

		const Proto::SchemaItemBusExtractor& extractor = message.schemaitem().busextractor();
		Q_UNUSED(extractor);

		return true;
	}

	void SchemaItemBusExtractor::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemBus::Draw(drawParam, schema, layer);

//		// Custom draw
//		//
//		QPainter* p = drawParam->painter();

//		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

//		if (std::abs(r.left() - r.right()) < 0.000001)
//		{
//			r.setRight(r.left() + 0.000001);
//		}

//		if (std::abs(r.bottom() - r.top()) < 0.000001)
//		{
//			r.setBottom(r.top() + 0.000001);
//		}

//		int dpiX = drawParam->dpiX();
//		double pinWidth = GetPinWidth(itemUnit(), dpiX);

//		// Draw line and symbol >>
//		//
//		QPen linePen(lineColor());
//		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
//		p->setPen(linePen);

//		p->drawLine(QPointF(r.left() + pinWidth, r.top()), QPointF(r.left() + pinWidth, r.bottom()));

//		// >>
//		//
//		QRectF arrowRect(r);
//		arrowRect.setRight(r.left() + pinWidth);

//		p->setPen(textColor());
//		DrawHelper::drawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

//		// Draw ConnectionID
//		//
//		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
//		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

//		p->setPen(textColor());

//		DrawHelper::drawText(p, m_font, itemUnit(), connectionId(), r, Qt::AlignHCenter | Qt::AlignTop);

//		// Draw Data (AppSignalID, CustomerSignalID, Caption, etc
//		//
//		QString appSignalId = this->appSignalId();

//		AppSignalParam signal;
//		signal.setAppSignalId(appSignalId);

//		AppSignalState signalState;
//		signalState.m_flags.valid = false;

//		bool signalFound = false;

//		if (drawParam->isMonitorMode() == true)
//		{
//			signal = drawParam->appSignalManager()->signalParam(appSignalId, &signalFound);
//			signalState = drawParam->appSignalManager()->signalState(appSignalId, nullptr);
//		}

//		QString dataText = SchemaItemSignal::getCoulumnText(drawParam, m_dataType, signal, signalState, m_analogFormat, m_precision);

//		DrawHelper::drawText(p, m_font, itemUnit(), dataText, r, Qt::AlignHCenter | Qt::AlignBottom);

		return;
	}

	bool SchemaItemBusExtractor::searchText(const QString& text) const
	{
		bool f0 = FblItemRect::searchText(text);
		bool f1 = busTypeId().contains(text, Qt::CaseInsensitive);

		return f0 || f1;
	}

//	double SchemaItemBusExtractor::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
//	{
//		// Cache values
//		//
//		m_cachedGridSize = gridSize;
//		m_cachedPinGridStep = pinGridStep;

//		// --
//		//
//		int pinCount = 2;

//		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
//		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

//		return minHeight;
//	}

	QString SchemaItemBusExtractor::buildName() const
	{
		return QString("BusExtractor %1").arg(busTypeId());
	}

}
