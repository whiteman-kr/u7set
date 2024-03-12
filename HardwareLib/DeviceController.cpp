#include "./include/HardwareLib/DeviceController.h"
#include "./include/HardwareLib/PropertyNames.h"

namespace Hardware
{
	//
	//
	// DeviceController
	//
	//
	DeviceController::DeviceController(bool preset /*= false*/, QObject* parent /*= nullptr*/) :
		DeviceObject(DeviceType::Controller, preset, parent)
	{
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::diagDataOffset, PropertyNames::categoryDiagnostics, true, DeviceController::diagDataOffset, DeviceController::setDiagDataOffset)
			->setUpdateFromPreset(true);

		return;
	}

	bool DeviceController::SaveData(Proto::Envelope* message, bool saveTree) const
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
		auto controllerMessage = message->MutableExtension(::Proto::deviceobject)->mutable_controller();

		controllerMessage->set_diagdataoffset(m_diagDataOffset);

		return true;
	}

	bool DeviceController::LoadData(const Proto::Envelope& message)
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
		if (deviceobject.has_controller() == false)
		{
			Q_ASSERT(deviceobject.has_controller());
			return false;
		}

		const auto& controllerMessage = deviceobject.controller();

		m_diagDataOffset = controllerMessage.diagdataoffset();

		return true;
	}

	int DeviceController::diagDataOffset() const
	{
		return m_diagDataOffset;
	}

	void DeviceController::setDiagDataOffset(int value)
	{
		m_diagDataOffset = value;
	}
} // namespace Hardware