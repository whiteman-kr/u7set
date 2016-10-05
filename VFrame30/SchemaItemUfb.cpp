#include "Stable.h"
#include "SchemaItemUfb.h"
#include "UfbSchema.h"
#include "SchemaItemSignal.h"

namespace VFrame30
{
	SchemaItemUfb::SchemaItemUfb(void) :
		SchemaItemUfb(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemUfb::SchemaItemUfb(SchemaUnit unit) :
		FblItemRect(unit)
	{
		auto schemaIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::ufbSchemaId, true, SchemaItemUfb::ufbSchemaId);
		auto captionProp = ADD_PROPERTY_GETTER(QString, PropertyNames::caption, true, SchemaItemUfb::ufbCaption);
		auto versionProp = ADD_PROPERTY_GETTER(int, PropertyNames::ufbSchemaVersion, true, SchemaItemUfb::ufbSchemaVersion);

		schemaIdProp->setCategory(PropertyNames::functionalCategory);
		captionProp->setCategory(PropertyNames::functionalCategory);
		versionProp->setCategory(PropertyNames::functionalCategory);
	}

	SchemaItemUfb::SchemaItemUfb(SchemaUnit unit, const UfbSchema* ufbSchema, QString* errorMsg) :
		FblItemRect(unit)
	{
		assert(errorMsg);

		assert(ufbSchema);
		if (ufbSchema == nullptr)
		{
			*errorMsg = tr("Pointer to UfbSchema is nullptr.");
			return;
		}

		auto schemaIdProp = ADD_PROPERTY_GETTER(QString, PropertyNames::ufbSchemaId, true, SchemaItemUfb::ufbSchemaId);
		auto captionProp = ADD_PROPERTY_GETTER(QString, PropertyNames::caption, true, SchemaItemUfb::ufbCaption);
		auto versionProp = ADD_PROPERTY_GETTER(int, PropertyNames::ufbSchemaVersion, true, SchemaItemUfb::ufbSchemaVersion);

		schemaIdProp->setCategory(PropertyNames::functionalCategory);
		captionProp->setCategory(PropertyNames::functionalCategory);
		versionProp->setCategory(PropertyNames::functionalCategory);

		// Create input output signals in VFrame30::FblEtem
		//
		updateElement(ufbSchema, errorMsg);

		return;
	}

	SchemaItemUfb::~SchemaItemUfb(void)
	{
	}

	void SchemaItemUfb::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const
	{
		QPainter* p = drawParam->painter();

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

		FontParam smallFont = m_font;
		smallFont.setDrawSize(m_font.drawSize() * 0.75);

		// Draw rect and pins
		//
		FblItemRect::Draw(drawParam, schema, pLayer);

		// Draw other
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		QRectF labelRect(r);	// save rect for future use

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw caption
		//
		QString text = m_ufbCaption;

		p->setPen(textColor());
		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw Label
		//
		if (drawParam->infoMode() == true)
		{
			QString labelText = label();

			labelRect.moveBottomLeft(labelRect.topRight());

			p->setPen(Qt::darkGray);
			DrawHelper::DrawText(p, smallFont, itemUnit(), labelText, labelRect, Qt::TextDontClip | Qt::AlignLeft | Qt::AlignBottom);
		}

		return;
	}

	// Serialization
	//
	bool SchemaItemUfb::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemUfb* ufbpb = message->mutable_schemaitem()->mutable_ufb();

		ufbpb->set_ufbschemaid(m_ufbSchemaId.toStdString());
		ufbpb->set_ufbcaption(m_ufbCaption.toStdString());
		ufbpb->set_ufbversion(m_ufbVersion);

		return true;
	}

	bool SchemaItemUfb::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
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
		if (message.schemaitem().has_ufb() == false)
		{
			assert(message.schemaitem().has_ufb());
			return false;
		}
		
		const Proto::SchemaItemUfb& ufbpb = message.schemaitem().ufb();

		m_ufbSchemaId = QString::fromStdString(ufbpb.ufbschemaid());
		m_ufbCaption = QString::fromStdString(ufbpb.ufbcaption());
		m_ufbVersion = ufbpb.ufbversion();

		return true;
	}

	QString SchemaItemUfb::buildName() const
	{
		return QString("%1 %2")
				.arg(m_ufbCaption)
				.arg(label());
	}


	bool SchemaItemUfb::updateElement(const UfbSchema* ufbSchema, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (m_ufbSchemaId.isEmpty() == false &&
			m_ufbSchemaId != ufbSchema->schemaID())
		{
			assert(false);
			*errorMessage += tr("Update %1 from different UFB %2.").arg(m_ufbSchemaId).arg(ufbSchema->schemaID());
			return false;
		}

		m_ufbSchemaId = ufbSchema->schemaID();
		m_ufbCaption = ufbSchema->caption();
		m_ufbVersion = ufbSchema->version();

		// Get in/outs from ufb schema
		//
		std::vector<const SchemaItemSignal*> ufbInputs;
		std::vector<const SchemaItemSignal*> ufbOutputs;

		ufbInputs.reserve(16);
		ufbOutputs.reserve(16);

		for (std::shared_ptr<SchemaLayer> layer : ufbSchema->Layers)
		{
			if (layer->compile() == true)
			{
				for (std::shared_ptr<SchemaItem> item : layer->Items)
				{
					const FblItemRect* fblItemRect = item->toFblItemRect();
					if (fblItemRect == nullptr)
					{
						continue;
					}

					if (fblItemRect->isInputSignalElement() == true)
					{
						ufbInputs.push_back(fblItemRect->toInputSignalElement());
						continue;
					}

					if (fblItemRect->isOutputSignalElement() == true)
					{
						ufbOutputs.push_back(fblItemRect->toOutputSignalElement());
						continue;
					}
				}

				break;
			}
		}

		// Create in/outs in this item
		//
		removeAllInputs();
		removeAllOutputs();

		for (const SchemaItemSignal* in : ufbInputs)
		{
			this->addInput(-1, in->appSignalIds());
		}

		for (const SchemaItemSignal* out: ufbOutputs)
		{
			this->addOutput(-1, out->appSignalIds());
		}

		// --
		//

		return true;
	}

	QString SchemaItemUfb::ufbSchemaId() const
	{
		return m_ufbSchemaId;
	}

	QString SchemaItemUfb::ufbCaption() const
	{
		return m_ufbCaption;
	}

	int SchemaItemUfb::ufbSchemaVersion() const
	{
		return m_ufbVersion;
	}
}
