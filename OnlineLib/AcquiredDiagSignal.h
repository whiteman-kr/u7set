#pragma once

#include "../HardwareLib/DeviceObject.h"
#include "../HardwareLib/DiagSignal.h"
#include "../UtilsLib/Address16.h"

using DiagSignalConstShared = std::shared_ptr<const Hardware::DiagSignal>;
using DiagSignalShared = std::shared_ptr<Hardware::DiagSignal>;

using DeviceObjectConstShared = std::shared_ptr<const Hardware::DeviceObject>;
using DeviceObjectShared = std::shared_ptr<Hardware::DeviceObject>;

namespace Network
{
	class AcquiredDiagObject;
	class AcquiredDiagSignal;
}

class AcquiredDiagObject
{
public:
	AcquiredDiagObject();
	AcquiredDiagObject(DeviceObjectConstShared devObj);

	void saveToProto(Network::AcquiredDiagObject* proto);
	bool loadFromProto(const Network::AcquiredDiagObject& proto);

public:
	QString equipmentID;
	Hardware::DeviceType deviceType = Hardware::DeviceType::DeviceTypeCount;	// means - not initialized
	Hash parentHash = 0;		// calcHash(parent.equipmentID)

	// Counters, not saved in proto

	int attentions = 0;
	int warnings = 0;
	int errors = 0;
	int fauls = 0;
};

class AcquiredDiagSignal
{
public:
	AcquiredDiagSignal();
	AcquiredDiagSignal(DiagSignalConstShared diagSignal);

	void saveToProto(Network::AcquiredDiagSignal* proto);
	void loadFromProto(const Network::AcquiredDiagSignal& proto);

public:
	QString equipmentId;			// only for debugging

	Hash equipmentIdHash;
	Hash parentHash = 0;			// calcHash(parent.equipmentID)

	E::DiagLevel diagLevel = E::DiagLevel::Message;
	Hash diagSignalTypeIdHash;
	bool isReflection = false;
	Hash reflectedSignalIdHash;
	Hash validitySignalIdHash;
	int valueSizeBit = 0;
	int discreteContainerSize = 0;
	bool logChanges = false;
	bool archive = false;
	bool reserved = false;
	E::ApertureType apertureType = E::ApertureType::AbsValue;
	double coarseAperture = 0;
	double fineAperture = 0;
	Address16 absAddr;			// absolute signal data address from beginning of FODIP (LM diagnosticts packet)
								//
								// calculate as: module.DiagDataOffset (in FODIP) + controller.DiagDataOffset + diagSignal.DataOffset
};

