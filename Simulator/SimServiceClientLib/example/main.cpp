
#include <CommonLib/Hash.h>
#include <CommonLib/Types.h>

#include <SimServiceClientLib/SimServiceClient.h>

#include <QElapsedTimer>
#include <QRegularExpression>

#include <cassert>
#include <functional>
#include <iostream>
#include <vector>


struct Actions
{
	static void exitFunc(Sim::SimServiceClient&, const QStringList&) { std::exit(0); }

	static void help()
	{
		std::cout << "Help:" << "\n";

		std::cout << "  exit                                Exit program." << "\n";
		std::cout << "  help, ?                             Print this help." << "\n";
		std::cout << "\n";
		std::cout << "  ping [data]                         Send a ping request. Optionally include data to echo." << "\n";
		std::cout << "  status                              Print simulator status (0 stopped, 1 running, 2 paused)." << "\n";
		std::cout << "  start                               Start simulation." << "\n";
		std::cout << "  pause                               Pause running simulation." << "\n";
		std::cout << "  stop                                Stop running simulation." << "\n";
		std::cout << "\n";
		std::cout << "  get_signal_list                     Get all AppSignal IDs." << "\n";
		std::cout << "  get_signal_param [#ID1 #IDN]        Get signal parameters. Without arguments, all signals are returned." << "\n";
		std::cout << "  get_signal_state <#ID1> ... [#IDN]  Get signal states." << "\n";
		std::cout << "\n";
		std::cout << "  get_module_list                     Get all modules.\n";
		std::cout << "  lm_status <LMID>                    Print LogicModule status.\n";
		lm_set_flags_help();
		std::cout << "\n";
		override_signal_help();
		remove_override_help();
		get_overridden_signals_help();
		return;
	}

	static void help2(Sim::SimServiceClient&, const QStringList&)
	{
		help();
		return;
	}

	static void ping(Sim::SimServiceClient& client, const QStringList& args)
	{
		using namespace Qt::Literals::StringLiterals;

		QByteArray payload = args.size() < 2 ? "Hello, World!"_ba : args[1].toLatin1();
		auto pingResult = client.Ping(payload);

		if (pingResult.has_value() == false)
		{
			std::cout << "Ping error: " << pingResult.error().toStdString() << "\n";
		}
		else
		{
			if (payload != pingResult)
			{
				std::cout << "Ping error: wrong payload, expected: " << payload.toStdString()
						  << ", received: " << pingResult.value().toStdString() << "\n";
			}
			else
			{
				std::cout << "Pink ok, reply: " << pingResult.value().toStdString() << "\n";
			}
		}

		return;
	}

	static void status(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto status = client.GetStatus();
		if (status.has_value() == false)
		{
			std::cout << "GetStatus() error: " << status.error().toStdString() << "\n";
		}
		else
		{
			std::cout << "GetStatus(): Project: " << status->project.toStdString();
			std::cout << ", State: " << status->state;
			std::cout << "\n";
		}

		return;
	}

	static void start(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto status = client.CommandStart();
		if (status.has_value() == false)
		{
			std::cout << "CommandStart() error: " << status.error().toStdString() << "\n";
		}

		Actions::status(client, {});
		return;
	}

	static void pause(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto status = client.CommandPause();
		if (status.has_value() == false)
		{
			std::cout << "CommandPause() error: " << status.error().toStdString() << "\n";
		}

		Actions::status(client, {});
		return;
	}

	static void stop(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto status = client.CommandStop();
		if (status.has_value() == false)
		{
			std::cout << "CommandStop() error: " << status.error().toStdString() << "\n";
		}

		Actions::status(client, {});
		return;
	}

	static void get_signal_list(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto signalList = client.GetSignalList();

		if (signalList.has_value() == true)
		{
			std::cout << "AppSignalCount: " << signalList->size() << "\n";
		}
		else
		{
			std::cout << "GetSignalList error: " << signalList.error().toStdString() << "\n";
		}

		return;
	}

	static void get_signal_param(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() <= 1)
		{
			// Get for all signals
			//
			auto signalParams = client.GetSignalParam();

			if (signalParams.has_value() == false)
			{
				std::cout << "GetSignalParam error: " << signalParams.error().toStdString() << "\n";
			}
			else
			{
				std::cout << "GetSignalParam received: " << signalParams->size() << " signals.\n";
			}
		}
		else
		{
			std::vector<Hash> hashes;
			std::transform(std::next(std::begin(args)),
						   std::end(args),
						   std::back_inserter(hashes),
						   [](const QString& appSignalId)
						   {
							   return ::calcHash(appSignalId);
						   });

			auto signalParams = client.GetSignalParam(hashes);

			if (signalParams.has_value() == false)
			{
				std::cout << "GetSignalParam error: " << signalParams.error().toStdString() << "\n";
			}
			else
			{
				const std::vector<::AppSignalParam>& params = signalParams.value();

				std::cout << "GetSignalParam received: " << signalParams->size() << " signals.\n";

				for (const ::AppSignalParam& sp : params)
				{
					std::cout << "--\n";
					std::cout << "\tAppSignalID:    " << sp.appSignalId().toStdString() << "\n";
					std::cout << "\tCustomSignalID: " << sp.customSignalId().toStdString() << "\n";
					std::cout << "\tLmEquipmentId:  " << sp.lmEquipmentId().toStdString() << "\n";
					std::cout << "\tCaption:  " << sp.caption().toLocal8Bit().toStdString() << "\n";
					std::cout << "\tTags:  " << sp.tagStringList().join(" ").toStdString() << "\n";
				}
			}
		}

		return;
	}

	static void get_signal_state(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() <= 1)
		{
			std::cout << "Use:\n\tget_signal_state <#ID1> [#ID2] ... [#IDN]\n";
		}
		else
		{
			std::vector<Hash> hashes;
			std::transform(std::next(std::begin(args)),
						   std::end(args),
						   std::back_inserter(hashes),
						   [](const QString& appSignalId)
						   {
							   return ::calcHash(appSignalId);
						   });

			auto result = client.GetSignalState(hashes);

			if (result.has_value() == false)
			{
				std::cout << "GetSignalState error: " << result.error().toStdString() << "\n";
			}
			else
			{
				const std::vector<::AppSignalState>& states = result.value();

				std::cout << "GetSignalState received: " << states.size() << " states.\n";

				for (const ::AppSignalState& state : states)
				{
					std::cout << "--\n";
					std::cout << "\tHash: 0x" << std::hex << state.m_hash << std::dec;
					std::cout << ", value: " << state.m_value;
					std::cout << ", flags: " << std::hex << state.m_flags.all << std::dec;
					std::cout << "\n";
				}
			}
		}

		return;
	}

	static void get_module_list(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto result = client.GetModuleList();
		if (result.has_value() == false)
		{
			std::cout << result.error().toStdString() << "\n";
			return;
		}

		const auto& modules = result.value();
		for (const auto& module : modules)
		{
			std::cout << module.equipmentId().toStdString() << ", IsLogicModule: " << module.isLogicModule() << "\n";
		}

		std::cout << modules.size() << " module(s)." << "\n";

		return;
	}

	static void lm_status(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() < 2)
		{
			std::cout << "Error: missing argument\n";
			help();
			return;
		}

		auto m = client.GetModule(QStringList{} << args[1]);
		if (m.has_value() == false)
		{
			std::cout << m.error().toStdString() << "\n";
		}
		else
		{
			assert(m->size() == 1);
			assert(m->at(0).isLogicModule() == true);

			auto lm = m->at(0).toLogicModule();
			std::cout << "LM:            " << lm.equipmentId().toStdString() << "\n";
			std::cout << "Subsystem:     " << lm.subsystemId().toStdString() << "\n";
			std::cout << "LM Number:     " << lm.lmNumber() << "\n";
			std::cout << "Channel:       " << lm.channel() << "\n";
			std::cout << "FaultMode:     " << lm.faultMode() << "\n";
			std::cout << "PowerOn:       " << lm.isPowerOn() << "\n";
			std::cout << "ArmingKey:     " << lm.armingKey() << "\n";
			std::cout << "TuningKey:     " << lm.tuningKey() << "\n";
			std::cout << "SorIsSet:      " << lm.sorIsSet() << "\n";
			std::cout << "SorSetSwitch1: " << lm.sorSetSwitch1() << "\n";
			std::cout << "SorSetSwitch2: " << lm.sorSetSwitch2() << "\n";
			std::cout << "SorSetSwitch3: " << lm.sorSetSwitch3() << "\n";
		}

		return;
	}


	static void lm_set_flags_help()
	{
		std::cout << "  lm_set_flag <LMID> <FLAG> <VALUE>   Set LogicModule flag, to value 0 or 1.\n";
		std::cout << "      Flags:\n";
		std::cout << "          POWER_ON\n";
		std::cout << "          ARMING_KEY\n";
		std::cout << "          TUNING_KEY\n";
		std::cout << "          SOR_SET_SWITCH_1[2,3]\n";
		std::cout << "          SOR_RESET - Only value 1 is meaningful\n";
	}

	static void lm_set_flag(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() < 4)
		{
			lm_set_flags_help();
			return;
		}

		QString lmEquipmentId = args[1];
		QString flag = args[2];
		QString value = args[3];

		if (value != "0" && value != "1")
		{
			lm_set_flags_help();
			return;
		}

		bool boolValue = value == "0" ? false : true;

		auto m = client.GetModule(QStringList{} << lmEquipmentId);
		if (m.has_value() == false)
		{
			std::cout << "GetModule error: " << m.error().toStdString() << "\n";
			return;
		}

		assert(m->size() == 1);
		assert(m->at(0).isLogicModule() == true);
		auto lm = m->at(0).toLogicModule();

		bool returnedValue = false;

		if (flag == "POWER_ON")
		{
			lm.setPowerOff(!boolValue);
			returnedValue = lm.isPowerOn();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "ARMING_KEY")
		{
			lm.setArmingKey(boolValue);
			returnedValue = lm.armingKey();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "TUNING_KEY")
		{
			lm.setTuningKey(boolValue);
			returnedValue = lm.tuningKey();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "SOR_SET_SWITCH_1")
		{
			lm.setSorSetSwitch1(boolValue);
			returnedValue = lm.sorSetSwitch1();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "SOR_SET_SWITCH_2")
		{
			lm.setSorSetSwitch2(boolValue);
			returnedValue = lm.sorSetSwitch2();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "SOR_SET_SWITCH_3")
		{
			lm.setSorSetSwitch3(boolValue);
			returnedValue = lm.sorSetSwitch3();
			std::cout << " Returned value: " << returnedValue << "\n";
			lm_status(client, args);
			return;
		}

		if (flag == "SOR_RESET")
		{
			lm.raiseSorResetSwitch();
			lm_status(client, args);
			return;
		}

		lm_set_flags_help();
		return;
	}

	static void override_signal_help()
	{
		std::cout << "  override_signal <#ID> <TYPE> <VALUE>   Override signal value.\n";
		std::cout << "      Type:\n";
		std::cout << "          bool    for discretes, value 0/1 \n";
		std::cout << "          float   for floating point value\n";
		std::cout << "          double  for floating point value\n";
		std::cout << "          int32   for signed int 32-bit\n";
		std::cout << "          script  override by script, value can be any, predefined script will be used.\n";
	}

	static void override_signal(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() < 4)
		{
			override_signal_help();
			return;
		}

		QString appSignalId = args[1];
		QString type = args[2];
		QString valueStr = args[3];

		Sim::SimServiceClient::OverrideValueT value;

		if (type == "bool")
		{
			value.emplace<bool>(valueStr == "0" ? false : true);
		}
		else if (type == "float")
		{
			bool ok = false;
			float f = valueStr.toFloat(&ok);

			if (ok == false)
			{
				std::cout << "Floating point format error\n";
				return;
			}

			value.emplace<double>(f); // double covers float
		}
		else if (type == "double")
		{
			bool ok = false;
			double d = valueStr.toDouble(&ok);

			if (ok == false)
			{
				std::cout << "Floating point format error\n";
				return;
			}

			value.emplace<double>(d); // double covers float
		}
		else if (type == "int32")
		{
			bool ok = false;
			int32_t i = static_cast<int32_t>(valueStr.toInt(&ok));

			if (ok == false)
			{
				std::cout << "32bit signed integer format error\n";
				return;
			}

			value.emplace<int32_t>(i); // double covers float
		}
		else if (type == "script")
		{
			static const QString script = R"(
// Square for Discrete
let counter = 0;

(function(lastValue, workcycle)
{
	const lowTime = 300;
	const highTime = 200;

	counter --;
	let result = lastValue;

	if (counter <= 0)
	{
		if (lastValue === 0)
		{
			counter = highTime / 5;
			result = 1;
		}
		else
		{
			counter = lowTime / 5;
			result = 0;
		}
	}
	return result;
})
)";
			value.emplace<QString>(script);
		}
		else
		{
			std::cout << "Unknown type: " << type.toStdString() << "\n";
			return;
		}

		Sim::SimServiceClient::OverrideSignalPair osp;
		osp.appSignalId = appSignalId;
		osp.value = value;

		std::vector<Sim::SimServiceClient::OverrideSignalPair> overrideSignals;
		overrideSignals.push_back(osp);

		auto result = client.OverrideSignals(overrideSignals);

		if (result.has_value() == false)
		{
			std::cout << "OverrideSignals error: " << result.error().join("\n").toStdString() << "\n";
			return;
		}

		return;
	}

	static void remove_override_help() { std::cout << "  remove_override <#ID1> ... [#IDN]      Remove overridden signal(s).\n"; }

	static void remove_override(Sim::SimServiceClient& client, const QStringList& args)
	{
		if (args.size() < 2)
		{
			remove_override_help();
			return;
		}


		QStringList appSignals = args;
		appSignals.removeFirst();

		auto result = client.RemoveOverrideSignals(appSignals);

		if (result.has_value() == false)
		{
			std::cout << "RemoveOverrideSignals error: " << result.error().toStdString() << "\n";
			return;
		}

		std::cout << "Ok" << "\n";
		std::cout << "CurrentlyOverriddenSignals:" << "\n";

		for (const QString& appSignalId : result.value())
		{
			std::cout << appSignalId.toStdString() << "\n";
		}

		return;
	}

	static void get_overridden_signals_help() { std::cout << "  get_overridden_signals                   Get all overridden signals.\n"; }

	static void get_overridden_signals(Sim::SimServiceClient& client, [[maybe_unused]] const QStringList& args)
	{
		auto result = client.GetOverriddenSignals();
		if (result.has_value() == false)
		{
			std::cout << "GetOverriddenSignals error: " << result.error().toStdString() << "\n";
			return;
		}

		for (const QString& appSignalId : result.value())
		{
			std::cout << appSignalId.toStdString() << "\n";
		}
	}
};

using ActionFunc = std::function<void(Sim::SimServiceClient& client, const QStringList& args)>;

const std::map<QString, ActionFunc> actions = {
	{QString("exit"), Actions::exitFunc},
	{QString("help"), Actions::help2},
	{QString("?"), Actions::help2},

	{QString("ping"), Actions::ping},
	{QString("status"), Actions::status},
	{QString("start"), Actions::start},
	{QString("stop"), Actions::stop},
	{QString("pause"), Actions::pause},

	{QString("get_signal_list"), Actions::get_signal_list},
	{QString("get_signal_param"), Actions::get_signal_param},
	{QString("get_signal_state"), Actions::get_signal_state},

	{QString("get_module_list"), Actions::get_module_list},
	{QString("lm_status"), Actions::lm_status},
	{QString("lm_set_flag"), Actions::lm_set_flag},

	{QString("override_signal"), Actions::override_signal},
	{QString("remove_override"), Actions::remove_override},
	{QString("get_overridden_signals"), Actions::get_overridden_signals},
};


int main()
{
	Sim::SimServiceClient client{"localhost:50051"};

	Actions::help();

	QString lmEquipmentId = "SRT_SHFS_EAB_TG3_CH01_MD00";

	do
	{
		std::cout << ">";

		std::string command;
		std::getline(std::cin, command); // Read the full line including spaces

		QString qcommand = QString::fromStdString(command);
		QStringList args = qcommand.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

		if (args.isEmpty() == true || actions.contains(args[0]) == false)
		{
			Actions::help();
			continue;
		}

		auto& f = actions.at(args[0]);

		{
			QElapsedTimer timer;
			timer.start();

			f(client, args);

			auto elapsed = timer.elapsed();
			std::cout << "Time, ms: " << elapsed << "\n";
		}

	} while (true);

	return 0;
}