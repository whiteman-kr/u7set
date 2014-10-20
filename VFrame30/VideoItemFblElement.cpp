#include "Stable.h"
#include "VideoItemFblElement.h"
#include "Scheme.h"

namespace VFrame30
{
	VideoItemFblElement::VideoItemFblElement(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemFblElement::VideoItemFblElement(SchemeUnit unit) :
		FblItemRect(unit)
	{
	}

	VideoItemFblElement::VideoItemFblElement(SchemeUnit unit, const Afbl::AfbElement& fblElement) :
		FblItemRect(unit),
		m_afbGuid(fblElement.guid()),
		m_params(fblElement.params())
	{
		// Создать входные и выходные сигналы в VFrame30::FblEtem
		//
		const std::vector<Afbl::AfbElementSignal>& inputSignals = fblElement.inputSignals();
		for (auto s = inputSignals.begin(); s != inputSignals.end(); ++s)
		{
			AddInput();
		}

		const std::vector<Afbl::AfbElementSignal>& outputSignals = fblElement.outputSignals();
		for (auto s = outputSignals.begin(); s != outputSignals.end(); ++s)
		{
			AddOutput();
		}

		// Проинициализировать паремтры значением по умолчанию
		//
		for (auto& p : m_params)
		{
			p.setValue(p.defaultValue());
		}
	}

	VideoItemFblElement::~VideoItemFblElement(void)
	{
	}

	void VideoItemFblElement::Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* pLayer) const
	{
		std::shared_ptr<Afbl::AfbElement> afb = scheme->afbCollection().get(afbGuid());
		if (afb.get() == nullptr)
		{
			// Such AfbItem was not found
			//
			assert(afb.get() != nullptr);
			return;
		}

		// Нарисовать прямоугольник и пины
		//
		FblItemRect::Draw(drawParam, scheme, pLayer);

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
			r.setLeft(r.left() + pinWidth + m_font.drawSize() / 4.0);
		}
		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth -  m_font.drawSize() / 4.0);
		}

		// Вывод названия Fbl элемента
		//
		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), afb->caption(), r);

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

		Proto::Write(vifble->mutable_afbguid(), m_afbGuid);

		for (const Afbl::AfbElementParam& p : m_params)
		{
			::Proto::FblElementParam* protoParam = vifble->mutable_params()->Add();
			p.SaveData(protoParam);
		}

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
		
		m_afbGuid = Proto::Read(vifble.afbguid());

		m_params.clear();
		for (int i = 0; i < vifble.params_size(); i++)
		{
			Afbl::AfbElementParam p;
			p.LoadData(vifble.params(i));
		}

		return true;
	}

	const QUuid& VideoItemFblElement::afbGuid() const
	{
		return m_afbGuid;
	}

	const std::vector<Afbl::AfbElementParam>& VideoItemFblElement::params() const
	{
		return m_params;
	}
}
