#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaLayer.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/DrawParam.h>

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

		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemBus* busitem = message->MutableExtension(Proto::schemaitem)->mutable_busitem();

		busitem->set_bustypeid(m_bus.busTypeId().toStdString());
		busitem->set_bustypehash(m_busTypeHash);

		Proto::Bus* busMessage = busitem->mutable_bus();
		result &= m_bus.save(busMessage);

		return result;
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
		if (message.GetExtension(Proto::schemaitem).has_busitem() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_busitem() == true);
			return false;
		}

		const Proto::SchemaItemBus& busitem = message.GetExtension(Proto::schemaitem).busitem();

		m_bus.setBusTypeId(QString::fromStdString(busitem.bustypeid()));
		m_busTypeHash = busitem.bustypehash();


		const Proto::Bus& busMessage = busitem.bus();
		result &= m_bus.load(busMessage);

		setBusPins(m_bus);

		return true;
	}

	void SchemaItemBus::draw(CDrawParam* drawParam) const
	{
		FblItemRect::draw(drawParam);

		QPainter* painter = drawParam->painter();
		QRectF r = itemRectPinIndent(drawParam);

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		DrawHelper::drawText(painter, m_font, itemUnit(), "bus type:\n" + busTypeId(), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	QString SchemaItemBus::buildName() const
	{
		assert(false);
		return {};
	}

	void SchemaItemBus::setBusPins(const AppSignalLib::Bus& bus)
	{
		Q_UNUSED(bus);
		assert(false);
	}

	QString SchemaItemBus::busTypeId() const
	{
		return m_bus.busTypeId();
	}

	const AppSignalLib::Bus& SchemaItemBus::busType() const
	{
		return m_bus;
	}

	void SchemaItemBus::setBusType(const AppSignalLib::Bus& bus)
	{
		m_bus = bus;
		setBusPins(bus);

		m_busTypeHash = m_bus.calcHash();
	}

	Hash SchemaItemBus::busTypeHash() const
	{
		return m_busTypeHash;
	}

	const AppSignalLib::Bus& SchemaItemBus::bus() const
	{
		return m_bus;
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
		addOutput(-1, E::SignalType::Bus, "bus_out");
	}

	SchemaItemBusComposer::~SchemaItemBusComposer()
	{
	}

	bool SchemaItemBusComposer::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemBus::SaveData(message);

		if (result == false ||
			message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemBusComposer* composer = message->MutableExtension(Proto::schemaitem)->mutable_buscomposer();

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
		if (message.GetExtension(Proto::schemaitem).has_buscomposer() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_buscomposer() == true);
			return false;
		}

		const Proto::SchemaItemBusComposer& composer = message.GetExtension(Proto::schemaitem).buscomposer();
		Q_UNUSED(composer);

		return true;
	}

	void SchemaItemBusComposer::draw(CDrawParam* drawParam) const
	{
		SchemaItemBus::draw(drawParam);

		QPainter* painter = drawParam->painter();

		QRectF r = itemRectPinIndent(drawParam);
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
		double dpiX = drawParam->realDpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
		QPointF pt2(drawParam->gridToDpi(vip.X - pinWidth, vip.Y));

		painter->setPen(pen);
		painter->drawLine(pt1, pt2);

		auto layer = parentLayer();
		Q_ASSERT(layer);

		int connectionCount = layer->GetPinPosConnectinCount(vip);

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

	double SchemaItemBusComposer::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = qBound(2, inputsCount(), 128);

		double pinVertGap =	VFrame30::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = VFrame30::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemBusComposer::toolTipText(double dpiX, double dpiY, double devicePixelRatio) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);
		Q_UNUSED(devicePixelRatio);

		QString html = QString(
R"(<p><b>BusComposer:</b> Creates a bus signal</p>
<p><b>BustTypeID:</b> %1</p>
<p><b>Inputs:</b></p>)")
.arg(busTypeId());

		html += "<table>";

		QString divider = QChar(0x2502);

		for (const AppSignalLib::BusSignal& busSignal : bus().busSignals())
		{
			QString type;
			switch (busSignal.type())
			{
			case E::SignalType::Analog:
				if (busSignal.analogFormat() == E::AnalogAppSignalFormat::SignedInt32)
				{
					type = "SI32";
					break;
				}
				if (busSignal.analogFormat() == E::AnalogAppSignalFormat::Float32)
				{
					type = "FP32";
					break;
				}

				Q_ASSERT(false);
				type = "UNKNOWN ANALOG";
				break;
			case E::SignalType::Discrete:
				type = "Discrete";
				break;
			case E::SignalType::Bus:
				type = QString("Bus (%1)").arg(busSignal.busTypeId());
				break;
			}

			QString stringPlacement;
			if (bus().autoSignalPlacement() == true)
			{
				stringPlacement = tr("Auto");
			}
			else
			{
				if (busSignal.type() == E::SignalType::Discrete)
				{
					stringPlacement = tr("Offset %1, Bit %2").arg(busSignal.inbusOffset()).arg(busSignal.inbusDiscreteBitNo());
				}
				else
				{
					stringPlacement = tr("Offset %1").arg(busSignal.inbusOffset());
				}
			}

			html += QString("<tr><td>  %2  </td><td>%1  %3  </td><td>%1  %4  </td><td>%1  %5  </td></tr>")
				.arg(divider)
				.arg(busSignal.signalId())
				.arg(busSignal.caption())
				.arg(type)
				.arg(stringPlacement);
		}

		html += "</table>";

		return html;
	}

	QString SchemaItemBusComposer::buildName() const
	{
		return QString("BusComposer %1").arg(busTypeId());
	}

	void SchemaItemBusComposer::setBusPins(const AppSignalLib::Bus& bus)
	{
		std::map<QString, AfbPin> oldInputs;	// We need to keep pin's GUIDs. If an item is met in UFB, then it will be copied
												// and old outputs GUID's
		for (const AfbPin& pin : inputs())
		{
			oldInputs[pin.caption()] = pin;
		}

		inputs().clear();

		for (const AppSignalLib::BusSignal& busSignal : bus.busSignals())
		{
			AfbPin& newPin = addInput(-1, busSignal.type(), busSignal.signalId());

			// Restore old pin guid, etc
			//
			if (auto foundOldPinIt = oldInputs.find(newPin.caption());
				foundOldPinIt != oldInputs.end())
			{
				newPin = foundOldPinIt->second;
			}
		}

		adjustHeight();

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
		addInput(-1, E::SignalType::Bus, "bus_in");
	}

	SchemaItemBusExtractor::~SchemaItemBusExtractor()
	{
	}

	bool SchemaItemBusExtractor::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemBus::SaveData(message);

		if (result == false ||
			message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemBusExtractor* extractor = message->MutableExtension(Proto::schemaitem)->mutable_busextractor();

		// Save specific properties' values
		//
		std::vector<std::shared_ptr<Property>> props = this->properties();

		for (const auto& p : props)
		{
			if (p->specific() == true)
			{
				::Proto::Property* protoProp = extractor->mutable_properties()->Add();
				::Proto::saveProperty(protoProp, p);
			}
		}

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
		if (message.GetExtension(Proto::schemaitem).has_busextractor() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_busextractor() == true);
			return false;
		}

		const Proto::SchemaItemBusExtractor& extractor = message.GetExtension(Proto::schemaitem).busextractor();

		// Load specific properties' values. They are already exists after calling setBus
		//
		std::vector<std::shared_ptr<Property>> specificProps = this->properties();

		for (const ::Proto::Property& p :  extractor.properties())
		{
			auto it = std::find_if(specificProps.begin(), specificProps.end(),
				[p](const std::shared_ptr<Property>& dp)
				{
					return dp->caption().toStdString() == p.name();
				});

			if (it == specificProps.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in" << label();
			}
			else
			{
				std::shared_ptr<Property> property = *it;
				assert(property->specific() == true);	// it's suppose to be specific property;

				bool loadOk = ::Proto::loadProperty(p, property);
				Q_UNUSED(loadOk);
				assert(loadOk);
			}
		}

		// Update pins
		//
		specificPropertyCouldBeChanged("", true);

		return true;
	}

	void SchemaItemBusExtractor::draw(CDrawParam* drawParam) const
	{
		SchemaItemBus::draw(drawParam);

		QPainter* painter = drawParam->painter();

		// Custom draw
		//
		QRectF r = itemRectPinIndent(drawParam);
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
		double dpiX = drawParam->realDpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		QPointF pt1(drawParam->gridToDpi(vip.X, vip.Y));
		QPointF pt2(drawParam->gridToDpi(vip.X + pinWidth, vip.Y));

		painter->setPen(pen);
		painter->drawLine(pt1, pt2);

		auto layer = parentLayer();
		Q_ASSERT(layer);

		int connectionCount = layer->GetPinPosConnectinCount(vip);

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

	double SchemaItemBusExtractor::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = qBound(2, outputsCount(), 128);

		double pinVertGap =	VFrame30::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = VFrame30::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemBusExtractor::toolTipText(double dpiX, double dpiY, double devicePixelRatio) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);
		Q_UNUSED(devicePixelRatio);

		QString html = QString(
R"(<p><b>BusExtractor:</b> Gets signal(s) from a bus</p>
<p><b>BustTypeID:</b> %1</p>
<p><b>Outputs:</b></p>)")
.arg(busTypeId());

		html += "<table>";

		QString divider = QChar(0x2502);

		for (const AppSignalLib::BusSignal& busSignal : bus().busSignals())
		{
			QString type;
			switch (busSignal.type())
			{
			case E::SignalType::Analog:
				if (busSignal.analogFormat() == E::AnalogAppSignalFormat::SignedInt32)
				{
					type = "SI32";
					break;
				}
				if (busSignal.analogFormat() == E::AnalogAppSignalFormat::Float32)
				{
					type = "FP32";
					break;
				}

				Q_ASSERT(false);
				type = "UNKNOWN ANALOG";
				break;
			case E::SignalType::Discrete:
				type = "Discrete";
				break;
			case E::SignalType::Bus:
				type = QString("Bus (%1)").arg(busSignal.busTypeId());
				break;
			}

			QString stringPlacement;
			if (bus().autoSignalPlacement() == true)
			{
				stringPlacement = tr("Auto");
			}
			else
			{
				if (busSignal.type() == E::SignalType::Discrete)
				{
					stringPlacement = tr("Offset %1, Bit %2").arg(busSignal.inbusOffset()).arg(busSignal.inbusDiscreteBitNo());
				}
				else
				{
					stringPlacement = tr("Offset %1").arg(busSignal.inbusOffset());
				}
			}

			html += QString("<tr><td>  %2  </td><td>%1  %3  </td><td>%1  %4  </td><td>%1  %5  </td></tr>")
				.arg(divider)
				.arg(busSignal.signalId())
				.arg(busSignal.caption())
				.arg(type)
				.arg(stringPlacement);
		}

		html += "</table>";

		return html;
	}

	QString SchemaItemBusExtractor::buildName() const
	{
		return QString("BusExtractor %1").arg(busTypeId());
	}

	void SchemaItemBusExtractor::specificPropertyCouldBeChanged(QString propertyName, const QVariant& value)
	{
		Q_UNUSED(propertyName)
		Q_UNUSED(value)

		std::map<QString, AfbPin> oldOuputs;	// We need to keep pin's GUIDs. If an item is met in UFB, then it will be copied
												// and old outputs GUID's are conected to associated IOs
												// Key is PinCaption
		for (const AfbPin& pin : outputs())
		{
			oldOuputs[pin.caption()] = pin;
		}

		outputs().clear();

		std::vector<std::shared_ptr<Property>> props = properties();

		for (const AppSignalLib::BusSignal& busSignal : busType().busSignals())
		{
			QString propName = "ShowOut_" + busSignal.signalId();

			auto it = std::find_if(props.begin(), props.end(),
					[&propName](const std::shared_ptr<Property>& p)
					{
						return p->caption() == propName;
					});

			if (it == props.end())
			{
				assert(false);
				AfbPin& newPin = addOutput(-1, busSignal.type(), busSignal.signalId());

				// Restore old pin guid, associatedIOs, etc
				//
				if (auto foundOldPinIt = oldOuputs.find(newPin.caption());
					foundOldPinIt != oldOuputs.end())
				{
					newPin = foundOldPinIt->second;
				}
			}
			else
			{
				std::shared_ptr<Property> p = *it;

				if (p->value().toBool() == false)
				{
				}
				else
				{
					AfbPin& newPin = addOutput(-1, busSignal.type(), busSignal.signalId());

					// Restore old pin guid, associatedIOs, etc
					//
					if (auto foundOldPinIt = oldOuputs.find(newPin.caption());
						foundOldPinIt != oldOuputs.end())
					{
						newPin = foundOldPinIt->second;
					}
				}
			}
		}

		adjustHeight();

		return;
	}

	void SchemaItemBusExtractor::setBusPins(const AppSignalLib::Bus& bus)
	{
		// Update ShowOut properties
		//
		std::vector<std::shared_ptr<Property>> props = properties();

		removeSpecificProperties();

		for (const AppSignalLib::BusSignal& busSignal : bus.busSignals())
		{
			QString propName = "ShowOut_" + busSignal.signalId();

			auto it = std::find_if(props.begin(), props.end(),
					[&propName](const std::shared_ptr<Property>& p)
					{
						return p->caption() == propName;
					});

			if (it == props.end())
			{
				auto p = this->addProperty(propName, PropertyNames::parametersCategory, true, QVariant(true));
				p->setSpecific(true);
			}
			else
			{
				addProperty(*it);
			}
		}

		specificPropertyCouldBeChanged("", true);
		adjustHeight();

		return;
	}

}
