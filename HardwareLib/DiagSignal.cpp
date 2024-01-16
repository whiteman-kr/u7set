#include "DiagSignal.h"
#include "DiagSignalType.h"


namespace Hardware
{
	//
	//
	// DiagSignal
	//
	//
	DiagSignal::DiagSignal(bool preset /*= false*/, QObject* parent /*= nullptr*/) noexcept :
		DeviceObject(DeviceType::DiagSignal, preset, parent)
	{
	}

	void DiagSignal::propertyDemand(const QString& prop)
	{
		DeviceObject::propertyDemand(prop);

		// Category DiagSignal
		//
		ADD_PROPERTY_GET_SET_CAT(E::DiagLevel, PropertyNames::level, PropertyNames::categoryDiagSignal, true, DiagSignal::level, DiagSignal::setLevel)
			->setUpdateFromPreset(true)
			.setViewOrder(100);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalTypeId, PropertyNames::categoryDiagSignal, true, DiagSignal::signalTypeId, DiagSignal::setSignalTypeId)
			->setUpdateFromPreset(true)
			.setViewOrder(101);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::validitySignalId, PropertyNames::categoryDiagSignal, true, DiagSignal::validitySignalId, DiagSignal::setValiditySignalId)
			->setUpdateFromPreset(true)
			.setViewOrder(102);

		// Category data
		//
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueOffset, PropertyNames::categoryData, true, DiagSignal::valueOffset, DiagSignal::setValueOffset)
			->setUpdateFromPreset(true)
			.setViewOrder(200);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueBit, PropertyNames::categoryData, true, DiagSignal::valueBit, DiagSignal::setValueBit)
			->setUpdateFromPreset(true)
			.setViewOrder(201);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueBitSize, PropertyNames::categoryData, true, DiagSignal::valueBitSize, DiagSignal::setValueBitSize)
			->setUpdateFromPreset(true)
			.setDescription(PropertyNames::valueBitSizeDescription)
			.setViewOrder(202);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::discreteContainerSize, PropertyNames::categoryData, true, DiagSignal::discreteContainerSize, DiagSignal::setDiscreteContainerSize)
			->setUpdateFromPreset(true)
			.setDescription(PropertyNames::discreteContainerSizeDescription)
			.setViewOrder(203);

		// Category MATS
		//
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::logChanges, PropertyNames::categoryMats, true, DiagSignal::logChanges, DiagSignal::setLogChanges)
			->setViewOrder(300);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::archive, PropertyNames::categoryMats, true, DiagSignal::archive, DiagSignal::setArchive)
			->setViewOrder(301);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::reserved, PropertyNames::categoryMats, true, DiagSignal::reserved, DiagSignal::setReserved)
			->setViewOrder(302);

		ADD_PROPERTY_GET_SET_CAT(E::ApertureType, PropertyNames::apertureType, PropertyNames::categoryMats, true, DiagSignal::apertureType, DiagSignal::setApertureType)
			->setViewOrder(400);

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fineAperture, PropertyNames::categoryMats, true, DiagSignal::fineAperture, DiagSignal::setFineAperture)
			->setViewOrder(401);

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::coarseAperture, PropertyNames::categoryMats, true, DiagSignal::coarseAperture, DiagSignal::setCoarseAperture)
			->setViewOrder(402);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::decimalPlaces, PropertyNames::categoryMats, true, DiagSignal::decimalPlaces, DiagSignal::setDecimalPlaces)
			->setViewOrder(403);

		return;
	}

	bool DiagSignal::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		::Proto::DeviceDiagSignal* signalMessage = message->mutable_deviceobject()->mutable_diagsignal();

		signalMessage->set_level(static_cast<int>(m_level));
		signalMessage->set_signaltypeid(m_signalTypeId.toUtf8());
		signalMessage->set_validitysignalid(m_validitySignalId.toUtf8());

		signalMessage->set_valueoffset(m_valueOffset);
		signalMessage->set_valuebit(m_valueBit);
		signalMessage->set_valuebitsize(m_valueBitSize);
		signalMessage->set_discretecontainersize(m_discreteContainerSize);

		signalMessage->set_logchanges(m_logChanges);
		signalMessage->set_archive(m_archive);
		signalMessage->set_reserved(m_reserved);

		signalMessage->set_coarseaperture(m_coarseAperture);
		signalMessage->set_fineaperture(m_fineAperture);
		signalMessage->set_aperturetype(static_cast<int>(m_apertureType));

		signalMessage->set_decimalplaces(m_decimalPlaces);

		return true;
	}

	bool DiagSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_diagsignal() == false)
		{
			Q_ASSERT(message.deviceobject().has_diagsignal());
			return false;
		}

		const Proto::DeviceDiagSignal& signalMessage = message.deviceobject().diagsignal();

		m_level = static_cast<E::DiagLevel>(signalMessage.level());
		m_signalTypeId = QString::fromUtf8(signalMessage.signaltypeid().data());
		m_validitySignalId = QString::fromUtf8(signalMessage.validitysignalid().data());

		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();
		m_valueBitSize = signalMessage.valuebitsize();
		m_discreteContainerSize = signalMessage.discretecontainersize();

		m_logChanges = signalMessage.logchanges();
		m_archive = signalMessage.archive();
		m_reserved = signalMessage.reserved();

		m_coarseAperture = signalMessage.coarseaperture();
		m_fineAperture = signalMessage.fineaperture();
		m_apertureType = static_cast<E::ApertureType>(signalMessage.aperturetype());

		m_decimalPlaces = signalMessage.decimalplaces();

		return true;
	}

	void DiagSignal::expandEquipmentId()
	{
		if (m_validitySignalId.isEmpty() == false)
		{
			if (hasParent() == true)
			{
				m_validitySignalId.replace(QLatin1String("$(PARENT)"), parent()->equipmentIdTemplate(), Qt::CaseInsensitive);
			}

			m_validitySignalId.replace(QLatin1String("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);
		}

		DeviceObject::expandEquipmentId();
		return;
	}

	E::DiagLevel DiagSignal::level() const
	{
		return m_level;
	}

	void DiagSignal::setLevel(E::DiagLevel value)
	{
		m_level = value;
	}

	const QString& DiagSignal::signalTypeId() const
	{
		return m_signalTypeId;
	}

	void DiagSignal::setSignalTypeId(const QString& value)
	{
		m_signalTypeId = value;
	}

	const QString& DiagSignal::validitySignalId() const
	{
		return m_validitySignalId;
	}

	void DiagSignal::setValiditySignalId(const QString& value)
	{
		m_validitySignalId = value;
	}

	int DiagSignal::valueOffset() const
	{
		return m_valueOffset;
	}

	void DiagSignal::setValueOffset(int value)
	{
		m_valueOffset = value;
	}

	int DiagSignal::valueBit() const
	{
		return m_valueBit;
	}

	void DiagSignal::setValueBit(int value)
	{
		m_valueBit = std::clamp(value, 0, value);
	}

	int DiagSignal::valueBitSize() const
	{
		return m_valueBitSize;
	}

	void DiagSignal::setValueBitSize(int value)
	{
		m_valueBitSize = std::clamp(value, 0, 256);
	}

	int DiagSignal::discreteContainerSize() const
	{
		return m_discreteContainerSize;
	}

	void DiagSignal::setDiscreteContainerSize(int value)
	{
		m_discreteContainerSize = std::clamp(value, 0, 1024);
	}

	bool DiagSignal::logChanges() const
	{
		return m_logChanges;
	}

	void DiagSignal::setLogChanges(bool value)
	{
		m_logChanges = value;
	}

	bool DiagSignal::archive() const
	{
		return m_archive;
	}

	void DiagSignal::setArchive(bool value)
	{
		m_archive = value;
	}

	bool DiagSignal::reserved() const
	{
		return m_reserved;
	}

	void DiagSignal::setReserved(bool value)
	{
		m_reserved = value;
	}

	double DiagSignal::coarseAperture() const
	{
		return m_coarseAperture;
	}

	void DiagSignal::setCoarseAperture(double value)
	{
		m_coarseAperture = value;
	}

	double DiagSignal::fineAperture() const
	{
		return m_fineAperture;
	}

	void DiagSignal::setFineAperture(double value)
	{
		m_fineAperture = value;
	}

	E::ApertureType DiagSignal::apertureType() const
	{
		return m_apertureType;
	}

	void DiagSignal::setApertureType(E::ApertureType value)
	{
		m_apertureType = value;
	}

	int DiagSignal::decimalPlaces() const
	{
		return m_decimalPlaces;
	}

	void DiagSignal::setDecimalPlaces(int value)
	{
		m_decimalPlaces = std::clamp(value, 0, 32);
	}

	const std::shared_ptr<Hardware::DiagSignalTypeObject>& DiagSignal::diagSignalType() const
	{
		return m_diagSignalType;
	}

	void DiagSignal::setDiagSignalType(std::shared_ptr<DiagSignalTypeObject> value)
	{
		if (value != nullptr)
		{
			Q_ASSERT(value->signalTypeId() == signalTypeId());
		}

		m_diagSignalType = std::move(value);
	}

	const std::shared_ptr<Hardware::DiagSignal>& DiagSignal::validitySignal() const
	{
		return m_validitySignal;
	}

	void DiagSignal::setValiditySignal(std::shared_ptr<DiagSignal> value)
	{
		if (value != nullptr)
		{
			Q_ASSERT(value->equipmentId() == validitySignalId());
		}

		m_validitySignal = std::move(value);
	}
} // namespace Hardware