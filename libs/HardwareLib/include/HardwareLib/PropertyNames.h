namespace Hardware
{
	// Property names
	//
	class PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		inline static const QString excludeFromBuild = QStringLiteral("ExcludeFromBuild");
		inline static const QString fileId = QStringLiteral("FileID");
		inline static const QString uuid = QStringLiteral("Uuid");
		inline static const QString equipmentIdTemplate = QStringLiteral("EquipmentIDTemplate");
		inline static const QString equipmentId = QStringLiteral("EquipmentID");
		inline static const QString caption = QStringLiteral("Caption");
		inline static const QString childRestriction = QStringLiteral("ChildRestriction");
		inline static const QString place = QStringLiteral("Place");
		inline static const QString specificProperties = QStringLiteral("SpecificProperties");
		inline static const QString signalSpecificProperties = QStringLiteral("SignalSpecificProperties");
		inline static const QString tags = QStringLiteral("Tags");
		inline static const QString tagsDescription = QStringLiteral("Space separated object's tags");

		inline static const QString preset = QStringLiteral("Preset");
		inline static const QString presetRoot = QStringLiteral("PresetRoot");
		inline static const QString presetName = QStringLiteral("PresetName");
		inline static const QString presetVersion = QStringLiteral("PresetVersion");
		inline static const QString presetObjectUuid = QStringLiteral("PresetObjectUuid");
		inline static const QString presetProtectedProperties = QStringLiteral("PresetProtectedProperties");
		inline static const QString presetProtectedPropertiesDescription =
			QStringLiteral("Protected from \"Update from Preset\" comma separated property list");

		inline static const QString lmDescriptionFile = QStringLiteral("LmDescriptionFile");
		inline static const QString lmNumber = QStringLiteral("LMNumber");
		inline static const QString lmSubsystemChannel = QStringLiteral("SubsystemChannel");
		inline static const QString lmSubsystemID = QStringLiteral("SubsystemID");

		inline static const QString type = QStringLiteral("Type");
		inline static const QString function = QStringLiteral("Function");
		inline static const QString byteOrder = QStringLiteral("ByteOrder");
		inline static const QString format = QStringLiteral("Format");
		inline static const QString memoryArea = QStringLiteral("MemoryArea");
		inline static const QString size = QStringLiteral("Size");
		inline static const QString units = QStringLiteral("Units");
		inline static const QString analogFormat = QStringLiteral("AnalogFormat");

		inline static const QString diagDataOffset = QStringLiteral("DiagDataOffset");
		inline static const QString inverseValue = QStringLiteral("InverseValue");
		inline static const QString normalState = QStringLiteral("NormalState");
		inline static const QString normalStateString0 = QStringLiteral("NormalStateString0");
		inline static const QString normalStateString1 = QStringLiteral("NormalStateString1");

		inline static const QString adcHighLimit = QStringLiteral("AdcHighLimit");
		inline static const QString adcLowLimit = QStringLiteral("AdcLowLimit");
		inline static const QString valueHighLimit = QStringLiteral("ValueHighLimit");
		inline static const QString valueLowLimit = QStringLiteral("ValueLowLimit");
		inline static const QString valueMultiplier = QStringLiteral("ValueMultiplier");
		// inline static const QString valueOffset = QStringLiteral("ValueOffset");
		inline static const QString useLimits = QStringLiteral("UseLimits");

		inline static const QString isReflection = QStringLiteral("IsReflection");
		inline static const QString reflectedSignalId = QStringLiteral("ReflectedSignalID");
		inline static const QString level = QStringLiteral("Level");
		inline static const QString valueOffset = QStringLiteral("ValueOffset");
		inline static const QString valueBit = QStringLiteral("ValueBit");
		inline static const QString valueBitSize = QStringLiteral("ValueBitSize");
		inline static const QString valueBitSizeDescription = QStringLiteral("Size of data in bits, usually 1, 16, 32...");
		inline static const QString discreteContainerSize = QStringLiteral("DiscreteContainerSize");
		inline static const QString discreteContainerSizeDescription = QStringLiteral("Container size of discrete signals, bytes");
		inline static const QString validitySignalId = QStringLiteral("ValiditySignalID");

		inline static const QString logChanges = QStringLiteral("LogChanges");
		inline static const QString archive = QStringLiteral("Archive");
		inline static const QString reserved = QStringLiteral("Reserved");
		inline static const QString coarseAperture = QStringLiteral("CoarseAperture");
		inline static const QString fineAperture = QStringLiteral("FineAperture");
		inline static const QString apertureType = QStringLiteral("ApertureType");
		inline static const QString decimalPlaces = QStringLiteral("DecimalPlaces");

		// static const QString appSignalLowAdc;
		// static const QString appSignalHighAdc;
		// static const QString appSignalLowEngUnits;
		// static const QString appSignalHighEngUnits;

		inline static const QString appSignalDataFormat = QStringLiteral("AppAnalogSignalFormat");
		inline static const QString appSignalBusTypeId = QStringLiteral("BusTypeID");

		inline static const QString hostname = QStringLiteral("Hostname");

		inline static const QString diagSignalTypeId = QStringLiteral("DiagSignalTypeID");
		inline static const QString systemSignalType = QStringLiteral("SystemSignalType");
		inline static const QString systemSignalTypeDescription =
			QStringLiteral("System signal types are predefined and cannot be changed or deleted.");

		inline static const QString categoryCommon = QStringLiteral("Common");
		inline static const QString categoryAppSignal = QStringLiteral("AppSignal");
		inline static const QString categoryDiagSignal = QStringLiteral("DiagSignal");
		inline static const QString categoryDiscrete = QStringLiteral("Type Discrete");
		inline static const QString categoryAnalog = QStringLiteral("Type Analog");
		inline static const QString categoryData = QStringLiteral("Data");
		inline static const QString categoryMats = QStringLiteral("MATS");
		inline static const QString categoryDiagnostics = QStringLiteral("Diagnostics");

		inline static const QString globalScript = QStringLiteral("GlobalScript");
	};
} // namespace Hardware