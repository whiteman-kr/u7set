#include <VFrame30/Context.h>
#include <VFrame30/ClientSchemaView.h>

namespace VFrame30
{

	Context::Context(const VFrame30::DiagStateController* diagStateController,
					 const VFrame30::AppSignalController* appSignalController,
					 const VFrame30::TuningController* tuningController,
					 VFrame30::IViewVariables* viewVariables,
					 ILogFile* log) :
		m_diagStateController(diagStateController),
		m_appSignalController(appSignalController),
		m_tuningController(tuningController),
		m_viewVariables(viewVariables),
		m_log(log)
	{
	}

	std::shared_ptr<Context> Context::create(const VFrame30::DiagStateController* diagStateController, const VFrame30::AppSignalController* appSignalController, const VFrame30::TuningController* tuningController, VFrame30::IViewVariables* viewVariables, ILogFile* log)
	{
		return std::make_shared<VFrame30::Context>(diagStateController, appSignalController, tuningController, viewVariables, log);
	}

	std::shared_ptr<Context> Context::create(VFrame30::SchemaView* schemaView)
	{
		auto clientSchemaView = dynamic_cast<VFrame30::ClientSchemaView*>(schemaView);

		if (clientSchemaView != nullptr)
		{
			return Context::create(clientSchemaView->diagStateController(),
								   clientSchemaView->appSignalController(),
								   clientSchemaView->tuningController(),
								   clientSchemaView,
								   clientSchemaView->logFile());
		}
		else
		{
			return Context::create(nullptr,
								   nullptr,
								   nullptr,
								   nullptr,
								   nullptr);
		}
	}

	void Context::reset()
	{
		*this = Context{};
	}

	const VFrame30::DiagStateController* Context::diagStateController() const
	{
		return m_diagStateController;
	}

	void Context::setDiagStateController(const VFrame30::DiagStateController* value)
	{
		m_diagStateController = value;
	}

	const VFrame30::AppSignalController* Context::appSignalController() const
	{
		return m_appSignalController;
	}

	void Context::setAppSignalController(const VFrame30::AppSignalController* value)
	{
		m_appSignalController = value;
	}

	const VFrame30::TuningController* Context::tuningController() const
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

} // namespace VFrame30
