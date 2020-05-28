#pragma once

#include "../lib/DeviceObject.h"
#include "../Builder/Busses.h"

class Signal;

namespace Builder
{
	class SignalMacro
	{
	public:
		static const QString START_TOKEN;
		static const QString END_TOKEN;

		static const QString BUS_SIGNAL_ID_SEPARATOR;

		static const QString BUS_TYPE;
		static const QString BUS_APP_SIGNAL_ID;
		static const QString BUS_CUSTOM_APP_SIGNAL_ID;
		static const QString BUS_CAPTION;

	public:
		static QString expandDeviceSignalTemplate(	const Hardware::DeviceObject& startDeviceObject,
									   const QString& templateStr,
									   QString* errMsg);

		static QString expandBusSignalCaptionTemplate(const Signal& busParentSignal,
											 BusShared bus,
											 const BusSignal& busSignal);

	private:

		static QString expandDeviceObjectMacro(	const Hardware::DeviceObject& startDeviceObject,
									const QString& macroStr,
									QString* errMsg);

		static const Hardware::DeviceObject* getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
																  const QString& parentObjectType,
																  QString* errMsg);
	};

}

