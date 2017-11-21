#include "Bus.h"
#include "PropertyNames.h"
#include <QDomDocument>
#include <QXmlStreamWriter>

namespace VFrame30
{
	//
	// BusSignal
	//
	BusSignal::BusSignal() :
		BusSignal(E::SignalType::Discrete)		// Here proprties will be created
	{
	}

	BusSignal::BusSignal(const BusSignal& src) :
		BusSignal(src.m_type)					// Here proprties will be created
	{
		*this = src;
	}

	BusSignal::BusSignal(E::SignalType type) :
		m_type(type)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::busSignalId, true, BusSignal::signalId, BusSignal::setSignalId);
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::caption, true, BusSignal::caption, BusSignal::setCaption);
		ADD_PROPERTY_GETTER(E::SignalType, PropertyNames::type, true, BusSignal::type);

		switch (type)
		{
		case E::SignalType::Analog:
			ADD_PROPERTY_GETTER_SETTER(E::AnalogAppSignalFormat, PropertyNames::analogFormat, true, BusSignal::analogFormat, BusSignal::setAnalogFormat);
			ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::units, true, BusSignal::units, BusSignal::setUnits);
			ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, BusSignal::precision, BusSignal::setPrecision);

			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::coarseAperture, PropertyNames::apertureCategory, true, BusSignal::coarseAperture, BusSignal::setCoarseAperture);
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fineAperture, PropertyNames::apertureCategory, true, BusSignal::fineAperture, BusSignal::setFineAperture);
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::adaptiveAperture, PropertyNames::apertureCategory, true, BusSignal::adaptiveAperture, BusSignal::setAdaptiveAperture);

			// Inbus settings (manual)
			//
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busInbusOffset, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusOffset, BusSignal::setInbusOffset);
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busInbusAnalogSize, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusAnalogSize, BusSignal::setInbusAnalogSize);
			ADD_PROPERTY_GET_SET_CAT(E::DataFormat, PropertyNames::busInbusAnalogFormat, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusAnalogFormat, BusSignal::setInbusAnalogFormat);
			ADD_PROPERTY_GET_SET_CAT(E::ByteOrder, PropertyNames::busInbusAnalogByteOrder, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusAnalogByteOrder, BusSignal::setInbusAnalogByteOrder);
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::busAnalogLowLimit, PropertyNames::busInbusSettingCategory, true, BusSignal::busAnalogLowLimit, BusSignal::setBusAnalogLowLimit);
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::busAnalogHightLimit, PropertyNames::busInbusSettingCategory, true, BusSignal::busAnalogHighLimit, BusSignal::setBusAnalogHightLimit);
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::busInbusAnalogLowLimit, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusAnalogLowLimit, BusSignal::setInbusAnalogLowLimit);
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::busInbusAnalogHightLimit, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusAnalogHighLimit, BusSignal::setInbusAnalogHightLimit);
			break;
		case E::SignalType::Discrete:
			// Inbus settings (manual)
			//
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busInbusOffset, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusOffset, BusSignal::setInbusOffset);
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busInbusDiscreteBitNo, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusDiscreteBitNo, BusSignal::setInbusDiscreteBitNo);
			break;
		case E::SignalType::Bus:
			// Inbus settings (manual)
			//
			ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busInbusOffset, PropertyNames::busInbusSettingCategory, true, BusSignal::inbusOffset, BusSignal::setInbusOffset);
			break;
		default:
			assert(false);
		}

		return;
	}

	BusSignal& BusSignal::operator= (const BusSignal& src)
	{
		if (this != &src)
		{
			Proto::BusSignal message;
			src.save(&message);
			this->load(message);
		}

		return *this;
	}

	bool BusSignal::save(Proto::BusSignal* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_signalid(m_signalId.toStdString());
		message->set_caption(m_caption.toStdString());
		message->set_type(static_cast<int>(m_type));
		message->set_units(m_units.toStdString());
		message->set_analogformat(static_cast<int>(m_analogFormat));
		message->set_precision(m_precision);
		message->set_coarseaperture(m_coarseAperture);
		message->set_fineaperture(m_fineAperture);
		message->set_adaptiveaperture(m_adaptiveAperture);
		message->set_bustypeid(m_busTypeId.toStdString());

		message->set_inbusoffset(m_inbusOffset);
		message->set_inbusdiscretebitno(m_inbusDiscreteBitNo);
		message->set_inbusanalogsize(m_inbusAnalogSize);
		message->set_inbusanalogformat(static_cast<int>(m_inbusAnalogFormat));
		message->set_inbusanalogbyteorder(static_cast<int>(m_inbusAnalogByteOrder));
		message->set_busanaloglowlimit(m_busAnalogLowLimit);
		message->set_busanaloghighlimit(m_busAnalogHighLimit);
		message->set_inbusanaloglowlimit(m_inbusAnalogLowLimit);
		message->set_inbusanaloghighlimit(m_inbusAnalogHighLimit);

		return true;
	}

	bool BusSignal::load(const Proto::BusSignal& message)
	{
		m_signalId = QString::fromStdString(message.signalid());
		m_caption = QString::fromStdString(message.caption());
		m_type = static_cast<E::SignalType>(message.type());
		m_units = QString::fromStdString(message.units());
		m_analogFormat = static_cast<E::AnalogAppSignalFormat>(message.analogformat());
		m_precision = message.precision();
		m_coarseAperture = message.coarseaperture();
		m_fineAperture = message.fineaperture();
		m_adaptiveAperture = message.adaptiveaperture();
		m_busTypeId = QString::fromStdString(message.bustypeid());

		m_inbusOffset = message.inbusoffset();
		m_inbusDiscreteBitNo = message.inbusdiscretebitno();
		m_inbusAnalogSize = message.inbusanalogsize();
		m_inbusAnalogFormat = static_cast<E::DataFormat>(message.inbusanalogformat());
		m_inbusAnalogByteOrder = static_cast<E::ByteOrder>(message.inbusanalogbyteorder());
		m_busAnalogLowLimit = message.busanaloglowlimit();
		m_busAnalogHighLimit = message.busanaloghighlimit();
		m_inbusAnalogLowLimit = message.inbusanaloglowlimit();
		m_inbusAnalogHighLimit = message.inbusanaloghighlimit();

		return true;
	}

	QString BusSignal::signalId() const
	{
		return m_signalId;
	}

	void BusSignal::setSignalId(const QString& value)
	{
		m_signalId = value.trimmed();
	}

	QString BusSignal::caption() const
	{
		return m_caption;
	}

	void BusSignal::setCaption(const QString& value)
	{
		m_caption = value.trimmed();
	}

	E::SignalType BusSignal::type() const
	{
		return m_type;
	}

	QString BusSignal::units() const
	{
		return m_units;
	}

	void BusSignal::setUnits(const QString& value)
	{
		m_units = value.trimmed();
	}

	E::AnalogAppSignalFormat BusSignal::analogFormat() const
	{
		return m_analogFormat;
	}

	void BusSignal::setAnalogFormat(E::AnalogAppSignalFormat value)
	{
		m_analogFormat = value;
	}

	int BusSignal::precision() const
	{
		return m_precision;
	}

	void BusSignal::setPrecision(int value)
	{
		m_precision = value;
	}

	double BusSignal::coarseAperture() const
	{
		return m_coarseAperture;
	}

	void BusSignal::setCoarseAperture(double aperture)
	{
		m_coarseAperture = aperture;
	}

	double BusSignal::fineAperture() const
	{
		return m_fineAperture;
	}

	void BusSignal::setFineAperture(double aperture)
	{
		m_fineAperture = aperture;
	}

	bool BusSignal::adaptiveAperture() const
	{
		return m_adaptiveAperture;
	}

	void BusSignal::setAdaptiveAperture(bool adaptive)
	{
		m_adaptiveAperture = adaptive;
	}

	QString BusSignal::busTypeId() const
	{
		return m_busTypeId;
	}

	void BusSignal::setBusTypeId(const QString& value)
	{
		assert(m_type == E::SignalType::Bus);
		m_busTypeId = value;
	}

	int BusSignal::inbusOffset() const
	{
		return m_inbusOffset;
	}

	void BusSignal::setInbusOffset(int value)
	{
		m_inbusOffset = value;
	}

	int BusSignal::inbusDiscreteBitNo() const
	{
		return m_inbusDiscreteBitNo;
	}

	void BusSignal::setInbusDiscreteBitNo(int value)
	{
		m_inbusDiscreteBitNo = value;
	}

	int BusSignal::inbusAnalogSize() const
	{
		return m_inbusAnalogSize;
	}

	void BusSignal::setInbusAnalogSize(int value)
	{
		m_inbusAnalogSize = value;
	}

	E::DataFormat BusSignal::inbusAnalogFormat() const
	{
		return m_inbusAnalogFormat;
	}

	void BusSignal::setInbusAnalogFormat(E::DataFormat value)
	{
		m_inbusAnalogFormat = value;
	}

	E::ByteOrder BusSignal::inbusAnalogByteOrder() const
	{
		return m_inbusAnalogByteOrder;
	}

	void BusSignal::setInbusAnalogByteOrder(E::ByteOrder value)
	{
		m_inbusAnalogByteOrder = value;
	}

	double BusSignal::busAnalogLowLimit() const
	{
		return m_busAnalogLowLimit;
	}

	void BusSignal::setBusAnalogLowLimit(double value)
	{
		m_busAnalogLowLimit = value;
	}

	double BusSignal::busAnalogHighLimit() const
	{
		return m_busAnalogHighLimit;
	}

	void BusSignal::setBusAnalogHightLimit(double value)
	{
		m_busAnalogHighLimit = value;
	}

	double BusSignal::inbusAnalogLowLimit() const
	{
		return m_inbusAnalogLowLimit;
	}

	void BusSignal::setInbusAnalogLowLimit(double value)
	{
		m_inbusAnalogLowLimit = value;
	}

	double BusSignal::inbusAnalogHighLimit() const
	{
		return m_inbusAnalogHighLimit;
	}

	void BusSignal::setInbusAnalogHightLimit(double value)
	{
		m_inbusAnalogHighLimit = value;
	}

	//
	// Bus
	//
	Bus::Bus() :
		PropertyObject(),
		Proto::ObjectSerialization<Bus>(Proto::ProtoCompress::Never),
		m_uuid(QUuid::createUuid())
	{
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::busTypeId, true, Bus::busTypeId,  Bus::setBusTypeId);

		auto fileNameProp = ADD_PROPERTY_GETTER(QString, PropertyNames::busTypeFileName, true, Bus::fileName);
		fileNameProp->setExpert(true);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::busAutoSignalPlacemanet, PropertyNames::busSettingCategory, true, Bus::autoSignalPlacement, Bus::setAutoSignalPlacement);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::busManualBusSize, PropertyNames::busSettingCategory, true, Bus::manualBusSize, Bus::setManualBusSize);

		return;
	}


	Bus::Bus(const Bus& src):
		Bus()	// Here properties will be created
	{
		*this = src;
	}

	Bus& Bus::operator= (const Bus& src)
	{
		if (this != &src)
		{
			Proto::Bus message;
			src.save(&message);
			this->load(message);
		}

		return *this;
	}

	bool Bus::save(Proto::Bus* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		bool ok = true;

		Proto::Write(message->mutable_uuid(), m_uuid);
		message->set_bustypeid(m_busTypeId.toStdString());
		message->set_autosignalplacement(m_autoSignalPlacement);
		message->set_manualbussize(m_manualBusSize);

		for (const BusSignal& bs : m_busSignals)
		{
			Proto::BusSignal* busSignalMessage = message->add_bussignals();
			ok &= bs.save(busSignalMessage);
		}

		return ok;
	}

	bool Bus::load(const Proto::Bus& message)
	{
		bool ok = true;

		m_uuid = Proto::Read(message.uuid());
		m_busTypeId = QString::fromStdString(message.bustypeid());
		m_autoSignalPlacement = message.autosignalplacement();
		m_manualBusSize = message.manualbussize();

		int busSignalCount = message.bussignals_size();

		m_busSignals.clear();
		m_busSignals.reserve(busSignalCount);

		for (int i = 0; i < busSignalCount; i++)
		{
			const Proto::BusSignal& busSignalMessage = message.bussignals(i);

			m_busSignals.emplace_back();
			ok &= m_busSignals.back().load(busSignalMessage);
		}

		return ok;
	}

	bool Bus::SaveData(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		std::string className = {"Bus"};
		quint32 classnamehash = CUtils::GetClassHashCode(className);
		message->set_classnamehash(classnamehash);

		bool ok = save(message->mutable_bus());

		return ok;
	}

	bool Bus::LoadData(const Proto::Envelope& message)
	{
		if (message.has_bus() == false ||
			message.classnamehash() != CUtils::GetClassHashCode("Bus"))
		{
			assert(message.has_bus());
			assert(message.classnamehash() == CUtils::GetClassHashCode("Bus"));
			return false;
		}

		bool ok = load(message.bus());

		return ok;
	}

	std::shared_ptr<Bus> Bus::CreateObject(const Proto::Envelope& message)
	{
		std::shared_ptr<Bus> object = std::make_shared<Bus>();
		object->LoadData(message);

		return object;
	}

	QUuid Bus::uuid() const
	{
		return m_uuid;
	}

	void Bus::setUuid(const QUuid& uuid)
	{
		m_uuid = uuid;
	}

	QString Bus::fileName() const
	{
		return m_fileName;
	}

	void Bus::setFileName(const QString& value)
	{
		m_fileName = value;
	}


	QString Bus::busTypeId() const
	{
		return m_busTypeId;
	}

	void Bus::setBusTypeId(const QString& value)
	{
		m_busTypeId = value.trimmed();
	}

	Hash Bus::calcHash() const
	{
		QByteArray data;
		bool ok = Save(data);

		if (ok == false)
		{
			return UNDEFINED_HASH;
		}

		Hash h = ::calcHash(data);
		return h;
	}

	const std::vector<BusSignal>& Bus::busSignals() const
	{
		return m_busSignals;
	}

	std::vector<BusSignal>& Bus::busSignals()
	{
		return m_busSignals;
	}

	void Bus::setBusSignals(const std::vector<BusSignal>& busSignals)
	{
		m_busSignals = busSignals;
	}

	void Bus::addSignal(const BusSignal& signal)
	{
		m_busSignals.push_back(signal);
	}

	bool Bus::removeSignalAt(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_busSignals.size()))
		{
			assert(false);
			return false;
		}

		m_busSignals.erase(m_busSignals.begin() + index);

		return true;
	}

	bool Bus::autoSignalPlacement() const
	{
		return m_autoSignalPlacement;
	}

	void Bus::setAutoSignalPlacement(bool value)
	{
		m_autoSignalPlacement = value;
	}

	int Bus::manualBusSize() const
	{
		return m_manualBusSize;
	}

	void Bus::setManualBusSize(int value)
	{
		m_manualBusSize = qBound(0, value, 256);
	}

	//
	// BusSet
	//
	bool BusSet::hasBus(QString busTypeId) const
	{
		auto it = std::find_if(m_busses.begin(), m_busses.end(),
			[&busTypeId](const VFrame30::Bus& bus)
			{
				return bus.busTypeId() == busTypeId;
			});
		return it != m_busses.end();
	}

	const VFrame30::Bus& BusSet::bus(QString busTypeId) const
	{
		auto it = std::find_if(m_busses.begin(), m_busses.end(),
			[&busTypeId](const VFrame30::Bus& bus)
			{
				return bus.busTypeId() == busTypeId;
			});

		if (it == m_busses.end())
		{
			static const VFrame30::Bus staticBus;
			return staticBus;
		}
		else
		{
			return *it;
		}
	}

	const std::vector<VFrame30::Bus>& BusSet::busses() const
	{
		return m_busses;
	}

	void BusSet::setBusses(const std::vector<VFrame30::Bus>& src)
	{
		m_busses = src;
	}
}
