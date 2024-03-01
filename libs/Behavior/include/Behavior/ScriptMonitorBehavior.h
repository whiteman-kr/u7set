#include "BehaviorColorPair.h"

namespace Behavior
{
	class MonitorBehavior;

	/// @brief ScriptMonitorBehavior allows to get MonitorBehavior color pair by tag.
	class ScriptMonitorBehavior
	{
		Q_GADGET

	public:
		ScriptMonitorBehavior(); 
		ScriptMonitorBehavior(std::shared_ptr<const MonitorBehavior> behavior);
		~ScriptMonitorBehavior();

		// Shallow copy.
		//
		ScriptMonitorBehavior(const ScriptMonitorBehavior&) = default;
		ScriptMonitorBehavior(ScriptMonitorBehavior&&) noexcept = default;
		ScriptMonitorBehavior& operator=(const ScriptMonitorBehavior&) = default;
		ScriptMonitorBehavior& operator=(ScriptMonitorBehavior&&) noexcept = default;

	public:
		/// @brief Returns the list of tags
		Q_INVOKABLE QStringList tags() const;

		/// @brief Returns the color pair for the tag.
		Q_INVOKABLE BehaviorColorPair colorByTag(const QString& tag) const;

		/// @brief Returns the color pair for the most priority tag.
		Q_INVOKABLE BehaviorColorPair colorByTagList(const QStringList& tags) const;

	private:
		std::shared_ptr<const MonitorBehavior> m_behavior;
	};

} // namespace Behavior

Q_DECLARE_METATYPE(Behavior::ScriptMonitorBehavior)