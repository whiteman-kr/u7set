#include "TuningObject.h"

#include<memory>

TuningObject::TuningObject()
	:
	  m_value(0.0),
	  m_editValue(0.0),
	  m_defaultValue(0.0),
	  m_lowLimit(0.0),
	  m_highLimit(0.0),
	  m_decimalPlaces(0),
	  m_valid(false),
	  m_underflow(false),
	  m_overflow(false),
	  m_analog(false),
	  m_appSignalHash(0)
{


}

QString TuningObject::customAppSignalID() const
{
	return m_customAppSignalID;
}

void TuningObject::setCustomAppSignalID(const QString& value)
{
	m_customAppSignalID = value;
}

QString TuningObject::equipmentID() const
{
	return m_equipmentID;
}

void TuningObject::setEquipmentID(const QString& value)
{
	m_equipmentID = value;
}

QString TuningObject::appSignalID() const
{
	return m_appSignalID;
}

void TuningObject::setAppSignalID(const QString& value)
{
	m_appSignalID = value;
	m_appSignalHash = ::calcHash(value);
}

QString TuningObject::caption() const
{
	return m_caption;
}

void TuningObject::setCaption(const QString& value)
{
	m_caption = value;
}

QString TuningObject::units() const
{
	return m_units;
}

void TuningObject::setUnits(const QString& value)
{
	m_units = value;
}

bool TuningObject::analog() const
{
	return m_analog;
}

void TuningObject::setAnalog(bool value)
{
	m_analog = value;
}

float TuningObject::value() const
{
	return m_value;

}

void TuningObject::setValue(float value)
{
    if (m_value != value)
    {
        m_redraw = true;

        m_value = value;

        m_underflow = m_value < m_lowLimit;

        m_overflow = m_value > m_highLimit;
    }
}

void TuningObject::onReceiveValue(float value, bool& writingFailed)
{
    setValue(value);

    writingFailed = false;

    if (m_writing == true)
    {
        if (value == m_editValue)
        {
            // Reset the writing flag
            //
            m_writing = false;

            m_writingCounter = 0;

            m_redraw = true;
        }
        else
        {
            const int MAX_TRY_WRITE_COUNT = 10;

            // Increase the writing counter and reset it if value is wrong in MAX_TRY_WRITE_COUNT tries
            //
            m_writingCounter++;

            if (m_writingCounter > MAX_TRY_WRITE_COUNT)
            {
                m_redraw = true;

                m_writing = false;

                m_writingCounter = 0;

                writingFailed = true;
            }
        }
    }
}

float TuningObject::editValue() const
{
	return m_editValue;
}

void TuningObject::onEditValue(float value)
{
    m_editValue = value;

    if (m_value == m_editValue)
    {
        m_userModified = false;
    }
    else
    {
        m_userModified = true;
    }

    m_redraw = true;
}

void TuningObject::onSendValue(float value)
{
    m_editValue = value;
}

float TuningObject::defaultValue() const
{
	return m_defaultValue;
}

void TuningObject::setDefaultValue(float value)
{
	m_defaultValue = value;
}

float TuningObject::lowLimit() const
{
	return m_lowLimit;
}

void TuningObject::setLowLimit(float value)
{
	m_lowLimit = value;
}

float TuningObject::highLimit() const
{
	return m_highLimit;
}

void TuningObject::setHighLimit(float value)
{
	m_highLimit = value;
}

float TuningObject::readLowLimit() const
{
    return m_readLowLimit;
}

void TuningObject::setReadLowLimit(float value)
{
    if (m_readLowLimit != value)
    {
        m_redraw = true;

        m_readLowLimit = value;
    }
}

float TuningObject::readHighLimit() const
{
    return m_readHighLimit;
}

void TuningObject::setReadHighLimit(float value)
{
    if (m_readHighLimit != value)
    {
        m_redraw = true;

        m_readHighLimit = value;
    }
}

int TuningObject::decimalPlaces() const
{
	return m_decimalPlaces;
}

void TuningObject::setDecimalPlaces(int value)
{
	m_decimalPlaces = value;
}

bool TuningObject::valid() const
{
    //return true;
    return m_valid;
}

void TuningObject::setValid(bool value)
{
    if (m_valid != value)
    {
        m_redraw = true;

        m_valid = value;
    }

}

bool TuningObject::underflow() const
{
	return m_underflow;
}

bool TuningObject::overflow() const
{
	return m_overflow;
}

Hash TuningObject::appSignalHash() const
{
	return m_appSignalHash;
}

bool TuningObject::redraw()
{
    bool result = m_redraw;

    m_redraw = false;

    return result;
}


bool TuningObject::userModified() const
{
    return m_userModified;
}

void TuningObject::clearUserModified()
{
    m_userModified = false;
}

bool TuningObject::writing() const
{
    return m_writing;
}

void TuningObject::setWriting(bool value)
{
    if (m_writing != value)
    {
        m_redraw = true;

        m_writing = value;

        m_writingCounter = 0;
    }
}

bool TuningObject::limitsUnbalance() const
{
    if (m_analog == true && m_valid == true)
    {
        return m_lowLimit != m_readLowLimit || m_highLimit != m_readHighLimit;
    }

    return false;

}

//
// TuningObjectStorage
//

TuningObjectStorage::TuningObjectStorage()
{

}


bool TuningObjectStorage::loadSignals(const QByteArray& data, QString *errorCode)
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
            std::shared_ptr<TuningObject> object = std::make_shared<TuningObject>();

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

            m_objectsMap[object->appSignalHash()] = (int)m_objects.size() - 1;

            continue;
        }

        reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
        *errorCode = reader.errorString();
        return !reader.hasError();
    }

    return !reader.hasError();
}

void TuningObjectStorage::invalidateSignals()
{
    int count = (int)m_objects.size();
    for (int i = 0; i < count; i++)
    {
        std::shared_ptr<TuningObject> object = m_objects[i];

        object->setValid(false);
    }

}


int TuningObjectStorage::objectCount() const
{
    return (int)m_objects.size();

}

bool TuningObjectStorage::objectExists(Hash hash) const
{
    return (m_objectsMap.find(hash) != m_objectsMap.end());
}

TuningObject* TuningObjectStorage::objectPtr(int index) const
{
    if (index < 0 || index >= m_objects.size())
    {
        assert(false);
        return nullptr;
    }

    return m_objects[index].get();
}

TuningObject* TuningObjectStorage::objectPtrByHash(Hash hash) const
{
    const auto it = m_objectsMap.find(hash);

    if (it == m_objectsMap.end())
    {
        assert(false);
        return nullptr;
    }

    return objectPtr(it->second);

}

/*std::vector<std::shared_ptr<> TuningObjectStorage::objects()
{
    return m_objects;
}

std::map<Hash, int> TuningObjectStorage::objectsMap()
{
    return m_objectsMap;
}
*/
