#include "Stable.h"
#include "TuningFilter.h"
#include "../lib/Types.h"

#include <algorithm>    // std::find

TuningFilterValue::TuningFilterValue()
{

}

QString TuningFilterValue::appSignalId() const
{
	return m_appSignalId;
}

void TuningFilterValue::setAppSignalId(const QString& value)
{
	m_appSignalId = value;
    m_appSignalHash = ::calcHash(m_appSignalId);
}

bool TuningFilterValue::useValue() const
{
	return m_useValue;
}

void TuningFilterValue::setUseValue(bool value)
{
	m_useValue = value;
}

float TuningFilterValue::value() const
{
	return m_value;
}

void TuningFilterValue::setValue(float value)
{
	m_value = value;
}

Hash TuningFilterValue::appSignalHash() const
{
    return m_appSignalHash;
}

bool TuningFilterValue::load(QXmlStreamReader& reader)
{
    if (reader.attributes().hasAttribute("AppSignalId"))
    {
        setAppSignalId(reader.attributes().value("AppSignalId").toString());
    }

    if (reader.attributes().hasAttribute("UseValue"))
    {
        setUseValue(reader.attributes().value("UseValue").toString() == "true");
    }

    if (reader.attributes().hasAttribute("Value"))
    {
        setValue(reader.attributes().value("Value").toFloat());
    }

    return true;
}

bool TuningFilterValue::save(QXmlStreamWriter& writer) const
{
    writer.writeStartElement("Value");
    writer.writeAttribute("AppSignalId", appSignalId());
    writer.writeAttribute("UseValue", useValue() ? "true" : "false");
    writer.writeAttribute("Value", QString::number(value()));
    writer.writeEndElement();

    return true;
}

//
// DialogCheckFilterSignals
//

DialogCheckFilterSignals::DialogCheckFilterSignals(QStringList& errorLog, QWidget* parent)
    :QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{

    QVBoxLayout* mainLayout = new QVBoxLayout();

    QTextEdit* edit = new QTextEdit();

    QString text = tr("<font size=\"4\">Errors have been occured while loading the database:<br><br>");

    for (auto s : errorLog)
    {
        text += s + "<br>";
    }

    text += tr("<br>Do you wish to remove these signals from presets?</font>");

    edit->setText(text);

    edit->setReadOnly(true);


    QPushButton* yesButton = new QPushButton(tr("Yes"));
    yesButton->setAutoDefault(false);

    QPushButton* noButton = new QPushButton(tr("No"));
    noButton->setDefault(true);

    m_buttonBox = new QDialogButtonBox();

    m_buttonBox->addButton(yesButton, QDialogButtonBox::YesRole);
    m_buttonBox->addButton(noButton, QDialogButtonBox::NoRole);

    m_buttonBox->setFocus();

    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &DialogCheckFilterSignals::buttonClicked);

    mainLayout->addWidget(edit);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);

    resize(800, 400);
}

void DialogCheckFilterSignals::buttonClicked(QAbstractButton* button)
{
    if (button == nullptr)
    {
        assert(button);
        return;
    }

    QDialogButtonBox::ButtonRole role = m_buttonBox->buttonRole(button);

    if (role == QDialogButtonBox::ButtonRole::YesRole)
    {
        accept();
    }

    if (role == QDialogButtonBox::ButtonRole::NoRole)
    {
        reject();
    }

}

//
// TuningFilter
//

TuningFilter::TuningFilter()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "StrID", true, TuningFilter::strID, TuningFilter::setStrID);
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, TuningFilter::caption, TuningFilter::setCaption);
	ADD_PROPERTY_GETTER_SETTER(SignalType, "SignalType", true, TuningFilter::signalType, TuningFilter::setSignalType);

	auto propFilterType = ADD_PROPERTY_GETTER(FilterType, "FilterType", true, TuningFilter::filterType);
	propFilterType->setCategory("Debug");

	auto propMask = ADD_PROPERTY_GETTER_SETTER(QString, "CustomAppSignalMasks", true, TuningFilter::customAppSignalIDMask, TuningFilter::setCustomAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "AppSignalMasks", true, TuningFilter::appSignalIDMask, TuningFilter::setAppSignalIDMask);
	propMask->setCategory("Masks");

	propMask = ADD_PROPERTY_GETTER_SETTER(QString, "EquipmentIDMasks", true, TuningFilter::equipmentIDMask, TuningFilter::setEquipmentIDMask);
	propMask->setCategory("Masks");

    auto propBackColor = ADD_PROPERTY_GETTER_SETTER(QColor, "BackColor", true, TuningFilter::backColor, TuningFilter::setBackColor);
    propBackColor->setCategory("Color");

    auto propTextColor = ADD_PROPERTY_GETTER_SETTER(QColor, "TextColor", true, TuningFilter::textColor, TuningFilter::setTextColor);
    propTextColor->setCategory("Color");

}

TuningFilter::TuningFilter(const TuningFilter& That)
	:TuningFilter()
{
	copy(That);
}

TuningFilter::TuningFilter(FilterType filterType)
	:TuningFilter()
{
	m_filterType = filterType;
}

TuningFilter& TuningFilter::operator=(const TuningFilter& That)
{
	copy(That);

	return *this;
}

void TuningFilter::copy(const TuningFilter& That)
{
	m_strID = That.m_strID;
	m_caption = That.m_caption;

    m_automatic = That.m_automatic;

	m_customAppSignalIDMasks = That.m_customAppSignalIDMasks;
	m_equipmentIDMasks = That.m_equipmentIDMasks;
	m_appSignalIDMasks = That.m_appSignalIDMasks;

	m_signalValuesMap = That.m_signalValuesMap;
	m_signalValuesVec = That.m_signalValuesVec;

	m_filterType = That.m_filterType;
	m_signalType = That.m_signalType;

	for (auto f : That.m_childFilters)
	{
		TuningFilter* fi = f.get();

		std::shared_ptr<TuningFilter> fiCopy = std::make_shared<TuningFilter>(*fi);

		addChild(fiCopy);
	}
}

TuningFilter::~TuningFilter()
{

}

bool TuningFilter::load(QXmlStreamReader& reader, bool automatic)
{
    if (isRoot() == false)
    {
        if (reader.attributes().hasAttribute("StrID"))
        {
            setStrID(reader.attributes().value("StrID").toString());
        }

        if (reader.attributes().hasAttribute("Caption"))
        {
            setCaption(reader.attributes().value("Caption").toString());
        }

        if (reader.attributes().hasAttribute("BackColor"))
        {
            setBackColor(QColor(reader.attributes().value("BackColor").toString()));
        }

        if (reader.attributes().hasAttribute("TextColor"))
        {
            setTextColor(QColor(reader.attributes().value("TextColor").toString()));
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

				TuningFilterValue ofv;

                ofv.load(reader);

				addValue(ofv);

				continue;
			}

			if (tagName == "Tree" || tagName == "Tab" || tagName == "Button")
			{
				TuningFilter::FilterType filterType = TuningFilter::FilterType::Tree;

				if (tagName == "Tab")
				{
					filterType = TuningFilter::FilterType::Tab;
				}

				if (tagName == "Button")
				{
					filterType = TuningFilter::FilterType::Button;
				}

				std::shared_ptr<TuningFilter> of = std::make_shared<TuningFilter>(filterType);

                if (of->load(reader, automatic) == false)
				{
					return false;
				}

                of->setAutomatic(automatic);

				addChild(of);

				continue;
			}

            reader.raiseError(tr("Unknown tag: ") + reader.name().toString());
			return false;
		}
	}while (t != QXmlStreamReader::EndElement);


	return true;
}

bool TuningFilter::save(QXmlStreamWriter& writer) const
{
    if (automatic() == true)
    {
        return true;
    }

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

    writer.writeAttribute("BackColor", backColor().name());
    writer.writeAttribute("TextColor", textColor().name());

    writer.writeAttribute("CustomAppSignalIDMask", customAppSignalIDMask());
	writer.writeAttribute("EquipmentIDMask", equipmentIDMask());
	writer.writeAttribute("AppSignalIDMask", appSignalIDMask());

	writer.writeAttribute("SignalType", E::valueToString<SignalType>((int)signalType()));

	writer.writeStartElement("Values");

	std::vector <TuningFilterValue> values = signalValues();
	for (const TuningFilterValue& v : values)
	{
        v.save(writer);
	}
	writer.writeEndElement();


	for (auto f : m_childFilters)
	{
		f->save(writer);
	}

	writer.writeEndElement();
	return true;
}

QString TuningFilter::strID() const
{
	return m_strID;
}

void TuningFilter::setStrID(const QString& value)
{
	m_strID = value;
}

QString TuningFilter::caption() const
{
	return m_caption;
}

void TuningFilter::setCaption(const QString& value)
{
	m_caption = value;
}

bool TuningFilter::automatic() const
{
    return m_automatic;
}

void TuningFilter::setAutomatic(bool value)
{
    m_automatic = value;
}


QString TuningFilter::customAppSignalIDMask() const
{
	QString result;
	for (auto s : m_customAppSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setCustomAppSignalIDMask(const QString& value)
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

QString TuningFilter::equipmentIDMask() const
{
	QString result;
	for (auto s : m_equipmentIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setEquipmentIDMask(const QString& value)
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

QString TuningFilter::appSignalIDMask() const
{
	QString result;
	for (auto s : m_appSignalIDMasks)
	{
		result += s + ';';
	}
	result.remove(result.length() - 1, 1);

	return result;
}

void TuningFilter::setAppSignalIDMask(const QString& value)
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


std::vector <TuningFilterValue> TuningFilter::signalValues() const
{
	std::vector <TuningFilterValue> result;

	for (Hash hash : m_signalValuesVec)
	{
		result.push_back(m_signalValuesMap.at(hash));
	}

	return result;
}

void TuningFilter::setValues(const std::vector <TuningFilterValue>& values)
{
	m_signalValuesVec.clear();
	m_signalValuesMap.clear();

	for (const TuningFilterValue&  v : values)
	{
		addValue(v);
	}
}

bool TuningFilter::value(Hash hash, TuningFilterValue& value)
{
    auto it = m_signalValuesMap.find(hash);
    if (it == m_signalValuesMap.end())
    {
        return false;
    }

    value = it->second;
    return true;

}

void TuningFilter::setValue(Hash hash, float value)
{
	auto it = m_signalValuesMap.find(hash);

	if (it == m_signalValuesMap.end())
	{
		assert(false);
		return;
	}

	TuningFilterValue& ofv = it->second;
	ofv.setUseValue(true);
	ofv.setValue(value);
}

bool TuningFilter::valueExists(Hash hash) const
{
	return m_signalValuesMap.find(hash) != m_signalValuesMap.end();
}

void TuningFilter::addValue(const TuningFilterValue& value)
{
    Hash hash = value.appSignalHash();
	if (valueExists(hash) == true)
	{
		assert(false);
		return;
	}

	m_signalValuesVec.push_back(hash);
	m_signalValuesMap[hash] = value;
}

int TuningFilter::valuesCount() const
{
    return (int)m_signalValuesVec.size();
}

void TuningFilter::removeValue(Hash hash)
{
	// remove from map
	//
	auto it = m_signalValuesMap.find(hash);

	if (it == m_signalValuesMap.end())
	{
		assert(false);
		return;
	}

	m_signalValuesMap.erase(it);

	// remove from vector
	//
	bool found = false;
	for (auto itv = m_signalValuesVec.begin(); itv != m_signalValuesVec.end(); itv++)
	{
		if (*itv == hash)
		{
			m_signalValuesVec.erase(itv);
			found = true;
			break;
		}
	}

	if (found == false)
	{
		assert(false);
		return;
	}
}

TuningFilter::FilterType TuningFilter::filterType() const
{
	return m_filterType;
}

void TuningFilter::setFilterType(FilterType value)
{
	m_filterType = value;
}

TuningFilter::SignalType TuningFilter::signalType() const
{
	return m_signalType;
}

void TuningFilter::setSignalType(SignalType value)
{
	m_signalType = value;
}

QColor TuningFilter::backColor() const
{
    return m_backColor;
}

void TuningFilter::setBackColor(const QColor& value)
{
    m_backColor = value;
}

QColor TuningFilter::textColor() const
{
    return m_textColor;
}

void TuningFilter::setTextColor(const QColor& value)
{
    m_textColor = value;
}

TuningFilter* TuningFilter::parentFilter() const
{
	return m_parentFilter;
}


bool TuningFilter::isEmpty() const
{
	if (m_signalType == SignalType::All &&
			m_signalValuesVec.empty() == true &&
			m_appSignalIDMasks.empty() == true &&
			m_customAppSignalIDMasks.empty() == true &&
            m_equipmentIDMasks.empty() == true
            )
	{
		return true;
	}

	return false;
}

bool TuningFilter::isRoot() const
{
	return filterType() == FilterType::Root;
}

bool TuningFilter::isTree() const
{
	return filterType() == FilterType::Tree;
}

bool TuningFilter::isTab() const
{
	return filterType() == FilterType::Tab;
}

bool TuningFilter::isButton() const
{
	return filterType() == FilterType::Button;
}

void TuningFilter::addTopChild(const std::shared_ptr<TuningFilter> &child)
{
	child->m_parentFilter = this;
	m_childFilters.insert(m_childFilters.begin(), child);
}

void TuningFilter::addChild(const std::shared_ptr<TuningFilter> &child)
{
	child->m_parentFilter = this;
	m_childFilters.push_back(child);
}

void TuningFilter::removeChild(const std::shared_ptr<TuningFilter>& child)
{
	bool found = false;

	for (auto it = m_childFilters.begin(); it != m_childFilters.end(); it++)
	{
		if (it->get() == child.get())
		{
			m_childFilters.erase(it);
			found = true;
			break;
		}
	}

	if (found == false)
	{
		assert(false);
		return;
	}
}

bool TuningFilter::removeChild(const QString& strID)
{
    bool found = false;

    for (auto it = m_childFilters.begin(); it != m_childFilters.end(); it++)
    {
        if (it->get()->strID() == strID)
        {
            m_childFilters.erase(it);
            found = true;
            break;
        }
    }

    return found;
}

void TuningFilter::removeAllChildren()
{
	m_childFilters.clear();

}

void TuningFilter::removeAutomaticChildren()
{
    std::vector<std::shared_ptr<TuningFilter>> childFiltersCopy;

    childFiltersCopy = m_childFilters;

    m_childFilters.clear();

    for (auto it = childFiltersCopy.begin(); it != childFiltersCopy.end(); it++)
    {
        std::shared_ptr<TuningFilter>& filter = *it;

        if (filter->automatic() == false)
        {
            m_childFilters.push_back(filter);
        }
    }
}

int TuningFilter::childFiltersCount() const
{
	return static_cast<int>(m_childFilters.size());

}

std::shared_ptr<TuningFilter> TuningFilter::childFilter(int index) const
{
	if (index <0 || index >= m_childFilters.size())
	{
		assert(false);
		return nullptr;
	}

	return m_childFilters[index];
}


bool TuningFilter::match(const TuningObject& object, bool checkValues) const
{
	if (isEmpty() == true)
	{
        return true;
	}

	if (signalType() == TuningFilter::SignalType::Analog && object.analog() == false)
	{
		return false;
	}
	if (signalType() == TuningFilter::SignalType::Discrete && object.analog() == true)
	{
		return false;
	}

	// List of appSignalId
	//
    if (checkValues == true && m_signalValuesVec.empty() == false)
	{
		if (valueExists(object.appSignalHash()) == false)
		{
			return false;
		}
	}
    // Mask for equipmentID
	//

    if (m_equipmentIDMasks.isEmpty() == false)
	{

		QString s = object.equipmentID();

		bool result = false;

		for (const QString& m : m_equipmentIDMasks)
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

		for (const QString& m : m_appSignalIDMasks)
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

	// Mask for customAppSignalID
	//

    if (m_customAppSignalIDMasks.isEmpty() == false)
	{
		QString s = object.customAppSignalID();

		bool result = false;

		for (const QString& m : m_customAppSignalIDMasks)
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

void TuningFilter::checkSignals(const TuningObjectStorage *objects, QStringList& errorLog, int &notFoundCounter)
{
    if (objects == nullptr)
    {
        assert(objects);
        return;
    }

    for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
    {
        const Hash& hash = it->first;
        const TuningFilterValue& value = it->second;

        if (objects->objectExists(hash) == false)
        {
            notFoundCounter++;

            errorLog.push_back(tr("Signal with AppSignalID <font color=\"red\">'%1'</font> was not found in the preset '%2'.").arg(value.appSignalId()).arg(caption()));
        }
    }

    int childCount = (int)m_childFilters.size();
    for (int i = 0; i < childCount; i++)
    {
        m_childFilters[i]->checkSignals(objects, errorLog, notFoundCounter);
    }
}

void TuningFilter::removeNotExistingSignals(const TuningObjectStorage *objects, int &removedCounter)
{
    if (objects == nullptr)
    {
        assert(objects);
        return;
    }

    std::vector<Hash> valuesToDelete;

    for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
    {
        const Hash& hash = it->first;

        if (objects->objectExists(hash) == false)
        {
            removedCounter++;
            valuesToDelete.push_back(hash);
        }
    }

    for (Hash hash : valuesToDelete)
    {
        auto itm = m_signalValuesMap.find(hash);
        if (itm != m_signalValuesMap.end())
        {
            m_signalValuesMap.erase(itm);
        }
        else
        {
            assert(false);
        }

        auto itv = std::find (m_signalValuesVec.begin(), m_signalValuesVec.end(), hash);
        if (itv != m_signalValuesVec.end())
        {
            m_signalValuesVec.erase(itv);
        }
        else
        {
            assert(false);
        }

    }

    int childCount = (int)m_childFilters.size();
    for (int i = 0; i < childCount; i++)
    {
        m_childFilters[i]->removeNotExistingSignals(objects, removedCounter);
    }
}

//
// ObjectFilterStorage
//

TuningFilterStorage::TuningFilterStorage()
{
	m_root = std::make_shared<TuningFilter>();
    m_root->setStrID("%FILTER%ROOT");
    m_root->setCaption(QObject::tr("All Signals"));
	m_root->setFilterType(TuningFilter::FilterType::Root);

}

TuningFilterStorage::TuningFilterStorage(const TuningFilterStorage& That)
{
	m_root = std::make_shared<TuningFilter>(*That.m_root.get());
	m_schemasDetails = That.m_schemasDetails;
}

bool TuningFilterStorage::load(const QString& fileName, QString* errorCode, bool automatic)
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

    return load(data, errorCode, automatic);

}

bool TuningFilterStorage::load(const QByteArray& data, QString* errorCode, bool automatic)
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
            if (m_root->load(reader, automatic) == false)
			{
				*errorCode = reader.errorString();
				return false;
			}

            m_root->setCaption(QObject::tr("All Signals"));

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();
}

bool TuningFilterStorage::save(const QString& fileName, QString* errorMsg)
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
        *errorMsg = QObject::tr("TuningFilterStorage::save: failed to save presets in file %1.").arg(fileName);
        return false;
    }

    f.write(data);

    return true;

}

bool TuningFilterStorage::copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters)
{
    // save data to clipboard
    //
    QByteArray data;
    QXmlStreamWriter writer(&data);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("ObjectFilterStorage");

    TuningFilter root(TuningFilter::FilterType::Root);

    for (auto filter : filters)
    {
        std::shared_ptr<TuningFilter> filterCopy = std::make_shared<TuningFilter>();

        *filterCopy = *filter;

        root.addChild(filterCopy);
    }

    root.save(writer);

    writer.writeEndElement();

    writer.writeEndElement();	// ObjectFilterStorage

    writer.writeEndDocument();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(data.toStdString().c_str());

    return true;
}

std::shared_ptr<TuningFilter> TuningFilterStorage::pasteFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString clipboardText = clipboard->text();

    if (clipboardText.isEmpty() == true)
    {
        return nullptr;
    }

    TuningFilterStorage clipboardStorage;

    QByteArray data = clipboardText.toUtf8();

    QString errorMsg;

    bool ok = clipboardStorage.load(data, &errorMsg, false);
    if (ok == false)
    {
        return nullptr;
    }

    return clipboardStorage.m_root;
}


int TuningFilterStorage::schemaDetailsCount()
{
	return static_cast<int>(m_schemasDetails.size());
}

SchemaDetails TuningFilterStorage::schemaDetails(int index)
{
	if (index < 0 || index >= m_schemasDetails.size())
	{
		assert(false);
		return SchemaDetails();
	}
	return m_schemasDetails[index];
}


bool TuningFilterStorage::loadSchemasDetails(const QByteArray& data, QString *errorCode)
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
				sd.m_appSignalIDs.reserve(array.size());
				for (int i = 0; i < array.size(); i++)
				{
					sd.m_appSignalIDs.push_back(array[i].toString());
				}

                if (sd.m_appSignalIDs.empty() == false)
                {
                    m_schemasDetails.push_back(sd);
                }
			}

			continue;
		}

		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	return !reader.hasError();

}

void TuningFilterStorage::createAutomaticFilters(const TuningObjectStorage* objects, bool bySchemas, bool byEquipment, const QStringList& tuningSourcesEquipmentIds)
{
    if (objects == nullptr)
    {
        assert(objects);
        return;
    }

    if (bySchemas == true)
	{
        m_root->removeChild("%AUTOFILTER%_SCHEMA");

		// Filter for Schema
		//
		std::shared_ptr<TuningFilter> ofSchema = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);
		ofSchema->setStrID("%AUTOFILTER%_SCHEMA");
        ofSchema->setCaption(QObject::tr("Schemas"));
        ofSchema->setAutomatic(true);

		for (const SchemaDetails& schemasDetails : m_schemasDetails)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);
			for (const QString& appSignalID : schemasDetails.m_appSignalIDs)
			{
                // find if this signal is a tuning signal
                //
                Hash hash = ::calcHash(appSignalID);

                if (objects->objectExists(hash) == false)
                {
                    continue;
                }

				TuningFilterValue ofv;
				ofv.setAppSignalId(appSignalID);
				ofTs->addValue(ofv);
			}

            if (ofTs->valuesCount() == 0)
            {
                // Do not add empty filters
                //
                continue;
            }

			ofTs->setStrID("%AUFOFILTER%_SCHEMA_" + schemasDetails.m_strId);

            //QString s = QString("%1 - %2").arg(schemasDetails.m_strId).arg(schemasDetails.m_caption);
            ofTs->setCaption(schemasDetails.m_caption);
            ofTs->setAutomatic(true);

			ofSchema->addChild(ofTs);
		}

		m_root->addTopChild(ofSchema);
	}

	if (byEquipment == true)
	{
        m_root->removeChild("%AUTOFILTER%_EQUIPMENT");

		// Filter for EquipmentId
		//
		std::shared_ptr<TuningFilter> ofEquipment = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);
		ofEquipment->setStrID("%AUTOFILTER%_EQUIPMENT");
        ofEquipment->setCaption(QObject::tr("Equipment"));
        ofEquipment->setAutomatic(true);

		for (const QString& ts : tuningSourcesEquipmentIds)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);
			ofTs->setEquipmentIDMask(ts);
			ofTs->setStrID("%AUFOFILTER%_EQUIPMENT_" + ts);
            ofTs->setCaption(ts);
            ofTs->setAutomatic(true);

			ofEquipment->addChild(ofTs);
		}

		m_root->addTopChild(ofEquipment);
	}
}

void TuningFilterStorage::removeAutomaticFilters()
{
    m_root->removeAutomaticChildren();

}

void TuningFilterStorage::checkSignals(const TuningObjectStorage *objects, bool& removedNotFound, QWidget* parentWidget)
{
    if (objects == nullptr)
    {
        assert(objects);
        return;
    }

    QStringList errorLog;

    removedNotFound = false;

    int notFoundCounter = 0;

    m_root->checkSignals(objects, errorLog, notFoundCounter);

    if (notFoundCounter == 0)
    {
        return;
    }

    DialogCheckFilterSignals d(errorLog, parentWidget);
    if (d.exec() == QDialog::Accepted)
    {
        int removedCounter = 0;

        m_root->removeNotExistingSignals(objects, removedCounter);

        removedNotFound = true;

        QMessageBox::warning(parentWidget, qApp->applicationName(), QObject::tr("%1 signals have been removed.").arg(removedCounter));
    }

}

