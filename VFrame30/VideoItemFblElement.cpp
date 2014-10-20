#include "Stable.h"
#include "VideoItemFblElement.h"

namespace VFrame30
{
	VideoItemFblElement::VideoItemFblElement(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	VideoItemFblElement::VideoItemFblElement(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	VideoItemFblElement::VideoItemFblElement(SchemeUnit unit, const Afbl::AfbElement& fblElement) :
		FblItemRect(unit),
		m_afblElement(fblElement)
	{
		// ������� ������� � �������� ������� � VFrame30::FblEtem
		//
		const std::vector<Afbl::AfbElementSignal>& inputSignals = m_afblElement.inputSignals();
		for (auto s = inputSignals.begin(); s != inputSignals.end(); ++s)
		{
			AddInput();
		}

		const std::vector<Afbl::AfbElementSignal>& outputSignals = m_afblElement.outputSignals();
		for (auto s = outputSignals.begin(); s != outputSignals.end(); ++s)
		{
			AddOutput();
		}

		// ������������������� �������� ��������� �� ���������
		//
		std::vector<Afbl::AfbElementParam> params = m_afblElement.params();
		for (auto p = params.begin(); p != params.end(); ++p)
		{
			p->setValue(p->defaultValue());
		}

		m_afblElement.setParams(params);
	}

	VideoItemFblElement::~VideoItemFblElement(void)
	{
	}

	void VideoItemFblElement::Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const
	{
		// ���������� ������������� � ����
		//
		FblItemRect::Draw(drawParam, pFrame, pLayer);

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

		// ����� �������� Fbl ��������
		//
		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), m_afblElement.caption(), r);

		return;
	}

	// Serialization
	//
	bool VideoItemFblElement::SaveData(Proto::Envelope* message) const
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
		Proto::VideoItemFblElement* vifble = message->mutable_videoitem()->mutable_videoitemfblelement();

		m_afblElement.Save(vifble->mutable_fblelement());
		//vifble->set_weight(weight);

		return true;
	}

	bool VideoItemFblElement::LoadData(const Proto::Envelope& message)
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
		if (message.videoitem().has_videoitemfblelement() == false)
		{
			assert(message.videoitem().has_videoitemfblelement());
			return false;
		}
		
		const Proto::VideoItemFblElement& vifble = message.videoitem().videoitemfblelement();
		
		m_afblElement.Load(vifble.fblelement());
		//fill = vifble.fill();

		return true;
	}

	const Afbl::AfbElement& VideoItemFblElement::fblElement() const
	{
		return m_afblElement;
	}
}
