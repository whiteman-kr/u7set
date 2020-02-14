#include "ClientBehavior.h"

//
// ClientBehavior
//
ClientBehavior::ClientBehavior()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "BehaviorID", true, behaviorId, setBehaviorId);
}

ClientBehavior::~ClientBehavior()
{

}

bool ClientBehavior::isMonitorBehavior() const
{
	return dynamic_cast<const MonitorBehavior*>(this) != nullptr;
}

bool ClientBehavior::isTuningClientBehavior() const
{
	return dynamic_cast<const TuningClientBehavior*>(this) != nullptr;
}

QString ClientBehavior::behaviorId() const
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
MonitorBehavior::MonitorBehavior()
{
	m_signalTagToColor["critical"] = QColor("#FF5733");
	m_signalTagToColor["attention"] = QColor("#FFBD33");
	m_signalTagToColor["general"] = QColor("#2A05EB");

	auto prop = ADD_PROPERTY_GETTER_SETTER(QColor, "SignalToTagCriticalColor", true, signalToTagCriticalColor, setSignalToTagCriticalColor);
	prop->setCategory("Appearance");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, "SignalToTagAttentionColor", true, signalToTagAttentionColor, setSignalToTagAttentionColor);
	prop->setCategory("Appearance");

	prop = ADD_PROPERTY_GETTER_SETTER(QColor, "SignalToTagGeneralColor", true, signalToTagGeneralColor, setSignalToTagGeneralColor);
	prop->setCategory("Appearance");

}

QColor MonitorBehavior::signalToTagCriticalColor() const
{
	return m_signalTagToColor.value("critical");
}

void MonitorBehavior::setSignalToTagCriticalColor(const QColor& color)
{
	m_signalTagToColor["critical"] = color;
}

QColor MonitorBehavior::signalToTagAttentionColor() const
{
	return m_signalTagToColor.value("attention");
}

void MonitorBehavior::setSignalToTagAttentionColor(const QColor& color)
{
	m_signalTagToColor["attention"] = color;
}

QColor MonitorBehavior::signalToTagGeneralColor() const
{
	return m_signalTagToColor.value("general");
}

void MonitorBehavior::setSignalToTagGeneralColor(const QColor& color)
{
	m_signalTagToColor["general"] = color;
}

void MonitorBehavior::saveToXml(QXmlStreamWriter& writer)
{
	writer.writeStartElement("SignalTagToColor");

	QHashIterator<QString, QColor> i(m_signalTagToColor);
	while (i.hasNext())
	{
		i.next();

		writer.writeStartElement("Item");
		writer.writeAttribute("tag", i.key());
		writer.writeAttribute("color", i.value().name());
		writer.writeEndElement();
	}

	writer.writeEndElement();
	return;
}

bool MonitorBehavior::loadFromXml(QXmlStreamReader& reader)
{
	m_signalTagToColor.clear();

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
				m_signalTagToColor[tag] = color;
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

QString ClientBehaviorStorage::dbFileName() const
{
	return m_fileName;
}

void ClientBehaviorStorage::add(std::shared_ptr<ClientBehavior> behavoiur)
{
	m_behavoiurs.push_back(behavoiur);
}

bool ClientBehaviorStorage::remove(int index)
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return false;
	}

	m_behavoiurs.erase(m_behavoiurs.begin() + index);
	return true;
}

int ClientBehaviorStorage::count() const
{
	return static_cast<int>(m_behavoiurs.size());
}

std::shared_ptr<ClientBehavior> ClientBehaviorStorage::get(int index) const
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return nullptr;
	}
	return m_behavoiurs[index];
}

std::shared_ptr<ClientBehavior> ClientBehaviorStorage::get(const QString& id) const
{
	for (auto s : m_behavoiurs)
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
	m_behavoiurs.clear();
}

const std::vector<std::shared_ptr<ClientBehavior>>& ClientBehaviorStorage::behavoiurs()
{
	return m_behavoiurs;
}

std::vector<std::shared_ptr<MonitorBehavior>> ClientBehaviorStorage::monitorBehavoiurs()
{
	std::vector<std::shared_ptr<MonitorBehavior>> result;

	for (auto b : m_behavoiurs)
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

std::vector<std::shared_ptr<TuningClientBehavior>> ClientBehaviorStorage::tuningClientBehavoiurs()
{
	std::vector<std::shared_ptr<TuningClientBehavior>> result;

	for (auto b : m_behavoiurs)
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

void ClientBehaviorStorage::save(QByteArray& data)
{
	QXmlStreamWriter writer(&data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("Behavior");
	for (auto s : m_behavoiurs)
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
				m_behavoiurs.push_back(s);
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
					m_behavoiurs.push_back(s);
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

