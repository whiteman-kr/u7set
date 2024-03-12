#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceSystem
	//
	//
	class DeviceSystem : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceSystem(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceSystem() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
} // namespace Hardware