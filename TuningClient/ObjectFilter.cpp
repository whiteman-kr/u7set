#include "ObjectFilter.h"
#include "../lib/Types.h"
#include "ObjectManager.h"
#include "Settings.h"

//
// ObjectFilter
//

ObjectFilter::ObjectFilter(FilterType filterType)
{
	m_filterType = filterType;
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
			if (reader.name() == "ObjectFilter")
			{
				std::shared_ptr<ObjectFilter> of = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Child);

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

				of->setParent(this);

				m_childFilters.push_back(of);
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
	writer.writeStartElement("ObjectFilter");

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

QStringList ObjectFilter::appSignalIds() const
{
	return m_appSignalIds;
}

void ObjectFilter::setAppSignalIds(const QStringList& value)
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

ObjectFilter* ObjectFilter::parent() const
{
	return m_parent;
}

void ObjectFilter::setParent(ObjectFilter* value)
{
	m_parent = value;
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

bool ObjectFilter::isChild() const
{
	return filterType() == FilterType::Child;
}

void ObjectFilter::addChild(std::shared_ptr<ObjectFilter> child)
{
	m_childFilters.push_back(child);
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
	ObjectFilter::FilterType filterType = ObjectFilter::FilterType::Child;

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

		if (reader.name() == "ObjectFilter")
		{
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

		if (reader.name() == "Tree")
		{
			filterType = ObjectFilter::FilterType::Tree;
			continue;
		}

		if (reader.name() == "Tabs")
		{
			filterType = ObjectFilter::FilterType::Tab;
			continue;
		}

		if (reader.name() == "Buttons")
		{
			filterType = ObjectFilter::FilterType::Button;
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

	QList<std::pair<QString, ObjectFilter::FilterType>> records;
	records.push_back(std::make_pair("Tree", ObjectFilter::FilterType::Tree));
	records.push_back(std::make_pair("Tabs", ObjectFilter::FilterType::Tab));
	records.push_back(std::make_pair("Buttons", ObjectFilter::FilterType::Button));

	for (auto r : records)
	{
		writer.writeStartElement(r.first);
		for (auto of : m_topFilters)
		{
			ObjectFilter* f = m_filtersMap[of].get();
			if (f == nullptr)
			{
				assert(f);
				return false;
			}

			if (f->filterType() != r.second)
			{
				continue;
			}

			f->save(writer);
		}
		writer.writeEndElement();
	}

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

ObjectFilter* ObjectFilterStorage::topFilter(int index)
{
	if (index < 0 || index >= m_topFilters.size())
	{
		assert(false);
		return nullptr;
	}

	Hash hash = m_topFilters[index];

	return filter(hash);
}

ObjectFilter* ObjectFilterStorage::filter(Hash hash)
{
	auto it = m_filtersMap.find(hash);
	if (it == m_filtersMap.end())
	{
		assert(false);
		return nullptr;
	}

	return it->second.get();
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
			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Child);
			ofTs->setAppSignalIds(s.m_appSignals);
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

			std::shared_ptr<ObjectFilter> ofTs = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Child);
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
		ObjectFilter* f = filter(tf);
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

		/*for (auto tf : m_topFilters)
		{
			ObjectFilter* f = filter(tf);
			if (f->isTree())
			{
				ofRoot->addChild(m_filtersMap[tf]);
				f->setParent(ofRoot.get());
			}
		}*/

		m_filtersMap[ofRoot->hash()] = ofRoot;
		m_topFilters.insert(m_topFilters.begin(), ofRoot->hash());
	}
}

ObjectFilterStorage theFilters;
ObjectFilterStorage theUserFilters;
