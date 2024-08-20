#pragma once

#include <CommonLib/PropertyObject.h>

class QXmlStreamWriter;
class QXmlStreamReader;

namespace Behavior
{
	class ClientBehavior;
	class MonitorBehavior;
	class TuningClientBehavior;
}

namespace Behavior
{
	//
	// ClientBehavior
	//
	class ClientBehavior : public PropertyObject
	{
	public:
		ClientBehavior() = default;
		ClientBehavior(const ClientBehavior& src);
		ClientBehavior(ClientBehavior&& src) noexcept;
		virtual ~ClientBehavior() = default;

		ClientBehavior& operator=(const ClientBehavior& src);
		ClientBehavior& operator=(ClientBehavior&& src) noexcept;

	protected:
		virtual void propertyDemand(const QString& prop) override;

	public:
		bool isMonitorBehavior() const;
		bool isTuningClientBehavior() const;

		const QString& behaviorId() const;
		void setBehaviorId(const QString& behaviorId);

	public:
		virtual void save(QXmlStreamWriter& writer);
		virtual bool load(QXmlStreamReader& reader);

	protected:
		virtual void saveToXml(QXmlStreamWriter& writer) = 0;
		virtual bool loadFromXml(QXmlStreamReader& reader) = 0;

	private:
		struct PrivateData
		{
			QString behaviorId;
		} m_data;
	};

} // namespace Behavior

