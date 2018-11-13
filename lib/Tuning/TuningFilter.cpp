#include "TuningFilter.h"
#include "../lib/Types.h"

namespace TuningTags
{
	static const QString tag_True = "true";
	static const QString tag_False = "false";

	static const QString tag_Tree = "Tree";
	static const QString tag_Tab = "Tab";
	static const QString tag_Button = "Button";
	static const QString tag_Value = "Value";
	static const QString tag_Values = "Values";

	static const QString tag_All = "All";
	static const QString tag_Analog = "Analog";
	static const QString tag_Discrete = "Discrete";

	static const QString tag_Project = "Project";
	static const QString tag_Schema = "Schema";
	static const QString tag_Equipment = "Equipment";
	static const QString tag_User = "User";

	static const char* prop_ValueColumn1AppSignalSuffixes = "Column%1AppSignalSuffixes";

	// Property names

	static const QLatin1String prop_Caption = QLatin1String("Caption");
	static const QLatin1String prop_SignalType = QLatin1String("SignalType");
	static const QLatin1String prop_Source = QLatin1String("Source");
	static const QLatin1String prop_ID = QLatin1String("ID");
	static const QLatin1String prop_CustomID = QLatin1String("CustomID");
	static const QLatin1String prop_InterfaceType = QLatin1String("InterfaceType");
	static const QLatin1String prop_CustomAppSignalMasks = QLatin1String("CustomAppSignalMasks");
	static const QLatin1String prop_AppSignalMasks = QLatin1String("AppSignalMasks");
	static const QLatin1String prop_EquipmentIDMasks = QLatin1String("EquipmentIDMasks");

	static const QLatin1String prop_BackColor = QLatin1String("BackColor");
	static const QLatin1String prop_TextColor = QLatin1String("TextColor");
	static const QLatin1String prop_BackSelectedColor = QLatin1String("BackSelectedColor");
	static const QLatin1String prop_TextSelectedColor = QLatin1String("TextSelectedColor");
	static const QLatin1String prop_HasDiscreteCounter = QLatin1String("HasDiscreteCounter");
	static const QLatin1String prop_ValueColumnsCount = QLatin1String("ValueColumnsCount");

	static const QLatin1String prop_ColumnCustomAppId = QLatin1String("ColumnCustomAppId");
	static const QLatin1String prop_ColumnAppId = QLatin1String("ColumnAppId");
	static const QLatin1String prop_ColumnEquipmentId = QLatin1String("ColumnEquipmentId");
	static const QLatin1String prop_ColumnCaption = QLatin1String("ColumnCaption");
	static const QLatin1String prop_ColumnUnits = QLatin1String("ColumnUnits");
	static const QLatin1String prop_ColumnType = QLatin1String("ColumnType");
	static const QLatin1String prop_ColumnLimits = QLatin1String("ColumnLimits");
	static const QLatin1String prop_ColumnDefault = QLatin1String("ColumnDefault");
	static const QLatin1String prop_ColumnValid = QLatin1String("ColumnValid");
	static const QLatin1String prop_ColumnOutOfRange = QLatin1String("ColumnOutOfRange");

	static const QLatin1String category_Columns = QLatin1String("Columns");
	static const QLatin1String category_ValueColumns = QLatin1String("ValueColumns");
}

TuningFilterValue::TuningFilterValue()
{

}

QString TuningFilterValue::appSignalId() const
{
	return m_appSignalId;
}

void TuningFilterValue::setAppSignalId(const QString& value)
{
	m_appSignalId = value;
	m_appSignalHash = ::calcHash(m_appSignalId);
}

bool TuningFilterValue::useValue() const
{
	return m_useValue;
}

void TuningFilterValue::setUseValue(bool value)
{
	m_useValue = value;
}

TuningValue TuningFilterValue::value() const
{
	return m_value;
}

void TuningFilterValue::setValue(TuningValue value)
{
	m_value = value;
}

Hash TuningFilterValue::appSignalHash() const
{
	return m_appSignalHash;
}

bool TuningFilterValue::load(QXmlStreamReader& reader)
{
	if (reader.attributes().hasAttribute("AppSignalId"))
	{
		setAppSignalId(reader.attributes().value("AppSignalId").toString());
	}

	if (reader.attributes().hasAttribute("UseValue"))
	{
		setUseValue(reader.attributes().value("UseValue").toString() == TuningTags::tag_True);
	}

	if (reader.attributes().hasAttribute("ValueType"))
	{
		TuningValue tv;

		tv.setType(static_cast<TuningValueType>(reader.attributes().value("ValueType").toInt()));

		switch (tv.type())
		{
		case TuningValueType::Discrete:
			tv.setDiscreteValue(reader.attributes().value("ValueDiscrete").toInt());
			break;

		case TuningValueType::SignedInt32:
			tv.setInt32Value(reader.attributes().value("ValueInt32").toInt());
			break;

		case TuningValueType::Float:
			tv.setFloatValue(reader.attributes().value("ValueFloat").toFloat());
			break;

		case TuningValueType::Double:
			tv.setDoubleValue(reader.attributes().value("ValueDouble").toDouble());
			break;

		default:
			assert(false);
			return false;
		}

		setValue(tv);
	}

	return true;
}

bool TuningFilterValue::save(QXmlStreamWriter& writer) const
{
	writer.writeStartElement("Value");
	writer.writeAttribute("AppSignalId", m_appSignalId);
	writer.writeAttribute("UseValue", m_useValue ? TuningTags::tag_True : TuningTags::tag_False);

	writer.writeAttribute("ValueType", QString::number(static_cast<int>(m_value.type())));

	switch (m_value.type())
	{
	case TuningValueType::Discrete:
		writer.writeAttribute("ValueDiscrete", QString::number(m_value.discreteValue()));
		break;

	case TuningValueType::SignedInt32:
		writer.writeAttribute("ValueInt32", QString::number(m_value.int32Value()));
		break;

	case TuningValueType::Float:
		writer.writeAttribute("ValueFloat", QString::number(m_value.floatValue()));
		break;

	case TuningValueType::Double:
		writer.writeAttribute("ValueDouble", QString::number(m_value.doubleValue()));
		break;

	default:
		assert(false);
		return false;
	}

	writer.writeEndElement();

	return true;
}



//
// TuningFilter
//

TuningFilter::TuningFilter()
{
	ADD_PROPERTY_GETTER_SETTER(QString, TuningTags::prop_Caption, true, TuningFilter::caption, TuningFilter::setCaption);
	ADD_PROPERTY_GETTER_SETTER(SignalType, TuningTags::prop_SignalType, true, TuningFilter::signalType, TuningFilter::setSignalType);

	ADD_PROPERTY_GETTER(QString, TuningTags::prop_ID, true, TuningFilter::ID);

	ADD_PROPERTY_GETTER_SETTER(QString, TuningTags::prop_CustomID, true, TuningFilter::customID, TuningFilter::setCustomID);

	ADD_PROPERTY_GETTER(InterfaceType, TuningTags::prop_InterfaceType, true, TuningFilter::interfaceType);

	auto propMask = ADD_PROPERTY_GETTER_SETTER(QString, TuningTags::prop_CustomAppSignalMasks, true, TuningFilter::customAppSignalIDMask, TuningFilter::setCustomAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, TuningTags::prop_AppSignalMasks, true, TuningFilter::appSignalIDMask, TuningFilter::setAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, TuningTags::prop_EquipmentIDMasks, true, TuningFilter::equipmentIDMask, TuningFilter::setEquipmentIDMask);
	propMask->setCategory("Masks");

	auto propBackColor = ADD_PROPERTY_GETTER_SETTER(QColor, TuningTags::prop_BackColor, true, TuningFilter::backColor, TuningFilter::setBackColor);
	propBackColor->setCategory("Appearance");

	auto propTextColor = ADD_PROPERTY_GETTER_SETTER(QColor, TuningTags::prop_TextColor, true, TuningFilter::textColor, TuningFilter::setTextColor);
	propTextColor->setCategory("Appearance");

	auto propBackSelectedColor = ADD_PROPERTY_GETTER_SETTER(QColor, TuningTags::prop_BackSelectedColor, true, TuningFilter::backSelectedColor, TuningFilter::setBackSelectedColor);
	propBackSelectedColor->setCategory("Appearance");

	auto propTextSelectedColor = ADD_PROPERTY_GETTER_SETTER(QColor, TuningTags::prop_TextSelectedColor, true, TuningFilter::textSelectedColor, TuningFilter::setTextSelectedColor);
	propTextSelectedColor->setCategory("Appearance");

	auto propHasCounter = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_HasDiscreteCounter, true, TuningFilter::hasDiscreteCounter, TuningFilter::setHasDiscreteCounter);
	propHasCounter->setCategory("Functions");

	auto propTabValuesCount = ADD_PROPERTY_GETTER_SETTER(int, TuningTags::prop_ValueColumnsCount, true, TuningFilter::valuesColumnCount, TuningFilter::setValuesColumnCount);
	propTabValuesCount->setCategory(TuningTags::category_ValueColumns);

	// Columns

	int order = 100;

	auto propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnCustomAppId, true, TuningFilter::columnCustomAppId, TuningFilter::setColumnCustomAppId);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnAppId, true, TuningFilter::columnAppId, TuningFilter::setColumnAppId);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnEquipmentId, true, TuningFilter::columnEquipmentId, TuningFilter::setColumnEquipmentId);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnCaption, true, TuningFilter::columnCaption, TuningFilter::setColumnCaption);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnUnits, true, TuningFilter::columnUnits, TuningFilter::setColumnUnits);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnType, true, TuningFilter::columnType, TuningFilter::setColumnType);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnLimits, true, TuningFilter::columnLimits, TuningFilter::setColumnLimits);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnDefault, true, TuningFilter::columnDefault, TuningFilter::setColumnDefault);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnValid, true, TuningFilter::columnValid, TuningFilter::setColumnValid);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

	propColumn = ADD_PROPERTY_GETTER_SETTER(bool, TuningTags::prop_ColumnOutOfRange, true, TuningFilter::columnOutOfRange, TuningFilter::setColumnOutOfRange);
	propColumn->setCategory(TuningTags::category_Columns);
	propColumn->setViewOrder(order++);

}

TuningFilter::TuningFilter(const TuningFilter& That)
	:TuningFilter()
{
	copy(That);

	updateOptionalProperties();
}

TuningFilter::TuningFilter(InterfaceType interfaceType)
	:TuningFilter()
{
	m_interfaceType = interfaceType;

	updateOptionalProperties();
}

TuningFilter::~TuningFilter()
{

}

TuningFilter& TuningFilter::operator=(const TuningFilter& That)
{
	copy(That);

	return* this;
}

bool TuningFilter::load(QXmlStreamReader& reader)
{
	if (isRoot() == false)
	{
		// For compatibility

		if (reader.attributes().hasAttribute("StrID"))  // This is for reading obsolete files!!!
		{
			setID(reader.attributes().value("StrID").toString());
		}
		if (reader.attributes().hasAttribute("CustomAppSignalIDMask"))
		{
			setCustomAppSignalIDMask(reader.attributes().value("CustomAppSignalIDMask").toString());
		}
		if (reader.attributes().hasAttribute("EquipmentIDMask"))
		{
			setEquipmentIDMask(reader.attributes().value("EquipmentIDMask").toString());
		}
		if (reader.attributes().hasAttribute("AppSignalIDMask"))
		{
			setAppSignalIDMask(reader.attributes().value("AppSignalIDMask").toString());
		}

		// END For compatibility

		if (reader.attributes().hasAttribute(TuningTags::prop_ID))
		{
			setID(reader.attributes().value(TuningTags::prop_ID).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_CustomID))
		{
			setCustomID(reader.attributes().value(TuningTags::prop_CustomID).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_Caption))
		{
			setCaption(reader.attributes().value(TuningTags::prop_Caption).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_BackColor))
		{
			setBackColor(QColor(reader.attributes().value(TuningTags::prop_BackColor).toString()));
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_TextColor))
		{
			setTextColor(QColor(reader.attributes().value(TuningTags::prop_TextColor).toString()));
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_BackSelectedColor))
		{
			setBackSelectedColor(QColor(reader.attributes().value(TuningTags::prop_BackSelectedColor).toString()));
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_TextSelectedColor))
		{
			setTextSelectedColor(QColor(reader.attributes().value(TuningTags::prop_TextSelectedColor).toString()));
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_HasDiscreteCounter))
		{
			setHasDiscreteCounter(reader.attributes().value(TuningTags::prop_HasDiscreteCounter).toString() == TuningTags::tag_True);
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_CustomAppSignalMasks))
		{
			setCustomAppSignalIDMask(reader.attributes().value(TuningTags::prop_CustomAppSignalMasks).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_AppSignalMasks))
		{
			setAppSignalIDMask(reader.attributes().value(TuningTags::prop_AppSignalMasks).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_EquipmentIDMasks))
		{
			setEquipmentIDMask(reader.attributes().value(TuningTags::prop_EquipmentIDMasks).toString());
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_SignalType))
		{
			QString v = reader.attributes().value(TuningTags::prop_SignalType).toString();
			if (v == TuningTags::tag_All)
			{
				setSignalType(SignalType::All);
			}
			else
			{
				if (v == TuningTags::tag_Analog)
				{
					setSignalType(SignalType::Analog);
				}
				else
				{
					if (v == TuningTags::tag_Discrete)
					{
						setSignalType(SignalType::Discrete);
					}
					else
					{
						reader.raiseError(tr("Unknown SignalType value: %1").arg(v));
						return false;
					}
				}
			}
		}

		if (reader.attributes().hasAttribute(TuningTags::prop_Source))
		{
			QString v = reader.attributes().value(TuningTags::prop_Source).toString();
			if (v == TuningTags::tag_Project)
			{
				setSource(Source::Project);
			}
			else
			{
				if (v == TuningTags::tag_Schema)
				{
					setSource(Source::Schema);
				}
				else
				{
					if (v == TuningTags::tag_Equipment)
					{
						setSource(Source::Equipment);
					}
					else
					{
						if (v == TuningTags::tag_User)
						{
							setSource(Source::User);
						}
						else
						{
							reader.raiseError(tr("Unknown Source value: %1").arg(v));
							return false;
						}
					}
				}
			}
		}

		// ValueColumns

		if (reader.attributes().hasAttribute(TuningTags::prop_ValueColumnsCount))
		{
			m_valueColumnsCount = reader.attributes().value(TuningTags::prop_ValueColumnsCount).toInt();

			if (m_valueColumnsCount < 0)
			{
				m_valueColumnsCount = 0;
			}
			if (m_valueColumnsCount > MAX_VALUES_COLUMN_COUNT)
			{
				m_valueColumnsCount = MAX_VALUES_COLUMN_COUNT;
			}

			m_valueColumnsAppSignalIdSuffixes.resize(m_valueColumnsCount);

			for (int i = 0; i < m_valueColumnsCount; i++)
			{
				QString propName = tr(TuningTags::prop_ValueColumn1AppSignalSuffixes).arg(i);

				if (reader.attributes().hasAttribute(propName) == true)
				{
					QString masks = reader.attributes().value(propName).toString();

					m_valueColumnsAppSignalIdSuffixes[i] = masks;
				}
			}
		}
		else
		{
			m_valueColumnsCount = 0;
			m_valueColumnsAppSignalIdSuffixes.clear();
		}

		// Columns

		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnCustomAppId))
		{
			setColumnCustomAppId(reader.attributes().value(TuningTags::prop_ColumnCustomAppId).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnAppId))
		{
			setColumnAppId(reader.attributes().value(TuningTags::prop_ColumnAppId).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnEquipmentId))
		{
			setColumnEquipmentId(reader.attributes().value(TuningTags::prop_ColumnEquipmentId).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnCaption))
		{
			setColumnCaption(reader.attributes().value(TuningTags::prop_ColumnCaption).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnUnits))
		{
			setColumnUnits(reader.attributes().value(TuningTags::prop_ColumnUnits).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnType))
		{
			setColumnType(reader.attributes().value(TuningTags::prop_ColumnType).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnLimits))
		{
			setColumnLimits(reader.attributes().value(TuningTags::prop_ColumnLimits).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnDefault))
		{
			setColumnDefault(reader.attributes().value(TuningTags::prop_ColumnDefault).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnValid))
		{
			setColumnValid(reader.attributes().value(TuningTags::prop_ColumnValid).toString() == TuningTags::tag_True);
		}
		if (reader.attributes().hasAttribute(TuningTags::prop_ColumnOutOfRange))
		{
			setColumnOutOfRange(reader.attributes().value(TuningTags::prop_ColumnOutOfRange).toString() == TuningTags::tag_True);
		}

	}

	int recurseLevel = 0;		//recurseLevel 1 = "Values", recurseLevel 2 = "Value"

	QXmlStreamReader::TokenType t;
	do
	{
		t = reader.readNext();

		if (t == QXmlStreamReader::EndElement && recurseLevel > 0)
		{
			// This is end element of "Value" or "Values", read next element
			//
			recurseLevel--;
			t = reader.readNext();
		}

		if (t == QXmlStreamReader::StartElement)
		{
			QString tagName = reader.name().toString();


			if (tagName == TuningTags::tag_Values)
			{
				recurseLevel++;

				continue;
			}

			if (tagName == TuningTags::tag_Value)
			{
				recurseLevel++;

				TuningFilterValue ofv;

				ofv.load(reader);

				addValue(ofv);

				continue;
			}

			if (tagName == TuningTags::tag_Tree || tagName == TuningTags::tag_Tab || tagName == TuningTags::tag_Button)
			{
				TuningFilter::InterfaceType filterType = TuningFilter::InterfaceType::Tree;

				if (tagName == TuningTags::tag_Tab)
				{
					filterType = TuningFilter::InterfaceType::Tab;
				}

				if (tagName == TuningTags::tag_Button)
				{
					filterType = TuningFilter::InterfaceType::Button;
				}

				std::shared_ptr<TuningFilter> of = std::make_shared<TuningFilter>(filterType);

				if (of->load(reader) == false)
				{
					return false;
				}

				addChild(of);

				continue;
			}

			reader.raiseError(tr("Unknown tag: ") + reader.name().toString());
			return false;
		}
	}while (t != QXmlStreamReader::EndElement);

	updateOptionalProperties();

	return true;
}

bool TuningFilter::save(QXmlStreamWriter& writer, bool filterBySourceType, Source saveSourceType) const
{
	if (isRoot() == true)
	{
		writer.writeStartElement("Root");
	}
	else
	{
		if (filterBySourceType == true)
		{
			if (saveSourceType != source())
			{
				return true;
			}
		}

		if (isTree() == true)
		{
			writer.writeStartElement(TuningTags::tag_Tree);
		}
		else
		{
			if (isTab())
			{
				writer.writeStartElement(TuningTags::tag_Tab);
			}
			else
			{
				if (isButton())
				{
					writer.writeStartElement(TuningTags::tag_Button);
				}
				else
				{
					assert(false);
					return false;
				}
			}
		}
	}

	writer.writeAttribute(TuningTags::prop_ID, ID());
	writer.writeAttribute(TuningTags::prop_CustomID, customID());
	writer.writeAttribute(TuningTags::prop_Caption, caption());

	writer.writeAttribute(TuningTags::prop_BackColor, backColor().name());
	writer.writeAttribute(TuningTags::prop_TextColor, textColor().name());

	writer.writeAttribute(TuningTags::prop_BackSelectedColor, backSelectedColor().name());
	writer.writeAttribute(TuningTags::prop_TextSelectedColor, textSelectedColor().name());

	writer.writeAttribute(TuningTags::prop_HasDiscreteCounter, hasDiscreteCounter() ? TuningTags::tag_True : TuningTags::tag_False);

	writer.writeAttribute(TuningTags::prop_CustomAppSignalMasks, customAppSignalIDMask());
	writer.writeAttribute(TuningTags::prop_AppSignalMasks, appSignalIDMask());
	writer.writeAttribute(TuningTags::prop_EquipmentIDMasks, equipmentIDMask());

	writer.writeAttribute(TuningTags::prop_SignalType, E::valueToString<SignalType>(static_cast<int>(signalType())));
	writer.writeAttribute(TuningTags::prop_Source, E::valueToString<Source>(static_cast<int>(source())));

	// ValueColumns

	if (static_cast<int>(m_valueColumnsAppSignalIdSuffixes.size()) != valuesColumnCount())
	{
		assert(false);
		return false;
	}

	writer.writeAttribute(TuningTags::prop_ValueColumnsCount, QString::number(valuesColumnCount()));

	for (int i = 0; i < valuesColumnCount(); i++)
	{
		QString propName = tr(TuningTags::prop_ValueColumn1AppSignalSuffixes).arg(i);
		writer.writeAttribute(propName, m_valueColumnsAppSignalIdSuffixes[i]);
	}

	// Columns

	writer.writeAttribute(TuningTags::prop_ColumnCustomAppId, columnCustomAppId() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnAppId, columnAppId() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnEquipmentId, columnEquipmentId() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnCaption, columnCaption() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnUnits, columnUnits() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnType, columnType() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnLimits, columnLimits() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnDefault, columnDefault() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnValid, columnValid() ? TuningTags::tag_True : TuningTags::tag_False);
	writer.writeAttribute(TuningTags::prop_ColumnOutOfRange, columnOutOfRange() ? TuningTags::tag_True : TuningTags::tag_False);

	writer.writeStartElement(TuningTags::tag_Values);

	std::vector <TuningFilterValue> valuesList = getValues();
	for (const TuningFilterValue& v : valuesList)
	{
		v.save(writer);
	}
	writer.writeEndElement();

	for (auto f : m_childFilters)
	{
		f->save(writer, filterBySourceType, saveSourceType);
	}

	writer.writeEndElement();

	return true;
}

bool TuningFilter::match(const AppSignalParam& object) const
{
	if (isEmpty() == true)
	{
		return true;
	}

	if (signalType() == TuningFilter::SignalType::Analog && object.isAnalog() == false)
	{
		return false;
	}
	if (signalType() == TuningFilter::SignalType::Discrete && object.isAnalog() == true)
	{
		return false;
	}

	// List of appSignalId
	//
	if (m_signalValuesVec.empty() == false)
	{
		if (valueExists(object.hash()) == false)
		{
			return false;
		}
	}

	if (processMaskList(object.equipmentId(), m_equipmentIDMasks) == false)
	{
		return false;
	}

	if (processMaskList(object.appSignalId(), m_appSignalIDMasks) == false)
	{
		return false;
	}

	if (processMaskList(object.customSignalId(), m_customAppSignalIDMasks) == false)
	{
		return false;
	}

	TuningFilter* parent = parentFilter();

	if (parent != nullptr)
	{
		return parent->match(object);
	}
	else
	{
		return true;
	}
}

void TuningFilter::checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters)
{
	for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
	{
		const Hash& hash = it->first;
		const TuningFilterValue& value = it->second;

		if (find(signalHashes.begin(), signalHashes.end(), hash) == signalHashes.end())
		{
			notFoundSignalsAndFilters.push_back(std::make_pair<QString, QString>(value.appSignalId(), caption()));
		}
	}

	int childCount = static_cast<int>(m_childFilters.size());
	for (int i = 0; i < childCount; i++)
	{
		m_childFilters[i]->checkSignals(signalHashes, notFoundSignalsAndFilters);
	}
}

void TuningFilter::removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter)
{
	std::vector<Hash> valuesToDelete;

	for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
	{
		const Hash& hash = it->first;

		if (find(signalHashes.begin(), signalHashes.end(), hash) == signalHashes.end())
		{
			removedCounter++;
			valuesToDelete.push_back(hash);
		}
	}

	for (Hash hash : valuesToDelete)
	{
		auto itm = m_signalValuesMap.find(hash);
		if (itm != m_signalValuesMap.end())
		{
			m_signalValuesMap.erase(itm);
		}
		else
		{
			assert(false);
		}

		auto itv = std::find (m_signalValuesVec.begin(), m_signalValuesVec.end(), hash);
		if (itv != m_signalValuesVec.end())
		{
			m_signalValuesVec.erase(itv);
		}
		else
		{
			assert(false);
		}

	}

	int childCount = static_cast<int>(m_childFilters.size());
	for (int i = 0; i < childCount; i++)
	{
		m_childFilters[i]->removeNotExistingSignals(signalHashes, removedCounter);
	}
}

const std::vector<Hash>& TuningFilter::equipmentHashes() const
{
	return m_equipmentHashes;
}

void TuningFilter::setEquipmentHashes(std::vector<Hash> value)
{
	m_equipmentHashes = value;
}

const std::vector<Hash>& TuningFilter::signalsHashes() const
{
	return m_signalsHashes;
}

void TuningFilter::setSignalsHashes(std::vector<Hash> value)
{
	m_signalsHashes = value;
}

QString TuningFilter::ID() const
{
	return m_ID;
}

void TuningFilter::setID(const QString& value)
{
	m_ID = value;
}

QString TuningFilter::customID() const
{
	return m_customID;
}

void TuningFilter::setCustomID(const QString& value)
{
	m_customID = value;
}

QString TuningFilter::caption() const
{
	return m_caption;
}

void TuningFilter::setCaption(const QString& value)
{
	m_caption = value;
}

bool TuningFilter::isSourceProject() const
{
	return m_source == Source::Project;
}

bool TuningFilter::isSourceEquipment() const
{
	return m_source == Source::Equipment;
}

bool TuningFilter::isSourceSchema() const
{
	return m_source == Source::Schema;
}

bool TuningFilter::isSourceUser() const
{
	return m_source == Source::User;
}

TuningFilter::Source TuningFilter::source() const
{
	return m_source;
}

void TuningFilter::setSource(Source value)
{
	m_source = value;
}

TuningFilter::InterfaceType TuningFilter::interfaceType() const
{
	return m_interfaceType;
}

void TuningFilter::setInterfaceType(InterfaceType value)
{
	m_interfaceType = value;
}

TuningFilter::SignalType TuningFilter::signalType() const
{
	return m_signalType;
}

void TuningFilter::setSignalType(SignalType value)
{
	m_signalType = value;
}

QColor TuningFilter::backColor() const
{
	return m_backColor;
}

void TuningFilter::setBackColor(const QColor& value)
{
	m_backColor = value;
}

QColor TuningFilter::textColor() const
{
	return m_textColor;
}

void TuningFilter::setTextColor(const QColor& value)
{
	m_textColor = value;
}

QColor TuningFilter::backSelectedColor() const
{
	return m_backSelectedColor;
}

void TuningFilter::setBackSelectedColor(const QColor& value)
{
	m_backSelectedColor = value;
}

QColor TuningFilter::textSelectedColor() const
{
	return m_textSelectedColor;
}

void TuningFilter::setTextSelectedColor(const QColor& value)
{
	m_textSelectedColor = value;
}

bool TuningFilter::hasDiscreteCounter() const
{
	return m_hasDiscreteCounter;
}

void TuningFilter::setHasDiscreteCounter(bool value)
{
	m_hasDiscreteCounter = value;
}

QString TuningFilter::customAppSignalIDMask() const
{
	QString result;
	for (auto s : m_customAppSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setCustomAppSignalIDMask(const QString& value)
{
	if (value.isEmpty() == true)
	{
		m_customAppSignalIDMasks.clear();
	}
	else
	{
		m_customAppSignalIDMasks = value.split(';');
	}

}

QString TuningFilter::equipmentIDMask() const
{
	QString result;
	for (auto s : m_equipmentIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setEquipmentIDMask(const QString& value)
{
	if (value.isEmpty() == true)
	{
		m_equipmentIDMasks.clear();
	}
	else
	{
		m_equipmentIDMasks = value.split(';');
	}
}

QString TuningFilter::appSignalIDMask() const
{
	QString result;
	for (auto s : m_appSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setAppSignalIDMask(const QString& value)
{
	if (value.isEmpty() == true)
	{
		m_appSignalIDMasks.clear();
	}
	else
	{
		m_appSignalIDMasks = value.split(';');
	}
}


std::vector <TuningFilterValue> TuningFilter::getValues() const
{
	std::vector <TuningFilterValue> result;

	for (Hash hash : m_signalValuesVec)
	{
		result.push_back(m_signalValuesMap.at(hash));
	}

	return result;
}

void TuningFilter::setValues(const std::vector <TuningFilterValue>& values)
{
	m_signalValuesVec.clear();
	m_signalValuesMap.clear();

	for (const TuningFilterValue&  v : values)
	{
		addValue(v);
	}
}

int TuningFilter::valuesCount() const
{
	return static_cast<int>(m_signalValuesVec.size());
}

bool TuningFilter::valueExists(Hash hash) const
{
	return m_signalValuesMap.find(hash) != m_signalValuesMap.end();
}

void TuningFilter::addValue(const TuningFilterValue& value)
{
	Hash hash = value.appSignalHash();

	if (valueExists(hash) == false)
	{
		m_signalValuesVec.push_back(hash);
	}

	m_signalValuesMap[hash] = value;
}

void TuningFilter::removeValue(Hash hash)
{
	// remove from map
	//
	auto it = m_signalValuesMap.find(hash);

	if (it == m_signalValuesMap.end())
	{
		assert(false);
		return;
	}

	m_signalValuesMap.erase(it);

	// remove from vector
	//
	bool found = false;
	for (auto itv = m_signalValuesVec.begin(); itv != m_signalValuesVec.end(); itv++)
	{
		if (*itv == hash)
		{
			m_signalValuesVec.erase(itv);
			found = true;
			break;
		}
	}

	if (found == false)
	{
		assert(false);
		return;
	}
}

bool TuningFilter::value(Hash hash, TuningFilterValue& value)
{
	auto it = m_signalValuesMap.find(hash);
	if (it == m_signalValuesMap.end())
	{
		return false;
	}

	value = it->second;

	if (value.useValue() == false)
	{
		return false;
	}

	return true;
}

void TuningFilter::setValue(const TuningFilterValue& value)
{
	auto it = m_signalValuesMap.find(value.appSignalHash());

	if (it == m_signalValuesMap.end())
	{
		assert(false);
		return;
	}

	TuningFilterValue& ofv = it->second;
	ofv.setUseValue(value.useValue());
	ofv.setValue(value.value());
}

TuningCounters TuningFilter::counters() const
{
	return m_counters;
}

void TuningFilter::setCounters(TuningCounters value)
{
	m_counters = value;
}

int TuningFilter::valuesColumnCount() const
{
	return m_valueColumnsCount;
}

void TuningFilter::setValuesColumnCount(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	if (value > MAX_VALUES_COLUMN_COUNT)
	{
		value = MAX_VALUES_COLUMN_COUNT;
	}

	m_valueColumnsCount = value;

	m_valueColumnsAppSignalIdSuffixes.resize(m_valueColumnsCount);
}

std::vector<QString> TuningFilter::valueColumnsAppSignalIdSuffixes() const
{
	return m_valueColumnsAppSignalIdSuffixes;
}


bool TuningFilter::columnCustomAppId() const
{
	return m_columnCustomAppId;
}

void TuningFilter::setColumnCustomAppId(bool value)
{
	m_columnCustomAppId = value;
}

bool TuningFilter::columnAppId() const
{
	return m_columnAppId;
}

void TuningFilter::setColumnAppId(bool value)
{
	m_columnAppId = value;
}

bool TuningFilter::columnEquipmentId() const
{
	return m_columnEquipmentId;
}

void TuningFilter::setColumnEquipmentId(bool value)
{
	m_columnEquipmentId= value;
}

bool TuningFilter::columnCaption() const
{
	return m_columnCaption;
}

void TuningFilter::setColumnCaption(bool value)
{
	m_columnCaption= value;
}

bool TuningFilter::columnUnits() const
{
	return m_columnUnits;
}

void TuningFilter::setColumnUnits(bool value)
{
	m_columnUnits = value;
}

bool TuningFilter::columnType() const
{
	return m_columnType;
}

void TuningFilter::setColumnType(bool value)
{
	m_columnType = value;
}

bool TuningFilter::columnLimits() const
{
	return m_columnLimits;
}

void TuningFilter::setColumnLimits(bool value)
{
	m_columnLimits = value;
}

bool TuningFilter::columnDefault() const
{
	return m_columnDefault;
}

void TuningFilter::setColumnDefault(bool value)
{
	m_columnDefault = value;
}

bool TuningFilter::columnValid() const
{
	return m_columnValid;
}

void TuningFilter::setColumnValid(bool value)
{
	m_columnValid = value;
}

bool TuningFilter::columnOutOfRange() const
{
	return m_columnOutOfRange;
}

void TuningFilter::setColumnOutOfRange(bool value)
{
	m_columnOutOfRange = value;
}

TuningFilter* TuningFilter::parentFilter() const
{
	return m_parentFilter;
}


bool TuningFilter::isEmpty() const
{
	if (m_signalType == SignalType::All &&
			m_signalValuesVec.empty() == true &&
			m_appSignalIDMasks.empty() == true &&
			m_customAppSignalIDMasks.empty() == true &&
			m_equipmentIDMasks.empty() == true
			)
	{
		return true;
	}

	return false;
}

bool TuningFilter::isRoot() const
{
	return interfaceType() == InterfaceType::Root;
}

bool TuningFilter::isTree() const
{
	return interfaceType() == InterfaceType::Tree;
}

bool TuningFilter::isTab() const
{
	return interfaceType() == InterfaceType::Tab;
}

bool TuningFilter::isButton() const
{
	return interfaceType() == InterfaceType::Button;
}

void TuningFilter::addTopChild(const std::shared_ptr<TuningFilter>& child)
{
	child->m_parentFilter = this;
	m_childFilters.insert(m_childFilters.begin(), child);
}

void TuningFilter::addChild(const std::shared_ptr<TuningFilter>& child)
{
	child->m_parentFilter = this;
	m_childFilters.push_back(child);
}

void TuningFilter::removeChild(const std::shared_ptr<TuningFilter>& child)
{
	bool found = false;

	for (auto it = m_childFilters.begin(); it != m_childFilters.end(); it++)
	{
		if (it->get() == child.get())
		{
			m_childFilters.erase(it);
			found = true;
			break;
		}
	}

	if (found == false)
	{
		assert(false);
		return;
	}
}

bool TuningFilter::removeChild(const QString& ID)
{
	bool found = false;

	for (auto it = m_childFilters.begin(); it != m_childFilters.end(); it++)
	{
		if (it->get()->ID() == ID)
		{
			m_childFilters.erase(it);
			found = true;
			break;
		}
	}

	return found;
}

void TuningFilter::removeAllChildren()
{
	m_childFilters.clear();

}

void TuningFilter::removeChildren(Source sourceType)
{
	std::vector<std::shared_ptr<TuningFilter>> childFiltersCopy;

	childFiltersCopy = m_childFilters;

	m_childFilters.clear();

	for (auto it = childFiltersCopy.begin(); it != childFiltersCopy.end(); it++)
	{
		std::shared_ptr<TuningFilter>& filter =* it;

		if (filter->source() != sourceType)
		{
			m_childFilters.push_back(filter);
		}
	}
}

int TuningFilter::childFiltersCount() const
{
	return static_cast<int>(m_childFilters.size());

}

std::shared_ptr<TuningFilter> TuningFilter::childFilter(int index) const
{
	if (index <0 || index >= m_childFilters.size())
	{
		assert(false);
		return nullptr;
	}

	return m_childFilters[index];
}

std::shared_ptr<TuningFilter> TuningFilter::childFilter(const QString& caption) const
{
	for (std::shared_ptr<TuningFilter> f : m_childFilters)
	{
		if (f->caption() == caption)
		{
			return f;
		}
	}

	return nullptr;
}

std::shared_ptr<TuningFilter> TuningFilter::findFilterById(const QString& id) const
{
	for (std::shared_ptr<TuningFilter> f : m_childFilters)
	{
		if (f->ID() == id)
		{
			return f;
		}

		std::shared_ptr<TuningFilter> result = f->findFilterById(id);
		if (result != nullptr)
		{
			return result;
		}
	}

	return nullptr;
}

void TuningFilter::updateOptionalProperties()
{
	setPropertyVisible(TuningTags::prop_BackColor, interfaceType() == InterfaceType::Tab || interfaceType() == InterfaceType::Button);

	setPropertyVisible(TuningTags::prop_TextColor, interfaceType() == InterfaceType::Button);
	setPropertyVisible(TuningTags::prop_BackSelectedColor, interfaceType() == InterfaceType::Button);
	setPropertyVisible(TuningTags::prop_TextSelectedColor, interfaceType() == InterfaceType::Button);

	// Value columns, add or remove unnecessary properties

	setPropertyVisible(TuningTags::prop_ValueColumnsCount, interfaceType() == InterfaceType::Tab);

	setPropertyVisible(TuningTags::prop_ColumnCustomAppId, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnAppId, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnEquipmentId, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnCaption, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnUnits, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnType, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnLimits, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnDefault, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnValid, interfaceType() == InterfaceType::Tab);
	setPropertyVisible(TuningTags::prop_ColumnOutOfRange, interfaceType() == InterfaceType::Tab);

	if (interfaceType() == InterfaceType::Tab)
	{
		if (static_cast<int>(m_valueColumnsAppSignalIdSuffixes.size()) != valuesColumnCount())
		{
			m_valueColumnsAppSignalIdSuffixes.resize(valuesColumnCount());
		}

		for (int i = 0; i < MAX_VALUES_COLUMN_COUNT; i++)
		{
			QString propName = tr(TuningTags::prop_ValueColumn1AppSignalSuffixes).arg(i);

			if (i < valuesColumnCount())
			{
				if (propertyExists(propName) == false)
				{
					addProperty(propName, TuningTags::category_ValueColumns, true, m_valueColumnsAppSignalIdSuffixes[i]);
				}
				else
				{
					m_valueColumnsAppSignalIdSuffixes[i] = propertyValue(propName).toString();
				}
			}
			else
			{
				if (propertyExists(propName) == true)
				{
					removeProperty(propName);
				}
			}
		}
	}

	//
}

void TuningFilter::copy(const TuningFilter& That)
{
	m_ID = That.m_ID;
	m_customID = That.m_customID;
	m_caption = That.m_caption;

	m_source = That.m_source;

	m_customAppSignalIDMasks = That.m_customAppSignalIDMasks;
	m_equipmentIDMasks = That.m_equipmentIDMasks;
	m_appSignalIDMasks = That.m_appSignalIDMasks;

	m_signalValuesMap = That.m_signalValuesMap;
	m_signalValuesVec = That.m_signalValuesVec;

	m_interfaceType = That.m_interfaceType;
	m_signalType = That.m_signalType;

	m_hasDiscreteCounter = That.m_hasDiscreteCounter;

	m_backColor = That.m_backColor;
	m_textColor = That.m_textColor;

	m_backSelectedColor = That.m_backSelectedColor;
	m_textSelectedColor = That.m_textSelectedColor;

	m_valueColumnsCount = That.m_valueColumnsCount;
	m_valueColumnsAppSignalIdSuffixes = That.m_valueColumnsAppSignalIdSuffixes;

	m_equipmentHashes = That.m_equipmentHashes;
	m_signalsHashes = That.m_signalsHashes;


	for (auto f : That.m_childFilters)
	{
		TuningFilter* fi = f.get();

		std::shared_ptr<TuningFilter> fiCopy = std::make_shared<TuningFilter>(*fi);

		addChild(fiCopy);
	}
}

bool TuningFilter::processMaskList(const QString& s, const QStringList& masks) const
{
	if (masks.isEmpty() == true)
	{
		return true;
	}

	int directCount = 0;
	int directMatch = 0;
	int invertedCount = 0;
	int invertedMatch = 0;

	for (QString m : masks)
	{
		if (m.isEmpty() == true)
		{
			continue;
		}

		bool invertMask = m.contains('!');
		m.remove('!');

		QRegExp rx(m.trimmed());
		rx.setPatternSyntax(QRegExp::Wildcard);

		if (invertMask == false)
		{
			directCount++;

			if (rx.exactMatch(s) == true)
			{
				directMatch++;
			}
		}

		if (invertMask == true)
		{
			invertedCount++;

			if (rx.exactMatch(s) == false)
			{
				invertedMatch++;
			}
		}
	}

	bool result = true;

	if (directCount != 0 && directMatch == 0)
	{
		result = false;
	}

	if (invertedCount != 0 && invertedCount != invertedMatch)
	{
		result = false;
	}

	return result;

}

void TuningFilter::setPropertyVisible(const QLatin1String& name, bool visible)
{
	if (propertyExists(name) == false)
	{
		assert(false);
		return;
	}
	std::shared_ptr<Property> prop = propertyByCaption(name);
	prop->setVisible(visible);
}

//
// ObjectFilterStorage
//

TuningFilterStorage::TuningFilterStorage()
{
	m_root = std::make_shared<TuningFilter>();
	m_root->setID("%FILTER%ROOT");
	m_root->setCaption(QObject::tr("All Signals"));
	m_root->setInterfaceType(TuningFilter::InterfaceType::Root);

}

TuningFilterStorage::TuningFilterStorage(const TuningFilterStorage& That)
{
	*this = That;
}

bool TuningFilterStorage::load(const QString& fileName, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	QFile f(fileName);

	if (f.exists() == false)
	{
		return true;
	}

	if (f.open(QFile::ReadOnly) == false)
	{
		*errorCode = QObject::tr("Error opening file:\r\n\r\n%1").arg(fileName);
		return false;
	}

	QByteArray data = f.readAll();

	return load(data, errorCode);

}

std::shared_ptr<TuningFilter> TuningFilterStorage::root()
{
	return m_root;
}

bool TuningFilterStorage::load(const QByteArray &data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

    QXmlStreamReader reader(data);

	if (reader.readNextStartElement() == false)
	{
		reader.raiseError(QObject::tr("Failed to load root element."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	if (reader.name() != "ObjectFilterStorage")
	{
		reader.raiseError(QObject::tr("The file is not an ObjectFilterStorage file."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	// Read signals
	//


	while (!reader.atEnd())
	{
		QXmlStreamReader::TokenType t = reader.readNext();

		if (t == QXmlStreamReader::TokenType::Characters)
		{
			continue;
		}

		if (t != QXmlStreamReader::TokenType::StartElement)
		{
			continue;
		}

		QString tagName = reader.name().toString();

		if (tagName == "Root")
		{
			if (m_root->load(reader) == false)
			{
				*errorCode = reader.errorString();
				return false;
			}

			m_root->setCaption(QObject::tr("All Signals"));

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

    return !reader.hasError();
}

bool TuningFilterStorage::save(QByteArray& data)
{
	QXmlStreamWriter writer(&data);

	writer.setCodec("UTF-8");

	writer.setAutoFormatting(true);

	writer.writeStartDocument();

	writer.writeStartElement("ObjectFilterStorage");

	m_root->save(writer, false, TuningFilter::Source::Project/*Not used!*/);

	writer.writeEndElement();

	writer.writeEndElement();	// ObjectFilterStorage

	writer.writeEndDocument();

	return true;
}

bool TuningFilterStorage::save(QByteArray& data, TuningFilter::Source saveSourceType)
{
    QXmlStreamWriter writer(&data);

	writer.setCodec("UTF-8");

    writer.setAutoFormatting(true);

	writer.writeStartDocument();

    writer.writeStartElement("ObjectFilterStorage");

	m_root->save(writer, true, saveSourceType);

    writer.writeEndElement();

    writer.writeEndElement();	// ObjectFilterStorage

    writer.writeEndDocument();

    return true;
}

bool TuningFilterStorage::save(const QString& fileName, QString* errorMsg, TuningFilter::Source saveSourceType)
{
	// save data to XML
	//

    QByteArray data;

	bool ok = save(data, saveSourceType);

    if (ok == false)
    {
        *errorMsg = QObject::tr("TuningFilterStorage::save: failed to save presets QByteArray.");
        return false;
    }

	QFile f(fileName);

	if (f.open(QFile::WriteOnly) == false)
	{
		*errorMsg = QObject::tr("TuningFilterStorage::save: failed to save presets in file %1.").arg(fileName);
		return false;
	}

	f.write(data);

	return true;

}

bool TuningFilterStorage::copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters)
{
	// save data to clipboard
	//
	QByteArray data;
	QXmlStreamWriter writer(&data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("ObjectFilterStorage");

	TuningFilter root(TuningFilter::InterfaceType::Root);

	for (auto filter : filters)
	{
		std::shared_ptr<TuningFilter> filterCopy = std::make_shared<TuningFilter>();

		*filterCopy = *filter;

		root.addChild(filterCopy);
	}

	root.save(writer, false, TuningFilter::Source::Project/*Not used!*/);

	writer.writeEndElement();

	writer.writeEndElement();	// ObjectFilterStorage

	writer.writeEndDocument();

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(data.toStdString().c_str());

	return true;
}

std::shared_ptr<TuningFilter> TuningFilterStorage::pasteFromClipboard()
{
	QClipboard* clipboard = QApplication::clipboard();
	QString clipboardText = clipboard->text();

	if (clipboardText.isEmpty() == true)
	{
		return nullptr;
	}

	TuningFilterStorage clipboardStorage;

	QByteArray data = clipboardText.toUtf8();

	QString errorMsg;

	bool ok = clipboardStorage.load(data, &errorMsg);
	if (ok == false)
	{
		return nullptr;
	}

	return clipboardStorage.m_root;
}

void TuningFilterStorage::add(std::shared_ptr<TuningFilter> filter, bool moveToTop)
{
	if (moveToTop == true)
	{
		m_root->addTopChild(filter);
	}
	else
	{
		m_root->addChild(filter);
	}
}

void TuningFilterStorage::checkFilterSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters)
{
	m_root->checkSignals(signalHashes, notFoundSignalsAndFilters);
}

void TuningFilterStorage::writeLogError(const QString& message)
{
	Q_UNUSED(message);
}

void TuningFilterStorage::writeLogWarning(const QString& message)
{
	Q_UNUSED(message);
}

void TuningFilterStorage::writeLogMessage(const QString& message)
{
	Q_UNUSED(message);
}


