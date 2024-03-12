#pragma once

#include "../CommonLib/Factory.h"
#include "./include/HardwareLib/DeviceObject.h"

namespace Hardware
{
	//
	// Factory
	//
	extern Factory<DeviceObject> s_deviceObjectFactory;
} // namespace Hardware