#include "../include/SimServiceClientLib/SimServiceModule.h"

#include "SimServiceClientImpl.h"

namespace
{
	// Get from module's data flag.
	//
	bool getModuleFlag(const Sim::ModuleStatePtr& data, ::RpctGrpc::ModuleFlagId flagId)
	{
		if (data == nullptr)
		{
			return false;
		}

		auto flagIt = data->flags().find(static_cast<int32_t>(flagId));
		if (flagIt == std::end(data->flags()))
		{
			return false;
		}

		return flagIt->second.boolvalue();
	}

	// Send request to set new value for flag.
	// Update returned flag in the data of the module.
	//
	tl::expected<void, QString> setModuleFlag(const Sim::ServiceClientImplPtr& client,
											  Sim::ModuleStatePtr& data,
											  const QString& equipmentId,
											  ::RpctGrpc::ModuleFlagId flagId,
											  bool value)
	{
		assert(client);
		if (data == nullptr)
		{
			assert(data);
			return tl::unexpected("Module is Null.");
		}

		auto r = client->SetModuleFlag(equipmentId, static_cast<int32_t>(flagId), value);
		if (r.has_value() == false)
		{
			return tl::unexpected(r.error());
		}

		// Update flag in the data
		//
		const ::RpctGrpc::SetModuleFlagReply& reply = r.value();
		if (reply.has_updatedvalue() == true)
		{
			assert(reply.updatedvalue().has_boolvalue());
			auto& flags = *(data->mutable_flags());
			flags[static_cast<int32_t>(flagId)].set_boolvalue(reply.updatedvalue().boolvalue());
		}

		return {};
	}
} // namespace

namespace Sim
{
	SimServiceModule::SimServiceModule(ServiceClientImplPtr client, ModuleStatePtr data) :
		m_client{std::move(client)},
		m_data{std::move(data)},
		m_equipmentId{m_data == nullptr ? QString{} : QString::fromStdString(m_data->equipmentid())}
	{
		if (m_client == nullptr)
		{
			throw std::runtime_error("Where is a client?");
		}
	}

	SimServiceModule::~SimServiceModule() = default;

	bool SimServiceModule::isNull() const
	{
		return m_data == nullptr || m_data->ModuleOneOf_case() == ::RpctGrpc::ModuleState::MODULEONEOF_NOT_SET;
	}

	tl::expected<void, QString> SimServiceModule::update()
	{
		auto r = m_client->GetModule({m_equipmentId});
		if (r.has_value() == true)
		{
			if (r->size() != 1)
			{
				assert(r.value().size() == 1);
				return tl::unexpected("GetModule() is expected to return exactly 1 module.");
			}

			*this = std::move(r.value().front());
		}
		else
		{
			return tl::unexpected(r.error());
		}

		return {};
	}

	const QString& SimServiceModule::equipmentId() const
	{
		return m_equipmentId;
	}

	bool SimServiceModule::isLogicModule() const
	{
		return m_data != nullptr && m_data->has_logicmodule();
	}

	SimServiceLogicModule SimServiceModule::toLogicModule() const
	{
		if (m_data == nullptr || m_data->has_logicmodule() == false)
		{
			throw std::bad_cast{};
		}

		return SimServiceLogicModule{m_client, m_data};
	}

	//
	// SimServiceLogicModule
	//
	SimServiceLogicModule::SimServiceLogicModule(ServiceClientImplPtr client, ModuleStatePtr data) :
		m_client{std::move(client)},
		m_data{std::move(data)},
		m_equipmentId{m_data == nullptr ? QString{} : QString::fromStdString(m_data->equipmentid())}
	{
		if (m_client == nullptr)
		{
			throw std::runtime_error("Where is a client?");
		}

		if (m_data == nullptr || m_data->has_logicmodule() == false)
		{
			throw std::bad_cast{};
		}

		return;
	}

	SimServiceLogicModule::~SimServiceLogicModule() = default;

	bool SimServiceLogicModule::isNull() const
	{
		return m_data == nullptr || m_data->ModuleOneOf_case() != ::RpctGrpc::ModuleState::kLogicModule;
	}

	tl::expected<void, QString> SimServiceLogicModule::update()
	{
		if (isNull() == true)
		{
			return tl::unexpected("Module is Null.");
		}

		auto r = m_client->GetModule({m_equipmentId});
		if (r.has_value() == true)
		{
			if (r->size() != 1)
			{
				assert(r.value().size() == 1);
				return tl::unexpected("GetModule() is expected to return exactly 1 module.");
			}

			auto& module = r.value().front();

			if (module.isLogicModule() == false)
			{
				return tl::unexpected("The module is not LogicModule anymore.");
			}

			*this = module.toLogicModule();
		}
		else
		{
			return tl::unexpected(r.error());
		}

		return {};
	}

	const QString& SimServiceLogicModule::equipmentId() const
	{
		return m_equipmentId;
	}

	QString SimServiceLogicModule::subsystemId() const
	{
		if (isNull() == true)
		{
			assert(isNull() == false);
			return {};
		}

		return QString::fromStdString(m_data->logicmodule().subsystemid());
	}

	int SimServiceLogicModule::lmNumber() const
	{
		if (isNull() == true)
		{
			assert(isNull() == false);
			return {};
		}

		return m_data->logicmodule().lmnumber();
	}

	int SimServiceLogicModule::channel() const
	{
		if (isNull() == true)
		{
			assert(isNull() == false);
			return {};
		}

		return m_data->logicmodule().channel();
	}

	bool SimServiceLogicModule::faultMode() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_FAULT);
	}

	bool SimServiceLogicModule::isPowerOn() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_POWER_ON);
	}

	bool SimServiceLogicModule::isPowerOff() const
	{
		return !isPowerOn();
	}

	tl::expected<void, QString> SimServiceLogicModule::setPowerOff(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_POWER_ON, !value);
	}

	bool SimServiceLogicModule::armingKey() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_ARMING_KEY);
	}

	tl::expected<void, QString> SimServiceLogicModule::setArmingKey(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_ARMING_KEY, value);
	}

	bool SimServiceLogicModule::tuningKey() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_TUNING_KEY);
	}

	tl::expected<void, QString> SimServiceLogicModule::setTuningKey(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_TUNING_KEY, value);
	}

	bool SimServiceLogicModule::sorIsSet() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_IS_SET);
	}

	bool SimServiceLogicModule::sorSetSwitch1() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_1);
	}

	tl::expected<void, QString> SimServiceLogicModule::setSorSetSwitch1(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_1, value);
	}

	bool SimServiceLogicModule::sorSetSwitch2() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_2);
	}

	tl::expected<void, QString> SimServiceLogicModule::setSorSetSwitch2(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_2, value);
	}

	bool SimServiceLogicModule::sorSetSwitch3() const
	{
		return getModuleFlag(m_data, ::RpctGrpc::ModuleFlagId::MF_LM_BOOL_SOR_SET_SWITCH_3);
	}

	tl::expected<void, QString> SimServiceLogicModule::setSorSetSwitch3(bool value)
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_SOR_SET_SWITCH_3, value);
	}

	tl::expected<void, QString> SimServiceLogicModule::raiseSorResetSwitch()
	{
		return setModuleFlag(m_client, m_data, m_equipmentId, ::RpctGrpc::MF_LM_BOOL_SOR_RESET, true);
	}
} // namespace Sim