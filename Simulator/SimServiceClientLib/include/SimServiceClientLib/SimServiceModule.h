#pragma once

#include <CommonLib/expected.hpp>
#include <memory>

namespace RpctGrpc
{
	class ModuleState;
};

namespace Sim
{
	class SimServiceLogicModule;
	class SimServiceClientImpl;

	using ModuleStatePtr = std::shared_ptr<::RpctGrpc::ModuleState>;
	using ServiceClientImplPtr = std::shared_ptr<SimServiceClientImpl>;


	//
	// SimServiceModule
	//
	class SimServiceModule final           // This class can be stored in vector.
	{
		friend class SimServiceClientImpl; // The only place where it can be created.
		SimServiceModule(ServiceClientImplPtr client, ModuleStatePtr data);

	public:
		SimServiceModule(SimServiceModule&&) noexcept = default;
		SimServiceModule& operator=(SimServiceModule&&) noexcept = default;
		~SimServiceModule();

	public:
		bool isNull() const;
		tl::expected<void, QString> update();

		const QString& equipmentId() const;

		bool isLogicModule() const;
		SimServiceLogicModule toLogicModule() const;

	private:
		ServiceClientImplPtr m_client;
		ModuleStatePtr m_data;
		QString m_equipmentId;
	};


	//
	// SimServiceLogicModule
	//
	class SimServiceLogicModule final
	{
		friend class SimServiceModule; // The only place where it can be created.
		SimServiceLogicModule(ServiceClientImplPtr client, ModuleStatePtr data);

	public:
		~SimServiceLogicModule();

	public:
		bool isNull() const;
		tl::expected<void, QString> update();

		const QString& equipmentId() const;

		[[nodiscard]] QString subsystemId() const;
		[[nodiscard]] int lmNumber() const;
		[[nodiscard]] int channel() const; // 0 - A, 1 - B, 2 - C, 3 - D

		[[nodiscard]] bool faultMode() const;

		[[nodiscard]] bool isPowerOn() const;
		[[nodiscard]] bool isPowerOff() const;
		tl::expected<void, QString> setPowerOff(bool value);

		[[nodiscard]] bool armingKey() const;
		tl::expected<void, QString> setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		tl::expected<void, QString> setTuningKey(bool value);

		[[nodiscard]] bool sorIsSet() const;

		[[nodiscard]] bool sorSetSwitch1() const;
		tl::expected<void, QString> setSorSetSwitch1(bool value);

		[[nodiscard]] bool sorSetSwitch2() const;
		tl::expected<void, QString> setSorSetSwitch2(bool value);

		[[nodiscard]] bool sorSetSwitch3() const;
		tl::expected<void, QString> setSorSetSwitch3(bool value);

		tl::expected<void, QString> raiseSorResetSwitch();

	private:
		ServiceClientImplPtr m_client;
		ModuleStatePtr m_data;
		QString m_equipmentId;
	};
} // namespace Sim