#include <VFrame30/ActuatorHeader.h>

#include <HardwareLib/Subsystem.h>
#include <VFrame30/PropertyNames.h>


namespace VFrame30
{
	//
	// ActuatorSignal
	//
	ActuatorSignal::ActuatorSignal()
	{
		init();
		return;
	}

	ActuatorSignal::ActuatorSignal(const ActuatorSignal& other)
	{
		Proto::ActuatorSignal message;

		other.save(message);
		load(message);

		init();
		return;
	}

	void ActuatorSignal::init()
	{
		removeAllProperties();

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::signalId,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::signalId,
								 ActuatorSignal::setSignalId)
			->setDescription(tr("SignalID for the actuator type. Only alphanumeric characters and underscores are allowed."))
			.setValidator("[A-Za-z0-9_]+")
			.setViewOrder(1);

		ADD_PROPERTY_GET_SET_CAT(E::SignalType,
								 PropertyNames::signalType,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::signalType,
								 ActuatorSignal::setSignalType)
			->setViewOrder(2);

		ADD_PROPERTY_GET_SET_CAT(E::AnalogAppSignalFormat,
								 PropertyNames::analogFormat,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::analogFormat,
								 ActuatorSignal::setAnalogFormat)
			->setDescription(tr("Analog signal format for the actuator type if applicable."))
			.setViewOrder(3);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::busTypeId,
								 PropertyNames::commonCategory,
								 true,
								 ActuatorSignal::busTypeId,
								 ActuatorSignal::setBusTypeId)
			->setDescription(tr("BusTypeID for the actuator type if applicable."))
			.setViewOrder(4);

		return;
	}

	void ActuatorSignal::save(Proto::ActuatorSignal& message) const
	{
		message.set_signalid(m_signalId.toStdString());
		message.set_signaltype(static_cast<int>(m_signalType));
		message.set_analogformat(static_cast<int>(m_analogFormat));
		message.set_bustypeid(m_busTypeId.toStdString());
	}

	void ActuatorSignal::load(const Proto::ActuatorSignal& message)
	{
		m_signalId = QString::fromStdString(message.signalid());
		m_signalType = static_cast<E::SignalType>(message.signaltype());
		m_analogFormat = static_cast<E::AnalogAppSignalFormat>(message.analogformat());
		m_busTypeId = QString::fromStdString(message.bustypeid());
	}

	QString ActuatorSignal::signalId() const
	{
		return m_signalId;
	}

	void ActuatorSignal::setSignalId(const QString& signalId)
	{
		m_signalId = signalId;
	}

	E::SignalType ActuatorSignal::signalType() const
	{
		return m_signalType;
	}

	void ActuatorSignal::setSignalType(E::SignalType signalType)
	{
		m_signalType = signalType;
	}

	E::AnalogAppSignalFormat ActuatorSignal::analogFormat() const
	{
		return m_analogFormat;
	}

	void ActuatorSignal::setAnalogFormat(E::AnalogAppSignalFormat analogFormat)
	{
		m_analogFormat = analogFormat;
	}

	QString ActuatorSignal::busTypeId() const
	{
		return m_busTypeId;
	}

	void ActuatorSignal::setBusTypeId(const QString& busTypeId)
	{
		m_busTypeId = busTypeId;
	}

	//
	// ActuatorHeader
	//
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

		ADD_PROPERTY_CAT_VAR(PropertyVector<ActuatorSignal>, PropertyNames::inputs, PropertyNames::commonCategory, true, m_inputs)
			->setDescription(tr("Input signals for the actuator type."))
			.setViewOrder(8);

		ADD_PROPERTY_CAT_VAR(PropertyVector<ActuatorSignal>, PropertyNames::outputs, PropertyNames::commonCategory, true, m_outputs)
			->setDescription(tr("Output signals for the actuator type."))
			.setViewOrder(9);

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

		for (const auto& input : m_inputs)
		{
			auto* inputMessage = m->add_inputs();
			input->save(*inputMessage);
		}

		for (const auto& output : m_outputs)
		{
			auto* outputMessage = m->add_outputs();
			output->save(*outputMessage);
		}

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

		m_inputs.clear();
		m_inputs.reserve(m.inputs_size());
		for (const auto& inputMessage : m.inputs())
		{
			auto s = m_inputs.createItem();
			s->load(inputMessage);
			m_inputs.push_back(std::move(s));
		}

		m_outputs.clear();
		m_outputs.reserve(m.outputs_size());
		for (const auto& outputMessage : m.outputs())
		{
			auto s = m_outputs.createItem();
			s->load(outputMessage);
			m_outputs.push_back(std::move(s));
		}

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