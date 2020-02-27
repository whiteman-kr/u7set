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
	m_tagToColors.reserve(3);
	m_tagToColors.push_back({criticalTag, std::make_pair(QRgb(0xD00000), QRgb(0xD00000))});
	m_tagToColors.push_back({attentionTag, std::make_pair(QRgb(0xF0F000), QRgb(0xF0F000))});
	m_tagToColors.push_back({generalTag, std::make_pair(QRgb(0x0F0FF0), QRgb(0x0F0FF0))});

	return;
}

MonitorBehavior::MonitorBehavior(const MonitorBehavior& src) noexcept :
	ClientBehavior(src),
	m_tagToColors(src.m_tagToColors)
{
}

MonitorBehavior& MonitorBehavior::operator=(const MonitorBehavior& src)
{
	ClientBehavior::operator= (src);
	m_tagToColors = src.m_tagToColors;
	return *this;
}

MonitorBehavior& MonitorBehavior::operator=(MonitorBehavior&& src)
{
	ClientBehavior::operator=(std::move(src));
	m_tagToColors = std::move(src.m_tagToColors);
	return *this;
}

QStringList MonitorBehavior::tags() const
{
	QStringList result;

	for (const auto& ttc : m_tagToColors)
	{
		result.push_back(ttc.tag);
	}

	return result;
}

void MonitorBehavior::setTag(int index, const QString& tag)
{
	if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
	{
		Q_ASSERT(false);
		return;
	}

	m_tagToColors[index].tag = tag;
	return;
}

bool MonitorBehavior::removeTagToColor(int index)
{
	if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
	{
		Q_ASSERT(false);
		return false;
	}

	m_tagToColors.erase(m_tagToColors.begin() + index);
	return true;
}

bool MonitorBehavior::moveTagToColor(int index, int step)
{
	if (index < 0 || index >= static_cast<int>(m_tagToColors.size()))
	{
		Q_ASSERT(false);
		return false;
	}

	int newIndex = index + step;

	if (newIndex < 0 || newIndex >= static_cast<int>(m_tagToColors.size()))
	{
		Q_ASSERT(false);
		return false;
	}

	std::swap(m_tagToColors[index], m_tagToColors[newIndex]);

	return true;
}

std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const QString& tag) const
{
	std::optional<std::pair<QRgb, QRgb>> result;

	for (const auto& ttc : m_tagToColors)
	{
		if (ttc.tag == tag)
		{
			result = ttc.colors;
			break;
		}
	}

	return result;
}

void MonitorBehavior::setTagToColors(const QString& tag, std::pair<QRgb, QRgb> colors)
{
	for (auto& ttc : m_tagToColors)
	{
		if (ttc.tag == tag)
		{
			ttc.colors = colors;
			return;
		}
	}

	m_tagToColors.push_back({tag, colors});
	return;
}

std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const std::set<QString>& tags) const
{
	// Tags in m_tagToColors have priorities from the highest to the lowest.
	// So that is why regular loop is used here.
	// For future implementation:
	// for input param 'tag' if it has a lot of items and m_tagToColor quite big too,
	// is possible to make optimization, via finding interset of two tags and m_tagToColor.
	//
	std::optional<std::pair<QRgb, QRgb>> result;

	for (const auto& ttc : m_tagToColors)	// Go through m_tagToColors as it is prioritized search
	{
		if (tags.find(ttc.tag) != tags.end())
		{
			result = ttc.colors;				// The first met tag has the highest priority
			break;
		}
	}

	return result;
}

std::optional<std::pair<QRgb, QRgb>> MonitorBehavior::tagToColors(const QStringList& tags) const
{
	std::set<QString> tagSet;
	for (const auto& t : tags)
	{
		tagSet.insert(t);
	}

	return tagToColors(tagSet);
}


void MonitorBehavior::saveToXml(QXmlStreamWriter& writer)
{
	writer.writeStartElement("SignalTagToColor");

	for (const auto& ttc : m_tagToColors)
	{
		writer.writeStartElement("Item");
		writer.writeAttribute("tag", ttc.tag);
		writer.writeAttribute("color1", QColor::fromRgb(ttc.colors.first).name());
		writer.writeAttribute("color2", QColor::fromRgb(ttc.colors.second).name());
		writer.writeEndElement();
	}

	writer.writeEndElement();
	return;
}

bool MonitorBehavior::loadFromXml(QXmlStreamReader& reader)
{
	m_tagToColors.clear();

	while (reader.readNextStartElement())
	{
		// SignalTagToColor
		//
		if(reader.name() == "SignalTagToColor")
		{
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

			QColor color1;
			QColor color2;

			if (reader.attributes().hasAttribute("color"))
			{
				color1 = QColor(reader.attributes().value("color").toString());
				color2 = color1;
			}
			else
			{
				if (reader.attributes().hasAttribute("color1"))
				{
					color1 = QColor(reader.attributes().value("color1").toString());
				}

				if (reader.attributes().hasAttribute("color2"))
				{
					color2 = QColor(reader.attributes().value("color2").toString());
				}
			}

			if (tag.isEmpty() == false && color1.isValid() == true && color2.isValid() == true)
			{
				m_tagToColors.push_back({tag, std::make_pair(color1.rgb(), color2.rgb())});
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

