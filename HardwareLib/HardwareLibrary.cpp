#include "HardwareLibrary.h"
#include "DeviceObject.h"
#include "DiagSignal.h"

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
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRoot>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceSystem>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceRack>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceChassis>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceModule>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceController>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceAppSignal>();
		Hardware::DeviceObjectFactory.Register<Hardware::DeviceAppSignal>("DeviceSignal"); // DeviceAppSignal used to be DeviceSignal, so create fabric for DeviceSignal too
		Hardware::DeviceObjectFactory.Register<Hardware::Workstation>();
		Hardware::DeviceObjectFactory.Register<Hardware::Software>();
		Hardware::DeviceObjectFactory.Register<Hardware::DiagSignal>();

		return;
	}


	void shutdown()
	{
		qDebug() << "Hardware::Shutdown";
		DeviceObject::PrintRefCounter();
		return;
	}
} // namespace Hardware