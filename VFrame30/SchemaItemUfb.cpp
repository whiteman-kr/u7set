#include "Stable.h"
#include "SchemaItemUfb.h"
#include "Schema.h"

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

		// Draw line under caption
		//

//		QPen captionLinePen(lineColor());
//		captionLinePen.setWidthF(0.0);		// Don't use getter!

//		p->setPen(captionLinePen);

//		p->drawLine(QPointF(r.left(), topDocPt() + m_font.drawSize() * 1.5),
//					QPointF(r.left() + r.width(), topDocPt() + m_font.drawSize() * 1.5));

		return;
	}

	// Serialization
	//
	bool SchemaItemUfb::SaveData(Proto::Envelope* message) const
	{
//		bool result = FblItemRect::SaveData(message);
//		if (result == false || message->has_schemaitem() == false)
//		{
//			assert(result);
//			assert(message->has_schemaitem());
//			return false;
//		}
	
//		// --
//		//
//		Proto::SchemaItemUfb* ufbpb = message->mutable_schemaitem()->mutable_ufb();

//		ufbpb->set_precision(m_precision);
//		ufbpb->set_label(m_label.toStdString());

		return true;
	}

	bool SchemaItemUfb::LoadData(const Proto::Envelope& message)
	{
		return false;
//		if (message.has_schemaitem() == false)
//		{
//			assert(message.has_schemaitem());
//			return false;
//		}

//		// --
//		//
//		bool result = FblItemRect::LoadData(message);
//		if (result == false)
//		{
//			return false;
//		}

//		// --
//		//
//		if (message.schemaitem().has_ufb() == false)
//		{
//			assert(message.schemaitem().has_ufb());
//			return false;
//		}
		
//		const Proto::SchemaItemUfb& ufbpb = message.schemaitem().ufb();
		
//		m_precision = ufbpb.precision();

//		if (ufbpb.has_label() == true)
//		{
//			m_label = QString::fromStdString(ufbpb.label());
//		}

//		// Add afb properties to class meta object
//		//
//		addSpecificParamProperties();

//		return ok;
	}

	QString SchemaItemUfb::buildName() const
	{
		return QString("%1 %2")
				.arg(m_ufbCaption)
				.arg(label());
	}


	bool SchemaItemUfb::updateElement(const UfbSchema* ufbSchema, QString* errorMessage)
	{
		return false;

//		if (errorMessage == nullptr)
//		{
//			assert(errorMessage);
//			return false;
//		}

//		if (m_afbElement.strID() != sourceAfb.strID())
//		{
//			assert(m_afbElement.strID() == sourceAfb.strID());
//			return false;
//		}

//		// Update params, m_afbElement contains old parameters
//		//
//		std::vector<Afb::AfbParam> newParams = sourceAfb.params();
//		const std::vector<Afb::AfbParam>& currentParams = m_afbElement.params();

//		for (Afb::AfbParam& p : newParams)
//		{
//			if (p.user() == false)
//			{
//				continue;
//			}

//			auto foundExistingParam = std::find_if(currentParams.begin(), currentParams.end(),
//					[&p](const Afb::AfbParam& mp)
//					{
//						return p.caption() == mp.caption();		// Don't use opIndex, it can be same (-1)
//					});

//			if (foundExistingParam != currentParams.end())
//			{
//				// Try to set old value to the param
//				//
//				const Afb::AfbParam& currentParam = *foundExistingParam;

//				if (p.value().type() == currentParam.value().type())
//				{
//					p.setValue(currentParam.value());

//					//qDebug() << "Param: " << currentParam.caption() << ", value: " << p.value();
//				}
//			}
//		}

//		// Update description
//		//
//		m_afbElement = sourceAfb;

//		std::swap(params(), newParams);		// The prev assignemnt (m_afbElement = sourceAfb) just reseted all paramas
//											// Set them to the actual values

//		// Update in/out pins
//		//
//		removeAllInputs();
//		removeAllOutputs();

//		const std::vector<Afb::AfbSignal>& inputSignals = m_afbElement.inputSignals();
//		for (const Afb::AfbSignal& s : inputSignals)
//		{
//			addInput(s);
//		}

//		const std::vector<Afb::AfbSignal>& outputSignals = m_afbElement.outputSignals();
//		for (const Afb::AfbSignal& s : outputSignals)
//		{
//			addOutput(s);
//		}

//		// Run afterCreationScript, lets assume we create allmost new itesm, as we deleted all inputs/outs and updated params
//		//
//		QString afterCreationScript = m_afbElement.afterCreationScript();

//		if (afterCreationScript.isEmpty() == false)
//		{
//			executeScript(afterCreationScript, m_afbElement, errorMessage);
//		}

//		// Here is remove all props and add new from m_params
//		//
//		addSpecificParamProperties();

//		return true;
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
		return m_version;
	}

	QString SchemaItemUfb::label() const
	{
		return m_label;
	}

	void SchemaItemUfb::setLabel(QString value)
	{
		m_label = value;
	}

}
