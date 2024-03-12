#include "./include/HardwareLib/Workstation.h"
#include "./include/HardwareLib/PropertyNames.h"

namespace Hardware
{
	//
	//
	// Workstation
	//
	//
	Workstation::Workstation(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Workstation, preset, parent)
	{
		//auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, Workstation::type, Workstation::setType)
		//typeProp->setUpdateFromPreset(true);

		addProperty<QString, Workstation, &Workstation::hostname, &Workstation::setHostname>(PropertyNames::hostname, PropertyNames::categoryCommon, true)
			->setUpdateFromPreset(false)
			.setEssential(true)
			.setExpert(false);

		auto p = propertyByCaption(PropertyNames::equipmentIdTemplate);
		Q_ASSERT(p);
		if (p != nullptr)
		{
			p->setEssential(true);
		}
	}

	bool Workstation::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->HasExtension(::Proto::deviceobject));
			return false;
		}

		// --
		//
		auto workstationMessage = message->MutableExtension(::Proto::deviceobject)->mutable_workstation();

		workstationMessage->set_type(m_type);
		workstationMessage->set_hostname(m_hostname.toStdString());

		return true;
	}

	bool Workstation::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::deviceobject));
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		const auto& deviceobject = message.GetExtension(::Proto::deviceobject);
		if (deviceobject.has_workstation() == false)
		{
			Q_ASSERT(deviceobject.has_workstation());
			return false;
		}

		const auto& workstationMessage = deviceobject.workstation();

		m_type = workstationMessage.type();
		m_hostname = QString::fromStdString(workstationMessage.hostname());

		return true;
	}

	int Workstation::type() const
	{
		return m_type;
	}

	void Workstation::setType(int value)
	{
		m_type = value;
	}

	QString Workstation::hostname() const
	{
		return m_hostname;
	}

	void Workstation::setHostname(QString value)
	{
		m_hostname = value;
	}
} // namespace Hardware