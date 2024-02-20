#include "PropertyNames.h"

namespace Hardware
{
	//
	//
	// PropertyNames
	//
	//
	const QString PropertyNames::fileId = "FileID";
	const QString PropertyNames::uuid = "Uuid";
	const QString PropertyNames::equipmentIdTemplate = "EquipmentIDTemplate";
	const QString PropertyNames::equipmentId = "EquipmentID";
	const QString PropertyNames::caption = "Caption";
	const QString PropertyNames::childRestriction = "ChildRestriction";
	const QString PropertyNames::place = "Place";
	const QString PropertyNames::specificProperties = "SpecificProperties";
	const QString PropertyNames::signalSpecificProperties = "SignalSpecificProperties";
	const QString PropertyNames::tags = "Tags";
	const QString PropertyNames::tagsDescription = "Space separated object's tags";

	const QString PropertyNames::preset = "Preset";
	const QString PropertyNames::presetRoot = "PresetRoot";
	const QString PropertyNames::presetName = "PresetName";
	const QString PropertyNames::presetVersion = "PresetVersion";
	const QString PropertyNames::presetObjectUuid = "PresetObjectUuid";
	const QString PropertyNames::presetProtectedProperties = "PresetProtectedProperties";
	const QString PropertyNames::presetProtectedPropertiesDescription = "Protected from \"Update from Preset\" comma separated property list";

	const QString PropertyNames::lmDescriptionFile = "LmDescriptionFile";
	const QString PropertyNames::lmNumber = "LMNumber";
	const QString PropertyNames::lmSubsystemChannel = "SubsystemChannel";
	const QString PropertyNames::lmSubsystemID = "SubsystemID";

	const QString PropertyNames::type = "Type";
	const QString PropertyNames::function = "Function";
	const QString PropertyNames::byteOrder = "ByteOrder";
	const QString PropertyNames::format = "Format";
	const QString PropertyNames::memoryArea = "MemoryArea";
	const QString PropertyNames::size = "Size";
	const QString PropertyNames::units = "Units";
	const QString PropertyNames::analogFormat = "AnalogFormat";

	const QString PropertyNames::diagDataOffset = "DiagDataOffset";
	const QString PropertyNames::inverseValue = "InverseValue";
	const QString PropertyNames::normalState = "NormalState";
	const QString PropertyNames::normalStateString0 = "NormalStateString0";
	const QString PropertyNames::normalStateString1 = "NormalStateString1";

	const QString PropertyNames::adcHighLimit = "AdcHighLimit";
	const QString PropertyNames::adcLowLimit = "AdcLowLimit";
	const QString PropertyNames::valueHighLimit = "ValueHighLimit";
	const QString PropertyNames::valueLowLimit = "ValueLowLimit";
	const QString PropertyNames::valueMultiplier = "ValueMultiplier";
	//const QString PropertyNames::valueOffset = "ValueOffset";
	const QString PropertyNames::useLimits = "UseLimits";

	const QString PropertyNames::isReflection = "IsReflection";
	const QString PropertyNames::reflectedSignalId = "ReflectedSignalID";
	const QString PropertyNames::level = "Level";
	const QString PropertyNames::valueOffset = "ValueOffset";
	const QString PropertyNames::valueBit = "ValueBit";
	const QString PropertyNames::valueBitSize = "ValueBitSize";
	const QString PropertyNames::valueBitSizeDescription = "Size of data in bits, usually 1, 16, 32...";
	const QString PropertyNames::discreteContainerSize = "DiscreteContainerSize";
	const QString PropertyNames::discreteContainerSizeDescription = "Container size of discrete signals, bytes";
	const QString PropertyNames::validitySignalId = "ValiditySignalID";
	const QString PropertyNames::appSignalDataFormat = "AppAnalogSignalFormat";
	const QString PropertyNames::appSignalBusTypeId = "BusTypeID";

	const QString PropertyNames::logChanges = "LogChanges";
	const QString PropertyNames::archive = "Archive";
	const QString PropertyNames::reserved = "Reserved";
	const QString PropertyNames::coarseAperture = "CoarseAperture";
	const QString PropertyNames::fineAperture = "FineAperture";
	const QString PropertyNames::apertureType = "ApertureType";
	const QString PropertyNames::decimalPlaces = "DecimalPlaces";

	const QString PropertyNames::hostname = "Hostname";

	const QString PropertyNames::diagSignalTypeId = "DiagSignalTypeID";

	const QString PropertyNames::systemSignalType = "SystemSignalType";
	const QString PropertyNames::systemSignalTypeDescription = "System signal types are predefined and cannot be changed or deleted.";

	const QString PropertyNames::categoryCommon = "Common";
	const QString PropertyNames::categoryAppSignal = "AppSignal";
	const QString PropertyNames::categoryDiagSignal = "DiagSignal";
	const QString PropertyNames::categoryDiscrete = "Type Discrete";
	const QString PropertyNames::categoryAnalog = "Type Analog";
	const QString PropertyNames::categoryData = "Data";
	const QString PropertyNames::categoryMats = "MATS";
	const QString PropertyNames::categoryDiagnostics = "Diagnostics";
}