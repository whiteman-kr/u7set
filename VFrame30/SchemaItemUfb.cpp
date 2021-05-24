#include "SchemaItemUfb.h"
#include "UfbSchema.h"
#include "SchemaItemSignal.h"
#include "PropertyNames.h"
#include "DrawParam.h"

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
		ADD_PROPERTY_GETTER(QString, PropertyNames::ufbSchemaId, true, SchemaItemUfb::ufbSchemaId)
			->setCategory(PropertyNames::functionalCategory);

		ADD_PROPERTY_GETTER(QString, PropertyNames::caption, true, SchemaItemUfb::ufbCaption)
			->setCategory(PropertyNames::functionalCategory);

		ADD_PROPERTY_GETTER(int, PropertyNames::ufbSchemaVersion, true, SchemaItemUfb::ufbSchemaVersion)
			->setCategory(PropertyNames::functionalCategory);

		return;
	}

	SchemaItemUfb::SchemaItemUfb(SchemaUnit unit, const UfbSchema* ufbSchema, QString* errorMsg) :
		SchemaItemUfb(unit)
	{
		Q_ASSERT(errorMsg);

		if (ufbSchema == nullptr)
		{
			Q_ASSERT(ufbSchema);
			*errorMsg = tr("Pointer to UfbSchema is nullptr.");
			return;
		}

		// Create input output signals in VFrame30::FblEtem
		//
		updateUfbElement(ufbSchema, errorMsg);

		return;
	}

	SchemaItemUfb::~SchemaItemUfb(void)
	{
	}

	void SchemaItemUfb::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const
	{
		QPainter* p = drawParam->painter();

		int dpiX = drawParam->dpiX();
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		FontParam smallFont = m_font;
		smallFont.setDrawSize(m_font.drawSize() * 0.75);

		// Draw rect and pins
		//
		FblItemRect::draw(drawParam, schema, pLayer);

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
		DrawHelper::drawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		// Draw specific properties
		//
		text.clear();
		r.setTop(topDocPt() + m_font.drawSize() * 1.4);
		r.setHeight(heightDocPt() - m_font.drawSize() * 1.4);

		auto props = PropertyObject::specificProperties();

		std::ranges::sort(props,
			[](const auto& p1, const auto& p2)
			{
				if (p1->category() == p2->category())
				{
					return p1->caption() < p2->caption();
				}
				else
				{
					return p1->category() < p2->category();
				}
			});

		for (const auto& prop : props)
		{
			Q_ASSERT(prop);
			Q_ASSERT(prop->specific() == true);

			if (prop->essential() == false)
			{
				continue;
			}

			QVariant propValue = prop->value();
			QString propValueText;

			switch (static_cast<QMetaType::Type>(propValue.type()))
			{
			case QMetaType::QString:
				{
					propValueText = propValue.toString().replace(QChar::LineFeed, QChar(' '));
				}
				break;
			case QMetaType::Int:
			case QMetaType::UInt:
				{
					propValueText = propValue.toString();
				}
				break;
			case QMetaType::Double:
				{
					char paramFormat = prop->precision() > 5 ? 'g' : 'f';

					propValueText.setNum(propValue.toDouble(), paramFormat, prop->precision());

					if (propValueText.contains(QChar('.')) == true &&
						propValueText.contains(QChar('e')) == false &&
						propValueText.size() > 2)
					{
						while(propValueText.endsWith('0'))
						{
							propValueText.chop(1);
						}

						if (propValueText.endsWith('.'))
						{
							propValueText.chop(1);
						}
					}
				}
				break;
			case QMetaType::Bool:
				{
					propValueText = propValue.toBool() ? tr("true") : tr("false");
				}
				break;
			default:
				Q_ASSERT(false);
				propValueText = tr("unknown type");
			}

			// Param string
			//
			QString paramStr = QString("%1: %2").arg(prop->caption(), propValueText);

			if (text.isEmpty() == true)
			{
				text = paramStr;
			}
			else
			{
				text.append(QString("\n%1").arg(paramStr));
			}
		}

		p->setPen(textColor());
		DrawHelper::drawText(p, smallFont, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom);

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

		ufbpb->set_specific_properties_struct(m_specificPropertiesStruct.toStdString());

		// Save specific properties' values
		//
		std::vector<std::shared_ptr<Property>> props = this->properties();

		for (const auto& p : props)
		{
			if (p->specific() == true)
			{
				::Proto::Property* protoProp = ufbpb->mutable_properties()->Add();
				Proto::saveProperty(protoProp, p);
			}
		}

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

		if (ufbpb.has_specific_properties_struct() == true)
		{
			m_specificPropertiesStruct = QString::fromStdString(ufbpb.specific_properties_struct());
			parseSpecificPropertiesStruct(m_specificPropertiesStruct);
		}
		else
		{
			m_specificPropertiesStruct.clear();
		}

		// Load specific properties' values. They are already exists after calling parseSpecificPropertiesStruct()
		//
		std::vector<std::shared_ptr<Property>> specificProps = PropertyObject::specificProperties();

		for (const ::Proto::Property& p : ufbpb.properties())
		{
			auto it = std::find_if(specificProps.begin(), specificProps.end(),
				[p](std::shared_ptr<Property>& dp)
				{
					return dp->caption().toStdString() == p.name();
				});

			if (it == specificProps.end())
			{
				qDebug() << "ERROR: Can't find property " << p.name().c_str() << " in" << m_ufbSchemaId;
			}
			else
			{
				std::shared_ptr<Property>& property = *it;
				Q_ASSERT(property->specific() == true);		// it's suppose to be specific property;

				bool loadOk = Proto::loadProperty(p, property);

				Q_UNUSED(loadOk);
				Q_ASSERT(loadOk);
			}
		}

		return true;
	}

	QString SchemaItemUfb::buildName() const
	{
		return QString("%1 %2")
				.arg(m_ufbCaption)
				.arg(label());
	}

	bool SchemaItemUfb::updateUfbElement(const UfbSchema* ufbSchema, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (m_ufbSchemaId.isEmpty() == false &&
			m_ufbSchemaId != ufbSchema->schemaId())
		{
			assert(false);
			*errorMessage += tr("Update %1 from different UFB %2.").arg(m_ufbSchemaId).arg(ufbSchema->schemaId());
			return false;
		}

		m_ufbSchemaId = ufbSchema->schemaId();
		m_ufbCaption = ufbSchema->caption();
		m_ufbVersion = ufbSchema->version();
		setSpecificProperties(ufbSchema->specificProperties());		// it creates specific properties from PropertyObject

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
				for (std::shared_ptr<SchemaItem>& item : layer->Items)
				{
					const SchemaItemSignal* itemSignal = item->toType<SchemaItemSignal>();
					if (itemSignal == nullptr)
					{
						continue;
					}

					// Check it this siganl item is just variable reference
					//
					QString appSignalIds = itemSignal->appSignalIds();
					if (appSignalIds.startsWith("$(") == true && appSignalIds.endsWith(")") == true)
					{
						// Do not add references to input or output list
						//
						continue;
					}

					// Add UFB input or output or nothing))
					//
					if (itemSignal->isInputSignalElement() == true)
					{
						ufbInputs.push_back(itemSignal->toInputSignalElement());
						continue;
					}

					if (itemSignal->isOutputSignalElement() == true)
					{
						ufbOutputs.push_back(itemSignal->toOutputSignalElement());
						continue;
					}
				}

				break;
			}
		}

		// Sort in/outs by vert pos
		//
		std::sort(ufbInputs.begin(), ufbInputs.end(),
				[](const SchemaItemSignal* s1, const SchemaItemSignal* s2)
				{
					return s1->topDocPt() < s2->topDocPt();
				});

		std::sort(ufbOutputs.begin(), ufbOutputs.end(),
				[](const SchemaItemSignal* s1, const SchemaItemSignal* s2)
				{
					return s1->topDocPt() < s2->topDocPt();
				});

		// Create in/outs in this item
		//
		removeAllInputs();
		removeAllOutputs();

		for (const SchemaItemSignal* in : ufbInputs)
		{
			this->addInput(-1, E::SignalType::Discrete, in->appSignalIds());
		}

		for (const SchemaItemSignal* out: ufbOutputs)
		{
			this->addOutput(-1, E::SignalType::Discrete, out->appSignalIds());
		}

		// --
		//
		adjustHeight();

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

	QString SchemaItemUfb::specificProperties() const
	{
		return m_specificPropertiesStruct;
	}

	void SchemaItemUfb::setSpecificProperties(QString value)
	{
		if (m_specificPropertiesStruct != value)
		{
			m_specificPropertiesStruct = value;
			parseSpecificPropertiesStruct(m_specificPropertiesStruct);
		}

		return;
	}
}
