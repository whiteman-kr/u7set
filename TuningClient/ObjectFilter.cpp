#include "ObjectFilter.h"
#include "../lib/Types.h"
#include "ObjectManager.h"
#include "Settings.h"

//
// ObjectFilter
//

ObjectFilter::ObjectFilter()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "StrID", true, ObjectFilter::strID, ObjectFilter::setStrID);
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, ObjectFilter::caption, ObjectFilter::setCaption);
	ADD_PROPERTY_GETTER_SETTER(SignalType, "SignalType", true, ObjectFilter::signalType, ObjectFilter::setSignalType);

	auto propFilterType = ADD_PROPERTY_GETTER(FilterType, "FilterType", true, ObjectFilter::filterType);
	propFilterType->setCategory("Debug");

	auto propMask = ADD_PROPERTY_GETTER_SETTER(QString, "CustomAppSignalMasks", true, ObjectFilter::customAppSignalIDMask, ObjectFilter::setCustomAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "AppSignalMasks", true, ObjectFilter::appSignalIDMask, ObjectFilter::setAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "EquipmentIDMasks", true, ObjectFilter::equipmentIDMask, ObjectFilter::setEquipmentIDMask);
	propMask->setCategory("Masks");

}

ObjectFilter::ObjectFilter(const ObjectFilter& That)
	:ObjectFilter()
{
	copy(That);
}

ObjectFilter::ObjectFilter(FilterType filterType)
	:ObjectFilter()
{
	m_filterType = filterType;
}

ObjectFilter& ObjectFilter::operator=(const ObjectFilter& That)
{
	copy(That);

	return *this;
}

void ObjectFilter::copy(const ObjectFilter& That)
{
	m_strID = That.m_strID;
	m_caption = That.m_caption;

	m_allowAll = That.m_allowAll;

	m_customAppSignalIDMasks = That.m_customAppSignalIDMasks;
	m_equipmentIDMasks = That.m_equipmentIDMasks;
	m_appSignalIDMasks = That.m_appSignalIDMasks;

	m_signalValues = That.m_signalValues;

	m_filterType = That.m_filterType;
	m_signalType = That.m_signalType;

	for (auto f : That.m_childFilters)
	{
		ObjectFilter* fi = f.get();

		std::shared_ptr<ObjectFilter> fiCopy = std::make_shared<ObjectFilter>(*fi);

		addChild(fiCopy);
	}
}

ObjectFilter::~ObjectFilter()
{
	//qDebug()<<"Deleting filter: "<<caption();

}

bool ObjectFilter::load(QXmlStreamReader& reader)
{

	if (reader.attributes().hasAttribute("StrID"))
	{
		setStrID(reader.attributes().value("StrID").toString());
	}

	if (reader.attributes().hasAttribute("Caption"))
	{
		setCaption(reader.attributes().value("Caption").toString());
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

	if (reader.attributes().hasAttribute("SignalType"))
	{
		QString v = reader.attributes().value("SignalType").toString();
		if (v == "All")
		{
			setSignalType(SignalType::All);
		}
		else
		{
			if (v == "Analog")
			{
				setSignalType(SignalType::Analog);
			}
			else
			{
				if (v == "Discrete")
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


			if (tagName == "Values")
			{
				recurseLevel++;

				continue;
			}

			if (tagName == "Value")
			{
				recurseLevel++;

				ObjectValue ofv;

				if (reader.attributes().hasAttribute("AppSignalId"))
				{
					ofv.appSignalId = reader.attributes().value("AppSignalId").toString();
				}

				if (reader.attributes().hasAttribute("Caption"))
				{
					ofv.caption = reader.attributes().value("Caption").toString();
				}

				if (reader.attributes().hasAttribute("UseValue"))
				{
					ofv.useValue = reader.attributes().value("UseValue").toString() == "true";
				}

				if (reader.attributes().hasAttribute("Analog"))
				{
					ofv.analog = reader.attributes().value("Analog").toString() == "true";
				}

				if (reader.attributes().hasAttribute("Value"))
				{
					ofv.value = reader.attributes().value("Value").toDouble();
				}

				if (reader.attributes().hasAttribute("DecimalPlaces"))
				{
					ofv.decimalPlaces = reader.attributes().value("DecimalPlaces").toInt();
				}

				m_signalValues.push_back(ofv);

				continue;
			}

			if (tagName == "Tree" || tagName == "Tab" || tagName == "Button")
			{
				ObjectFilter::FilterType filterType = ObjectFilter::FilterType::Tree;

				if (tagName == "Tab")
				{
					filterType = ObjectFilter::FilterType::Tab;
				}

				if (tagName == "Button")
				{
					filterType = ObjectFilter::FilterType::Button;
				}

				std::shared_ptr<ObjectFilter> of = std::make_shared<ObjectFilter>(filterType);

				if (of->load(reader) == false)
				{
					return false;
				}

				addChild(of);

				continue;
			}

			reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
			return false;
		}
	}while (t != QXmlStreamReader::EndElement);


	return true;
}

bool ObjectFilter::save(QXmlStreamWriter& writer)
{
	if (isRoot() == true)
	{
		writer.writeStartElement("Root");
	}
	else
	{
		if (isTree() == true)
		{
			writer.writeStartElement("Tree");
		}
		else
		{
			if (isTab())
			{
				writer.writeStartElement("Tab");
			}
			else
			{
				if (isButton())
				{
					writer.writeStartElement("Button");
				}
				else
				{
					assert(false);
					return false;
				}
			}
		}
	}

	writer.writeAttribute("StrID", strID());
	writer.writeAttribute("Caption", caption());

	writer.writeAttribute("CustomAppSignalIDMask", customAppSignalIDMask());
	writer.writeAttribute("EquipmentIDMask", equipmentIDMask());
	writer.writeAttribute("AppSignalIDMask", appSignalIDMask());

	writer.writeAttribute("SignalType", E::valueToString<SignalType>((int)signalType()));

	writer.writeStartElement("Values");
	for (const ObjectValue& v : m_signalValues)
	{
		writer.writeStartElement("Value");
		writer.writeAttribute("AppSignalId", v.appSignalId);
		writer.writeAttribute("Caption", v.caption);
		writer.writeAttribute("UseValue", v.useValue ? "true" : "false");
		writer.writeAttribute("Analog", v.analog ? "true" : "false");
		writer.writeAttribute("Value", QString::number(v.value, 'f', v.decimalPlaces));
		if (v.analog == true)
		{
			writer.writeAttribute("DecimalPlaces", QString::number(v.decimalPlaces));
		}
		writer.writeEndElement();
	}
	writer.writeEndElement();


	for (auto f : m_childFilters)
	{
		f->save(writer);
	}

	writer.writeEndElement();
	return true;
}


QString ObjectFilter::strID() const
{
	return m_strID;
}

void ObjectFilter::setStrID(const QString& value)
{
	m_strID = value;
}

QString ObjectFilter::caption() const
{
	return m_caption;
}

void ObjectFilter::setCaption(const QString& value)
{
	m_caption = value;
}


QString ObjectFilter::customAppSignalIDMask() const
{
	QString result;
	for (auto s : m_customAppSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void ObjectFilter::setCustomAppSignalIDMask(const QString& value)
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

QString ObjectFilter::equipmentIDMask() const
{
	QString result;
	for (auto s : m_equipmentIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void ObjectFilter::setEquipmentIDMask(const QString& value)
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

QString ObjectFilter::appSignalIDMask() const
{
	QString result;
	for (auto s : m_appSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void ObjectFilter::setAppSignalIDMask(const QString& value)
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


std::vector <ObjectValue> ObjectFilter::signalValues() const
{
	return m_signalValues;
}

void ObjectFilter::setValues(const std::vector <ObjectValue>& values)
{
	m_signalValues = values;
}

void ObjectFilter::setValue(const QString& appSignalId, double value)
{
	for (ObjectValue& ofv : m_signalValues)
	{
		if (ofv.appSignalId == appSignalId)
		{
			ofv.useValue = true;
			ofv.value = value;
			return;
		}
	}
}

bool ObjectFilter::valueExists(const QString& appSignalId)
{
	for (const ObjectValue& ofv : m_signalValues)
	{
		if (ofv.appSignalId == appSignalId)
		{
			return true;
		}
	}
	return false;
}


void ObjectFilter::addValue(const ObjectValue& value)
{
	if (valueExists(value.appSignalId) == true)
	{
		assert(false);
		return;
	}

	m_signalValues.push_back(value);
}

void ObjectFilter::removeValue(const QString& appSignalId)
{
	int index = -1;

	for (const ObjectValue& ofv : m_signalValues)
	{
		index++;

		if (ofv.appSignalId == appSignalId)
		{
			break;
		}
	}

	if (index == -1)
	{
		assert(false);
		return;
	}

	m_signalValues.erase(m_signalValues.begin() + index);
}

ObjectFilter::FilterType ObjectFilter::filterType() const
{
	return m_filterType;
}

void ObjectFilter::setFilterType(FilterType value)
{
	m_filterType = value;
}

ObjectFilter::SignalType ObjectFilter::signalType() const
{
	return m_signalType;
}

void ObjectFilter::setSignalType(SignalType value)
{
	m_signalType = value;
}

ObjectFilter* ObjectFilter::parentFilter() const
{
	return m_parentFilter;
}

bool ObjectFilter::allowAll() const
{
	return m_allowAll;
}

void ObjectFilter::setAllowAll(bool value)
{
	m_allowAll = value;
}

bool ObjectFilter::isEmpty() const
{
	if (m_signalType == SignalType::All &&
			m_signalValues.empty() == true &&
			m_appSignalIDMasks.empty() == true &&
			m_customAppSignalIDMasks.empty() == true &&
			m_equipmentIDMasks.empty() == true)
	{
		return true;
	}

	return false;
}

bool ObjectFilter::isRoot() const
{
	return filterType() == FilterType::Root;
}

bool ObjectFilter::isTree() const
{
	return filterType() == FilterType::Tree;
}

bool ObjectFilter::isTab() const
{
	return filterType() == FilterType::Tab;
}

bool ObjectFilter::isButton() const
{
	return filterType() == FilterType::Button;
}

void ObjectFilter::addTopChild(std::shared_ptr<ObjectFilter> child)
{
	child->m_parentFilter = this;
	m_childFilters.insert(m_childFilters.begin(), child);
}

void ObjectFilter::addChild(std::shared_ptr<ObjectFilter> child)
{
	child->m_parentFilter = this;
	m_childFilters.push_back(child);
}

void ObjectFilter::removeChild(std::shared_ptr<ObjectFilter> child)
{
	int index = -1;

	for (auto it : m_childFilters)
	{
		index++;
		if (it.get() == child.get())
		{
			break;
		}
	}

	if (index == -1)
	{
		assert(false);
	}
	else
	{
		m_childFilters.erase(m_childFilters.begin() + index);
	}
}

void ObjectFilter::removeAllChildren()
{
	m_childFilters.clear();

}

int ObjectFilter::childFiltersCount() const
{
	return static_cast<int>(m_childFilters.size());

}

std::shared_ptr<ObjectFilter> ObjectFilter::childFilter(int index) const
{
	if (index <0 || index >= m_childFilters.size())
	{
		assert(false);
		return nullptr;
	}

	return m_childFilters[index];
}


bool ObjectFilter::match(const TuningObject& object)
{
	if (allowAll() == true)
	{
		return true;
	}
	if (isEmpty() == true)
	{
		return true;
	}

	if (signalType() == ObjectFilter::SignalType::Analog && object.analog() == false)
	{
		return false;
	}
	if (signalType() == ObjectFilter::SignalType::Discrete && object.analog() == true)
	{
		return false;
	}

	// Mask for equipmentID
	//

	if (m_equipmentIDMasks.isEmpty() == false)
	{

		QString s = object.equipmentID();

		bool result = false;

		for (QString m : m_equipmentIDMasks)
		{
			if (m.isEmpty() == true)
			{
				continue;
			}
			QRegExp rx(m.trimmed());
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (rx.exactMatch(s))
			{
				result = true;
				break;
			}
		}
		if (result == false)
		{
			return false;
		}
	}

	// Mask for appSignalId
	//

	if (m_appSignalIDMasks.isEmpty() == false)
	{

		QString s = object.appSignalID();

		bool result = false;

		for (QString m : m_appSignalIDMasks)
		{
			if (m.isEmpty() == true)
			{
				continue;
			}
			QRegExp rx(m.trimmed());
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (rx.exactMatch(s))
			{
				result = true;
				break;
			}
		}
		if (result == false)
		{
			return false;
		}
	}

	// List of appSignalId
	//
	if (m_signalValues.empty() == false)
	{
		QString s = object.appSignalID();

		bool result = false;

		for (const ObjectValue& v : m_signalValues)
		{
			if (v.appSignalId == s)
			{
				result = true;
				break;
			}
		}
		if (result == false)
		{
			return false;
		}
	}

	// Mask for customAppSignalID
	//

	if (m_customAppSignalIDMasks.isEmpty() == false)
	{
		QString s = object.customAppSignalID();

		bool result = false;

		for (QString m : m_customAppSignalIDMasks)
		{
			if (m.isEmpty() == true)
			{
				continue;
			}
			QRegExp rx(m.trimmed());
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (rx.exactMatch(s))
			{
				result = true;
				break;
			}
		}
		if (result == false)
		{
			return false;
		}
	}

	return true;
}


//
// ObjectFilterStorage
//

ObjectFilterStorage::ObjectFilterStorage()
{
	m_root = std::make_shared<ObjectFilter>();
	m_root->setStrID("\\");
	m_root->setCaption("\\");
	m_root->setFilterType(ObjectFilter::FilterType::Root);

}

ObjectFilterStorage::ObjectFilterStorage(const ObjectFilterStorage& That)
{
	m_root = std::make_shared<ObjectFilter>(*That.m_root.get());
	m_schemasDetails = That.m_schemasDetails;
}

bool ObjectFilterStorage::load(const QString& fileName, QString* errorCode)
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

bool ObjectFilterStorage::load(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_root->removeAllChildren();

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

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();
}

bool ObjectFilterStorage::save(const QString& fileName)
{
	// save data to XML
	//
	QByteArray data;
	QXmlStreamWriter writer(&data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("ObjectFilterStorage");

	m_root->save(writer);

	writer.writeEndElement();

	writer.writeEndElement();	// ObjectFilterStorage

	writer.writeEndDocument();

	QFile f(fileName);

	if (f.open(QFile::WriteOnly) == false)
	{
		return false;
	}

	f.write(data);

	return true;

}


int ObjectFilterStorage::schemaDetailsCount()
{
	return static_cast<int>(m_schemasDetails.size());
}

SchemaDetails ObjectFilterStorage::schemaDetails(int index)
{
	if (index < 0 || index >= m_schemasDetails.size())
	{
		assert(false);
		return SchemaDetails();
	}
	return m_schemasDetails[index];
}


bool ObjectFilterStorage::loadSchemasDetails(const QByteArray& data, QString *errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_schemasDetails.clear();

	QXmlStreamReader reader(data);

	if (reader.readNextStartElement() == false)
	{
		reader.raiseError(QObject::tr("Failed to load root element."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	if (reader.name() != "Schemas")
	{
		reader.raiseError(QObject::tr("The file is not an SchemasDetails file."));
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

		if (reader.name() == "Schema")
		{
			if (reader.attributes().hasAttribute("Details"))
			{
				QString details = reader.attributes().value("Details").toString();
				if (details.isEmpty() == true)
				{
					continue;
				}

				QJsonDocument document = QJsonDocument::fromJson(details.toUtf8());
				if (document.isEmpty() == true || document.isNull() == true || document.isObject() == false)
				{
					continue;
				}

				QJsonObject jDetails = document.object();
				if (jDetails.isEmpty() == true)
				{
					continue;
				}

				SchemaDetails sd;

				QJsonValue jValue = jDetails.value("SchemaID");
				if (jValue.isNull() == false && jValue.isUndefined() == false && jValue.isString() == true)
				{
					sd.m_strId = jValue.toString();
				}


				jValue = jDetails.value("Caption");
				if (jValue.isNull() == false && jValue.isUndefined() == false && jValue.isString() == true)
				{
					sd.m_caption = jValue.toString();
				}

				QJsonArray array = jDetails.value("Signals").toArray();
				sd.m_appSignals.reserve(array.size());
				for (int i = 0; i < array.size(); i++)
				{
					sd.m_appSignals.push_back(array[i].toString());
				}

				m_schemasDetails.push_back(sd);
			}

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();

}

void ObjectFilterStorage::createAutomaticFilters()
{
	if (theSettings.filterBySchema() == true)
	{

		// Filter for Schema
		//
		std::shared_ptr<ObjectFilter> ofSchema = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
		ofSchema->setStrID("%AUTOFILTER%_SCHEMA");
		ofSchema->setCaption("Schemas");

		for (auto s : m_schemasDetails)
		{
			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
			for (auto s : s.m_appSignals)
			{
				ObjectValue ofv;
				ofv.appSignalId = s;
				ofTs->addValue(ofv);
			}

			ofTs->setStrID("%AUFOFILTER%_SCHEMA_" + s.m_strId);
			ofTs->setCaption(s.m_caption);

			ofSchema->addChild(ofTs);
		}

		m_root->addTopChild(ofSchema);
	}

	if (theSettings.filterByEquipment() == true)
	{
		// Filter for EquipmentId
		//
		std::shared_ptr<ObjectFilter> ofEquipment = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
		ofEquipment->setStrID("%AUTOFILTER%_EQUIPMENT");
		ofEquipment->setCaption("Equipment");

		for (int i = 0; i < theObjects.tuningSourcesCount(); i++)
		{
			QString ts = theObjects.tuningSourceEquipmentId(i);

			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
			ofTs->setEquipmentIDMask(ts);
			ofTs->setStrID("%AUFOFILTER%_EQUIPMENT_" + ts);
			ofTs->setCaption(ts);

			ofEquipment->addChild(ofTs);
		}

		m_root->addTopChild(ofEquipment);
	}

}

ObjectFilterStorage theFilters;
ObjectFilterStorage theUserFilters;
