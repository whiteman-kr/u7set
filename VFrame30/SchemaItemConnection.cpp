#include "SchemaItemConnection.h"
#include "SchemaItemSignal.h"
#include "../lib/Signal.h"
#include "../lib/AppSignalManager.h"

namespace VFrame30
{

	SchemaItemConnection::SchemaItemConnection() :
		SchemaItemConnection(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemConnection::SchemaItemConnection(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::connectionId,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemConnection::connectionId,
								 SchemaItemConnection::setConnectionId);
	}

	SchemaItemConnection::~SchemaItemConnection()
	{
	}

	bool SchemaItemConnection::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemConnection* connectionitem = message->mutable_schemaitem()->mutable_connectionitem();

		connectionitem->set_connectionid(m_connectionId.toStdString());

		return true;
	}

	bool SchemaItemConnection::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_connectionitem() == false)
		{
			assert(message.schemaitem().has_connectionitem() == true);
			return false;
		}

		const Proto::SchemaItemConnection& connectionitem = message.schemaitem().connectionitem();

		m_connectionId = QString::fromStdString(connectionitem.connectionid());

		return true;
	}

	void SchemaItemConnection::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);
		return;
	}

	QString SchemaItemConnection::buildName() const
	{
		assert(false);
		return QString();
	}

	QString SchemaItemConnection::connectionId() const
	{
		return m_connectionId;
	}

	void SchemaItemConnection::setConnectionId(const QString& value)
	{
		m_connectionId = value;
		m_connectionId = m_connectionId.trimmed();
	}

	//
	//
	//			SchemaItemTransmitter
	//
	//
	SchemaItemTransmitter::SchemaItemTransmitter() :
		SchemaItemTransmitter(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemTransmitter::SchemaItemTransmitter(SchemaUnit unit) :
		SchemaItemConnection(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::pinCount,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemTransmitter::pinCount,
								 SchemaItemTransmitter::setPinCount);

		setPinCount(1);
	}

	SchemaItemTransmitter::~SchemaItemTransmitter()
	{
	}

	bool SchemaItemTransmitter::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemConnection::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemTransmitter* transmitter = message->mutable_schemaitem()->mutable_transmitteritem();

		transmitter->set_pincount(m_pinCount);

		return true;
	}

	bool SchemaItemTransmitter::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemConnection::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_transmitteritem() == false)
		{
			assert(message.schemaitem().has_transmitteritem() == true);
			return false;
		}

		const Proto::SchemaItemTransmitter& transmitter = message.schemaitem().transmitteritem();

		setPinCount(transmitter.pincount());

		return true;
	}

	void SchemaItemTransmitter::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemConnection::Draw(drawParam, schema, layer);

		// Custom draw
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

		// Draw line and symbol >>
		//
		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(linePen);

		p->drawLine(QPointF(r.right() - pinWidth, r.top()), QPointF(r.right() - pinWidth, r.bottom()));

		// >>
		//
		QRectF arrowRect(r);
		arrowRect.setLeft(r.right() - pinWidth);

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

		// Draw ConnectionID
		//
		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), connectionId(), r, Qt::AlignHCenter | Qt::AlignTop);

		return;
	}

	bool SchemaItemTransmitter::searchText(const QString& text) const
	{
		bool f = connectionId().contains(text, Qt::CaseInsensitive);
		return f;
	}

	QString SchemaItemTransmitter::buildName() const
	{
		return QString("Transmitter %1").arg(connectionId());
	}

	int SchemaItemTransmitter::pinCount() const
	{
		return m_pinCount;
	}

	void SchemaItemTransmitter::setPinCount(int value)
	{
		if (value < 1)
		{
			value = 1;
		}

		if (value > 64)
		{
			value = 64;
		}

		m_pinCount = value;

		removeAllInputs();

		for (int i = 0; i < m_pinCount; i++)
		{
			addInput(i, QString("in_%1").arg(QString::number(i + 1)));
		}

		double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);

		if (heightDocPt() < minHeight)
		{
			SetHeightInDocPt(minHeight);
		}
	}

	//
	//
	//			SchemaItemTransmitter
	//
	//

	SchemaItemReceiver::SchemaItemReceiver() :
		SchemaItemReceiver(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemReceiver::SchemaItemReceiver(SchemaUnit unit) :
		SchemaItemConnection(unit)
	{
		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::showValidityPin,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemReceiver::showValidity,
								 SchemaItemReceiver::setShowValidity);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::appSignalId,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemReceiver::appSignalId,
								 SchemaItemReceiver::setAppSignalId);

		ADD_PROPERTY_GET_SET_CAT(E::ColumnData,
								 PropertyNames::dataType,
								 PropertyNames::monitorCategory,
								 true,
								 SchemaItemReceiver::data,
								 SchemaItemReceiver::setData);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::precision,
								 PropertyNames::monitorCategory,
								 true,
								 SchemaItemReceiver::precision,
								 SchemaItemReceiver::setPrecision);

		ADD_PROPERTY_GET_SET_CAT(E::AnalogFormat,
								 PropertyNames::analogFormat,
								 PropertyNames::monitorCategory,
								 true,
								 SchemaItemReceiver::analogFormat,
								 SchemaItemReceiver::setAnalogFormat);

		setShowValidity(true);
	}

	SchemaItemReceiver::~SchemaItemReceiver()
	{
	}

	bool SchemaItemReceiver::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemConnection::SaveData(message);

		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemReceiver* receiver = message->mutable_schemaitem()->mutable_receiveritem();

		receiver->set_showvalidity(m_showValidity);
		receiver->set_appsignalid(m_appSignalId.toStdString());
		receiver->set_datatype(static_cast<int>(m_dataType));
		receiver->set_precision(m_precision);
		receiver->set_analogformat(static_cast<int>(m_analogFormat));

		return true;
	}

	bool SchemaItemReceiver::LoadData(const Proto::Envelope& message)
	{
		bool result = SchemaItemConnection::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_receiveritem() == false)
		{
			assert(message.schemaitem().has_receiveritem() == true);
			return false;
		}

		const Proto::SchemaItemReceiver& receiver = message.schemaitem().receiveritem();

		m_showValidity = receiver.showvalidity();
		m_appSignalId = QString::fromStdString(receiver.appsignalid());
		m_dataType = static_cast<E::ColumnData>(receiver.datatype());
		m_precision = receiver.precision();
		m_analogFormat = static_cast<E::AnalogFormat>(receiver.analogformat());

		return true;
	}

	void SchemaItemReceiver::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		SchemaItemConnection::Draw(drawParam, schema, layer);

		// Custom draw
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

		// Draw line and symbol >>
		//
		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(linePen);

		p->drawLine(QPointF(r.left() + pinWidth, r.top()), QPointF(r.left() + pinWidth, r.bottom()));

		// >>
		//
		QRectF arrowRect(r);
		arrowRect.setRight(r.left() + pinWidth);

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

		// Draw ConnectionID
		//
		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), connectionId(), r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw Data (AppSignalID, CustomerSignalID, Caption, etc
		//
		QString appSignalId = this->appSignalId();

		Signal signal;
		signal.setAppSignalID(appSignalId);

		AppSignalState signalState;
		signalState.flags.valid = false;

		bool signalFound = false;

		if (drawParam->isMonitorMode() == true)
		{
			signalFound = drawParam->appSignalManager()->signal(appSignalId, &signal);
			signalState = drawParam->appSignalManager()->signalState(appSignalId);
		}

		QString dataText = SchemaItemSignal::getCoulumnText(drawParam, m_dataType, signal, signalState, m_analogFormat, m_precision);

		DrawHelper::DrawText(p, m_font, itemUnit(), dataText, r, Qt::AlignHCenter | Qt::AlignBottom);

		return;
	}

	bool SchemaItemReceiver::searchText(const QString& text) const
	{
		bool f1 = m_appSignalId.contains(text, Qt::CaseInsensitive);
		bool f2 = connectionId().contains(text, Qt::CaseInsensitive);

		return f1 | f2;
	}

	double SchemaItemReceiver::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = 2;

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemReceiver::buildName() const
	{
		return QString("Receiver %1").arg(connectionId());
	}

	const QString& SchemaItemReceiver::appSignalId() const
	{
		return m_appSignalId;
	}

	void SchemaItemReceiver::setAppSignalId(const QString& value)
	{
		m_appSignalId = value;
		m_appSignalId = m_appSignalId.trimmed();
	}

	bool SchemaItemReceiver::showValidity() const
	{
		return m_showValidity;
	}

	void SchemaItemReceiver::setShowValidity(bool value)
	{
		m_showValidity = value;

		removeAllOutputs();

		addOutput(0, QLatin1String("out"));

		if (m_showValidity == true)
		{
			addOutput(1, QLatin1String("validity"));
		}
	}

	bool SchemaItemReceiver::isValidityPin(const QUuid& pinGuid) const
	{
		assert(pinGuid.isNull() == false);

		if (showValidity() == false)
		{
			// There is no validity output
			//
			return false;
		}

		// pin 0 is always output
		// pin 1 is always validity
		// its defined in setShowValidity function
		//
		const std::list<VFrame30::AfbPin>& outs = outputs();

		if (outs.size() != 2)
		{
			assert(outs.size() == 2);
			return false;
		}

		if (outs.back().guid() == pinGuid)
		{
			return true;
		}
		else
		{
			// if it's not validity this must be "output"
			//
			assert(outs.front().guid() == pinGuid);
			return false;
		}
	}

	bool SchemaItemReceiver::isOutputPin(const QUuid& pinGuid) const
	{
		assert(pinGuid.isNull() == false);

		// pin 0 is always output
		// pin 1 is always validity
		// its defined in setShowValidity function
		//

		const std::list<VFrame30::AfbPin>& outs = outputs();

		if (outs.size() < 1 || outs.size() > 2)
		{
			assert(outs.size() >= 1 && outs.size() <=2);
			return false;
		}

		bool result = outs.front().guid() == pinGuid;
		return result;
	}

	E::ColumnData SchemaItemReceiver::data() const
	{
		return m_dataType;
	}

	void SchemaItemReceiver::setData(E::ColumnData value)
	{
		m_dataType = value;
	}

	int SchemaItemReceiver::precision() const
	{
		return m_precision;
	}

	void SchemaItemReceiver::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 12)
		{
			value = 12;
		}

		m_precision = value;
	}

	E::AnalogFormat SchemaItemReceiver::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemReceiver::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}

}
