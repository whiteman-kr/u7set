#include "Stable.h"
#include "VideoItemFblElement.h"

namespace VFrame30
{
	CVideoItemFblElement::CVideoItemFblElement(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemFblElement::CVideoItemFblElement(SchemeUnit unit) :
		CFblItemRect(unit)
	{
	}

	CVideoItemFblElement::CVideoItemFblElement(SchemeUnit unit, const Fbl::FblElement& fblElement) :
		CFblItemRect(unit)
	{
		m_fblElement = fblElement;

		// Создать входные и выходные сигналы в VFrame30::FblEtem
		//
		const std::vector<Fbl::FblElementSignal>& inputSignals = m_fblElement.inputSignals();
		for (auto s = inputSignals.begin(); s != inputSignals.end(); ++s)
		{
			AddInput();
		}

		const std::vector<Fbl::FblElementSignal>& outputSignals = m_fblElement.outputSignals();
		for (auto s = outputSignals.begin(); s != outputSignals.end(); ++s)
		{
			AddOutput();
		}

		// Проинициализировать паремтры значением по умолчанию
		//
		std::vector<Fbl::FblElementParam> params = m_fblElement.params();
		for (auto p = params.begin(); p != params.end(); ++p)
		{
			p->setValue(p->defaultValue());
		}
		m_fblElement.setParams(params);
	}

	CVideoItemFblElement::~CVideoItemFblElement(void)
	{
	}

	void CVideoItemFblElement::Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const
	{
		// Нарисовать прямоугольник и пины
		//
		CFblItemRect::Draw(drawParam, pFrame, pLayer);

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

		// Вывод названия Fbl элемента
		//
		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), m_fblElement.caption(), r);

		return;
	}

	// Serialization
	//
	bool CVideoItemFblElement::SaveData(Proto::Envelope* message) const
	{
		bool result = CFblItemRect::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}
	
		// --
		//
		Proto::VideoItemFblElement* vifble = message->mutable_videoitem()->mutable_videoitemfblelement();

		m_fblElement.Save(vifble->mutable_fblelement());
		//vifble->set_weight(weight);

		return true;
	}

	bool CVideoItemFblElement::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CFblItemRect::LoadData(message);
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
		
		m_fblElement.Load(vifble.fblelement());
		//fill = vifble.fill();

		return true;
	}

	const Fbl::FblElement& CVideoItemFblElement::fblElement() const
	{
		return m_fblElement;
	}
}
