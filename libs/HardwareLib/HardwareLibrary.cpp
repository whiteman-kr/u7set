#include "./include/HardwareLib/HardwareLibrary.h"
#include "./include/HardwareLib/DeviceRoot.h"
#include "./include/HardwareLib/DeviceSystem.h"
#include "./include/HardwareLib/DeviceRack.h"
#include "./include/HardwareLib/DeviceChassis.h"
#include "./include/HardwareLib/DeviceModule.h"
#include "./include/HardwareLib/DeviceController.h"
#include "./include/HardwareLib/DeviceAppSignal.h"
#include "./include/HardwareLib/Workstation.h"
#include "./include/HardwareLib/Software.h"
#include "./include/HardwareLib/DiagSignal.h"
#include "DeviceObjectFactory.h"

namespace Hardware
{
	void init()
	{
		qDebug() << "Hardware::init";

		// --
		//
		static std::atomic<bool> firstRun = false;

		bool expected = false;
		bool firstTime = firstRun.compare_exchange_strong(expected, true);

		if (firstTime == false)
		{
			qDebug() << "Hardware::init is called for the second time";
			Q_ASSERT(firstTime);
			return;
		}

		// Register all device types factory.
		//
		s_deviceObjectFactory.Register<Hardware::DeviceRoot>();
		s_deviceObjectFactory.Register<Hardware::DeviceSystem>();
		s_deviceObjectFactory.Register<Hardware::DeviceRack>();
		s_deviceObjectFactory.Register<Hardware::DeviceChassis>();
		s_deviceObjectFactory.Register<Hardware::DeviceModule>();
		s_deviceObjectFactory.Register<Hardware::DeviceController>();
		s_deviceObjectFactory.Register<Hardware::DeviceAppSignal>();
		s_deviceObjectFactory.Register<Hardware::DeviceAppSignal>("DeviceSignal"); // DeviceAppSignal used to be DeviceSignal, so create fabric for DeviceSignal too
		s_deviceObjectFactory.Register<Hardware::Workstation>();
		s_deviceObjectFactory.Register<Hardware::Software>();
		s_deviceObjectFactory.Register<Hardware::DiagSignal>();

		return;
	}


	void shutdown()
	{
		qDebug() << "Hardware::Shutdown";
		DeviceObject::PrintRefCounter();
		return;
	}

	bool canCreateDevice(quint32 classNameHash)
	{
		return s_deviceObjectFactory.isRegistered(classNameHash);
	}
} // namespace Hardware