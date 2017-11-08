#include "TuningFilter.h"
#include "../lib/Types.h"

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

TuningValue TuningFilterValue::value() const
{
	return m_value;
}

void TuningFilterValue::setValue(TuningValue value)
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

	if (reader.attributes().hasAttribute("ValueType"))
	{
		TuningValue tv;
		tv.type = static_cast<TuningValueType>(reader.attributes().value("ValueType").toInt());
		switch (tv.type)
		{
		case TuningValueType::Discrete:
		case TuningValueType::SignedInteger:
			tv.intValue = reader.attributes().value("ValueInt").toInt();
			break;
		case TuningValueType::Float:
			tv.floatValue = reader.attributes().value("ValueFloat").toFloat();
			break;
		case TuningValueType::Double:
			tv.doubleValue = reader.attributes().value("ValueDouble").toDouble();
			break;
		default:
			assert(false);
			return false;
		}

		setValue(tv);
	}

	return true;
}

bool TuningFilterValue::save(QXmlStreamWriter& writer) const
{
	writer.writeStartElement("Value");
	writer.writeAttribute("AppSignalId", m_appSignalId);
	writer.writeAttribute("UseValue", m_useValue ? "true" : "false");

	writer.writeAttribute("ValueType", QString::number(static_cast<int>(m_value.type)));

	switch (m_value.type)
	{
	case TuningValueType::Discrete:
	case TuningValueType::SignedInteger:
		writer.writeAttribute("ValueInt", QString::number(m_value.intValue));
		break;
	case TuningValueType::Float:
		writer.writeAttribute("ValueFloat", QString::number(m_value.floatValue));
		break;
	case TuningValueType::Double:
		writer.writeAttribute("ValueDouble", QString::number(m_value.doubleValue));
		break;
	default:
		assert(false);
		return false;
	}

	writer.writeEndElement();

	return true;
}

//
// DialogCheckFilterSignals
//

DialogCheckFilterSignals::DialogCheckFilterSignals(std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{

	QVBoxLayout* mainLayout = new QVBoxLayout();

	QTextEdit* edit = new QTextEdit();

	QString text = tr("<font size=\"4\">Errors have been occured while loading the database:<br><br>");

	for (const std::pair<QString, QString>& p: notFoundSignalsAndFilters)
	{
		QString msg = tr("Signal with AppSignalID <font color=\"red\">'%1'</font> was not found in the preset '%2'.").arg(p.first).arg(p.second);

		text += msg + "<br>";
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
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, TuningFilter::caption, TuningFilter::setCaption);
	ADD_PROPERTY_GETTER_SETTER(SignalType, "SignalType", true, TuningFilter::signalType, TuningFilter::setSignalType);

	//ADD_PROPERTY_GETTER_SETTER(QString, "ID", true, TuningFilter::ID, TuningFilter::setID);
	ADD_PROPERTY_GETTER(QString, "ID", true, TuningFilter::ID);

	ADD_PROPERTY_GETTER_SETTER(QString, "CustomID", true, TuningFilter::customID, TuningFilter::setCustomID);

	//ADD_PROPERTY_GETTER_SETTER(InterfaceType, "InterfaceType", true, TuningFilter::interfaceType, TuningFilter::setInterfaceType);
	ADD_PROPERTY_GETTER(InterfaceType, "InterfaceType", true, TuningFilter::interfaceType);

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

TuningFilter::TuningFilter(InterfaceType interfaceType)
	:TuningFilter()
{
	m_interfaceType = interfaceType;
}

TuningFilter& TuningFilter::operator=(const TuningFilter& That)
{
	copy(That);

	return* this;
}

void TuningFilter::copy(const TuningFilter& That)
{
	m_ID = That.m_ID;
	m_customID = That.m_customID;
	m_caption = That.m_caption;

	m_source = That.m_source;

	m_customAppSignalIDMasks = That.m_customAppSignalIDMasks;
	m_equipmentIDMasks = That.m_equipmentIDMasks;
	m_appSignalIDMasks = That.m_appSignalIDMasks;

	m_signalValuesMap = That.m_signalValuesMap;
	m_signalValuesVec = That.m_signalValuesVec;

	m_interfaceType = That.m_interfaceType;
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

bool TuningFilter::load(QXmlStreamReader& reader)
{
	if (isRoot() == false)
	{
		if (reader.attributes().hasAttribute("StrID"))  // This is for reading obsolete files!!!
		{
			setID(reader.attributes().value("StrID").toString());
		}

		if (reader.attributes().hasAttribute("ID"))
		{
			setID(reader.attributes().value("ID").toString());
		}

		if (reader.attributes().hasAttribute("CustomID"))
		{
			setCustomID(reader.attributes().value("CustomID").toString());
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

		if (reader.attributes().hasAttribute("Source"))
		{
			QString v = reader.attributes().value("Source").toString();
			if (v == "Project")
			{
				setSource(Source::Project);
			}
			else
			{
				if (v == "Schema")
				{
					setSource(Source::Schema);
				}
				else
				{
					if (v == "Equipment")
					{
						setSource(Source::Equipment);
					}
					else
					{
						if (v == "User")
						{
							setSource(Source::User);
						}
						else
						{
							reader.raiseError(tr("Unknown Source value: %1").arg(v));
							return false;
						}
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
				TuningFilter::InterfaceType filterType = TuningFilter::InterfaceType::Tree;

				if (tagName == "Tab")
				{
					filterType = TuningFilter::InterfaceType::Tab;
				}

				if (tagName == "Button")
				{
					filterType = TuningFilter::InterfaceType::Button;
				}

				std::shared_ptr<TuningFilter> of = std::make_shared<TuningFilter>(filterType);

				if (of->load(reader) == false)
				{
					return false;
				}

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
	if (isSourceUser() == false && isSourceProject() == false)
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

	writer.writeAttribute("ID", ID());
	writer.writeAttribute("CustomID", customID());
	writer.writeAttribute("Caption", caption());

	writer.writeAttribute("BackColor", backColor().name());
	writer.writeAttribute("TextColor", textColor().name());

	writer.writeAttribute("CustomAppSignalIDMask", customAppSignalIDMask());
	writer.writeAttribute("EquipmentIDMask", equipmentIDMask());
	writer.writeAttribute("AppSignalIDMask", appSignalIDMask());

	writer.writeAttribute("SignalType", E::valueToString<SignalType>(static_cast<int>(signalType())));
	writer.writeAttribute("Source", E::valueToString<Source>(static_cast<int>(source())));

	writer.writeStartElement("Values");

	std::vector <TuningFilterValue> valuesList = getValues();
	for (const TuningFilterValue& v : valuesList)
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

QString TuningFilter::ID() const
{
	return m_ID;
}

void TuningFilter::setID(const QString& value)
{
	m_ID = value;
}

QString TuningFilter::customID() const
{
	return m_customID;
}

void TuningFilter::setCustomID(const QString& value)
{
	m_customID = value;
}

QString TuningFilter::caption() const
{
	return m_caption;
}

void TuningFilter::setCaption(const QString& value)
{
	m_caption = value;
}

bool TuningFilter::isSourceProject() const
{
	return m_source == Source::Project;
}

bool TuningFilter::isSourceEquipment() const
{
	return m_source == Source::Equipment;
}

bool TuningFilter::isSourceSchema() const
{
	return m_source == Source::Schema;
}

bool TuningFilter::isSourceUser() const
{
	return m_source == Source::User;
}

TuningFilter::Source TuningFilter::source() const
{
	return m_source;
}


void TuningFilter::setSource(Source value)
{
	m_source = value;
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


std::vector <TuningFilterValue> TuningFilter::getValues() const
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

	if (value.useValue() == false)
	{
		return false;
	}

	return true;

}

void TuningFilter::setValue(const TuningFilterValue& value)
{
	auto it = m_signalValuesMap.find(value.appSignalHash());

	if (it == m_signalValuesMap.end())
	{
		assert(false);
		return;
	}

	TuningFilterValue& ofv = it->second;
	ofv.setUseValue(value.useValue());
	ofv.setValue(value.value());
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
	return static_cast<int>(m_signalValuesVec.size());
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

TuningFilter::InterfaceType TuningFilter::interfaceType() const
{
	return m_interfaceType;
}

void TuningFilter::setInterfaceType(InterfaceType value)
{
	m_interfaceType = value;
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
	return interfaceType() == InterfaceType::Root;
}

bool TuningFilter::isTree() const
{
	return interfaceType() == InterfaceType::Tree;
}

bool TuningFilter::isTab() const
{
	return interfaceType() == InterfaceType::Tab;
}

bool TuningFilter::isButton() const
{
	return interfaceType() == InterfaceType::Button;
}

void TuningFilter::addTopChild(const std::shared_ptr<TuningFilter>& child)
{
	child->m_parentFilter = this;
	m_childFilters.insert(m_childFilters.begin(), child);
}

void TuningFilter::addChild(const std::shared_ptr<TuningFilter>& child)
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

bool TuningFilter::removeChild(const QString& ID)
{
	bool found = false;

	for (auto it = m_childFilters.begin(); it != m_childFilters.end(); it++)
	{
		if (it->get()->ID() == ID)
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

void TuningFilter::removeChildren(Source sourceType)
{
	std::vector<std::shared_ptr<TuningFilter>> childFiltersCopy;

	childFiltersCopy = m_childFilters;

	m_childFilters.clear();

	for (auto it = childFiltersCopy.begin(); it != childFiltersCopy.end(); it++)
	{
		std::shared_ptr<TuningFilter>& filter =* it;

		if (filter->source() != sourceType)
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


bool TuningFilter::match(const AppSignalParam& object) const
{
	if (isEmpty() == true)
	{
		return true;
	}

	if (signalType() == TuningFilter::SignalType::Analog && object.isAnalog() == false)
	{
		return false;
	}
	if (signalType() == TuningFilter::SignalType::Discrete && object.isAnalog() == true)
	{
		return false;
	}

	// List of appSignalId
	//
	if (m_signalValuesVec.empty() == false)
	{
		if (valueExists(object.hash()) == false)
		{
			return false;
		}
	}
	// Mask for equipmentID
	//

	if (m_equipmentIDMasks.isEmpty() == false)
	{

		QString s = object.equipmentId();

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

		QString s = object.appSignalId();

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
		QString s = object.customSignalId();

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

	TuningFilter* parent = parentFilter();

	if (parent != nullptr)
	{
		return parent->match(object);
	}
	else
	{
		return true;
	}
}

void TuningFilter::checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters)
{
	for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
	{
		const Hash& hash = it->first;
		const TuningFilterValue& value = it->second;

		if (find(signalHashes.begin(), signalHashes.end(), hash) == signalHashes.end())
		{
			notFoundSignalsAndFilters.push_back(std::make_pair<QString, QString>(value.appSignalId(), caption()));
		}
	}

	int childCount = static_cast<int>(m_childFilters.size());
	for (int i = 0; i < childCount; i++)
	{
		m_childFilters[i]->checkSignals(signalHashes, notFoundSignalsAndFilters);
	}
}

void TuningFilter::removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter)
{
	std::vector<Hash> valuesToDelete;

	for (auto it = m_signalValuesMap.begin(); it != m_signalValuesMap.end(); it++)
	{
		const Hash& hash = it->first;

		if (find(signalHashes.begin(), signalHashes.end(), hash) == signalHashes.end())
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

	int childCount = static_cast<int>(m_childFilters.size());
	for (int i = 0; i < childCount; i++)
	{
		m_childFilters[i]->removeNotExistingSignals(signalHashes, removedCounter);
	}
}

//
// ObjectFilterStorage
//

TuningFilterStorage::TuningFilterStorage()
{
	m_root = std::make_shared<TuningFilter>();
	m_root->setID("%FILTER%ROOT");
	m_root->setCaption(QObject::tr("All Signals"));
	m_root->setInterfaceType(TuningFilter::InterfaceType::Root);

}

TuningFilterStorage::TuningFilterStorage(const TuningFilterStorage& That)
{
	*this = That;
}

bool TuningFilterStorage::load(const QString& fileName, QString* errorCode)
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

bool TuningFilterStorage::load(const QByteArray &data, QString* errorCode)
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
			if (m_root->load(reader) == false)
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

bool TuningFilterStorage::save(QByteArray& data)
{
    QXmlStreamWriter writer(&data);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("ObjectFilterStorage");

    m_root->save(writer);

    writer.writeEndElement();

    writer.writeEndElement();	// ObjectFilterStorage

    writer.writeEndDocument();

    return true;

}

bool TuningFilterStorage::save(const QString& fileName, QString* errorMsg)
{
	// save data to XML
	//

    QByteArray data;

    bool ok = save(data);

    if (ok == false)
    {
        *errorMsg = QObject::tr("TuningFilterStorage::save: failed to save presets QByteArray.");
        return false;
    }

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

	TuningFilter root(TuningFilter::InterfaceType::Root);

	for (auto filter : filters)
	{
		std::shared_ptr<TuningFilter> filterCopy = std::make_shared<TuningFilter>();

		* filterCopy =* filter;

		root.addChild(filterCopy);
	}

	root.save(writer);

	writer.writeEndElement();

	writer.writeEndElement();	// ObjectFilterStorage

	writer.writeEndDocument();

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(data.toStdString().c_str());

	return true;
}

std::shared_ptr<TuningFilter> TuningFilterStorage::pasteFromClipboard()
{
	QClipboard* clipboard = QApplication::clipboard();
	QString clipboardText = clipboard->text();

	if (clipboardText.isEmpty() == true)
	{
		return nullptr;
	}

	TuningFilterStorage clipboardStorage;

	QByteArray data = clipboardText.toUtf8();

	QString errorMsg;

	bool ok = clipboardStorage.load(data, &errorMsg);
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

VFrame30::SchemaDetails  TuningFilterStorage::schemaDetails(int index)
{
	if (index < 0 || index >= m_schemasDetails.size())
	{
		assert(false);
		return VFrame30::SchemaDetails();
	}
	return m_schemasDetails[index];
}


bool TuningFilterStorage::loadSchemasDetails(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_schemasDetails.clear();

	VFrame30::SchemaDetailsSet detailsSet;

	if (detailsSet.Load(data) == false)
	{
		*errorCode = QObject::tr("Failed to parse SchemaDetailsSet.");
		return false;
	}

	m_schemasDetails = detailsSet.schemasDetails();

	return true;

}

void TuningFilterStorage::createAutomaticFilters(const TuningSignalManager* objects, bool bySchemas, bool byEquipment, const QStringList& tuningSourcesEquipmentIds)
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
		std::shared_ptr<TuningFilter> ofSchema = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
		ofSchema->setID("%AUTOFILTER%_SCHEMA");
		ofSchema->setCaption(QObject::tr("Schemas"));
		ofSchema->setSource(TuningFilter::Source::Schema);

		for (const VFrame30::SchemaDetails& schemasDetails : m_schemasDetails)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			for (const QString& appSignalID : schemasDetails.m_signals)
			{
				// find if this signal is a tuning signal
				//
				Hash hash = ::calcHash(appSignalID);

				if (objects->signalExists(hash) == false)
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

			ofTs->setID("%AUFOFILTER%_SCHEMA_" + schemasDetails.m_schemaId);

			//QString s = QString("%1 - %2").arg(schemasDetails.m_Id).arg(schemasDetails.m_caption);
			ofTs->setCaption(schemasDetails.m_caption);
			ofTs->setSource(TuningFilter::Source::Schema);

			ofSchema->addChild(ofTs);
		}

		m_root->addTopChild(ofSchema);
	}

	if (byEquipment == true)
	{
		m_root->removeChild("%AUTOFILTER%_EQUIPMENT");

		// Filter for EquipmentId
		//
		std::shared_ptr<TuningFilter> ofEquipment = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
		ofEquipment->setID("%AUTOFILTER%_EQUIPMENT");
		ofEquipment->setCaption(QObject::tr("Equipment"));
		ofEquipment->setSource(TuningFilter::Source::Equipment);

		for (const QString& ts : tuningSourcesEquipmentIds)
		{
			std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofTs->setEquipmentIDMask(ts);
			ofTs->setID("%AUFOFILTER%_EQUIPMENT_" + ts);
			ofTs->setCaption(ts);
			ofTs->setSource(TuningFilter::Source::Equipment);

			ofEquipment->addChild(ofTs);
		}

		m_root->addTopChild(ofEquipment);
	}
}

void TuningFilterStorage::removeFilters(TuningFilter::Source sourceType)
{
	m_root->removeChildren(sourceType);

}

void TuningFilterStorage::checkFilterSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters)
{
	m_root->checkSignals(signalHashes, notFoundSignalsAndFilters);
}

void TuningFilterStorage::checkAndRemoveFilterSignals(const std::vector<Hash>& signalHashes, bool& removedNotFound, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parentWidget)
{
	removedNotFound = false;

	m_root->checkSignals(signalHashes, notFoundSignalsAndFilters);

	if (notFoundSignalsAndFilters.empty() == true)
	{
		return;
	}

	DialogCheckFilterSignals d(notFoundSignalsAndFilters, parentWidget);
	if (d.exec() == QDialog::Accepted)
	{
		int removedCounter = 0;

		m_root->removeNotExistingSignals(signalHashes, removedCounter);

		removedNotFound = true;

		QMessageBox::warning(parentWidget, qApp->applicationName(), QObject::tr("%1 signals have been removed.").arg(removedCounter));
	}

}

void TuningFilterStorage::writeLogError(const QString& message)
{
	Q_UNUSED(message);
}

void TuningFilterStorage::writeLogWarning(const QString& message)
{
	Q_UNUSED(message);
}

void TuningFilterStorage::writeLogMessage(const QString& message)
{
	Q_UNUSED(message);
}


