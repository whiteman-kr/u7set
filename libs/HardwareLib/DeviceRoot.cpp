#include "./include/HardwareLib/DeviceRoot.h"

namespace Hardware
{
	//
	//
	// DeviceRoot
	//
	//
	DeviceRoot::DeviceRoot(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Root, preset, parent)
	{
	}
}