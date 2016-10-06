#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
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

std::vector<TuningObject> ObjectManager::objects()
{
	QMutexLocker l(&m_mutex);
	return m_objects;
}

int ObjectManager::tuningSourcesCount()
{
	QMutexLocker l(&m_mutex);
	return static_cast<int>(m_tuningSourcesList.size());
}

QString ObjectManager::tuningSourceEquipmentId(int index)
{
	QMutexLocker l(&m_mutex);
	if (index < 0 || index >= m_tuningSourcesList.size())
	{
		assert(false);
		return QString();
	}

	return m_tuningSourcesList[index];
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

