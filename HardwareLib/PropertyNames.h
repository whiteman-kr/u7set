namespace Hardware
{
	// Property names
	//
	class PropertyNames
	{
	public:
		PropertyNames() = delete;

	public:
		static const QString fileId;
		static const QString uuid;
		static const QString equipmentIdTemplate;
		static const QString equipmentId;
		static const QString caption;
		static const QString childRestriction;
		static const QString place;
		static const QString specificProperties;
		static const QString signalSpecificProperties;
		static const QString tags;
		static const QString tagsDescription;

		static const QString preset;
		static const QString presetRoot;
		static const QString presetName;
		static const QString presetVersion;
		static const QString presetObjectUuid;
		static const QString presetProtectedProperties;
		static const QString presetProtectedPropertiesDescription;

		static const QString lmDescriptionFile;
		static const QString lmNumber;
		static const QString lmSubsystemChannel;
		static const QString lmSubsystemID;

		static const QString type;
		static const QString function;
		static const QString byteOrder;
		static const QString format;
		static const QString memoryArea;
		static const QString size;
		static const QString units;
		static const QString analogFormat;

		static const QString diagDataOffset;
		static const QString inverseValue;
		static const QString normalState;
		static const QString normalStateString0;
		static const QString normalStateString1;

		static const QString adcHighLimit;
		static const QString adcLowLimit;
		static const QString valueHighLimit;
		static const QString valueLowLimit;
		static const QString valueMultiplier;
		//static const QString valueOffset;
		static const QString useLimits;

		static const QString isReflection;
		static const QString reflectedSignalId;
		static const QString level;
		static const QString valueOffset;
		static const QString valueBit;
		static const QString valueBitSize;
		static const QString valueBitSizeDescription;
		static const QString discreteContainerSize;
		static const QString discreteContainerSizeDescription;
		static const QString validitySignalId;

		static const QString logChanges;
		static const QString archive;
		static const QString reserved;
		static const QString coarseAperture;
		static const QString fineAperture;
		static const QString apertureType;
		static const QString decimalPlaces;

		static const QString appSignalLowAdc;
		static const QString appSignalHighAdc;
		static const QString appSignalLowEngUnits;
		static const QString appSignalHighEngUnits;
		static const QString appSignalDataFormat;
		static const QString appSignalBusTypeId;

		static const QString hostname;

		static const QString diagSignalTypeId;

		static const QString systemSignalType;
		static const QString systemSignalTypeDescription;

		static const QString categoryCommon;
		static const QString categoryAppSignal;
		static const QString categoryDiagSignal;
		static const QString categoryDiscrete;
		static const QString categoryAnalog;
		static const QString categoryData;
		static const QString categoryMats;
		static const QString categoryDiagnostics;
	};
}