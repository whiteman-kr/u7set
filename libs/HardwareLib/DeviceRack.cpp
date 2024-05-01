#include <HardwareLib/DeviceRack.h>
#include <HardwareLib/PropertyNames.h>

namespace Hardware
{
	//
	//
	// DeviceRack
	//
	//
	DeviceRack::DeviceRack(bool preset /*= false*/, QObject* parent/* = nullptr*/) :
		DeviceObject(DeviceType::Rack, preset, parent)
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

	bool DeviceRack::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
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
		[[maybe_unused]] auto rackMessage = message->MutableExtension(::Proto::deviceobject)->mutable_rack();

		return true;
	}

	bool DeviceRack::LoadData(const Proto::Envelope& message)
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

		if (deviceobject.has_rack() == false)
		{
			Q_ASSERT(deviceobject.has_rack());
			return false;
		}

		[[maybe_unused]] const auto& rackMessage = deviceobject.rack();

		return true;
	}
} // namespace Hardware