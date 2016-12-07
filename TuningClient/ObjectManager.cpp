#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
}

int ObjectManager::objectCount()
{
    return m_objects.size();

}

TuningObject ObjectManager::object(int index)
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index >= m_objects.size())
	{
		assert(false);
		return TuningObject();
	}

	return m_objects[index];

}

TuningObject* ObjectManager::objectPtr(int index)
{
    if (index < 0 || index >= m_objects.size())
    {
        assert(false);
        return nullptr;
    }

    return &m_objects[index];
}

TuningObject* ObjectManager::objectPtrByHash(quint64 hash)
{
    auto it = m_objectsHashMap.find(hash);

    if (it == m_objectsHashMap.end())
    {
        assert(false);
        return nullptr;
    }

    return objectPtr(it->second);

}

std::vector<TuningObject> ObjectManager::objects()
{
	QMutexLocker l(&m_mutex);
	return m_objects;
}

QStringList ObjectManager::tuningSourcesEquipmentIds()
{
	QMutexLocker l(&m_mutex);
	return m_tuningSourcesList;
}

bool ObjectManager::loadSignals(const QByteArray& data, QString *errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	QMutexLocker l(&m_mutex);

	m_objects.clear();

    m_objectsHashMap.clear();

	QXmlStreamReader reader(data);

	if (reader.readNextStartElement() == false)
	{
		reader.raiseError(QObject::tr("Failed to load root element."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	if (reader.name() != "TuningSignals")
	{
		reader.raiseError(QObject::tr("The file is not an TuningSignals file."));
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

		if (reader.name() == "TuningSignal")
		{
			TuningObject object;

			if (reader.attributes().hasAttribute("AppSignalID"))
			{
				object.setAppSignalID(reader.attributes().value("AppSignalID").toString());
			}

			if (reader.attributes().hasAttribute("CustomAppSignalID"))
			{
				object.setCustomAppSignalID(reader.attributes().value("CustomAppSignalID").toString());
			}

			if (reader.attributes().hasAttribute("EquipmentID"))
			{
				object.setEquipmentID(reader.attributes().value("EquipmentID").toString());
			}

			if (reader.attributes().hasAttribute("Caption"))
			{
				object.setCaption(reader.attributes().value("Caption").toString());
			}

			if (reader.attributes().hasAttribute("Type"))
			{
				QString t = reader.attributes().value("Type").toString();
				object.setAnalog(t == "A");
			}

			if (reader.attributes().hasAttribute("DecimalPlaces"))
			{
				object.setDecimalPlaces(reader.attributes().value("DecimalPlaces").toString().toInt());
			}

			if (reader.attributes().hasAttribute("DefaultValue"))
			{
				QString v = reader.attributes().value("DefaultValue").toString();
				object.setDefaultValue(v.toDouble());
			}

			if (reader.attributes().hasAttribute("LowLimit"))
			{
				QString v = reader.attributes().value("LowLimit").toString();
				object.setLowLimit(v.toDouble());
			}

			if (reader.attributes().hasAttribute("HighLimit"))
			{
				QString v = reader.attributes().value("HighLimit").toString();
				object.setHighLimit(v.toDouble());
			}


            m_objects.push_back(object);

            m_objectsHashMap[object.appSignalHash()] = m_objects.size() - 1;

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	// Create Tuning sources
	//
	m_tuningSourcesList.clear();

	for (auto o : m_objects)
	{
		if (m_tuningSourcesList.indexOf(o.equipmentID()) == -1)
		{
			m_tuningSourcesList.append(o.equipmentID());

		}
	}

	return !reader.hasError();
}

