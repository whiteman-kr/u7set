#include "TuningSignal.h"

#include<memory>

//
// TuningSignalState
//

bool TuningSignalState::valid() const
{
	//return true;
	return m_flags.m_valid;
}

void TuningSignalState::setValid(bool value)
{
	if (m_flags.m_valid != value)
	{
		m_flags.m_needRedraw = true;

		m_flags.m_valid = value;
	}

}

bool TuningSignalState::underflow() const
{
	return m_flags.m_underflow;
}

bool TuningSignalState::overflow() const
{
	return m_flags.m_overflow;
}

bool TuningSignalState::needRedraw()
{
	bool result = m_flags.m_needRedraw;

	m_flags.m_needRedraw = false;

	return result;
}


bool TuningSignalState::userModified() const
{
	return m_flags.m_userModified;
}

void TuningSignalState::clearUserModified()
{
	m_flags.m_userModified = false;
}

bool TuningSignalState::writing() const
{
	return m_flags.m_writing;
}

void TuningSignalState::setWriting(bool value)
{
	if (m_flags.m_writing != value)
	{
		m_flags.m_needRedraw = true;

		m_flags.m_writing = value;

		m_writingCounter = 0;
	}
}

//
// TuningSignal
//


TuningSignal::TuningSignal()
{
}

QString TuningSignal::customAppSignalID() const
{
	return m_customAppSignalID;
}

void TuningSignal::setCustomAppSignalID(const QString& value)
{
	m_customAppSignalID = value;
}

QString TuningSignal::equipmentID() const
{
	return m_equipmentID;
}

void TuningSignal::setEquipmentID(const QString& value)
{
	m_equipmentID = value;
}

QString TuningSignal::appSignalID() const
{
	return m_appSignalID;
}

void TuningSignal::setAppSignalID(const QString& value)
{
	m_appSignalID = value;
	m_appSignalHash = ::calcHash(value);
}

QString TuningSignal::caption() const
{
	return m_caption;
}

void TuningSignal::setCaption(const QString& value)
{
	m_caption = value;
}

QString TuningSignal::units() const
{
	return m_units;
}

void TuningSignal::setUnits(const QString& value)
{
	m_units = value;
}

bool TuningSignal::analog() const
{
	return m_analog;
}

void TuningSignal::setAnalog(bool value)
{
	m_analog = value;
}

float TuningSignal::value() const
{
	return state.m_value;

}

void TuningSignal::setValue(float value)
{
	if (state.m_value != value)
	{
		state.m_flags.m_needRedraw = true;

		state.m_value = value;

		state.m_flags.m_underflow = state.m_value < m_lowLimit;

		state.m_flags.m_overflow = state.m_value > m_highLimit;
	}
}

void TuningSignal::onReceiveValue(float value, bool& writingFailed)
{
    setValue(value);

    writingFailed = false;

	if (state.m_flags.m_writing == true)
    {
		if (value == state.m_editValue)
        {
            // Reset the writing flag
            //
			state.m_flags.m_writing = false;

			state.m_writingCounter = 0;

			state.m_flags.m_needRedraw = true;
        }
        else
        {
            const int MAX_TRY_WRITE_COUNT = 10;

            // Increase the writing counter and reset it if value is wrong in MAX_TRY_WRITE_COUNT tries
            //
			state.m_writingCounter++;

			if (state.m_writingCounter > MAX_TRY_WRITE_COUNT)
            {
				state.m_flags.m_needRedraw = true;

				state.m_flags.m_writing = false;

				state.m_writingCounter = 0;

                writingFailed = true;
            }
        }
    }
}

float TuningSignal::editValue() const
{
	return state.m_editValue;
}

void TuningSignal::onEditValue(float value)
{
	state.m_editValue = value;

	if (state.m_value == state.m_editValue)
    {
		state.m_flags.m_userModified = false;
    }
    else
    {
		state.m_flags.m_userModified = true;
    }

	state.m_flags.m_needRedraw = true;
}

void TuningSignal::onSendValue(float value)
{
	state.m_editValue = value;
}

float TuningSignal::defaultValue() const
{
	return m_defaultValue;
}

void TuningSignal::setDefaultValue(float value)
{
	m_defaultValue = value;
}

float TuningSignal::lowLimit() const
{
	return m_lowLimit;
}

void TuningSignal::setLowLimit(float value)
{
	m_lowLimit = value;
}

float TuningSignal::highLimit() const
{
	return m_highLimit;
}

void TuningSignal::setHighLimit(float value)
{
	m_highLimit = value;
}

float TuningSignal::readLowLimit() const
{
    return m_readLowLimit;
}

void TuningSignal::setReadLowLimit(float value)
{
    if (m_readLowLimit != value)
    {
		state.m_flags.m_needRedraw = true;

        m_readLowLimit = value;
    }
}

float TuningSignal::readHighLimit() const
{
    return m_readHighLimit;
}

void TuningSignal::setReadHighLimit(float value)
{
    if (m_readHighLimit != value)
    {
		state.m_flags.m_needRedraw = true;

        m_readHighLimit = value;
    }
}

int TuningSignal::decimalPlaces() const
{
	return m_decimalPlaces;
}

void TuningSignal::setDecimalPlaces(int value)
{
	m_decimalPlaces = value;
}

Hash TuningSignal::appSignalHash() const
{
	return m_appSignalHash;
}


bool TuningSignal::limitsUnbalance() const
{
	if (m_analog == true && state.m_flags.m_valid == true)
    {
        return m_lowLimit != m_readLowLimit || m_highLimit != m_readHighLimit;
    }

    return false;

}

//
// TuningSignalStorage
//

TuningSignalStorage::TuningSignalStorage()
{

}


bool TuningSignalStorage::loadSignals(const QByteArray& data, QString *errorCode)
{
    if (errorCode == nullptr)
    {
        assert(errorCode);
        return false;
    }

    m_objects.clear();

    m_objectsMap.clear();

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
            std::shared_ptr<TuningSignal> object = std::make_shared<TuningSignal>();

            if (reader.attributes().hasAttribute("AppSignalID"))
            {
                object->setAppSignalID(reader.attributes().value("AppSignalID").toString());
            }

            if (reader.attributes().hasAttribute("CustomAppSignalID"))
            {
                object->setCustomAppSignalID(reader.attributes().value("CustomAppSignalID").toString());
            }

            if (reader.attributes().hasAttribute("EquipmentID"))
            {
                object->setEquipmentID(reader.attributes().value("EquipmentID").toString());
            }

            if (reader.attributes().hasAttribute("Caption"))
            {
                object->setCaption(reader.attributes().value("Caption").toString());
            }

            if (reader.attributes().hasAttribute("Type"))
            {
                QString t = reader.attributes().value("Type").toString();
                object->setAnalog(t == "A");
            }

            if (reader.attributes().hasAttribute("DecimalPlaces"))
            {
                object->setDecimalPlaces(reader.attributes().value("DecimalPlaces").toString().toInt());
            }

            if (reader.attributes().hasAttribute("DefaultValue"))
            {
                QString v = reader.attributes().value("DefaultValue").toString();
                object->setDefaultValue(v.toFloat());
            }

            if (reader.attributes().hasAttribute("LowLimit"))
            {
                QString v = reader.attributes().value("LowLimit").toString();
                object->setLowLimit(v.toFloat());
            }

            if (reader.attributes().hasAttribute("HighLimit"))
            {
                QString v = reader.attributes().value("HighLimit").toString();
                object->setHighLimit(v.toFloat());
            }


            m_objects.push_back(object);

            m_objectsMap[object->appSignalHash()] = static_cast<int>(m_objects.size()) - 1;

            continue;
        }

        reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
        *errorCode = reader.errorString();
        return !reader.hasError();
    }

    return !reader.hasError();
}

void TuningSignalStorage::invalidateSignals()
{
	for (std::shared_ptr<TuningSignal>& object : m_objects)
	{
		object->state.setValid(false);
	}
}


int TuningSignalStorage::objectCount() const
{
    return static_cast<int>(m_objects.size());

}

bool TuningSignalStorage::objectExists(Hash hash) const
{
    return (m_objectsMap.find(hash) != m_objectsMap.end());
}

TuningSignal* TuningSignalStorage::objectPtr(int index) const
{
    if (index < 0 || index >= m_objects.size())
    {
        assert(false);
        return nullptr;
    }

    return m_objects[index].get();
}

TuningSignal* TuningSignalStorage::objectPtrByHash(Hash hash) const
{
    const auto it = m_objectsMap.find(hash);

    if (it == m_objectsMap.end())
    {
        assert(false);
        return nullptr;
    }

    return objectPtr(it->second);

}

