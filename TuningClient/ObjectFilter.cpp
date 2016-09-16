#include "ObjectFilter.h"
#include "../lib/Types.h"

//
// ObjectFilter
//

ObjectFilter::ObjectFilter(FilterType filterType)
{
	m_filterType = filterType;
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


	QXmlStreamReader::TokenType t;
	do
	{
		t = reader.readNext();

		if (t == QXmlStreamReader::StartElement)
		{
			if (reader.name() == "ObjectFilter")
			{
				std::shared_ptr<ObjectFilter> of = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Child);

				if (of->load(reader) == false)
				{
					return false;
				}

				childFilters.push_back(of);
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

	writer.writeAttribute("FilterType", E::valueToString<FilterType>((int)filterType()));
	writer.writeAttribute("SignalType", E::valueToString<SignalType>((int)signalType()));

	for (auto f : childFilters)
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

			if (of->load(reader) == false)
			{
				return false;
			}

			filters.push_back(of);

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
		for (auto of : filters)
		{
			if (of->filterType() != r.second)
			{
				continue;
			}

			of->save(writer);
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

void ObjectFilterStorage::clear()
{
	filters.clear();
}

ObjectFilterStorage theFilters;
ObjectFilterStorage theUserFilters;
