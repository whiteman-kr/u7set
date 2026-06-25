#include <VFrame30/ActuatorHeader.h>

#include <HardwareLib/Subsystem.h>
#include <VFrame30/PropertyNames.h>


namespace VFrame30
{
	ActuatorHeader::ActuatorHeader(QObject* parent) :
		PropertyObject(parent)
	{
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::ActuatorTypeId,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::actuatorTypeId,
								 ActuatorHeader::setActuatorTypeId)
			->setDescription(tr("Unique identifier for the actuator type. Only alphanumeric characters and underscores are allowed."))
			.setValidator("[A-Za-z0-9_]+")
			.setViewOrder(1);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::caption,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::caption,
								 ActuatorHeader::setCaption)
			->setDescription(tr("Caption for the actuator type."))
			.setViewOrder(2);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::acmPreset,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::acmPresetName,
								 ActuatorHeader::setAcmPresetName)
			->setDescription(tr("ACM preset name for the actuator type."))
			.setViewOrder(3);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::descriptionFile,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::descriptionFile,
								 ActuatorHeader::setDescriptionFile)
			->setDescription(tr("Description file for the actuator type."))
			.setViewOrder(4);

		// Subsystem
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::subsystemId,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::subsystemId,
								 ActuatorHeader::setSubsystemId)
			->setDescription(tr("SubsystemID for the actuator type."))
			.setViewOrder(5);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::lmNumber,
								 PropertyNames::actuatorCategory,
								 true,
								 ActuatorHeader::lmNumber,
								 ActuatorHeader::setLmNumber)
			->setDescription(tr("LM number in subsystem for the actuator type."))
			.setViewOrder(6);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::excludeFromBuild,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorHeader::excludeFromBuild,
								 ActuatorHeader::setExcludeFromBuild)
			->setDescription(tr("Exclude the actuator type from the build process."))
			.setViewOrder(7);

		return;
	}

	std::shared_ptr<ActuatorHeader> ActuatorHeader::CreateObject(const Proto::Envelope& message)
	{
		auto result = std::make_shared<ActuatorHeader>();
		if (result->LoadData(message) == false)
		{
			result.reset();
			return result;
		}

		return result;
	}

	bool ActuatorHeader::SaveData(::Proto::Envelope* message) const
	{
		message->set_classnamehash(::ClassNameHashCode("ActuatorHeader"));

		auto* m = message->MutableExtension(::Proto::actuatorHeader);

		m->set_actuatortypeid(m_actuatorTypeId.toStdString());
		m->set_caption(m_caption.toStdString());

		m->set_acmpresetname(m_acmPresetName.toStdString());
		m->set_descriptionfile(m_descriptionFile.toStdString());

		m->set_lmnumber(m_lmNumber);
		m->set_subsystemid(m_subsystemId.toStdString());

		m->set_excludefrombuild(m_excludeFromBuild);

		return true;
	}

	bool ActuatorHeader::LoadData(const ::Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::actuatorHeader) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::actuatorHeader));
			return false;
		}

		const auto& m = message.GetExtension(::Proto::actuatorHeader);

		m_actuatorTypeId = QString::fromStdString(m.actuatortypeid());
		m_caption = QString::fromStdString(m.caption());

		m_acmPresetName = QString::fromStdString(m.acmpresetname());
		m_descriptionFile = QString::fromStdString(m.descriptionfile());

		m_lmNumber = m.lmnumber();
		m_subsystemId = QString::fromStdString(m.subsystemid());

		m_excludeFromBuild = m.excludefrombuild();

		return true;
	}

	void ActuatorHeader::propertyDemand([[maybe_unused]] const QString& prop) {}

	QString ActuatorHeader::actuatorTypeId() const
	{
		return m_actuatorTypeId;
	}

	void ActuatorHeader::setActuatorTypeId(const QString& actuatorTypeId)
	{
		m_actuatorTypeId = actuatorTypeId;
	}

	QString ActuatorHeader::caption() const
	{
		return m_caption;
	}

	void ActuatorHeader::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	QString ActuatorHeader::acmPresetName() const
	{
		return m_acmPresetName;
	}

	void ActuatorHeader::setAcmPresetName(const QString& acmPresetName)
	{
		m_acmPresetName = acmPresetName;
	}

	QString ActuatorHeader::descriptionFile() const
	{
		return m_descriptionFile;
	}

	void ActuatorHeader::setDescriptionFile(const QString& descriptionFile)
	{
		m_descriptionFile = descriptionFile;
	}

	int ActuatorHeader::lmNumber() const
	{
		return m_lmNumber;
	}

	void ActuatorHeader::setLmNumber(int lmNumber)
	{
		m_lmNumber = std::clamp(lmNumber, 0, maxLmNumber());
	}

	int ActuatorHeader::maxLmNumber()
	{
		return Hardware::Subsystem::MaxChannelValue;
	}

	QString ActuatorHeader::subsystemId() const
	{
		return m_subsystemId;
	}

	void ActuatorHeader::setSubsystemId(const QString& subsystemId)
	{
		m_subsystemId = subsystemId;
	}

	bool ActuatorHeader::excludeFromBuild() const
	{
		return m_excludeFromBuild;
	}

	void ActuatorHeader::setExcludeFromBuild(bool excludeFromBuild)
	{
		m_excludeFromBuild = excludeFromBuild;
	}

} // namespace VFrame30