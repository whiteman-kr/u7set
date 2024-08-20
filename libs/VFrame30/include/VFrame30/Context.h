#pragma once
#include "../UtilsLib/ILogFile.h"
#include <VFrame30/AppSignalController.h>
#include <VFrame30/DiagStateController.h>
#include <VFrame30/IViewVariables.h>
#include <VFrame30/TuningController.h>
#include <memory>
#include "Context.h"

namespace VFrame30
{
	class SchemaView;

	class Context
	{
	public:
		Context(const VFrame30::DiagStateController* diagStateController,
				const VFrame30::AppSignalController* appSignalController,
				const VFrame30::TuningController* tuningController,
				VFrame30::IViewVariables* viewVariables,
				ILogFile* log);

		Context(const Context&) = delete;
		Context(Context&&) noexcept = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) noexcept = delete;
		~Context() = default;

	public:
		static std::shared_ptr<Context> create(const VFrame30::DiagStateController* diagStateController,
											   const VFrame30::AppSignalController* appSignalController,
											   const VFrame30::TuningController* tuningController,
											   VFrame30::IViewVariables* viewVariables,
											   ILogFile* log);

		static std::shared_ptr<Context> create(VFrame30::SchemaView* clientSchemaView);

		//void reset();

		const VFrame30::DiagStateController* diagStateController() const;
		void setDiagStateController(const VFrame30::DiagStateController* value);

		const VFrame30::AppSignalController* appSignalController() const;
		void setAppSignalController(const VFrame30::AppSignalController* value);

		const VFrame30::TuningController* tuningController() const;
		void setTuningController(const VFrame30::TuningController* value);

		VFrame30::IViewVariables* viewVariables();
		const VFrame30::IViewVariables* viewVariables() const;

		ILogFile* log();

	private:
		const VFrame30::DiagStateController* m_diagStateController = nullptr;
		const VFrame30::AppSignalController* m_appSignalController = nullptr;
		const VFrame30::TuningController* m_tuningController = nullptr;
		IViewVariables* m_viewVariables = nullptr;
		ILogFile* m_log = nullptr;
	};

} // namespace VFrame30
