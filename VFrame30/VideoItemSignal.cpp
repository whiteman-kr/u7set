#include "Stable.h"
#include "VideoItemSignal.h"

namespace VFrame30
{
	//
	// CVideoItemSignal
	//
	VideoItemSignal::VideoItemSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemSignal::VideoItemSignal(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	VideoItemSignal::~VideoItemSignal(void)
	{
	}

	void VideoItemSignal::Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* layer) const
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

		DrawHelper::DrawText(p, m_font, itemUnit(), signalStrIds(), r);

		return;
	}

	QString VideoItemSignal::signalStrIds() const
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

	void VideoItemSignal::setSignalStrIds(const QString& s)
	{
		m_signalStrIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
	}

	QStringList* VideoItemSignal::mutable_signalStrIds()
	{
		return &m_signalStrIds;
	}

	bool VideoItemSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		Proto::VideoItemSignal* signal = message->mutable_videoitem()->mutable_signal();

		for (const QString& strId : m_signalStrIds)
		{
			::Proto::wstring* ps = signal->add_signalstrids();
			Proto::Write(ps, strId);
		}

		return true;
	}

	bool VideoItemSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
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
		if (message.videoitem().has_signal() == false)
		{
			assert(message.videoitem().has_signal());
			return false;
		}

		const Proto::VideoItemSignal& signal = message.videoitem().signal();

		m_signalStrIds.clear();
		m_signalStrIds.reserve(signal.signalstrids_size());

		for (int i = 0; i < signal.signalstrids_size(); i++)
		{
			QString s = Proto::Read(signal.signalstrids().Get(i));
			m_signalStrIds.push_back(s);
		}

		return true;
	}


	//
	// CVideoItemInputSignal
	//
	VideoItemInputSignal::VideoItemInputSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemInputSignal::VideoItemInputSignal(SchemeUnit unit) :
		VideoItemSignal(unit)
	{
		AddOutput();
	}

	VideoItemInputSignal::~VideoItemInputSignal(void)
	{
#ifdef _DEBUG
		assert(outputsCount() == 1);
#endif 
	}

	// Serialization
	//
	bool VideoItemInputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = VideoItemSignal::SaveData(message);
		
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemInputSignal* inputSignal = */message->mutable_videoitem()->mutable_inputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool VideoItemInputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = VideoItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		// --
		//
		if (message.videoitem().has_inputsignal() == false)
		{
			assert(message.videoitem().has_inputsignal());
			return false;
		}

		/*const Proto::VideoItemInputSignal& inputSignal = */message.videoitem().inputsignal();
		//fill = inputSignal.fill();

		return true;
	}
	
	//
	// CVideoItemOutputSignal
	//
	VideoItemOutputSignal::VideoItemOutputSignal(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemOutputSignal::VideoItemOutputSignal(SchemeUnit unit) :
		VideoItemSignal(unit)
	{
		AddInput();
	}

	VideoItemOutputSignal::~VideoItemOutputSignal(void)
	{
#ifdef _DEBUG
		assert(inputsCount() == 1);
#endif
	}

	// Serialization
	//
	bool VideoItemOutputSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = VideoItemSignal::SaveData(message);
		
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* outputSignal = */message->mutable_videoitem()->mutable_outputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool VideoItemOutputSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = VideoItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_outputsignal() == false)
		{
			assert(message.videoitem().has_outputsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& outputSignal = */message.videoitem().outputsignal();
		//fill = inputSignal.fill();

		return true;
	}

}

