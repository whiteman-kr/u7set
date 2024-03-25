#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceChassis
	//
	//
	class DeviceChassis : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceChassis(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceChassis() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	public:
		[[nodiscard]] std::shared_ptr<DeviceModule> findLogicModule();
		[[nodiscard]] std::shared_ptr<DeviceModule> findLogicModuleOrBvb();

		// Properties
		//
	public:
		[[nodiscard]] int type() const;
		void setType(int value);

		// Data
		//
	private:
		int m_type = 0;
	};
} // namespace Hardware