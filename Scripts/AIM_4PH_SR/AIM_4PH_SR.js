// Generate configuration for module AIM-4PH-SR
//
//
function generate_aim4ph_sr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    // required properties names
    //
    const ELECTRIC_HIGH_LIMIT = "ElectricHighLimit";
    const ELECTRIC_LOW_LIMIT = "ElectricLowLimit";
    const ELECTRIC_UNIT = "ElectricUnit";
    const HIGH_PHYSICAL_UNITS = "HighPhysicalUnits";
    const LOW_PHYSICAL_UNITS = "LowPhysicalUnits";
    const SENSOR_TYPE = "SensorType";
    const HIGH_VALID_RANGE = "HighValidRange";
    const LOW_VALID_RANGE = "LowValidRange";
    const HIGH_ENG_UNITS = "HighEngineeringUnits";
    const LOW_ENG_UNITS = "LowEngineeringUnits";
    const FILTERING_TIME = "FilteringTime";
    const SPREAD_TOLERANCE = "SpreadTolerance";

    // electric range of measurement channel is 0..5.1V
    //
    const ELECTRIC_MEASUREMENT_HIGH_LIMIT = 5.1;    // may be 5.25
    const ELECTRIC_MEASUREMENT_LOW_LIMIT = 0;

    // default values of calculating parameters
    //
    const TS_CONSTANT = 200 * 0.000001;
    const DEFAULT_FILTERING_TIME = valToADC(0/*5000*/ * 0.000001 / TS_CONSTANT, 0, 65535, 0, 0xffff);
    const DEFAULT_k1 = 1.0;
    const DEFAULT_K2 = 0.0;
    const DEFAULT_SPREAD_TOLERANCE = Math.round((0xffff - 0) * 0.005);		// 2% = 328h
    const DEFAULT_FLAGS = 0;

    // Valid range should be is specified in Engineering units
    // Here default values set not right (in Volts) but according to module documentation
    //
    const DEFAULT_HIGH_VALID_RANGE = 5.0;
    const DEFAULT_LOW_VALID_RANGE = 0.0;

    // Specified default Physical units 1..5V corresponds to
    // input electric value 4..20 mA getting on resistor Rload = 250Ohm
    //
    const DEFAULT_HIGH_PHYSICAL_UNITS = 5;
    const DEFAULT_LOW_PHYSICAL_UNITS = 1;

    //

    const INPUTS_COUNT = 32;    // 32 * (A + B channels)

    //

    let result = true;
    let ptr = 0;

    // InputController object getting
    //
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");

    if (inControllerObject === null || inControllerObject.isController() === false)
    {
        log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
        return false;
    }

    let inController = inControllerObject.toController();

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
        let highPhysicalRange = 5;
        let lowPhysicalRange = 0;

        let res = true;

        //
        
        if (signalA === null && signalB === null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrIdA);
            log.wrnCFG3007(signalStrIdB);
        }
        else
        {
            // here signalA is not null OR signalB is not null
            //
            if (signalA === null)
            {
                log.wrnCFG3007(signalStrIdA);
                signalA = signalB;
            }

            if (signalB === null)
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
                HIGH_PHYSICAL_UNITS,
                LOW_PHYSICAL_UNITS,
                SENSOR_TYPE,
                HIGH_VALID_RANGE,
                LOW_VALID_RANGE,
                HIGH_ENG_UNITS,
                LOW_ENG_UNITS,
                FILTERING_TIME,
                SPREAD_TOLERANCE
            ];

            res = true;
            
            for(const propName of propsToCheck)
            {
                let propValueA = signalA.propertyValue(propName);

                if (propValueA === undefined)
                {
                    log.errCFG3000(propName, signalStrIdA);
                    res = false;
                    continue;
                }

                let propValueB = signalB.propertyValue(propName);

                if (propValueB === undefined)
                {
                    log.errCFG3000(propName, signalStrIdA);
                    res = false;
                    continue;
                }

                // Properties of signals A and B must be same

                if (propValueA !== propValueB)
                {
                    log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, propName);
                    res = false;
                }
            }

            if (res == false)
            {
                result = false;
                continue;
            }

            // enum IssueCompareMode values
            //
            const CMP_MODE_EQUAL = 0;
            const CMP_MODE_LESS = 1;
            const CMP_MODE_MORE = 2;

            const propsToCompare = [

                // HighElectricLimit must NOT be less or equal to LowElectricLimit
                //
                [ ELECTRIC_HIGH_LIMIT, CMP_MODE_LESS, ELECTRIC_LOW_LIMIT ],
                [ ELECTRIC_HIGH_LIMIT, CMP_MODE_EQUAL, ELECTRIC_LOW_LIMIT ],

                // HighPhysicalUnits must NOT be less or equal to LowPhysicalUnits
                //
                [ HIGH_PHYSICAL_UNITS, CMP_MODE_LESS, LOW_PHYSICAL_UNITS ],
                [ HIGH_PHYSICAL_UNITS, CMP_MODE_EQUAL, LOW_PHYSICAL_UNITS ],

                // EngUnits and ValidRage limits must NOT be equal
                //
                [ HIGH_ENG_UNITS, CMP_MODE_EQUAL, LOW_ENG_UNITS ],
                [ HIGH_VALID_RANGE, CMP_MODE_EQUAL, LOW_VALID_RANGE ],
            ];

            res = true;

            for(const cmpParams of propsToCompare)
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
                    log.errCFG3013(prop1Name, prop1Value, cmpMode, prop2Name, prop2Value, 4, signalStrIdA);
                    res = false;
                }
            }

            if (res == false)
            {
                result = false;
                continue;
            }

            res = true;

            let straightRange = signalA.highEngineeringUnits() > signalA.lowEngineeringUnits();

            // Relation of HighValidRange to LowValidRange must be same
            // with relation of HighEngUnits to LowEngUnits in straight and reverse ranges.
            //
            if (straightRange === true &&
                signalA.highValidRange() < signalA.lowValidRange())
            {
                log.errCFG3013(HIGH_VALID_RANGE, signalA.highValidRange(), CMP_MODE_LESS,
                               LOW_VALID_RANGE, signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
                res = false;
            }

            if (straightRange === false &&
                signalA.highValidRange() > signalA.lowValidRange())
            {
                // error
                log.errCFG3013(HIGH_VALID_RANGE, signalA.highValidRange(), CMP_MODE_MORE,
                               LOW_VALID_RANGE, signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
                res = false;
            }

            if (res == false)
            {
                result = false;
                continue;
            }

            //

			let tf = signalA.filteringTime();
			
            if (tf < 0 * TS_CONSTANT || tf > 65535 * TS_CONSTANT)
			{
                log.errCFG3010(FILTERING_TIME, tf, 0 * TS_CONSTANT, 65535 * TS_CONSTANT, 6, signalStrIdA);
                result = false;
			}
			
            tf = tf / TS_CONSTANT;
		
            filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
	
            spreadTolerance = Math.round((signalA.spreadTolerance() * 0.01) * 65535);

            lowPhysicalRange = signalA.lowPhysicalUnits();
            highPhysicalRange = signalA.highPhysicalUnits();

            let y1 = signalA.lowEngineeringUnits();
            let y2 = signalA.highEngineeringUnits();
			
            let x1 = lowPhysicalRange;
            let x2 = highPhysicalRange;

            if (x1 === x2)   // this check already done above, WTF?
            {
                log.errINT1001("division by 0 (x1 == x2)")
                return false;
			}

            k1 = (y2 - y1) / (x2 - x1);	// K
            k2 = y1 - k1 * x1;			// B

			let flags = 0;
			
			// Check valid ranges
            //
            highValidRange = signalA.highValidRange();
            lowValidRange = signalA.lowValidRange();

            // max valid range distanse from engineering units range is -5%...+5%
            //
            let dist = (highPhysicalRange - lowPhysicalRange) * 0.05;

            let lowestValidRange = (lowPhysicalRange - dist) * k1 + k2;
            let highestValidRange = (highPhysicalRange + dist) * k1 + k2;

            res = true;

            if ((straightRange == true && lowValidRange < lowestValidRange) ||
                (straightRange == false && lowValidRange > lowestValidRange))
            {
                // Property %1 has wrong value (%2), valid range is %3..%4 [precision %5](signal %6).
                //
                log.errCFG3010(LOW_VALID_RANGE, lowValidRange, lowestValidRange, highestValidRange,
                               4, signalStrIdA);
                res = false;
            }

            if ((straightRange == true && highValidRange > highestValidRange) ||
                (straightRange == false && highValidRange < highestValidRange))
            {
                // Property %1 has wrong value (%2), valid range is %3..%4 [precision %5](signal %6).
                //
                log.errCFG3010(HIGH_VALID_RANGE, highValidRange, lowestValidRange, highestValidRange,
                               4, signalStrIdA);
                res = false;
            }

            if (res === false)
            {
                result = false;
                continue;
            }
        }

        // values of *ValigRange for writing in module configuration
        // where cfgHighValidRange always must be greate than cfgLowValidRange
        //
        let cfgHighValidRange = highValidRange;
        let cfgLowValidRange = lowValidRange;

        if (cfgHighValidRange < cfgLowValidRange)
        {
            cfgHighValidRange = lowValidRange;
            cfgLowValidRange = highValidRange;
        }

        //
        // Write log and configuration data
        //
        let logStr = "    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime +
                    "; [" + frame + ":" + (ptr + 2) + "] SpreadTolerance = " + spreadTolerance +
                    "; [" + frame + ":" + (ptr + 4) + "] K1 = " + k1 +
                    "; [" + frame + ":" + (ptr + 8) + "] K2 = " + k2;

        logStr +=   "; HighPhysicalRange = " + highPhysicalRange +
                    "; LowPhysicalRange = " + lowPhysicalRange;

        logStr += "; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + cfgHighValidRange +
                  "; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + cfgLowValidRange +
                  "; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + flags + "\r\n";

        confFirmware.writeLog(logStr);

        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, FILTERING_TIME, filteringTime) == false)          // InA Filtering time constant
        {
            return false;
        }
        ptr += 2;

        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, SPREAD_TOLERANCE, spreadTolerance) == false)      // InA SpreadTolerance
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


        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, HIGH_VALID_RANGE, cfgHighValidRange) == false)         // InA High bound
        {
            return false;
        }
        ptr += 4;

        if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, LOW_VALID_RANGE, cfgLowValidRange) == false)          // InA Low Bound
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

    if (result == false)
    {
        return false;
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
