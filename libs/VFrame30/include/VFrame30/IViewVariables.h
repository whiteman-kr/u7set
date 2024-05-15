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

		/// @brief Get the list of view variables.
		[[nodiscard]] virtual QStringList viewVariables() const = 0;

		/// @brief Check if a view variable exists.
		[[nodiscard]] virtual bool viewVariableExists(const QString& name) const = 0;

		/// @brief Get the value of a view variable.
		[[nodiscard]] virtual QVariant viewVariable(const QString& name) const = 0;

		/// @brief Set the value of a view variable.
		virtual void setViewVariable(const QString& name, const QVariant& value) = 0;
	};

} // namespace VFrame30
