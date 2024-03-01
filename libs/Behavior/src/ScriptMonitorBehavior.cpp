#include "../include/Behavior/ScriptMonitorBehavior.h"
#include "../include/Behavior/MonitorBehavior.h"

namespace Behavior
{
	ScriptMonitorBehavior::ScriptMonitorBehavior() :
		m_behavior{std::make_shared<const MonitorBehavior>()}
	{
		return;
	}


	ScriptMonitorBehavior::ScriptMonitorBehavior(std::shared_ptr<const MonitorBehavior> behavior) :
		m_behavior(std::move(behavior))
	{
		return;
	}

	ScriptMonitorBehavior::~ScriptMonitorBehavior() = default;

	QStringList ScriptMonitorBehavior::tags() const
	{
		QStringList result;

		if (m_behavior != nullptr)
		{
			result = m_behavior->tags();
		}
		else
		{
			Q_ASSERT(false);
		}

		return result;
	}

	BehaviorColorPair ScriptMonitorBehavior::colorByTag(const QString& tag) const
	{
		BehaviorColorPair result;

		if (m_behavior == nullptr)
		{
			Q_ASSERT(m_behavior);
			return result;
		}

		auto opt = m_behavior->tagToColors(tag);
		if (opt.has_value() == true)
		{
			result.isValid = true;
			result.color1 = QColor::fromRgb(opt.value().first);
			result.color2 = QColor::fromRgb(opt.value().second);
		}

		return result;
	}

	BehaviorColorPair ScriptMonitorBehavior::colorByTagList(const QStringList& tags) const
	{
		BehaviorColorPair result;

		if (m_behavior == nullptr)
		{
			Q_ASSERT(m_behavior);
			return result;
		}

		auto opt = m_behavior->tagToColors(tags);
		if (opt.has_value() == true)
		{
			result.isValid = true;
			result.color1 = QColor::fromRgb(opt.value().first);
			result.color2 = QColor::fromRgb(opt.value().second);
		}

		return result;
	}
} // namespace Behavior

