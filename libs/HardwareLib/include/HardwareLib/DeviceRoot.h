#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DeviceRoot
	//
	//
	class DeviceRoot : public DeviceObject
	{
		Q_OBJECT

	public:
		explicit DeviceRoot(bool preset = false, QObject* parent = nullptr);
		virtual ~DeviceRoot() = default;
	};
} // namespace Hardware