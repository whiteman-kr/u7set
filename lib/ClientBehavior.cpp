#include "ClientBehavior.h"

//
// ClientBehavior
//
ClientBehavior::ClientBehavior()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "BehaviorID", true, behaviorId, setBehaviorId);
}

ClientBehavior::ClientBehavior(const ClientBehavior& src) noexcept :
	m_behaviorId(src.behaviorId())
{
}

ClientBehavior::ClientBehavior(ClientBehavior&& src) noexcept :
	//PropertyObject(std::move(src)),		// PropertyObject cannot be moved
	m_behaviorId(std::move(src.behaviorId()))
{
}

ClientBehavior::~ClientBehavior()
{
}

ClientBehavior& ClientBehavior::operator=(const ClientBehavior& src)
{
	m_behaviorId = src.m_behaviorId;
	return *this;
}

ClientBehavior& ClientBehavior::operator=(ClientBehavior&& src)
{
	// PropertyObject::operator=(std::move(src)); // Property object class cannot be moved
	m_behaviorId = std::move(src.m_behaviorId);
	return *this;
}

bool ClientBehavior::isMonitorBehavior() const
{
	return dynamic_cast<const MonitorBehavior*>(this) != nullptr;
}

bool ClientBehavior::isTuningClientBehavior() const
{
	return dynamic_cast<const TuningClientBehavior*>(this) != nullptr;
}

const QString& ClientBehavior::behaviorId() const
{
	return m_behaviorId;
}

void ClientBehavior::setBehaviorId(const QString& id)
{
	m_behaviorId = id;
}

void ClientBehavior::save(QXmlStreamWriter& writer)
{
	writer.writeAttribute("ID", behaviorId());
	saveToXml(writer);
	return;
}

bool ClientBehavior::load(QXmlStreamReader& reader)
{
	if (reader.attributes().hasAttribute("ID"))
	{
		setBehaviorId(reader.attributes().value("ID").toString());
	}

	if (behaviorId().isEmpty())
	{
		setBehaviorId(("ID"));
	}

	return loadFromXml(reader);
}

//
// MonitorBehavior
//
const QString MonitorBehavior::criticalTag{"critical"};
const QString MonitorBehavior::attentionTag{"attention"};
const QString MonitorBehavior::generalTag{"general"};

MonitorBehavior::MonitorBehavior()
{
	m_tagToColor.reserve(3);
	m_tagToColor.push_back({criticalTag, QRgb(0xD00000)});
	m_tagToColor.push_back({attentionTag, QRgb(0xF0F000)});
	m_tagToColor.push_back({generalTag, QRgb(0x0F0FF0)});

	auto prop = ADD_PROPERTY_GETTER_SETTER(QColor, criticalTag, true, tagCriticalToColor, setTagCriticalToColor);
	prop->setCategory("Tag to Color");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, attentionTag, true, tagAttentionToColor, setTagAttentionToColor);
	prop->setCategory("Tag to Color");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, generalTag, true, tagGeneralToColor, setTagGeneralToColor);
	prop->setCategory("Tag to Color");

	return;
}

MonitorBehavior::MonitorBehavior(const MonitorBehavior& src) noexcept :
	ClientBehavior(src),
	m_tagToColor(src.m_tagToColor)
{
}

MonitorBehavior& MonitorBehavior::operator=(const MonitorBehavior& src)
{
	ClientBehavior::operator= (src);
	m_tagToColor = src.m_tagToColor;
	return *this;
}

MonitorBehavior& MonitorBehavior::operator=(MonitorBehavior&& src)
{
	ClientBehavior::operator=(std::move(src));
	m_tagToColor = std::move(src.m_tagToColor);
	return *this;
}

QColor MonitorBehavior::tagCriticalToColor() const
{
	auto result = tagToColor(criticalTag);
	return result.has_value() ? result.value() : QColor{};
}

void MonitorBehavior::setTagCriticalToColor(const QColor& color)
{
	setTagToColor(criticalTag, color.rgb());
}

QColor MonitorBehavior::tagAttentionToColor() const
{
	auto result = tagToColor(attentionTag);
	return result.has_value() ? result.value() : QColor{};
}

void MonitorBehavior::setTagAttentionToColor(const QColor& color)
{
	setTagToColor(attentionTag, color.rgb());
}

QColor MonitorBehavior::tagGeneralToColor() const
{
	auto result = tagToColor(generalTag);
	return result.has_value() ? result.value() : QColor{};
}

void MonitorBehavior::setTagGeneralToColor(const QColor& color)
{
	setTagToColor(generalTag, color.rgb());
}

std::optional<QRgb> MonitorBehavior::tagToColor(const QString& tag) const
{
	std::optional<QRgb> result;

	for (const auto& ttc : m_tagToColor)
	{
		if (ttc.tag == tag)
		{
			result = ttc.color;
			break;
		}
	}

	return result;
}

void MonitorBehavior::setTagToColor(const QString& tag, QRgb color)
{
	for (auto& ttc : m_tagToColor)
	{
		if (ttc.tag == tag)
		{
			ttc.color = color;
			return;
		}
	}

	m_tagToColor.push_back({tag, color});
	return;
}

std::optional<QRgb> MonitorBehavior::tagToColor(const std::set<QString>& tags) const
{
	// Tags in m_tagToColor have priorities from the highest to the lowest.
	// So that is why regular loop is used here.
	// For future implementation:
	// for input param 'tag' if it has a lot of items and m_tagToColor quite big too,
	// is possible to make optimization, via finding interset of two tags and m_tagToColor.
	//
	std::optional<QRgb> result;

	for (const auto& ttc : m_tagToColor)	// Go through m_tagToColor as it is prioritized search
	{
		if (tags.find(ttc.tag) != tags.end())
		{
			result = ttc.color;				// The first met tag has the highest priority
			break;
		}
	}

	return result;
}

std::optional<QRgb> MonitorBehavior::tagToColor(const QStringList& tags) const
{
	std::set<QString> tagSet;
	for (const auto& t : tags)
	{
		tagSet.insert(t);
	}

	return tagToColor(tagSet);
}

void MonitorBehavior::saveToXml(QXmlStreamWriter& writer)
{
	writer.writeStartElement("SignalTagToColor");

	for (const auto& ttc : m_tagToColor)
	{
		writer.writeStartElement("Item");
		writer.writeAttribute("tag", ttc.tag);
		writer.writeAttribute("color", QColor::fromRgb(ttc.color).name());
		writer.writeEndElement();
	}

	writer.writeEndElement();
	return;
}

bool MonitorBehavior::loadFromXml(QXmlStreamReader& reader)
{
	m_tagToColor.clear();

	while (reader.readNextStartElement())
	{
		// SignalTagToColor
		//
		if(reader.name() == "SignalTagToColor")
		{
			reader.readNext();
			continue;
		}

		// Item
		//
		if(reader.name() == "Item")
		{
			QString tag;
			if (reader.attributes().hasAttribute("tag"))
			{
				tag = reader.attributes().value("tag").toString();
			}

			QColor color;
			if (reader.attributes().hasAttribute("color"))
			{
				color = QColor(reader.attributes().value("color").toString());
			}

			if (tag.isEmpty() == false && color.isValid() == true)
			{
				m_tagToColor.push_back({tag, color.rgb()});
			}

			reader.readNext();
			continue;
		}

		// Unknown tag
		//
		Q_ASSERT(false);
		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		return !reader.hasError();
	}

	reader.readNext();
	return !reader.hasError();
}

//
// TuningClientBehavior
//
TuningClientBehavior::TuningClientBehavior()
{
	m_tagToColor["defaultMismatchBackColor"] = QColor(Qt::yellow);
	m_tagToColor["defaultMismatchTextColor"] = QColor(Qt::black);
	m_tagToColor["unappliedBackColor"] = QColor(Qt::gray);
	m_tagToColor["unappliedTextColor"] = QColor(Qt::black);

	auto prop = ADD_PROPERTY_GETTER_SETTER(QColor, "DefaultMismatchBackColor", true, defaultMismatchBackColor, setDefaultMismatchBackColor);
	prop->setCategory("Appearance");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, "DefaultMismatchTextColor", true, defaultMismatchTextColor, setDefaultMismatchTextColor);
	prop->setCategory("Appearance");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, "UnappliedBackColor", true, unappliedBackColor, setUnappliedBackColor);
	prop->setCategory("Appearance");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, "UnappliedTextColor", true, defaultUnappliedTextColor, setDefaultUnappliedTextColor);
	prop->setCategory("Appearance");
}

QColor TuningClientBehavior::defaultMismatchBackColor() const
{
	return m_tagToColor.value("defaultMismatchBackColor");
}

void TuningClientBehavior::setDefaultMismatchBackColor(const QColor& color)
{
	m_tagToColor["defaultMismatchBackColor"] = color;
}

QColor TuningClientBehavior::defaultMismatchTextColor() const
{
	return m_tagToColor.value("defaultMismatchTextColor");
}

void TuningClientBehavior::setDefaultMismatchTextColor(const QColor& color)
{
	m_tagToColor["defaultMismatchTextColor"] = color;
}

QColor TuningClientBehavior::unappliedBackColor() const
{
	return m_tagToColor.value("unappliedBackColor");
}

void TuningClientBehavior::setUnappliedBackColor(const QColor& color)
{
	m_tagToColor["unappliedBackColor"] = color;
}

QColor TuningClientBehavior::defaultUnappliedTextColor() const
{
	return m_tagToColor.value("unappliedTextColor");
}

void TuningClientBehavior::setDefaultUnappliedTextColor(const QColor& color)
{
	m_tagToColor["unappliedTextColor"] = color;
}

void TuningClientBehavior::saveToXml(QXmlStreamWriter& writer)
{
	writer.writeStartElement("TagToColor");

	QHashIterator<QString, QColor> i(m_tagToColor);
	while (i.hasNext())
	{
		i.next();

		writer.writeStartElement("Item");
		writer.writeAttribute("tag", i.key());
		writer.writeAttribute("color", i.value().name());
		writer.writeEndElement();
	}

	writer.writeEndElement();
}

bool TuningClientBehavior::loadFromXml(QXmlStreamReader& reader)
{
	m_tagToColor.clear();

	//
	while (reader.readNextStartElement())
	{
		// TagToColor tag
		//
		if(reader.name() == "TagToColor")
		{
			reader.readNext();
			continue;
		}

		// Item tag
		//
		if(reader.name() == "Item")
		{
			QString tag;
			if (reader.attributes().hasAttribute("tag"))
			{
				tag = reader.attributes().value("tag").toString();
			}

			QColor color;
			if (reader.attributes().hasAttribute("color"))
			{
				color = QColor(reader.attributes().value("color").toString());
			}

			if (tag.isEmpty() == false && color.isValid() == true)
			{
				m_tagToColor[tag] = color;
			}

			reader.readNext();
			continue;
		}

		// Unknown tag
		//
		Q_ASSERT(false);
		reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
		return !reader.hasError();
	}

	reader.readNext();
	return !reader.hasError();
}

//
// ClientBehaviorStorage
//
ClientBehaviorStorage::ClientBehaviorStorage()
{
}

ClientBehaviorStorage::ClientBehaviorStorage(const ClientBehaviorStorage& src)
{
	ClientBehaviorStorage::operator=(src);
	return;
}

ClientBehaviorStorage::ClientBehaviorStorage(ClientBehaviorStorage&& src) noexcept :
	m_behaviors(std::move(src.m_behaviors)),
	m_fileName(std::move(src.m_fileName))
{
}

ClientBehaviorStorage& ClientBehaviorStorage::operator=(const ClientBehaviorStorage& src)
{
	m_fileName = src.m_fileName;

	QByteArray ba;
	src.save(&ba);

	QString errorCode;
	this->load(ba, &errorCode);

	return *this;
}

ClientBehaviorStorage& ClientBehaviorStorage::operator=(ClientBehaviorStorage&& src)
{
	m_fileName = std::move(src.m_fileName);
	m_behaviors = std::move(src.m_behaviors);

	return *this;
}

QString ClientBehaviorStorage::dbFileName() const
{
	return m_fileName;
}

void ClientBehaviorStorage::add(std::shared_ptr<ClientBehavior> behavoiur)
{
	m_behaviors.push_back(behavoiur);
}

bool ClientBehaviorStorage::remove(int index)
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return false;
	}

	m_behaviors.erase(m_behaviors.begin() + index);
	return true;
}

int ClientBehaviorStorage::count() const
{
	return static_cast<int>(m_behaviors.size());
}

std::shared_ptr<ClientBehavior> ClientBehaviorStorage::get(int index) const
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return nullptr;
	}
	return m_behaviors[index];
}

std::shared_ptr<ClientBehavior> ClientBehaviorStorage::get(const QString& id) const
{
	for (auto s : m_behaviors)
	{
		if (s->behaviorId() == id)
		{
			return s;
		}
	}

	assert(false);
	return nullptr;
}

void ClientBehaviorStorage::clear()
{
	m_behaviors.clear();
}

const std::vector<std::shared_ptr<ClientBehavior>>& ClientBehaviorStorage::behaviors()
{
	return m_behaviors;
}

std::vector<std::shared_ptr<MonitorBehavior>> ClientBehaviorStorage::monitorBehaviors()
{
	std::vector<std::shared_ptr<MonitorBehavior>> result;

	for (auto b : m_behaviors)
	{
		if (b->isMonitorBehavior())
		{
			std::shared_ptr<MonitorBehavior> mb = std::dynamic_pointer_cast<MonitorBehavior>(b);
			if (mb == nullptr)
			{
				Q_ASSERT(mb);
				continue;
			}

			result.push_back(mb);
		}
	}

	return result;
}

std::vector<std::shared_ptr<TuningClientBehavior>> ClientBehaviorStorage::tuningClientBehaviors()
{
	std::vector<std::shared_ptr<TuningClientBehavior>> result;

	for (auto b : m_behaviors)
	{
		if (b->isTuningClientBehavior())
		{
			std::shared_ptr<TuningClientBehavior> cb = std::dynamic_pointer_cast<TuningClientBehavior>(b);
			if (cb == nullptr)
			{
				Q_ASSERT(cb);
				continue;
			}

			result.push_back(cb);
		}
	}

	return result;
}

void ClientBehaviorStorage::save(QByteArray* data) const
{
	QXmlStreamWriter writer(data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("Behavior");
	for (auto s : m_behaviors)
	{
		if (s->isMonitorBehavior())
		{
			writer.writeStartElement("MonitorBehavior");
		}
		else
		{

			if (s->isTuningClientBehavior())
			{
				writer.writeStartElement("TuningClientBehavior");
			}
			else
			{
				Q_ASSERT(false);
				writer.writeStartElement(s->metaObject()->className());
			}
		}

		s->save(writer);
		writer.writeEndElement();
	}

	writer.writeEndElement();
	writer.writeEndDocument();

	return;
}

bool ClientBehaviorStorage::load(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		Q_ASSERT(errorCode);
		return false;
	}

	QXmlStreamReader reader(data);

	clear();

	if (reader.readNextStartElement() == false)
	{
		reader.raiseError(QObject::tr("Failed to load root element."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	if (reader.name() != "Behavior")
	{
		reader.raiseError(QObject::tr("The file is not an Behavior file."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	// Read signals
	//
	while (reader.readNextStartElement())
	{
		if(reader.name() == "MonitorBehavior")
		{
			std::shared_ptr<MonitorBehavior> s = std::make_shared<MonitorBehavior>();

			if (s->load(reader) == true)
			{
				m_behaviors.push_back(s);
			}
			else
			{
				*errorCode = reader.errorString();
				return !reader.hasError();
			}
		}
		else
		{
			if(reader.name() == "TuningClientBehavior")
			{
				std::shared_ptr<TuningClientBehavior> s = std::make_shared<TuningClientBehavior>();

				if (s->load(reader) == true)
				{
					m_behaviors.push_back(s);
				}
				else
				{
					*errorCode = reader.errorString();
					return !reader.hasError();
				}
			}
			else
			{
				Q_ASSERT(false);
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				*errorCode = reader.errorString();
				return !reader.hasError();
			}
		}

		QXmlStreamReader::TokenType endToken = reader.readNext();
		if (endToken != QXmlStreamReader::EndElement)
		{
			Q_ASSERT(false);
			reader.raiseError(QObject::tr("Wrong tag type, expected EndElement: ") + reader.name().toString());
			*errorCode = reader.errorString();
			return !reader.hasError();
		}
	}

	return !reader.hasError();
}

