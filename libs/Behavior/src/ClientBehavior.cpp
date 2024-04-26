#include <Behavior/ClientBehavior.h>
#include <Behavior/MonitorBehavior.h>
#include <Behavior/TuningClientBehavior.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>


namespace Behavior
{
	//
	// ClientBehavior
	//
	ClientBehavior::ClientBehavior(const ClientBehavior& src) :
		PropertyObject{},
		m_data{src.m_data}
	{
	}

	ClientBehavior::ClientBehavior(ClientBehavior&& src) noexcept :
		PropertyObject{}, // PropertyObject cannot be moved
		m_data{std::move(src.m_data)}
	{
	}

	ClientBehavior& ClientBehavior::operator=(const ClientBehavior& src)
	{
		if (this != &src)
		{
			m_data = src.m_data;
		}

		return *this;
	}

	ClientBehavior& ClientBehavior::operator=(ClientBehavior&& src)  noexcept
	{
		if (this != &src)
		{
			m_data = std::move(src.m_data);
		}

		return *this;
	}

	void ClientBehavior::propertyDemand(const QString& /*prop*/)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, "BehaviorID", true, behaviorId, setBehaviorId);
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
		return m_data.behaviorId;
	}

	void ClientBehavior::setBehaviorId(const QString& id)
	{
		m_data.behaviorId = id;
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

} // namespace Behavior
