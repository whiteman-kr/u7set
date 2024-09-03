#ifndef ISCHEMAVIEWHISTORY_H
#define ISCHEMAVIEWHISTORY_H

namespace VFrame30
{
	class ISchemaViewHistory
	{
	public:
		virtual ~ISchemaViewHistory() = default;

		[[nodiscard]] virtual bool canBackHistory() const = 0;
		[[nodiscard]] virtual bool canForwardHistory() const = 0;

		virtual void historyBack() = 0;
		virtual void historyForward() = 0;
	};
}

#endif // ISCHEMAVIEWHISTORY_H
