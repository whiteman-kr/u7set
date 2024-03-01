#pragma once

#include <memory>
#include <vector>

namespace Behavior
{
    class ClientBehavior;
	class MonitorBehavior;
	class TuningClientBehavior;
}

using ClientBehaviorPtr = std::shared_ptr<Behavior::ClientBehavior>;

namespace Behavior
{
    class ClientBehavior;

	//
	// ClientBehaviorStorage
	//
	class ClientBehaviorStorage
	{
	public:
		ClientBehaviorStorage();
		ClientBehaviorStorage(const ClientBehaviorStorage& src);
		ClientBehaviorStorage(ClientBehaviorStorage&& src) noexcept;

		~ClientBehaviorStorage();

		ClientBehaviorStorage& operator=(const ClientBehaviorStorage& scr);
		ClientBehaviorStorage& operator=(ClientBehaviorStorage&& scr) noexcept;

	public:
		QString dbFileName() const;

		void add(ClientBehaviorPtr behavior);
		bool remove(int index);

		int count() const;

		ClientBehaviorPtr get(int index) const;
		ClientBehaviorPtr get(const QString& id) const;

		void clear();

		const std::vector<ClientBehaviorPtr>& behaviors();

		std::vector<std::shared_ptr<Behavior::MonitorBehavior>> monitorBehaviors();
		std::vector<std::shared_ptr<Behavior::TuningClientBehavior>> tuningClientBehaviors();

		void save(QByteArray* data) const;
		bool load(const QByteArray& data, QString* errorCode);

	private:
		std::vector<ClientBehaviorPtr> m_behaviors;
		QString m_fileName = "ClientBehavior.xml";
	};

} // namespace Behavior