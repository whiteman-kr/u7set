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

	/*auto propHash = ADD_PROPERTY_GETTER(Hash, "Hash", true, ObjectFilter::hash);
	propHash->setCategory("Debug");

	auto propParentHash = ADD_PROPERTY_GETTER(Hash, "ParentHash", true, ObjectFilter::parentHash);
	propParentHash->setCategory("Debug");*/

	auto propFilterType = ADD_PROPERTY_GETTER(FilterType, "FilterType", true, ObjectFilter::filterType);
	propFilterType->setCategory("Debug");

	auto propMask = ADD_PROPERTY_GETTER_SETTER(QString, "CustomAppSignalMasks", true, ObjectFilter::customAppSignalIDMask, ObjectFilter::setCustomAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "AppSignalMasks", true, ObjectFilter::appSignalIDMask, ObjectFilter::setAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "EquipmentIDMasks", true, ObjectFilter::equipmentIDMask, ObjectFilter::setEquipmentIDMask);
	propMask->setCategory("Masks");

	auto propSignals = ADD_PROPERTY_GETTER_SETTER(QString, "AppSignalIds", true, ObjectFilter::appSignalIds, ObjectFilter::setAppSignalIds);
	propSignals->setCategory("Signals");

}

ObjectFilter::ObjectFilter(FilterType filterType):ObjectFilter()
{
	m_filterType = filterType;
}

ObjectFilter::ObjectFilter(const ObjectFilter& That):ObjectFilter()
{
	m_strID = That.m_strID;
	m_caption = That.m_caption;

	m_allowAll = That.m_allowAll;
	m_denyAll = That.m_allowAll;

	m_hash = That.m_hash;

	m_customAppSignalIDMasks = That.m_customAppSignalIDMasks;
	m_equipmentIDMasks = That.m_equipmentIDMasks;
	m_appSignalIDMasks = That.m_appSignalIDMasks;
	m_appSignalIds = That.m_appSignalIds;

	m_filterType = That.m_filterType;
	m_signalType = That.m_signalType;

	m_parentHash = That.m_parentHash;

	for (auto f : That.m_childFilters)
	{
		ObjectFilter* fi = f.get();

		std::shared_ptr<ObjectFilter> fiCopy = std::make_shared<ObjectFilter>(*fi);

		addChild(fiCopy);
	}
}

bool ObjectFilter::load(QXmlStreamReader& reader, std::map<Hash, std::shared_ptr<ObjectFilter>>& filtersMap)
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

	QXmlStreamReader::TokenType t;
	do
	{
		t = reader.readNext();

		if (t == QXmlStreamReader::StartElement)
		{
			QString tagName = reader.name().toString();

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

				if (of->load(reader, filtersMap) == false)
				{
					return false;
				}

				of->setStrID(m_strID + "_" + of->strID());

				if (filtersMap.find(of->hash()) != filtersMap.end())
				{
					reader.raiseError(QObject::tr("string identifier '%1' of the filter is not unque.").arg(of->strID()));
					return false;
				}

				filtersMap[of->hash()] = of;
				of->setParentHash(hash());

				addChild(of);

			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				return false;
			}
		}
	}while (t != QXmlStreamReader::EndElement);


	return true;
}

bool ObjectFilter::save(QXmlStreamWriter& writer)
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

	writer.writeAttribute("StrID", strID());
	writer.writeAttribute("Caption", caption());

	writer.writeAttribute("CustomAppSignalIDMask", customAppSignalIDMask());
	writer.writeAttribute("EquipmentIDMask", equipmentIDMask());
	writer.writeAttribute("AppSignalIDMask", appSignalIDMask());

	writer.writeAttribute("SignalType", E::valueToString<SignalType>((int)signalType()));

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
	m_hash = ::calcHash(value);
}

Hash ObjectFilter::hash() const
{
	return m_hash;
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

QString ObjectFilter::appSignalIds() const
{
	QString result;
	for (auto s : m_appSignalIds)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void ObjectFilter::setAppSignalIds(const QString &value)
{
	if (value.isEmpty() == true)
	{
		m_appSignalIds.clear();
	}
	else
	{
		m_appSignalIds = value.split(';');
	}
}

void ObjectFilter::setAppSignalIdsList(const QStringList& value)
{
	m_appSignalIds = value;
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

Hash ObjectFilter::parentHash() const
{
	return m_parentHash;
}

void ObjectFilter::setParentHash(Hash value)
{
	m_parentHash = value;
}

bool ObjectFilter::allowAll() const
{
	return m_allowAll;
}

void ObjectFilter::setAllowAll(bool value)
{
	m_allowAll = value;
}

bool ObjectFilter::denyAll() const
{
	return m_denyAll;
}

void ObjectFilter::setDenyAll(bool value)
{
	m_denyAll = value;
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

void ObjectFilter::addChild(std::shared_ptr<ObjectFilter> child)
{
	m_childFilters.push_back(child);
}

void ObjectFilter::removeChild(std::shared_ptr<ObjectFilter> child)
{
	int index = -1;

	for (auto it : m_childFilters)
	{
		index++;
		if (it->hash() == child->hash())
		{
			break;
		}
	}

	if (index != -1)
	{
		m_childFilters.erase(m_childFilters.begin() + index);
		return;
	}

	assert(false);
	return;
}

int ObjectFilter::childFiltersCount()
{
	return static_cast<int>(m_childFilters.size());

}

ObjectFilter* ObjectFilter::childFilter(int index)
{
	if (index <0 || index >= m_childFilters.size())
	{
		assert(false);
		return nullptr;
	}

	return m_childFilters[index].get();
}


bool ObjectFilter::match(const TuningObject& object)
{
	if (allowAll() == true)
	{
		return true;
	}
	if (denyAll() == true)
	{
		return false;
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
	if (m_appSignalIds.isEmpty() == false)
	{
		QString s = object.appSignalID();

		bool result = false;

		for (auto id : m_appSignalIds)
		{
			if (id == s)
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

}

ObjectFilterStorage::ObjectFilterStorage(const ObjectFilterStorage& That)
{
	m_topFilters = That.m_topFilters;
	m_schemasDetails = That.m_schemasDetails;

	for (auto f : That.m_filtersMap)
	{
		// create objects copies
		//
		Hash hash = f.first;
		ObjectFilter* filter = f.second.get();

		m_filtersMap[hash] = std::make_shared<ObjectFilter>(*filter);
	}
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

	m_filtersMap.clear();
	m_topFilters.clear();

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

			if (of->load(reader, m_filtersMap) == false)
			{
				*errorCode = reader.errorString();
				return false;
			}

			if (m_filtersMap.find(of->hash()) != m_filtersMap.end())
			{
				reader.raiseError(QObject::tr("string identifier '%1' of the filter is not unque.").arg(of->strID()));
				*errorCode = reader.errorString();
				return false;
			}

			m_filtersMap[of->hash()] = of;
			m_topFilters.push_back(of->hash());

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

	for (auto of : m_topFilters)
	{
		ObjectFilter* f = m_filtersMap[of].get();
		if (f == nullptr)
		{
			assert(f);
			return false;
		}

		f->save(writer);
	}
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

int ObjectFilterStorage::topFilterCount()
{
	return static_cast<int>(m_topFilters.size());
}

std::shared_ptr<ObjectFilter> ObjectFilterStorage::topFilter(int index)
{
	if (index < 0 || index >= m_topFilters.size())
	{
		assert(false);
		return nullptr;
	}

	Hash hash = m_topFilters[index];

	return filter(hash);
}

bool ObjectFilterStorage::addTopFilter(const std::shared_ptr<ObjectFilter> filter)
{
	return addFilter(nullptr, filter);
}

bool ObjectFilterStorage::addFilter(ObjectFilter* parent, const std::shared_ptr<ObjectFilter> filter)
{
	if (m_filtersMap.find(filter->hash()) != m_filtersMap.end())
	{
		assert(false);
		return false;
	}

	m_filtersMap[filter->hash()] = filter;

	if (parent != nullptr)
	{
		filter->setParentHash(parent->hash());
		parent->addChild(filter);
	}
	else
	{
		m_topFilters.push_back(filter->hash());
	}

	return true;
}

bool ObjectFilterStorage::removeFilter(Hash hash)
{
	if (m_filtersMap.find(hash) == m_filtersMap.end())
	{
		assert(false);
		return false;
	}

	std::shared_ptr<ObjectFilter> f = m_filtersMap[hash];

	if (f->parentHash() != 0)
	{
		// remove this filter from parent
		//
		std::shared_ptr<ObjectFilter> p = filter(f->parentHash());
		if (p == nullptr)
		{
			assert(p);
			return false;
		}

		p->removeChild(f);
	}

	//remove it from top filters

	auto it = std::find(m_topFilters.begin(), m_topFilters.end(), hash);
	if (it != m_topFilters.end())
	{
		m_topFilters.erase(it);
	}

	// remove it from map

	auto fptr = m_filtersMap.find(hash);
	if (fptr != m_filtersMap.end())
	{
		m_filtersMap.erase(fptr);
	}

	return true;

}

std::shared_ptr<ObjectFilter> ObjectFilterStorage::filter(Hash hash)
{
	auto it = m_filtersMap.find(hash);
	if (it == m_filtersMap.end())
	{
		assert(false);
		return nullptr;
	}

	return it->second;
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
		ofSchema->setCaption("Filter by Schema");
		ofSchema->setDenyAll(true);

		for (auto s : m_schemasDetails)
		{
			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
			ofTs->setAppSignalIdsList(s.m_appSignals);
			ofTs->setStrID("%AUFOFILTER%_SCHEMA_" + s.m_strId);
			ofTs->setCaption(s.m_caption);
			m_filtersMap[ofTs->hash()] = ofTs;

			ofSchema->addChild(ofTs);
		}

		m_filtersMap[ofSchema->hash()] = ofSchema;
		m_topFilters.insert(m_topFilters.begin(), ofSchema->hash());
	}

	if (theSettings.filterByEquipment() == true)
	{
		// Filter for EquipmentId
		//
		std::shared_ptr<ObjectFilter> ofEquipment = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
		ofEquipment->setStrID("%AUTOFILTER%_EQUIPMENT");
		ofEquipment->setCaption("Filter by EquipmentId");
		ofEquipment->setDenyAll(true);

		for (int i = 0; i < theObjects.tuningSourcesCount(); i++)
		{
			QString ts = theObjects.tuningSourceEquipmentId(i);

			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
			ofTs->setEquipmentIDMask(ts);
			ofTs->setStrID("%AUFOFILTER%_EQUIPMENT_" + ts);
			ofTs->setCaption(ts);
			m_filtersMap[ofTs->hash()] = ofTs;

			ofEquipment->addChild(ofTs);
		}

		m_filtersMap[ofEquipment->hash()] = ofEquipment;
		m_topFilters.insert(m_topFilters.begin(), ofEquipment->hash());
	}

	// Root Filter for All in tree
	//
	bool createRootFilter = false;
	for (auto tf : m_topFilters)
	{
		std::shared_ptr<ObjectFilter> f = filter(tf);
		if (f->isTree())
		{
			createRootFilter = true;
			break;
		}
	}
	if (createRootFilter == true)
	{
		std::shared_ptr<ObjectFilter> ofRoot = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);
		ofRoot->setStrID("%AUTOFILTER%_ROOT");
		ofRoot->setCaption("All objects");
		ofRoot->setAllowAll(true);

		m_filtersMap[ofRoot->hash()] = ofRoot;
		m_topFilters.insert(m_topFilters.begin(), ofRoot->hash());
	}
}

ObjectFilterStorage theFilters;
ObjectFilterStorage theUserFilters;
