#ifndef BUS_H
#define BUS_H

#include "../lib/PropertyObject.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/Types.h"
#include "../lib/Hash.h"

class QDomElement;
class QXmlStreamWriter;

namespace VFrame30
{
	//
	// BusSignal
	//
	class VFRAME30LIBSHARED_EXPORT BusSignal : public PropertyObject
	{
	public:
		BusSignal();
		BusSignal(const BusSignal& src);
		BusSignal(E::SignalType type);
		BusSignal& operator= (const BusSignal& src);

	public:
		bool save(Proto::BusSignal* message) const;
		bool load(const Proto::BusSignal& message);

	public:
		QString signalId() const;
		void setSignalId(const QString& value);

		QString caption() const;
		void setCaption(const QString& value);

		E::SignalType type() const;

		QString units() const;
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
		QString m_signalId = QLatin1String("ID");
		QString m_caption = QLatin1String("Caption");
		E::SignalType m_type = E::SignalType::Discrete;
		QString m_units;

		// AnalogSignal settings
		//
		E::AnalogAppSignalFormat m_analogFormat = E::AnalogAppSignalFormat::Float32;
		int m_precision = 2;
		double m_coarseAperture = 1;
		double m_fineAperture = 0.5;
		bool m_adaptiveAperture = false;

		// BusSignalSettings
		//
		QString m_busTypeId;

		// Manual signal settings
		//
		int m_inbusOffset = 0;
		int m_inbusDiscreteBitNo = 0;

		int m_inbusAnalogSize = 32;
		E::DataFormat m_inbusAnalogFormat  = E::DataFormat::SignedInt;
		E::ByteOrder m_inbusAnalogByteOrder = E::ByteOrder::BigEndian;

		double m_busAnalogLowLimit = 0.0;
		double m_busAnalogHighLimit = 65535.0;

		double m_inbusAnalogLowLimit = 0.0;
		double m_inbusAnalogHighLimit = 65535.0;
	};

	//
	// Bus
	//
	class VFRAME30LIBSHARED_EXPORT Bus :
			public PropertyObject,
			public Proto::ObjectSerialization<Bus>
	{
	public:
		Bus();
		Bus(const Bus& src);
		Bus& operator= (const Bus& src);

		// Serializatin implementation of Proto::ObjectSerialization<>
		//
		friend Proto::ObjectSerialization<Bus>;

	public:
		bool save(Proto::Bus* message) const;
		bool load(const Proto::Bus& message);

	protected:
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

		QString fileName() const;
		void setFileName(const QString& value);

		QString busTypeId() const;
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

	private:
		QUuid m_uuid;
		QString m_fileName;

		QString m_busTypeId = "BUSTYPEID";
		std::vector<BusSignal> m_busSignals;

		bool m_autoSignalPlacement = true;
		int m_manualBusSize = 0;
	};

	//
	// BusSet
	//
	class VFRAME30LIBSHARED_EXPORT BusSet
	{
	public:
		BusSet() = default;

	public:
		bool hasBus(QString busTypeId) const;

		const VFrame30::Bus& bus(QString busTypeId) const;

		const std::vector<VFrame30::Bus>& busses() const;
		void setBusses(const std::vector<VFrame30::Bus>& src);

	private:
		std::vector<VFrame30::Bus> m_busses;
	};

}

#endif // BUS_H
