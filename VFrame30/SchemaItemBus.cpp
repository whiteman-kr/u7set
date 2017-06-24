#include "SchemaItemBus.h"
#include "SchemaLayer.h"
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
		auto p = ADD_PROPERTY_GETTER(QString, PropertyNames::busTypeId, true, SchemaItemBus::busTypeId);
		p->setCategory(PropertyNames::functionalCategory);
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

		busitem->set_bustypeid(m_bus.busTypeId().toStdString());
		busitem->set_bustypehash(m_busTypeHash);

		QByteArray busXml;
		m_bus.save(&busXml);
		busitem->set_bustypexml(busXml.constData(), busXml.size());

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

		m_bus.setBusTypeId(QString::fromStdString(busitem.bustypeid()));
		m_busTypeHash = busitem.bustypehash();

		QByteArray xml = QByteArray::fromRawData(busitem.bustypexml().data(),
												 static_cast<int>(busitem.bustypexml().size()));
		QString erorrMessaage;
		m_bus.load(xml, &erorrMessaage);

		return true;
	}

	void SchemaItemBus::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(painter->device());

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		DrawHelper::drawText(painter, m_font, itemUnit(), "bus type:\n" + busTypeId(), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	QString SchemaItemBus::buildName() const
	{
		assert(false);
		return QString();
	}

	void SchemaItemBus::setBusPins(const Bus& bus)
	{
		Q_UNUSED(bus);
		assert(false);
	}

	QString SchemaItemBus::busTypeId() const
	{
		return m_bus.busTypeId();
	}

	const VFrame30::Bus& SchemaItemBus::busType() const
	{
		return m_bus;
	}

	void SchemaItemBus::setBusType(const VFrame30::Bus& bus)
	{
		m_bus = bus;
		setBusPins(bus);

		m_busTypeHash = m_bus.calcHash();
	}

	Hash SchemaItemBus::busTypeHash() const
	{
		return m_busTypeHash;
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
		addOutput(-1, "bus_out");
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
		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		// Draw bold left line
		//
		QPen pen(lineColor());
		//pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
		pen.setWidthF(BusSideLineWidth);
		pen.setCapStyle(Qt::FlatCap);
		painter->setPen(pen);

		QLineF leftLine = {QPointF(r.right() - pen.widthF() / 2.0, r.top()),
						   QPointF(r.right() - pen.widthF() / 2.0 , r.bottom())};

		painter->drawLine(leftLine);

		// Draw bold output pin
		//
		if (outputsCount() != 1)
		{
			assert(outputsCount() == 1);
			return;
		}

		const std::vector<AfbPin>& outputPins = outputs();
		const AfbPin& output = outputPins[0];

		// Get pin position
		//
		SchemaPoint vip;
		GetConnectionPointPos(output.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

		// Draw pin
		//
		int dpiX = drawParam->dpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
		QPointF pt2(drawParam->gridToDpi(vip.X - pinWidth, vip.Y));

		painter->setPen(pen);
		painter->drawLine(pt1, pt2);

		int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

		if (connectionCount > 1)
		{
			painter->setBrush(pen.color());
			painter->setPen(pen);
			DrawPinJoint(painter, pt1.x(), pt1.y(), pinWidth);
			painter->setBrush(Qt::NoBrush);
		}
		else
		{
			// Draw red cross error mark
			//
			QPen redPen(QColor(0xE0B00000));
			redPen.setWidthF(pen.widthF());

			painter->setPen(redPen);
			DrawPinCross(painter, pt1.x(), pt1.y(), pinWidth);
		}

		return;
	}

	bool SchemaItemBusComposer::searchText(const QString& text) const
	{
		return	FblItemRect::searchText(text) ||
				busTypeId().contains(text, Qt::CaseInsensitive);
	}

	double SchemaItemBusComposer::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = qBound(2, inputsCount(), 128);

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemBusComposer::buildName() const
	{
		return QString("BusComposer %1").arg(busTypeId());
	}

	void SchemaItemBusComposer::setBusPins(const VFrame30::Bus& bus)
	{
		inputs().clear();

		for (const VFrame30::BusSignal& busSignal : bus.busSignals())
		{
			addInput(-1, busSignal.name());
		}

		return;
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
		addInput(-1, "bus_in");
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

		QPainter* painter = drawParam->painter();

		// Custom draw
		//
		QRectF r = itemRectPinIndent(painter->device());
		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		// Draw bold left line
		//
		QPen pen(lineColor());
		//pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
		pen.setWidthF(BusSideLineWidth);
		pen.setCapStyle(Qt::FlatCap);
		painter->setPen(pen);

		QLineF leftLine = {QPointF(r.left() + pen.widthF() / 2.0, r.top()),
						   QPointF(r.left() + pen.widthF() / 2.0 , r.bottom())};

		painter->drawLine(leftLine);

		// Draw bold input pin
		//
		if (inputsCount() != 1)
		{
			assert(inputsCount() == 1);
			return;
		}

		const std::vector<AfbPin>& inputPins = inputs();
		const AfbPin& input = inputPins[0];

		// Get pin position
		//
		SchemaPoint vip;
		GetConnectionPointPos(input.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

		// Draw pin
		//
		int dpiX = drawParam->dpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
		QPointF pt2(drawParam->gridToDpi(vip.X + pinWidth, vip.Y));

		painter->setPen(pen);
		painter->drawLine(pt1, pt2);

		int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

		if (connectionCount > 1)
		{
			painter->setBrush(pen.color());
			painter->setPen(pen);
			DrawPinJoint(painter, pt1.x(), pt1.y(), pinWidth);
			painter->setBrush(Qt::NoBrush);
		}
		else
		{
			// Draw red cross error mark
			//
			QPen redPen(QColor(0xE0B00000));
			redPen.setWidthF(pen.widthF());

			painter->setPen(redPen);
			DrawPinCross(painter, pt1.x(), pt1.y(), pinWidth);
		}



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

	double SchemaItemBusExtractor::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = qBound(2, outputsCount(), 128);

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemBusExtractor::buildName() const
	{
		return QString("BusExtractor %1").arg(busTypeId());
	}

	void SchemaItemBusExtractor::setBusPins(const VFrame30::Bus& bus)
	{
		outputs().clear();

		for (const VFrame30::BusSignal& busSignal : bus.busSignals())
		{
			addOutput(-1, busSignal.name());
		}

		return;
	}

}
