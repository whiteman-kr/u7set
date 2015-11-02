#include "Stable.h"
#include "SchemeItemSignal.h"

namespace VFrame30
{
	//
	// CSchemeItemSignal
	//
	SchemeItemSignal::SchemeItemSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemSignal::SchemeItemSignal(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	SchemeItemSignal::~SchemeItemSignal(void)
	{
	}

	void SchemeItemSignal::Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* layer) const
	{
		FblItemRect::Draw(drawParam, scheme, layer);

		//--
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
		QPaintDevice* paintDevice = p->device();
		if (paintDevice == nullptr)
		{
			assert(paintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = paintDevice->logicalDpiX();
		}

		double pinWidth = GetPinWidth(itemUnit(), dpiX);


		if (multiChannel() == true)
		{
			QPen pen(lineColor());
			pen.setWidthF(m_weight);		// Don't use getter!
			p->setPen(pen);

			if (inputsCount() > 0)
			{
				const std::list<AfbPin>& inputPins = inputs();
				assert(inputPins.empty() == false);

				p->drawLine(QPointF(r.left() + (pinWidth / 3.0) * 2.0, inputPins.front().y() - pinWidth / 4.0),
							QPointF(r.left() + (pinWidth / 3.0) * 1.0, inputPins.front().y() + pinWidth / 4.0));
			}

			if (outputsCount() > 0)
			{
				const std::list<AfbPin>& pins = outputs();
				assert(pins.empty() == false);

				p->drawLine(QPointF(r.right() - (pinWidth / 3.0) * 2.0, pins.front().y() + pinWidth / 4.0),
							QPointF(r.right() - (pinWidth / 3.0) * 1.0, pins.front().y() - pinWidth / 4.0));
			}
		}

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw Signals StrIDs
		//
		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), signalStrIds(), r, Qt::AlignLeft | Qt::AlignTop);

		return;
	}

	QString SchemeItemSignal::signalStrIds() const
	{
		QString result;

		for (QString s : m_signalStrIds)
		{
			s = s.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(s);
		}

		return result;
	}

	void SchemeItemSignal::setSignalStrIds(const QString& s)
	{
		m_signalStrIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* SchemeItemSignal::mutable_signalStrIds()
	{
		return &m_signalStrIds;
	}

	bool SchemeItemSignal::multiChannel() const
	{
		return m_multiChannel;
	}

	void SchemeItemSignal::setMultiChannel(bool value)
	{
		m_multiChannel = value;
	}

	bool SchemeItemSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		Proto::SchemeItemSignal* signal = message->mutable_schemeitem()->mutable_signal();

		for (const QString& strId : m_signalStrIds)
		{
			::Proto::wstring* ps = signal->add_signalstrids();
			Proto::Write(ps, strId);
		}

		signal->set_multichannel(m_multiChannel);

		return true;
	}

	bool SchemeItemSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}
		
		// --
		//
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemeitem().has_signal() == false)
		{
			assert(message.schemeitem().has_signal());
			return false;
		}

		const Proto::SchemeItemSignal& signal = message.schemeitem().signal();

		m_signalStrIds.clear();
		m_signalStrIds.reserve(signal.signalstrids_size());

		for (int i = 0; i < signal.signalstrids_size(); i++)
		{
			QString s;
			Proto::Read(signal.signalstrids().Get(i), &s);
			m_signalStrIds.push_back(s);
		}

		m_multiChannel = signal.multichannel();

		return true;
	}


	//
	// CSchemeItemInputSignal
	//
	SchemeItemInput::SchemeItemInput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemInput::SchemeItemInput(SchemeUnit unit) :
		SchemeItemSignal(unit)
	{
		addOutput();
		setSignalStrIds("#IN_STRID");
	}

	SchemeItemInput::~SchemeItemInput(void)
	{
		assert(outputsCount() == 1);
	}

	QString SchemeItemInput::buildName() const
	{
		return QString("Input (%1)").arg(signalStrIds());
	}

	// Serialization
	//
	bool SchemeItemInput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemeItemSignal::SaveData(message);
		
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemInputSignal* inputSignal = */message->mutable_schemeitem()->mutable_inputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemeItemInput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		bool result = SchemeItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		// --
		//
		if (message.schemeitem().has_inputsignal() == false)
		{
			assert(message.schemeitem().has_inputsignal());
			return false;
		}

		/*const Proto::VideoItemInputSignal& inputSignal = */message.schemeitem().inputsignal();
		//fill = inputSignal.fill();

		return true;
	}
	
	//
	// CSchemeItemOutputSignal
	//
	SchemeItemOutput::SchemeItemOutput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemOutput::SchemeItemOutput(SchemeUnit unit) :
		SchemeItemSignal(unit)
	{
		addInput();
		setSignalStrIds("#OUT_STRID");
	}

	SchemeItemOutput::~SchemeItemOutput(void)
	{
		assert(inputsCount() == 1);
	}

	QString SchemeItemOutput::buildName() const
	{
		return QString("Output (%1)").arg(signalStrIds());
	}

	// Serialization
	//
	bool SchemeItemOutput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemeItemSignal::SaveData(message);
		
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* outputSignal = */message->mutable_schemeitem()->mutable_outputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemeItemOutput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		bool result = SchemeItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemeitem().has_outputsignal() == false)
		{
			assert(message.schemeitem().has_outputsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& outputSignal = */message.schemeitem().outputsignal();
		//fill = inputSignal.fill();

		return true;
	}

}

