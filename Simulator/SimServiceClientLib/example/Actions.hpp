#include <iostream>

namespace Sim
{
	class SimServiceClient;
}

struct Actions
{
	static void exitFunc(Sim::SimServiceClient&, const QStringList&);

	static void help();
	static void help2(Sim::SimServiceClient&, const QStringList&);

	static void ping(Sim::SimServiceClient& client, const QStringList& args);
	static void status(Sim::SimServiceClient& client, const QStringList& args);

	static void start(Sim::SimServiceClient& client, const QStringList& args);
	static void pause(Sim::SimServiceClient& client, const QStringList& args);
	static void stop(Sim::SimServiceClient& client, const QStringList& args);

	static void get_signal_list(Sim::SimServiceClient& client, const QStringList& args);
	static void get_signal_param(Sim::SimServiceClient& client, const QStringList& args);
	static void get_signal_state(Sim::SimServiceClient& client, const QStringList& args);

	static void get_module_list(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args);
	static void lm_status(Sim::SimServiceClient& client, const QStringList& args);
	static void lm_set_flags_help();
	static void lm_set_flag(Sim::SimServiceClient& client, const QStringList& args);

	static void override_signal_help();
	static void override_signal(Sim::SimServiceClient& client, const QStringList& args);

	static void remove_override_help();
	static void remove_override(Sim::SimServiceClient& client, const QStringList& args);

	static void get_overridden_signals_help();
	static void get_overridden_signals(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args);

	static void take_snapshot(Sim::SimServiceClient& client, const QStringList& args);
	static void apply_snapshot(Sim::SimServiceClient& client, const QStringList& args);

	// --
	//
	using ActionFunc = std::function<void(Sim::SimServiceClient& client, const QStringList& args)>;

	static const std::map<QString, ActionFunc> actions;
};