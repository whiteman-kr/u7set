#include <HardwareLib/DeviceChassis.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>

namespace Hardware
{
	//
	//
	// DeviceChassis
	//
	//
	DeviceChassis::DeviceChassis(bool preset /*= false*/, QObject* parent/* = nullptr*/) :
		DeviceObject(DeviceType::Chassis, preset, parent)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(int, "Type", true, DeviceChassis::type, DeviceChassis::setType);
		typeProp->setUpdateFromPreset(true);
		typeProp->setExpert(true);

		auto p = propertyByCaption(PropertyNames::place);
		if (p == nullptr)
		{
			Q_ASSERT(p);
		}
		else
		{
			p->setEssential(true);
		}

	}

	bool DeviceChassis::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
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
		auto* chassisMessage = message->MutableExtension(::Proto::deviceobject)->mutable_chassis();

		chassisMessage->set_type(m_type);

		return true;
	}

	bool DeviceChassis::LoadData(const Proto::Envelope& message)
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
		if (deviceobject.has_chassis() == false)
		{
			Q_ASSERT(deviceobject.has_chassis());
			return false;
		}

		auto&& chassisMessage = deviceobject.chassis();

		m_type = chassisMessage.type();

		return true;
	}

	std::shared_ptr<DeviceModule> DeviceChassis::findLogicModule()
	{
		for (const auto& child : m_children)
		{
			if (child == nullptr)
			{
				Q_ASSERT(child);
				continue;
			}

			if (child->isModule())
			{
				std::shared_ptr<DeviceModule> module = child->toModule();

				if (module == nullptr)
				{
					Q_ASSERT(module);
					continue;
				}

				if (module->isLogicModule())
				{
					return module;
				}
			}
		}

		return {};
	}

	std::shared_ptr<DeviceModule> DeviceChassis::findLogicModuleOrBvb()
	{
		for (const auto& child : m_children)
		{
			if (child == nullptr)
			{
				Q_ASSERT(child);
				continue;
			}

			if (child->isModule())
			{
				std::shared_ptr<DeviceModule> module = child->toModule();

				if (module == nullptr)
				{
					Q_ASSERT(module);
					continue;
				}

				if (module->isLogicModule() || module->isBvb())
				{
					return module;
				}
			}
		}

		return {};
	}

	int DeviceChassis::type() const
	{
		return m_type;
	}

	void DeviceChassis::setType(int value)
	{
		m_type = value;
	}
} // namespace Hardware