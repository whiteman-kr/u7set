// Generate configuration for module AIM4PH
//
//
function generate_aim4ph_sr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    // InputController object getting
    //
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");

    if (inControllerObject == null || inControllerObject.isController() == false)
    {
        log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
        return false;
    }

    let inController = inControllerObject.toController();

    // UnitsConverter object getting
    //
    let unitsConverter = confFirmware.jsGetUnitsConvertor();
    if (unitsConverter == null)
    {
        log.errINT1001("confFirmware.jsGetUnitsConverter returned null");
        return false;
    }

    // default values of calculating parameters
    //
    const TS_CONSTANT = 200 * 0.000001;
    const DEFAULT_FILTERING_TIME = valToADC(0/*5000*/ * 0.000001 / TS_CONSTANT, 0, 65535, 0, 0xffff);
    const DEFAULT_HIGH_VALID_RANGE = 5.0;
    const DEFAULT_LOW_VALID_RANGE = 0.0;
    const DEFAULT_k1 = 1.0;
    const DEFAULT_K2 = 0.0;
    const DEFAULT_SPREAD_TOLERANCE = Math.round((0xffff - 0) * 0.005);		// 2% = 328h
    const DEFAULT_FLAGS = 0;

    // enum IssueCompareMode values
    //
    const CMP_MODE_EQUAL = 0;
    const CMP_MODE_LESS = 1;
    const CMP_MODE_MORE = 2;

    // properties names
    //
    const ELECTRIC_HIGH_LIMIT = "ElectricHighLimit";
    const ELECTRIC_LOW_LIMIT = "ElectricLowLimit";
    const ELECTRIC_UNIT = "ElectricUnit";
    const SENSOR_TYPE = "SensorType";
    const RLOAD_OHM = "Rload_Ohm";
    const HIGH_VALID_RANGE = "HighValidRange";
    const LOW_VALID_RANGE = "LowValidRange";
    const HIGH_ENG_UNITS = "HighEngineeringUnits";
    const LOW_ENG_UNITS = "LowEngineeringUnits";
    const FILTERING_TIME = "FilteringTime";
    const SPREAD_TOLERANCE = "SpreadTolerance";

    //

    const INPUTS_COUNT = 32;

    //

    let result = true;
    let ptr = 0;

    // Generation of module AIM-4PH-SR configuration (640 bytes)
    //
    for (let i = 0; i < INPUTS_COUNT; i++)
    {
        // find a signal with Place = i
        //
        let signalStrId = inController.equipmentId + "_IN";

        let entry = i + 1;
        if (entry < 10)
        {
            signalStrId = signalStrId + "0";
        }
        signalStrId = signalStrId + entry;

        let signalStrIdA = signalStrId + "A";
        let signalStrIdB = signalStrId + "B";

        let signalA = signalSet.getSignalByEquipmentID(signalStrIdA);
        let signalB = signalSet.getSignalByEquipmentID(signalStrIdB);

        // calculating parameters initialization
        //
        let filteringTime = DEFAULT_FILTERING_TIME;
        let highValidRange = DEFAULT_HIGH_VALID_RANGE;
        let lowValidRange = DEFAULT_LOW_VALID_RANGE;
        let k1 = DEFAULT_k1;
        let k2 = DEFAULT_K2;
        let spreadTolerance = DEFAULT_SPREAD_TOLERANCE;
        let flags = DEFAULT_FLAGS;

        // this parameters will calculated only for NON default values
        //
        let highPhysicalRange = 0;
        let lowPhysicalRange = 0;

        let defaultValues = true;

        //
        
        if (signalA == null && signalB == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrIdA);
        }
        else
        {
            defaultValues = false;

            // here signalA is not null OR signalB is not null
            //
            if (signalA == null)
            {
                log.wrnCFG3007(signalStrIdA);
                signalA = signalB;
            }

            if (signalB == null)
			{
				log.wrnCFG3007(signalStrIdB);
				signalB = signalA;
			}

            // here guaranteed that signalA AND signalB is not null

            // check required signals properties
            //
            const propsToCheck =
            [
                ELECTRIC_HIGH_LIMIT,
                ELECTRIC_LOW_LIMIT,
                ELECTRIC_UNIT,
                SENSOR_TYPE,
                RLOAD_OHM,
                HIGH_VALID_RANGE,
                LOW_VALID_RANGE,
                HIGH_ENG_UNITS,
                LOW_ENG_UNITS,
                FILTERING_TIME,
                SPREAD_TOLERANCE
            ];

            for(propName of propsToCheck)
            {
                let propValueA = signalA.propertyValue(propName);

                if (propValueA == undefined)
                {
                    log.errCFG3000(propName, signalStrIdA);
                    result = false;
                    continue;
                }

                let propValueB = signalB.propertyValue(propName);

                if (propValueB == undefined)
                {
                    log.errCFG3000(propName, signalStrIdA);
                    result = false;
                    continue;
                }

                // Properties of signals A and B must be the same

                if (propValueA != propValueB)
                {
                    log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, propName);
                }
            }

            if (result == false)
            {
                return false;
            }

            const propsToCompare = [
                [ ELECTRIC_HIGH_LIMIT, CMP_MODE_LESS, ELECTRIC_LOW_LIMIT ],
                [ ELECTRIC_HIGH_LIMIT, CMP_MODE_EQUAL, ELECTRIC_LOW_LIMIT ],
                [ HIGH_ENG_UNITS, CMP_MODE_EQUAL, LOW_ENG_UNITS ],
                [ HIGH_VALID_RANGE, CMP_MODE_EQUAL, LOW_VALID_RANGE ],
            ];

            for(cmpParams of propsToCompare)
            {
                let prop1Name = cmpParams[0];
                let cmpMode = cmpParams[1];
                let prop2Name = cmpParams[2];

                let prop1Value = signalA.propertyValue(prop1Name);
                let prop2Value = signalA.propertyValue(prop2Name);

                let cmpResult = false;

                switch(cmpMode)
                {
                case CMP_MODE_LESS: cmpResult = prop1Value < prop2Value; break;
                case CMP_MODE_EQUAL: cmpResult = prop1Value === prop2Value; break;
                case CMP_MODE_MORE: cmpResult = prop1Value > prop2Value; break;
                }

                if (cmpResult == true)
                {
                    log.errCFG3013(prop1Name, prop1Value, cmpMode, prop2Name, prop2Value, 6, signalStrIdA);
                    result = false;
                }
            }

            if (result == false)
            {
                return false;
            }

            if (signalA.highEngineeringUnits() > signalA.lowEngineeringUnits() && signalA.highValidRange() < signalA.lowValidRange())
            {
                log.errCFG3013(HIGH_VALID_RANGE, signalA.highValidRange(), CMP_MODE_LESS,
                               LOW_VALID_RANGE, signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
                result = false;
            }
            if (signalA.highEngineeringUnits() < signalA.lowEngineeringUnits() && signalA.highValidRange() > signalA.lowValidRange())
            {
                // error
                log.errCFG3013(HIGH_VALID_RANGE, signalA.highValidRange(), CMP_MODE_MORE,
                               LOW_VALID_RANGE, signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
                result = false;
            }

            if (result == false)
            {
                return false;
            }

            //

            let electricHighLimit = signalA.electricHighLimit();
            let electricLowLimit = signalA.electricLowLimit();
            let electricUnit = signalA.electricUnit();

            let sensorType = signalA.sensorType();
            let rload = signalA.rloadOhm();

            let highValidRange = signalA.highValidRange();
            let lowValidRange = signalA.lowValidRange();

            let highEngUnits = signalA.highEngineeringUnits();
            let lowEngUnits = signalA.lowEngineeringUnits();

			// Convert electric to physical
			
            let highPhysical = unitsConverter.electricToPhysical_Input(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, rload);
            let lowPhysical = unitsConverter.electricToPhysical_Input(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, rload);
			
			if (highPhysical.ok == false)
			{
				switch (highPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(highPhysical.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrIdA);
					}
						break;
					case UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrIdA);
					}
						break;
					case UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrIdA);
					}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysical.errorCode + "), signal " + signalStrIdA);
				}
			}
            else
            {
                highPhysicalRange = highPhysical.toDouble;
            }

			if (lowPhysical.ok == false)
			{
				switch (lowPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(lowPhysical.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrIdA);
					}
						break;
					case UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrIdA);
					}
						break;
					case UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrIdA);
					}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysical.errorCode + "), signal " + signalStrIdA);
				}
			}
            else
            {
                lowPhysicalRange = lowPhysical.toDouble;
            }
			
            if (highPhysicalRange == lowPhysicalRange)
			{
				// error
                log.errCFG3013("calculated HighPhysical", highPhysicalRange, CMP_MODE_EQUAL, "calculated LowPhysical", lowPhysicalRange, 0, signalStrIdA);
			}

			// end of convert electric to physical

			let tf = signalA.filteringTime();
			
            if (tf < 0 * TS_CONSTANT || tf > 65535 * TS_CONSTANT)
			{
                log.errCFG3010("FilteringTime", tf, 0 * TS_CONSTANT, 65535 * TS_CONSTANT, 6, signalStrIdA);
			}
			
            tf = tf / TS_CONSTANT;
		
            filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
	
            spreadTolerance = Math.round((signalA.spreadTolerance() * 0.01) * 65535);

            let y1 = lowEngUnits;
            let y2 = highEngUnits;
			
			let x1 = lowPhysical.toDouble;
			let x2 = highPhysical.toDouble;

			if (x1 == x2) // Prevent division by zero
			{
				x1 = 0;	
				x2 = 1;
			}

            k1 = (y2 - y1) / (x2 - x1);	// K
            k2 = y1 - k1 * x1;			// B

			let flags = 0;
			
			// Check valid ranges
			
			let decimalPlaces = signalA.propertyValue("DecimalPlaces");

			let lowValidRangeMin = 0;
			let highValidRangeMax = 5;

			let lowValidRangeMinEngineering = lowValidRangeMin * k1 + k2;
			let highValidRangeMaxEngineering = highValidRangeMax * k1 + k2;
			
			// Round this value to supplied decimal places
			
			lowValidRangeMinEngineering = parseFloat(lowValidRangeMinEngineering.toFixed(decimalPlaces));
			highValidRangeMaxEngineering = parseFloat(highValidRangeMaxEngineering.toFixed(decimalPlaces));
			
			//

			if (lowValidRange < lowValidRangeMinEngineering)
			{
				log.errCFG3010("LowValidRange", lowValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrIdA);
			}
			if (highValidRange > highValidRangeMaxEngineering)
			{
				log.errCFG3010("HighValidRange", highValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrIdA);
			}
        }

        //
        // Write log and configuration data
        //
        let logStr = "    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime +
                    "; [" + frame + ":" + (ptr + 2) + "] SpreadTolerance = " + spreadTolerance +
                    "; [" + frame + ":" + (ptr + 4) + "] K1 = " + k1 +
                    "; [" + frame + ":" + (ptr + 8) + "] K2 = " + k2;

        if (defaultValues == false)
        {
            logStr +=   "; HighPhysicalRange = " + highPhysicalRange +
                        "; LowPhysicalRange = " + lowPhysicalRange;
        }

        logStr += "; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + highValidRange +
                  "; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + lowValidRange +
                  "; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + flags + "\r\n";

        confFirmware.writeLog(logStr);

        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "FilteringTime", filteringTime) == false)          // InA Filtering time constant
        {
            return false;
        }
        ptr += 2;

        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "SpreadTolerance", spreadTolerance) == false)      // InA SpreadTolerance
        {
            return false;
        }
        ptr += 2;

        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", k1) == false)         // K1
        {
            return false;
        }
        ptr += 4;


        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K2", k2) == false)         // K2
        {
            return false;
        }
        ptr += 4;


        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", highValidRange) == false)         // InA High bound
        {
            return false;
        }
        ptr += 4;

        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", lowValidRange) == false)          // InA Low Bound
        {
            return false;
        }
        ptr += 4;

        ptr += 4;	// Reserved

        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", flags) == false)      // InA WordOfFlags
        {
            return false;
        }
        ptr += 2;
    }

    ptr = 888;
   
    // final crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
    ptr += 8;
    
	ptr = 1008;

    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    let dataTransmittingEnableFlag = false;
    let dataReceiveEnableFlag = true;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 7;
    let dataFramesQuantity = 0;
 
    let txId = module.moduleFamily + module.moduleVersion;
    
    if (generate_txRxIoConfig(confFirmware, module.equipmentId, LMNumber, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId) == false)
	{
		return false;
	}
    ptr += 8;
    
    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }
    
    return result;
}
