// Generate configuration for module WAIM
//
//
function generate_waim(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_WAIM");
		return false;
	}
	
	let equipmentID = module.propertyValue("EquipmentID");
	
	let checkProperties = ["Place", "ModuleVersion"];
	for (let cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], equipmentID);
			return false;
		}
	}

    let ptr = 0;
    
    let WAIMSignalMaxCount = 32;
	
	let tsConstant = 200 * 0.000001;
    
    let defaultTf = valToADC(0/*5000*/ * 0.000001 / tsConstant, 0, 65535, 0, 0xffff);
    let defaultHighBound = 10.0;
    let defaultLowBound = -10.0;
	let defaultK1 = 1.0;
	let defaultK2 = 0.0;
    let defaultSpreadTolerance = Math.round((0xffff - 0) * 0.005);		// 2% = 328h
    
    let inController = module.jsFindChildObjectByMask(equipmentID + "_CTRLIN");
    if (inController == null)
    {
		log.errCFG3004(equipmentID + "_CTRLIN", equipmentID);
		return false;
    }
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (let i = 0; i < WAIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
		let signalStrId = inController.propertyValue("EquipmentID") + "_IN";
		
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
        
        if (signalA == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrIdA);
			
			confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + ptr + "] Tf = " + defaultTf + 
			"; [" + frame + ":" + (ptr + 2) + "] SpreadTolerance = " + defaultSpreadTolerance +
			"; [" + frame + ":" + (ptr + 4) + "] K1 = " + defaultK1 +
			"; [" + frame + ":" + (ptr + 8) + "] L2 = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + defaultLowBound +
			"; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + 0 + "\r\n");
            
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Tf", defaultTf) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 2;

            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SpreadTolerance", defaultSpreadTolerance) == false)      // InA SpreadTolerance
			{
				return false;
			}
            ptr += 2;

            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1", defaultK1) == false)      // InA DefaultK1
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K2", defaultK2) == false)      // InA DefaultK2
			{
				return false;
			}
            ptr += 4;


            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "HighValidRange", defaultHighBound) == false)         // InA High bound
			{
				return false;
			}
            ptr += 4;
			
            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "LowValidRange", defaultLowBound) == false)          // InA Low Bound
			{
				return false;
			}
            ptr += 4;
			
			ptr += 4;	// Reserved
			
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "WordOfFlags", 0) == false)      // InA DefaultWordOfFlags
			{
				return false;
			}
            ptr += 2;
			
        }
        else
        {
			if (signalB == null)
			{
				log.wrnCFG3007(signalStrIdB);
				signalB = signalA;
			}
			
			let compareEqual = 0;
			let compareLess = 1;
			let compareMore = 2;
			
			let unitsConvertor = confFirmware.jsGetUnitsConvertor();
			if (unitsConvertor == null)
			{
				log.errINT1001("confFirmware.jsGetUnitsConvertor returned null");
				return false;
			}

			let electricHighLimit = signalA.propertyValue("ElectricHighLimit");
			if (electricHighLimit == undefined) 
			{
				log.errCFG3000("ElectricHighLimit", signalStrIdA);
				return false;
			}	

			let electricLowLimit = signalA.propertyValue("ElectricLowLimit");
			if (electricLowLimit == undefined) 
			{
				log.errCFG3000("ElectricLowLimit", signalStrIdA);
				return false;
			}	
			
			let electricUnit = signalA.propertyValue("ElectricUnit");
			if (electricUnit == undefined) 
			{
				log.errCFG3000("ElectricUnit", signalStrIdA);
				return false;
			}	
			
			let sensorType = signalA.propertyValue("SensorType");
			if (sensorType == undefined) 
			{
				log.errCFG3000("SensorType", signalStrIdA);
				return false;
			}	
			
			let highLimit = signalA.highEngineeringUnits();
			let lowLimit = signalA.lowEngineeringUnits();
			
			let highValidRange = signalA.highValidRange();
			let lowValidRange = signalA.lowValidRange();

			let electricHighLimitB = signalB.propertyValue("ElectricHighLimit");
			if (electricHighLimitB == undefined) 
			{
				log.errCFG3000("ElectricHighLimit", signalStrIdB);
				return false;
			}

			let electricLowLimitB = signalB.propertyValue("ElectricLowLimit");
			if (electricLowLimitB == undefined) 
			{
				log.errCFG3000("ElectricLowLimit", signalStrIdB);
				return false;
			}	
			
			let electricUnitB = signalB.propertyValue("ElectricUnit");
			if (electricUnitB == undefined) 
			{
				log.errCFG3000("ElectricUnit", signalStrIdB);
				return false;
			}	

			let sensorTypeB = signalB.propertyValue("SensorType");
			if (sensorTypeB == undefined) 
			{
				log.errCFG3000("SensorType", signalStrIdB);
				return false;
			}	
			
			let highLimitB = signalB.highEngineeringUnits();
			let lowLimitB = signalB.lowEngineeringUnits();
			
			let highValidRangeB = signalB.highValidRange();
			let lowValidRangeB = signalB.lowValidRange();

			// Check properties of signal A
            
			if (electricHighLimit < electricLowLimit)
			{
				// error
				log.errCFG3013("ElectricHighLimit", electricHighLimit, compareLess, "ElectricLowLimit", electricLowLimit, 0, signalStrIdA);
			}
			if (electricHighLimit == electricLowLimit)
			{
				// error
				log.errCFG3013("ElectricHighLimit", electricHighLimit, compareEqual, "ElectricLowLimit", electricLowLimit, 0, signalStrIdA);
			}
			if (signalA.highEngineeringUnits() == signalA.lowEngineeringUnits())
			{
				// error
				log.errCFG3013("HighEngineeringUnits", signalA.highEngineeringUnits(), compareEqual, "LowEngineeringUnits", signalA.lowEngineeringUnits(), signalA.decimalPlaces(), signalStrIdA);
			}
			if (signalA.highValidRange() == signalA.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signalA.highValidRange(), compareEqual, "LowValidRange", signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
			}
			if (signalA.highEngineeringUnits() > signalA.lowEngineeringUnits() && signalA.highValidRange() < signalA.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signalA.highValidRange(), compareLess, "LowValidRange", signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
			}
			if (signalA.highEngineeringUnits() < signalA.lowEngineeringUnits() && signalA.highValidRange() > signalA.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signalA.highValidRange(), compareMore, "LowValidRange", signalA.lowValidRange(), signalA.decimalPlaces(), signalStrIdA);
			}
		
			// Check properties of signals A and B, they must be the same
		
			if (signalA.highValidRange() != signalB.highValidRange())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "HighValidRange");
			}

			if (signalA.lowValidRange() != signalB.lowValidRange())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "LowValidRange");
			}

			if (electricHighLimit != electricHighLimitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "ElectricHighLimit");
			}

			if (electricLowLimit != electricLowLimitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "ElectricLowLimit");
			}

			if (electricUnit != electricUnitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "ElectricUnit");
			}

			if (sensorType != sensorTypeB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "SensorType");
			}

			if (signalA.highEngineeringUnits() != signalB.highEngineeringUnits())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "HighEngineeringUnits");
			}

			if (signalA.lowEngineeringUnits() != signalB.lowEngineeringUnits())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "LowEngineeringUnits");
			}

			if (signalA.filteringTime() != signalB.filteringTime())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "FilteringTime");
			}

			if (signalA.spreadTolerance() != signalB.spreadTolerance())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.propertyValue("EquipmentID"), "SpreadTolerance");
			}
			
			// Convert electric to physical
			
			let highPhysical = unitsConvertor.electricToPhysical_Input(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0/*rload*/);
			let lowPhysical = unitsConvertor.electricToPhysical_Input(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0/*rload*/);
			
			if (highPhysical.ok == false)
			{
				switch (highPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(highPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrIdA);
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
			if (lowPhysical.ok == false)
			{
				switch (lowPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(lowPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrIdA);
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
			
			if (highPhysical.toDouble == lowPhysical.toDouble)
			{
				// error
				log.errCFG3013("calculated HighPhysical", highPhysical.toDouble, compareEqual, "calculated LowPhysical", lowPhysical.toDouble, 0, signalStrIdA);
			}
			// end of convert electric to physical

			let tf = signalA.filteringTime();
			
			if (tf < 0 * tsConstant || tf > 65535 * tsConstant)
			{
				log.errCFG3010("FilteringTime", tf, 0 * tsConstant, 65535 * tsConstant, 6, signalStrIdA);
			}
			
			tf = tf / tsConstant;
		
            let filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
	
			let spreadTolerance = Math.round((signalA.spreadTolerance() * 0.01) * 65535);

			let y1 = lowLimit;
			let y2 = highLimit;
			
			let x1 = lowPhysical.toDouble;
			let x2 = highPhysical.toDouble;

			if (x1 == x2) // Prevent division by zero
			{
				x1 = 0;	
				x2 = 1;
			}

			let k1 = (y2 - y1) / (x2 - x1);	// K
			let k2 = y1 - k1 * x1;			// B

			let flags = 0;

			// Check valid ranges
			
			let decimalPlaces = signalA.propertyValue("DecimalPlaces");
			
			let lowValidRangeMin = -10.9;
			let highValidRangeMax = 10.9

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

			//

			confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime + 
			"; [" + frame + ":" + (ptr + 2) + "] SpreadTolerance = " + spreadTolerance +
			"; [" + frame + ":" + (ptr + 4) + "] K1 = " + k1 +
			"; [" + frame + ":" + (ptr + 8) + "] K2 = " + k2 +
			"; HighPhysicalRange = " + highPhysical.toDouble +
			"; LowPhysicalRange = " + lowPhysical.toDouble +
			"; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + highValidRange +
			"; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + lowValidRange +
			"; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + flags + "\r\n");

            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "FilteringTime", filteringTime) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 2;

            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SpreadTolerance", spreadTolerance) == false)      // InA SpreadTolerance
			{
				return false;
			}
            ptr += 2;

			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1", k1) == false)         // K1
			{
				return false;
			}
            ptr += 4;
			
            
			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1", k2) == false)         // K2
			{
				return false;
			}
            ptr += 4;
			
            
            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "HighValidRange", highValidRange) == false)         // InA High bound
			{
				return false;
			}
            ptr += 4;
			
            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "LowValidRange", lowValidRange) == false)          // InA Low Bound
			{
				return false;
			}
            ptr += 4;
			
			ptr += 4;	// Reserved
			
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "WordOfFlags", flags) == false)      // InA WordOfFlags
			{
				return false;
			}
            ptr += 2;
		
        }
    }

    ptr = 888;
   
    // final crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, equipmentID, frame, 0, ptr, ptr);   //CRC-64
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
 
    let txId = module.propertyValue("ModuleFamily") + module.propertyValue("ModuleVersion");
    
    if (generate_txRxIoConfig(confFirmware, equipmentID, LMNumber, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId) == false)
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
    
    return true;
}