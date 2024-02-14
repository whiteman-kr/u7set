#pragma once

#include "../CommonLib/Types.h"
#include "../CommonLib/Hash.h"
#include "../CommonLib/PropertyObject.h"
#include "../Proto/ProtoSerialization.h"


class QDomElement;
class QXmlStreamWriter;

namespace AppSignalLib
{
	//
	// BusSignal
	//
	class BusSignal : public PropertyObject
	{
	public:
		BusSignal();
		BusSignal(const BusSignal& src);
		BusSignal(E::SignalType type);
		
		BusSignal& operator=(const BusSignal& src);
		BusSignal& operator=(BusSignal&& src) noexcept;

	private:
		void updateProperties();

	public:
		bool save(Proto::BusSignal* message) const;
		bool load(const Proto::BusSignal& message);

	public:
		const QString& signalId() const;
		void setSignalId(const QString& value);

		const QString& caption() const;
		void setCaption(const QString& value);

		E::SignalType type() const;

		const QString& units() const;
		void setUnits(const QString& value);

		E::AnalogAppSignalFormat analogFormat() const;
		void setAnalogFormat(E::AnalogAppSignalFormat value);

		int precision() const;
		void setPrecision(int value);

		double coarseAperture() const;
		void setCoarseAperture(double aperture);

		double fineAperture() const;
		void setFineAperture(double aperture);

		bool adaptiveAperture() const;
		void setAdaptiveAperture(bool adaptive);

		QString busTypeId() const;
		void setBusTypeId(const QString& value);

		// Manual setting properties
		//
		int inbusOffset() const;
		void setInbusOffset(int value);

		int inbusDiscreteBitNo() const;
		void setInbusDiscreteBitNo(int value);

		int inbusAnalogSize() const;
		void setInbusAnalogSize(int value);

		E::DataFormat inbusAnalogFormat() const;
		void setInbusAnalogFormat(E::DataFormat value);

		E::ByteOrder inbusAnalogByteOrder() const;
		void setInbusAnalogByteOrder(E::ByteOrder value);

		double busAnalogLowLimit() const;
		void setBusAnalogLowLimit(double value);

		double busAnalogHighLimit() const;
		void setBusAnalogHightLimit(double value);

		double inbusAnalogLowLimit() const;
		void setInbusAnalogLowLimit(double value);

		double inbusAnalogHighLimit() const;
		void setInbusAnalogHightLimit(double value);

	private:
		struct
		{
			QString signalId = QLatin1String("ID");
			QString caption = QLatin1String("Caption");
			E::SignalType type = E::SignalType::Discrete;
			QString units;

			// AnalogSignal settings
			//
			E::AnalogAppSignalFormat analogFormat = E::AnalogAppSignalFormat::Float32;
			int precision = 2;
			double coarseAperture = 1;
			double fineAperture = 0.5;
			bool adaptiveAperture = false;

			// BusSignalSettings
			//
			QString busTypeId;

			// Manual signal settings
			//
			int inbusOffset = 0;
			int inbusDiscreteBitNo = 0;

			int inbusAnalogSize = 32;
			E::DataFormat inbusAnalogFormat = E::DataFormat::SignedInt;
			E::ByteOrder inbusAnalogByteOrder = E::ByteOrder::BigEndian;

			double busAnalogLowLimit = 0.0;
			double busAnalogHighLimit = 65535.0;

			double inbusAnalogLowLimit = 0.0;
			double inbusAnalogHighLimit = 65535.0;
		} m_data;
	};

	//
	// Bus
	//
	class Bus :
			public PropertyObject,
			public Proto::ObjectSerialization<Bus>
	{
	public:
		Bus();
		Bus(const Bus& src);
		Bus& operator= (const Bus& src);

		// Serialization implementation of Proto::ObjectSerialization<>
		//
		friend Proto::ObjectSerialization<Bus>;

	public:
		bool save(Proto::Bus* message) const;
		bool load(const Proto::Bus& message);

	public:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		static std::shared_ptr<Bus> CreateObject(const Proto::Envelope& message);

		// End of implementation of Proto::ObjectSerialization<>
		//

		// Properties
		//
	public:
		QUuid uuid() const;
		void setUuid(const QUuid& uuid);

		const QString& fileName() const;
		void setFileName(const QString& value);

		const QString& busTypeId() const;
		void setBusTypeId(const QString& value);

		Hash calcHash() const;

		const std::vector<BusSignal>& busSignals() const;
		std::vector<BusSignal>& busSignals();

		void setBusSignals(const std::vector<BusSignal>& busSignals);
		void addSignal(const BusSignal& signal);
		bool removeSignalAt(int index);

		bool autoSignalPlacement() const;
		void setAutoSignalPlacement(bool value);

		int manualBusSize() const;
		void setManualBusSize(int value);

		bool enableManualBusSize() const;
		void setEnableManualBusSize(bool enable);

	private:
		QUuid m_uuid;
		QString m_fileName;

		QString m_busTypeId = "BUSTYPEID";
		std::vector<BusSignal> m_busSignals;

		bool m_autoSignalPlacement = true;
		int m_manualBusSize = 0;
		bool m_enableManualBusSize = false;

		bool m_enableManualBusSizeIsNotIntialized = false;		// This property is not shown for users in BusType editor!
																// Required only for FIRST loading of Bus message after
																// adding of enableManualBusSize field.

	public:
		static const char* mimeType; // = "application/x-radiybus";
	};

	//
	// BusSet
	//
	class BusSet
	{
	public:
		BusSet() = default;
		BusSet(const BusSet&) = default;
		BusSet(BusSet&&) = default;
		BusSet& operator=(const BusSet&) = default;
		BusSet& operator=(BusSet&&) = default;
		~BusSet() = default;

	public:
		void clear();
		bool hasBus(QString busTypeId) const;

		const AppSignalLib::Bus& bus(QString busTypeId) const;

		const std::vector<AppSignalLib::Bus>& busses() const;
		void setBusses(const std::vector<AppSignalLib::Bus>& src);
		void setBusses(std::vector<AppSignalLib::Bus>&& src);

	private:
		std::vector<AppSignalLib::Bus> m_busses;
	};

}
