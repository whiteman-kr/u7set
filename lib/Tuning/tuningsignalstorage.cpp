#include "TuningSignalStorage.h"



//
// TuningSignalStorage
//

TuningSignalStorage::TuningSignalStorage()
{

}


bool TuningSignalStorage::loadSignals(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_signals.clear();

	m_signalsMap.clear();

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
			std::shared_ptr<AppSignalParam> object = std::make_shared<AppSignalParam>();

			if (reader.attributes().hasAttribute("AppSignalID"))
			{
				object->setAppSignalId(reader.attributes().value("AppSignalID").toString());
			}

			if (reader.attributes().hasAttribute("CustomAppSignalID"))
			{
				object->setCustomSignalId(reader.attributes().value("CustomAppSignalID").toString());
			}

			if (reader.attributes().hasAttribute("EquipmentID"))
			{
				object->setEquipmentId(reader.attributes().value("EquipmentID").toString());
			}

			if (reader.attributes().hasAttribute("Caption"))
			{
				object->setCaption(reader.attributes().value("Caption").toString());
			}

			if (reader.attributes().hasAttribute("Type"))
			{
				QString t = reader.attributes().value("Type").toString();
				object->setType(t == "A" ? E::SignalType::Analog : E::SignalType::Discrete);
			}

			if (reader.attributes().hasAttribute("DecimalPlaces"))
			{
				object->setPrecision(reader.attributes().value("DecimalPlaces").toString().toInt());
			}

			if (reader.attributes().hasAttribute("DefaultValue"))
			{
				QString v = reader.attributes().value("DefaultValue").toString();
				object->setTuningDefaultValue(v.toFloat());
			}

			if (reader.attributes().hasAttribute("LowLimit"))
			{
				QString v = reader.attributes().value("LowLimit").toString();
				object->setLowEngineeringUnits(v.toFloat());
			}

			if (reader.attributes().hasAttribute("HighLimit"))
			{
				QString v = reader.attributes().value("HighLimit").toString();
				object->setHighEngineeringUnits(v.toFloat());
			}

			object->setHash(::calcHash(object->appSignalId()));

			m_signals.push_back(object);

			m_signalsMap[object->hash()] = static_cast<int>(m_signals.size()) - 1;

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();
}


int TuningSignalStorage::signalsCount() const
{
	return static_cast<int>(m_signals.size());

}

bool TuningSignalStorage::signalExists(Hash hash) const
{
	return (m_signalsMap.find(hash) != m_signalsMap.end());
}

AppSignalParam* TuningSignalStorage::signalPtrByIndex(int index) const
{
	if (index < 0 || index >= m_signals.size())
	{
		assert(false);
		return nullptr;
	}

	return m_signals[index].get();
}

AppSignalParam* TuningSignalStorage::signalPtrByHash(Hash hash) const
{
	const auto it = m_signalsMap.find(hash);

	if (it == m_signalsMap.end())
	{
		assert(false);
		return nullptr;
	}

	return signalPtrByIndex(it->second);

}
