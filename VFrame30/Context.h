#pragma once
#include <memory>
#include "../UtilsLib/ILogFile.h"
#include "AppSignalController.h"
#include "TuningController.h"
#include "IViewVariables.h"

namespace VFrame30
{
	class SchemaView;

	class Context
	{
	public:
		Context(const VFrame30::AppSignalController* appSignalController,
				const VFrame30::TuningController* tuningController,
				VFrame30::IViewVariables* viewVariables,
				ILogFile* log);

		Context() = default;
		Context(const Context&) = default;
		Context(Context&&) = default;
		~Context() = default;

		Context& operator=(const Context&) = default;
		Context& operator=(Context&&) = default;

	public:
		static std::shared_ptr<Context> create(const VFrame30::AppSignalController* appSignalController,
											   const VFrame30::TuningController* tuningController,
											   VFrame30::IViewVariables* viewVariables,
											   ILogFile* log);

		static std::shared_ptr<Context> create(VFrame30::SchemaView* clientSchemaView);

		void reset();

		const VFrame30::AppSignalController* appSignalController() const noexcept;
		void setAppSignalController(const VFrame30::AppSignalController* value);

		const VFrame30::TuningController* tuningController() const noexcept;
		void setTuningController(const VFrame30::TuningController* value);

		VFrame30::IViewVariables* viewVariables();
		const VFrame30::IViewVariables* viewVariables() const;

		ILogFile* log();

	private:
		const VFrame30::AppSignalController* m_appSignalController = nullptr;
		const VFrame30::TuningController* m_tuningController = nullptr;
		IViewVariables* m_viewVariables = nullptr;
		ILogFile* m_log = nullptr;
	};

}

