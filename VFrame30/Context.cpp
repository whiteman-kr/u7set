#include "Context.h"
#include "ClientSchemaView.h"

namespace VFrame30
{

	Context::Context(const VFrame30::AppSignalController* appSignalController,
					 const VFrame30::TuningController* tuningController,
					 VFrame30::IViewVariables* viewVariables,
					 ILogFile* log) :
		m_appSignalController(appSignalController),
		m_tuningController(tuningController),
		m_viewVariables(viewVariables),
		m_log(log)
	{
	}

	std::shared_ptr<Context> Context::create(const VFrame30::AppSignalController* appSignalController,
											 const VFrame30::TuningController* tuningController,
											 VFrame30::IViewVariables* viewVariables,
											 ILogFile* log)
	{
		return std::make_shared<VFrame30::Context>(appSignalController, tuningController, viewVariables, log);
	}

	std::shared_ptr<Context> Context::create(VFrame30::SchemaView* schemaView)
	{
		auto clientSchemaView = dynamic_cast<VFrame30::ClientSchemaView*>(schemaView);

		if (clientSchemaView != nullptr)
		{
			return Context::create(clientSchemaView->appSignalController(),
								   clientSchemaView->tuningController(),
								   clientSchemaView,
								   clientSchemaView->logFile());
		}
		else
		{
			return Context::create(nullptr,
								   nullptr,
								   nullptr,
								   nullptr);
		}
	}

	void Context::reset()
	{
		*this = Context{};
	}

	const VFrame30::AppSignalController* Context::appSignalController() const noexcept
	{
		return m_appSignalController;
	}

	void Context::setAppSignalController(const VFrame30::AppSignalController* value)
	{
		m_appSignalController = value;
	}

	const VFrame30::TuningController* Context::tuningController() const noexcept
	{
		return m_tuningController;
	}

	void Context::setTuningController(const VFrame30::TuningController* value)
	{
		m_tuningController = value;
	}

	VFrame30::IViewVariables* Context::viewVariables()
	{
		return m_viewVariables;
	}

	const VFrame30::IViewVariables* Context::viewVariables() const
	{
		return m_viewVariables;
	}

	ILogFile* Context::log()
	{
		return m_log;
	}

}
