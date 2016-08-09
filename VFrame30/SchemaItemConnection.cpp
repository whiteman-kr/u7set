#include "SchemaItemConnection.h"
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

//		//--
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

//		int dpiX = 96;
//		QPaintDevice* pPaintDevice = p->device();
//		if (pPaintDevice == nullptr)
//		{
//			assert(pPaintDevice);
//			dpiX = 96;
//		}
//		else
//		{
//			dpiX = pPaintDevice->logicalDpiX();
//		}

//		double pinWidth = GetPinWidth(itemUnit(), dpiX);

//		if (inputsCount() > 0)
//		{
//			r.setLeft(r.left() + pinWidth);
//		}

//		if (outputsCount() > 0)
//		{
//			r.setRight(r.right() - pinWidth);
//		}

//		r.setLeft(r.left() + m_font.drawSize() / 4.0);
//		r.setRight(r.right() - m_font.drawSize() / 4.0);

//		// Draw Signals StrIDs
//		//
//		QString text = valueToString();

//		p->setPen(textColor());

//		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignTop);

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
		linePen.setWidthF(weight());
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
		m_appSignalId.fromStdString(receiver.appsignalid());

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
		linePen.setWidthF(weight());
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

		// Draw AppSignalID
		//
		QString signalText = appSignalId();

		if (drawParam->isMonitorMode() == true)
		{
			// Try to get CustomSignalID
			//
			Signal appSignal;

			bool ok = drawParam->appSignalManager()->signal(appSignalId(), &appSignal);

			if (ok == true)
			{
				signalText = appSignal.customAppSignalID();
			}
		}

		DrawHelper::DrawText(p, m_font, itemUnit(), signalText, r, Qt::AlignHCenter | Qt::AlignBottom);

		return;
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
}
