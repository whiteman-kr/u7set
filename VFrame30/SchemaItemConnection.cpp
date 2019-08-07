#include "SchemaItemConnection.h"
#include "SchemaItemSignal.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "AppSignalController.h"
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
								 SchemaItemConnection::connectionIds,
								 SchemaItemConnection::setConnectionIds);
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

		connectionitem->set_connectionid(connectionIds().toStdString());

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

		setConnectionIds(QString::fromStdString(connectionitem.connectionid()));

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

	QString SchemaItemConnection::connectionIds() const
	{
		return m_connectionIds.join(QChar::LineFeed);
	}

	void SchemaItemConnection::setConnectionIds(const QString& value)
	{
		m_connectionIds = value.split(QRegExp("\\W+"), QString::SkipEmptyParts);

		if (double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);
			heightDocPt() < minHeight)
		{
			SetHeightInDocPt(minHeight);
		}

		return;
	}

	const QStringList& SchemaItemConnection::connectionIdsAsList() const
	{
		return m_connectionIds;
	}

	void SchemaItemConnection::setConnectionIdsAsList(const QStringList& value)
	{
		m_connectionIds = value;

		if (double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);
			heightDocPt() < minHeight)
		{
			SetHeightInDocPt(minHeight);
		}

		return;
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

		int dpiX = drawParam->dpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		// Draw line and symbol >>
		//
		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(linePen);

		p->drawLine(drawParam->gridToDpi(r.right() - pinWidth, r.top()),
					drawParam->gridToDpi(r.right() - pinWidth, r.bottom()));

		// >>
		//
		QRectF arrowRect(r);
		arrowRect.setLeft(r.right() - pinWidth);

		p->setPen(textColor());
		//DrawHelper::drawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);
		DrawHelper::drawText(p, m_font, itemUnit(), QChar(0x25BA), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

		// Draw ConnectionID
		//
		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

		p->setPen(textColor());

		DrawHelper::drawText(p, m_font, itemUnit(), connectionIds(), r, Qt::AlignHCenter | Qt::AlignVCenter);

		return;
	}

	double SchemaItemTransmitter::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = std::max(connectionIdsAsList().size(), inputsCount());

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemTransmitter::buildName() const
	{
		return QString("Transmitter %1").arg(connectionIds());
	}

	QString SchemaItemTransmitter::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString str = QString("Transmitter: "
							  "\n\tConnectionID: %1"
							  "\n"
							  "\nHint: Press F2 to edit ConnectionID")
						.arg(connectionIds());

		return str;
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

		if (m_pinCount != inputsCount())
		{
			removeAllInputs();

			for (int i = 0; i < m_pinCount; i++)
			{
				addInput(i, E::SignalType::Discrete, QString("in_%1").arg(QString::number(i + 1)));
			}

			if (double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);
				heightDocPt() < minHeight)
			{
				SetHeightInDocPt(minHeight);
			}
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

//		ADD_PROPERTY_GET_SET_CAT(bool,
//								 PropertyNames::multiLine,
//								 PropertyNames::appearanceCategory,
//								 true,
//								 SchemaItemReceiver::multiline,
//								 SchemaItemReceiver::setMultiline);

		auto strIdProperty = ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::appSignalId,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemReceiver::appSignalIds,
								 SchemaItemReceiver::setAppSignalIds);
		strIdProperty->setValidator(PropertyNames::appSignalIDsValidator);

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
//		receiver->set_multiline(m_multiline);
		receiver->set_appsignalids(appSignalIds().toStdString());
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
//		m_multiline = receiver.multiline();
		setAppSignalIds(QString::fromStdString(receiver.appsignalids()));
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

		int dpiX = drawParam->dpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		// --
		//
		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(linePen);

		// Draw slash lines
		//
		if (appSignalIdsAsList().size() > 1)
		{
			drawMultichannelSlashLines(drawParam, linePen);
		}

		// Draw line and symbol >>
		//
		p->drawLine(drawParam->gridToDpi(r.left() + pinWidth, r.top()),
					drawParam->gridToDpi(r.left() + pinWidth, r.bottom()));

		// >>
		//
		QRectF arrowRect(r);
		arrowRect.setRight(r.left() + pinWidth);

		p->setPen(textColor());

		//DrawHelper::drawText(p, m_font, itemUnit(), QLatin1String("\xBB"), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);
		DrawHelper::drawText(p, m_font, itemUnit(), QChar(0x25BA), arrowRect, Qt::AlignHCenter | Qt::AlignVCenter);

		r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
		r.setRight(r.right() - pinWidth - m_font.drawSize() / 4.0);

		double maxGridSize = qMax(drawParam->gridSize() * drawParam->pinGridStep(), m_font.drawSize());
		double lineHeight = CUtils::snapToGrid(maxGridSize, drawParam->gridSize());

		// Draw Data (AppSignalID, CustomerSignalID, Caption, etc
		//
		AppSignalParam signal;
		AppSignalState signalState;

		int textRow = 0;
		for (const QString& appSignalId : this->appSignalIdsAsList())
		{
			signal.setAppSignalId(appSignalId);
			signalState.m_flags.valid = false;

			if (drawParam->isMonitorMode() == true)
			{
				signal = drawParam->appSignalController()->signalParam(appSignalId, nullptr);
				signalState = drawParam->appSignalController()->signalState(appSignalId, nullptr);
			}

			QRectF signalRect = {r.left(), r.top() + lineHeight * textRow, r.width(), lineHeight};

			QString dataText = SchemaItemSignal::getCoulumnText(drawParam,
																m_dataType,
																signal,
																signalState,
																signal,				// There is no impact signalfor connection
																signalState,		// There is no impact signalfor connection
																m_analogFormat,
																m_precision);

			DrawHelper::drawText(p, m_font, itemUnit(), dataText, signalRect, Qt::AlignHCenter | Qt::AlignBottom);

			textRow ++;
		}

		// Draw ConnectionIDs
		//
		p->setPen(textColor());

		for (const QString& connectionId : connectionIdsAsList())
		{
			QRectF connIdRect = {r.left(), r.top() + lineHeight * textRow, r.width(), lineHeight};
			DrawHelper::drawText(p, m_font, itemUnit(), connectionId, connIdRect, Qt::AlignHCenter | Qt::AlignVCenter);

			textRow ++;
		}

		// Draw highlights for m_appSignalIds
		//
		for (const QString& appSignalId : m_appSignalIds)
		{
			if (drawParam->hightlightIds().contains(appSignalId) == true)
			{
				QRectF highlightRect = boundingRectInDocPt(drawParam);
				drawHighlightRect(drawParam, highlightRect);
				break;
			}
		}

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
		int pinCount = qBound(2, connectionIdsAsList().size() + appSignalIdsAsList().size(), 256);

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	QString SchemaItemReceiver::buildName() const
	{
		return QString("Receiver %1").arg(connectionIds());
	}

	QString SchemaItemReceiver::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString str = QString("Receiver: "
							  "\n\tConnectionID: %1"
							  "\n\tAppSignalID: %2"
							  "\n"
							  "\nHint: Press F2 to edit AppSignalID and ConnectionID")
						.arg(connectionIds())
						.arg(appSignalIds());

		return str;
	}

	QString SchemaItemReceiver::appSignalIds() const
	{
		return m_appSignalIds.join(QChar::LineFeed);
	}

	void SchemaItemReceiver::setAppSignalIds(const QString& value)
	{
		setAppSignalIdsAsList(value.split(QRegExp("\\s+"), QString::SkipEmptyParts));
		return;
	}

	const QStringList& SchemaItemReceiver::appSignalIdsAsList() const
	{
		return m_appSignalIds;
	}

	void SchemaItemReceiver::setAppSignalIdsAsList(const QStringList& value)
	{
		m_appSignalIds = value;

		if (double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);
			heightDocPt() < minHeight)
		{
			SetHeightInDocPt(minHeight);
		}

		return;
	}

	bool SchemaItemReceiver::showValidity() const
	{
		return m_showValidity;
	}

	void SchemaItemReceiver::setShowValidity(bool value)
	{
		m_showValidity = value;

		removeAllOutputs();

		addOutput(0, E::SignalType::Discrete, QLatin1String("out"));

		if (m_showValidity == true)
		{
			addOutput(1, E::SignalType::Discrete, QLatin1String("validity"));
		}
	}

//	bool SchemaItemReceiver::multiline() const
//	{
//		return m_multiline;
//	}

//	void SchemaItemReceiver::setMultiline(bool value)
//	{
//		m_multiline = value;
//	}

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
		const std::vector<VFrame30::AfbPin>& outs = outputs();

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

		const std::vector<VFrame30::AfbPin>& outs = outputs();

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
