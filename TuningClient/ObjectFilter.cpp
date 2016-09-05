#include "ObjectFilter.h"
#include "../lib/Types.h"

//
// ObjectFilter
//

ObjectFilter::ObjectFilter()
{

}

bool ObjectFilter::load(QXmlStreamReader& reader,
		  std::map<QString, std::shared_ptr<ObjectFilter>>& filtersMap,
			std::vector<std::shared_ptr<ObjectFilter>>& filtersVector)
{

	if (reader.attributes().hasAttribute("StrID"))
	{
		setStrID(reader.attributes().value("StrID").toString());
	}

	qDebug()<<strID();

	if (reader.attributes().hasAttribute("ParentStrID"))
	{
		setParentStrID(reader.attributes().value("ParentStrID").toString());
	}

	if (reader.attributes().hasAttribute("Caption"))
	{
		setCaption(reader.attributes().value("Caption").toString());
	}

	if (reader.attributes().hasAttribute("User"))
	{
		setUser(reader.attributes().value("User").toString() == "true" ? true : false);
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
				std::shared_ptr<ObjectFilter> of = std::make_shared<ObjectFilter>();

				if (of->load(reader, filtersMap, filtersVector) == false)
				{
					return false;
				}

				of->setParentStrID(strID());

				of->setFilterType(ObjectFilter::FilterType::Child);
				filtersMap[of->strID()] = of;
				filtersVector.push_back(of);
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
	/*writer.writeAttribute("StrID", strID());
	writer.writeAttribute("ParentStrID", parentStrID());
	writer.writeAttribute("Caption", caption());

	writer.writeAttribute("User", user() ? "true" : "false");

	writer.writeAttribute("CustomAppSignalIDMask", customAppSignalIDMask());
	writer.writeAttribute("EquipmentIDMask", equipmentIDMask());
	writer.writeAttribute("AppSignalIDMask", appSignalIDMask());

	writer.writeAttribute("FilterType", E::valueToString<FilterType>((int)filterType()));
	writer.writeAttribute("SignalType", E::valueToString<SignalType>((int)signalType()));*/

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

QString ObjectFilter::parentStrID() const
{
	return m_parentStrID;
}

void ObjectFilter::setParentStrID(const QString& value)
{
	m_parentStrID = value;
}

QString ObjectFilter::caption() const
{
	return m_caption;
}

void ObjectFilter::setCaption(const QString& value)
{
	m_caption = value;
}

bool ObjectFilter::user() const
{
	return m_user;
}

void ObjectFilter::setUser(bool value)
{
	m_user = value;
}

QString ObjectFilter::customAppSignalIDMask() const
{
	return m_customAppSignalIDMask;
}

void ObjectFilter::setCustomAppSignalIDMask(const QString& value)
{
	m_customAppSignalIDMask = value;
}

QString ObjectFilter::equipmentIDMask() const
{
	return m_equipmentIDMask;
}

void ObjectFilter::setEquipmentIDMask(const QString& value)
{
	m_equipmentIDMask = value;
}

QString ObjectFilter::appSignalIDMask() const
{
	return m_appSignalIDMask;
}

void ObjectFilter::setAppSignalIDMask(const QString& value)
{
	m_appSignalIDMask = value;
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



//
// ObjectFilterStorage
//

ObjectFilterStorage::ObjectFilterStorage()
{

	//std::shared_ptr<ObjectFilter> test1 = std::make_shared<ObjectFilter>();
	//m_filters.push_back(test1);

}

bool ObjectFilterStorage::load(const QString& fileName)
{
	m_errorCode.clear();

	QFile f(fileName);
	if (f.open(QFile::ReadOnly) == false)
	{
		return false;
	}

	QByteArray data = f.readAll();

	QXmlStreamReader reader(data);

	if (reader.readNextStartElement() == false)
	{
		reader.raiseError(QObject::tr("Failed to load root element."));
		m_errorCode = reader.errorString();
		return !reader.hasError();
	}

	if (reader.name() != "ObjectFilterStorage")
	{
		reader.raiseError(QObject::tr("The file is not an ObjectFilterStorage file."));
		m_errorCode = reader.errorString();
		return !reader.hasError();
	}

	// Read signals
	//
	ObjectFilter::FilterType filterType = ObjectFilter::FilterType::Tree;

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
			std::shared_ptr<ObjectFilter> of = std::make_shared<ObjectFilter>();

			if (of->load(reader, m_filtersMap, m_filtersVector) == false)
			{
				return false;
			}

			of->setFilterType(filterType);

			m_filtersMap[of->strID()] = of;
			m_filtersVector.push_back(of);

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
		m_errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();
}

bool ObjectFilterStorage::save(const QString& fileName, bool user)
{
	m_errorCode.clear();
/*
	// save data to XML
	//
	QByteArray data;
	QXmlStreamWriter writer(&data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("ObjectFilterStorage");
	for (auto of : m_filtersVector)
	{
		if (of->user() != user)
		{
			continue;
		}

		writer.writeStartElement("ObjectFilter");
		of->save(writer);
		writer.writeEndElement();
	}
	writer.writeEndElement();

	writer.writeEndDocument();

	QFile f(fileName);

	if (f.open(QFile::WriteOnly) == false)
	{
		return false;
	}

	f.write(data);
*/
	return true;

}

QString ObjectFilterStorage::errorCode()
{
	return m_errorCode;

}

int ObjectFilterStorage::filtersCount() const
{
	return (int)m_filtersVector.size();

}

ObjectFilter* ObjectFilterStorage::filter(int index)
{
	if (index < 0 || index >= filtersCount())
	{
		assert(false);
		return nullptr;
	}

	return m_filtersVector[index].get();

}

ObjectFilter* ObjectFilterStorage::filter(const QString& filterId)
{
	auto it = m_filtersMap.find(filterId);
	if (it == m_filtersMap.end())
	{
		assert(false);
		return nullptr;
	}

	return it->second.get();
}


ObjectFilterStorage theFilters;
