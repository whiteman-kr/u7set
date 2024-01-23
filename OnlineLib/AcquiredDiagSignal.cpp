#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "../UtilsLib/WUtils.h"
#include "AcquiredDiagSignal.h"

// -----------------------------------------------------------------------------------------
//
// AcquiredDiagObject class implementation
//
// -----------------------------------------------------------------------------------------

AcquiredDiagObject::AcquiredDiagObject()
{
}

AcquiredDiagObject::AcquiredDiagObject(DeviceObjectConstShared devObj)
{
	TEST_PTR_RETURN(devObj);

	equipmentID = devObj->equipmentIdTemplate();
	deviceType =devObj->deviceType();

	auto parent = devObj->parent();

	if (parent != nullptr)
	{
		parentHash = calcHash(parent->equipmentIdTemplate());
	}
}

void AcquiredDiagObject::saveToProto(Network::AcquiredDiagObject* proto)
{
	TEST_PTR_RETURN(proto);

	proto->set_equipmentid(equipmentID.toStdString());
	proto->set_devicetype(TO_INT(deviceType));
	proto->set_parenthash(parentHash);
}

bool AcquiredDiagObject::loadFromProto(const Network::AcquiredDiagObject& proto)
{
	equipmentID = QString::fromStdString(proto.equipmentid());
	deviceType = static_cast<Hardware::DeviceType>(proto.devicetype());
	parentHash = proto.parenthash();

	return true;
}

// -----------------------------------------------------------------------------------------
//
// AcquiredDiagSignal class implementation
//
// -----------------------------------------------------------------------------------------

AcquiredDiagSignal::AcquiredDiagSignal()
{
}

AcquiredDiagSignal::AcquiredDiagSignal(DiagSignalConstShared diagSignal)
{
	TEST_PTR_RETURN(diagSignal);

	equipmentIdHash = calcHash(diagSignal->equipmentIdTemplate());

	auto parent = diagSignal->parent();

	if (parent != nullptr)
	{
		parentHash = calcHash(parent->equipmentIdTemplate());
	}

	diagLevel = diagSignal->level();
	diagSignalTypeIdHash = calcHash(diagSignal->signalTypeId());
	isReflection = diagSignal->isReflection();
	reflectedSignalIdHash = calcHash(diagSignal->reflectedSignalId());
	validitySignalIdHash = calcHash(diagSignal->validitySignalId());
	valueSizeBit = diagSignal->valueBitSize();
	discreteContainerSize = diagSignal->discreteContainerSize();
	logChanges = diagSignal->logChanges();
	archive = diagSignal->archive();
	reserved = diagSignal->reserved();
	apertureType = diagSignal->apertureType();
	coarseAperture = diagSignal->coarseAperture();
	fineAperture = diagSignal->fineAperture();

	// signal data address from beginning of module diag data offset in RUP diag packet
	// includes DiagDataOffset of parent controllers
	//
	absAddr = Address16(diagSignal->valueOffset(), diagSignal->valueBit());
}

void AcquiredDiagSignal::saveToProto(Network::AcquiredDiagSignal* proto)
{
	TEST_PTR_RETURN(proto);

	proto->set_equipmentidhash(equipmentIdHash);
	proto->set_parenthash(parentHash);
	proto->set_diaglevel(TO_INT(diagLevel));
	proto->set_diagsignaltypeidhash(diagSignalTypeIdHash);

	proto->set_isreflection(isReflection);
	proto->set_reflectedsignalidhash(reflectedSignalIdHash);
	proto->set_validitysignalidhash(validitySignalIdHash);
	proto->set_valuesizebit(valueSizeBit);
	proto->set_discretecontainersize(discreteContainerSize);
	proto->set_logchanges(logChanges);
	proto->set_archive(archive);
	proto->set_reserved(reserved);
	proto->set_aperturetype(TO_INT(apertureType));
	proto->set_coarseaperture(coarseAperture);
	proto->set_fineaperture(fineAperture);
	proto->set_absaddrbit(absAddr.bitAddress());
}

void AcquiredDiagSignal::loadFromProto(const Network::AcquiredDiagSignal& proto)
{
	equipmentIdHash = proto.equipmentidhash();
	parentHash = proto.parenthash();

	diagLevel = static_cast<E::DiagLevel>(proto.diaglevel());
	diagSignalTypeIdHash = proto.diagsignaltypeidhash();
	isReflection = proto.isreflection();
	reflectedSignalIdHash = proto.reflectedsignalidhash();
	validitySignalIdHash = proto.validitysignalidhash();
	valueSizeBit = proto.valuesizebit();
	discreteContainerSize = proto.discretecontainersize();
	logChanges = proto.logchanges();
	archive = proto.archive();
	reserved = proto.reserved();
	apertureType = static_cast<E::ApertureType>(proto.aperturetype());
	coarseAperture = proto.coarseaperture();
	fineAperture = proto.fineaperture();
	absAddr.setBitAddress(proto.absaddrbit());
}

