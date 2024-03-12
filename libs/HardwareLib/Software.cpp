#include "./include/HardwareLib/Software.h"
#include "./include/HardwareLib/Workstation.h"
#include "./include/HardwareLib/PropertyNames.h"

namespace Hardware
{
	//
	//
	// Software
	//
	//
	Software::Software(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Software, preset, parent)
	{
		ADD_PROPERTY_GETTER_SETTER(E::SoftwareType, PropertyNames::type, true, Software::softwareType, Software::setSoftwareType)
			->setExpert(true)
			.setUpdateFromPreset(true);
	}

	bool Software::SaveData(Proto::Envelope* message, bool saveTree) const
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
		auto softwareMessage = message->MutableExtension(::Proto::deviceobject)->mutable_software();

		softwareMessage->set_type(static_cast<int>(m_softwareType));

		return true;
	}

	bool Software::LoadData(const Proto::Envelope& message)
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

		if (deviceobject.has_software() == false)
		{
			Q_ASSERT(deviceobject.has_software());
			return false;
		}

		const auto& softwareMessage = deviceobject.software();

		m_softwareType = static_cast<E::SoftwareType>(softwareMessage.type());

		return true;
	}

	QString Software::hostname() const
	{
		const Hardware::Workstation* ws = getParentWorkstation();

		if (ws == nullptr)
		{
			Q_ASSERT(false);
			return QString();
		}

		return ws->hostname().trimmed();
	}

	E::SoftwareType Software::softwareType() const
	{
		return m_softwareType;
	}

	void Software::setSoftwareType(E::SoftwareType value)
	{
		m_softwareType = value;
	}
} // namespace Hardware