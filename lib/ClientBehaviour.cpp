#include "ClientBehaviour.h"

//
// ClientBehaviour
//
ClientBehaviour::ClientBehaviour()
{
	ADD_PROPERTY_GETTER_SETTER(QString, "BehaviourID", true, behaviourId, setBehaviourId);
}

ClientBehaviour::~ClientBehaviour()
{

}

bool ClientBehaviour::isMonitorBehaviour() const
{
	return dynamic_cast<const MonitorBehaviour*>(this) != nullptr;
}

bool ClientBehaviour::isTuningClientBehaviour() const
{
	return dynamic_cast<const TuningClientBehaviour*>(this) != nullptr;
}

QString ClientBehaviour::behaviourId() const
{
	return m_behaviourId;
}

void ClientBehaviour::setBehaviourId(const QString& id)
{
	m_behaviourId = id;

}

void ClientBehaviour::save(QXmlStreamWriter& writer)
{
	writer.writeAttribute("ID", behaviourId());

	saveToXml(writer);

	return;
}

bool ClientBehaviour::load(QXmlStreamReader& reader)
{
	if (reader.attributes().hasAttribute("ID"))
	{
		setBehaviourId(reader.attributes().value("ID").toString());
	}

	if (behaviourId().isEmpty())
	{
		setBehaviourId(("ID"));
	}

	return loadFromXml(reader);
}

//
// MonitorBehaviour
//
MonitorBehaviour::MonitorBehaviour()
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

QColor MonitorBehaviour::signalToTagCriticalColor() const
{
	return m_signalTagToColor.value("critical");
}

void MonitorBehaviour::setSignalToTagCriticalColor(const QColor& color)
{
	m_signalTagToColor["critical"] = color;
}

QColor MonitorBehaviour::signalToTagAttentionColor() const
{
	return m_signalTagToColor.value("attention");
}

void MonitorBehaviour::setSignalToTagAttentionColor(const QColor& color)
{
	m_signalTagToColor["attention"] = color;
}

QColor MonitorBehaviour::signalToTagGeneralColor() const
{
	return m_signalTagToColor.value("general");
}

void MonitorBehaviour::setSignalToTagGeneralColor(const QColor& color)
{
	m_signalTagToColor["general"] = color;
}

void MonitorBehaviour::saveToXml(QXmlStreamWriter& writer)
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

bool MonitorBehaviour::loadFromXml(QXmlStreamReader& reader)
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
// TuningClientBehaviour
//
TuningClientBehaviour::TuningClientBehaviour()
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

QColor TuningClientBehaviour::defaultMismatchBackColor() const
{
	return m_tagToColor.value("defaultMismatchBackColor");
}

void TuningClientBehaviour::setDefaultMismatchBackColor(const QColor& color)
{
	m_tagToColor["defaultMismatchBackColor"] = color;
}

QColor TuningClientBehaviour::defaultMismatchTextColor() const
{
	return m_tagToColor.value("defaultMismatchTextColor");
}

void TuningClientBehaviour::setDefaultMismatchTextColor(const QColor& color)
{
	m_tagToColor["defaultMismatchTextColor"] = color;
}

QColor TuningClientBehaviour::unappliedBackColor() const
{
	return m_tagToColor.value("unappliedBackColor");
}

void TuningClientBehaviour::setUnappliedBackColor(const QColor& color)
{
	m_tagToColor["unappliedBackColor"] = color;
}

QColor TuningClientBehaviour::defaultUnappliedTextColor() const
{
	return m_tagToColor.value("unappliedTextColor");
}

void TuningClientBehaviour::setDefaultUnappliedTextColor(const QColor& color)
{
	m_tagToColor["unappliedTextColor"] = color;
}

void TuningClientBehaviour::saveToXml(QXmlStreamWriter& writer)
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

bool TuningClientBehaviour::loadFromXml(QXmlStreamReader& reader)
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
// ClientBehaviourStorage
//
ClientBehaviourStorage::ClientBehaviourStorage()
{

}

QString ClientBehaviourStorage::dbFileName() const
{
	return m_fileName;
}

void ClientBehaviourStorage::add(std::shared_ptr<ClientBehaviour> behavoiur)
{
	m_behavoiurs.push_back(behavoiur);
}

bool ClientBehaviourStorage::remove(int index)
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return false;
	}

	m_behavoiurs.erase(m_behavoiurs.begin() + index);
	return true;
}

int ClientBehaviourStorage::count() const
{
	return static_cast<int>(m_behavoiurs.size());
}

std::shared_ptr<ClientBehaviour> ClientBehaviourStorage::get(int index) const
{
	if (index < 0 || index >= count())
	{
		Q_ASSERT(false);
		return nullptr;
	}
	return m_behavoiurs[index];
}

std::shared_ptr<ClientBehaviour> ClientBehaviourStorage::get(const QString& id) const
{
	for (auto s : m_behavoiurs)
	{
		if (s->behaviourId() == id)
		{
			return s;
		}
	}

	assert(false);
	return nullptr;
}

void ClientBehaviourStorage::clear()
{
	m_behavoiurs.clear();
}

const std::vector<std::shared_ptr<ClientBehaviour>>& ClientBehaviourStorage::behavoiurs()
{
	return m_behavoiurs;
}

std::vector<std::shared_ptr<MonitorBehaviour>> ClientBehaviourStorage::monitorBehavoiurs()
{
	std::vector<std::shared_ptr<MonitorBehaviour>> result;

	for (auto b : m_behavoiurs)
	{
		if (b->isMonitorBehaviour())
		{
			std::shared_ptr<MonitorBehaviour> mb = std::dynamic_pointer_cast<MonitorBehaviour>(b);
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

std::vector<std::shared_ptr<TuningClientBehaviour>> ClientBehaviourStorage::tuningClientBehavoiurs()
{
	std::vector<std::shared_ptr<TuningClientBehaviour>> result;

	for (auto b : m_behavoiurs)
	{
		if (b->isTuningClientBehaviour())
		{
			std::shared_ptr<TuningClientBehaviour> cb = std::dynamic_pointer_cast<TuningClientBehaviour>(b);
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

void ClientBehaviourStorage::save(QByteArray& data)
{
	QXmlStreamWriter writer(&data);

	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	writer.writeStartElement("Behaviour");
	for (auto s : m_behavoiurs)
	{
		if (s->isMonitorBehaviour())
		{
			writer.writeStartElement("MonitorBehaviour");
		}
		else
		{

			if (s->isTuningClientBehaviour())
			{
				writer.writeStartElement("TuningClientBehaviour");
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

bool ClientBehaviourStorage::load(const QByteArray& data, QString* errorCode)
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

	if (reader.name() != "Behaviour")
	{
		reader.raiseError(QObject::tr("The file is not an Behaviour file."));
		*errorCode = reader.errorString();
		return !reader.hasError();
	}

	// Read signals
	//
	while (reader.readNextStartElement())
	{
		if(reader.name() == "MonitorBehaviour")
		{
			std::shared_ptr<MonitorBehaviour> s = std::make_shared<MonitorBehaviour>();

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
			if(reader.name() == "TuningClientBehaviour")
			{
				std::shared_ptr<TuningClientBehaviour> s = std::make_shared<TuningClientBehaviour>();

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

