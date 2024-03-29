#include "./include/HardwareLib/DeviceSystem.h"
#include "./include/HardwareLib/PropertyNames.h"

namespace Hardware
{
	//
	//
	// DeviceSystem
	//
	//
	DeviceSystem::DeviceSystem(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::System, preset, parent)
	{
		auto p = propertyByCaption(PropertyNames::equipmentIdTemplate);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}
	}

	bool DeviceSystem::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
	{
		bool result = DeviceObject::SaveData(message, saveTree, predicate);
		if (result == false || message->HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->HasExtension(::Proto::deviceobject));
			return false;
		}

		// --
		//
		[[maybe_unused]] auto systemMessage = message->MutableExtension(::Proto::deviceobject)->mutable_system();

		return true;
	}

	bool DeviceSystem::LoadData(const Proto::Envelope& message)
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

		if (deviceobject.has_system() == false)
		{
			Q_ASSERT(deviceobject.has_system());
			return false;
		}

		[[maybe_unused]] auto&& systemMessage = deviceobject.system();

		return true;
	}
} // namespace Hardware