#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceRack
	//
	//
	class DeviceRack : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceRack(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceRack() = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;
	};
} // namespace Hardware