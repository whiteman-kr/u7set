#pragma once
#include <QString>
#include <QVariant>

namespace VFrame30
{
	/*! \class IViewVariables
	\brief An interface for accessing schema view variables.

	This interface provides variables for macro expansion in AppSignalIDs.
	Example: "#ABC_$(INPUTNO)", text $(INPUTNO) will be changed to the value of variable INPUTNO.
	*/
	class IViewVariables
	{
	public:
		virtual ~IViewVariables() = default;

		[[nodiscard]] virtual bool variableExists(const QString& name) const = 0;

		[[nodiscard]] virtual QVariant variable(const QString& name) const = 0;
		virtual void setVariable(const QString& name, const QVariant& value) = 0;
	};

} // namespace VFrame30
