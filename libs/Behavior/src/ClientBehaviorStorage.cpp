#include "../include/Behavior/ClientBehaviorStorage.h"
#include "../include/Behavior/MonitorBehavior.h"
#include "../include/Behavior/TuningClientBehavior.h"


namespace Behavior
{
	//
	// ClientBehaviorStorage
	//
    ClientBehaviorStorage::ClientBehaviorStorage() = default;

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

    ClientBehaviorStorage::~ClientBehaviorStorage() = default;

	ClientBehaviorStorage& ClientBehaviorStorage::operator=(const ClientBehaviorStorage& src)
	{
		// Create a deep copy.
		//
		m_fileName = src.m_fileName;

		QByteArray ba;
		src.save(&ba);

		QString errorCode;
		this->load(ba, &errorCode);

		return *this;
	}

	ClientBehaviorStorage& ClientBehaviorStorage::operator=(ClientBehaviorStorage&& src) noexcept
	{
		m_fileName = std::move(src.m_fileName);
		m_behaviors = std::move(src.m_behaviors);

		return *this;
	}

	QString ClientBehaviorStorage::dbFileName() const
	{
		return m_fileName;
	}

	void ClientBehaviorStorage::add(ClientBehaviorPtr behavior)
	{
		m_behaviors.push_back(behavior);
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

		if (reader.name() != QLatin1String("Behavior"))
		{
			reader.raiseError(QObject::tr("The file is not an Behavior file."));
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if (reader.name() == QLatin1String("MonitorBehavior"))
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
				if (reader.name() == QLatin1String("TuningClientBehavior"))
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
} // namespace Behavior