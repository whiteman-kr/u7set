#pragma once

namespace AppSignalLib
{
	class PropertyNames
	{
	  public:
		PropertyNames() = delete;

	  public:
		inline static const QString type{"Type"};
		inline static const QString precision{"Precision"};
		inline static const QString analogFormat{"AnalogFormat"};
		inline static const QString caption{"Caption"};

		inline static const QString units{"Units"};

		inline static const QString coarseAperture{"CoarseAperture"};
		inline static const QString fineAperture{"FineAperture"};
		inline static const QString adaptiveAperture{"AdaptiveAperture"};

		inline static const QString busTypeId{"BusTypeID"};
		inline static const QString busTypeFileName{"FileName"};
		inline static const QString busAutoSignalPlacement{"AutoSignalPlacement"};
		inline static const QString busEnableManualBusSize{"EnableManualBusSize"};
		inline static const QString busManualBusSize{"ManualBusSize"};
		inline static const QString busSignalId{"SignalID"};
		inline static const QString busInbusOffset{"Offset"};
		inline static const QString busInbusDiscreteBitNo{"BitNo"};
		inline static const QString busInbusAnalogSize{"Size"};
		inline static const QString busInbusAnalogFormat{"Format"};
		inline static const QString busInbusAnalogByteOrder{"ByteOrder"};
		inline static const QString busAnalogLowLimit{"BusSignalLowLimit"};
		inline static const QString busAnalogHightLimit{"BusSignalHighLimit"};
		inline static const QString busInbusAnalogLowLimit{"InbusSignalLowLimit"};
		inline static const QString busInbusAnalogHightLimit{"InbusSignalHighLimit"};

		inline static const QString busSettingCategory{"Bus Settings"};
		inline static const QString busInbusSettingCategory{"InBus Settings (Manual Signal Placement)"};

		inline static const QString apertureCategory{"Aperture"};
	};
} // namespace AppSiganlLib
