// Generate configuration for module RIM
//
//
function generate_rim(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    let ptr = 0;
    
    let RIMSignalMaxCount = 8;
	
	let tsConstant = 2000 * 0.000001;	// 2000 us = 2 ms
    
    let defaultTf = valToADC(0.1 / tsConstant, 0, 65535, 0, 0xffff);
    let defaultHighBound = 100;
    let defaultLowBound = 0;
	let defaultK1 = 1.0;
	let defaultK2 = 0.0;
	let defaultR0 = 100;
	let defaultWordOfFlags = 16;
    let defaultSpreadTolerance = Math.round((0xffff - 0) * 0.005);		// 2% = 328h
    
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (let i = 0; i < RIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
		let signalStrId = inController.equipmentId+ "_IN";
		
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
			"; [" + frame + ":" + (ptr + 8) + "] K2 = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + defaultLowBound +
			"; [" + frame + ":" + (ptr + 20) + "] R0 = " + defaultR0 +
			"; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + defaultWordOfFlags + "\r\n");
            
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "Tf", defaultTf) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 2;

            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "SpreadTolerance", defaultSpreadTolerance) == false)      // InA SpreadTolerance
			{
				return false;
			}
            ptr += 2;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", defaultK1) == false)      // InA DefaultK1
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K2", defaultK2) == false)      // InA DefaultK2
			{
				return false;
			}
            ptr += 4;


            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", defaultHighBound) == false)         // InA High bound
			{
				return false;
			}
            ptr += 4;
			
            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", defaultLowBound) == false)          // InA Low Bound
			{
				return false;
			}
            ptr += 4;
			
            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "R0_Ohm", defaultR0) == false)          // R0
			{
				return false;
			}
			ptr += 4;	// Reserved
			
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", defaultWordOfFlags) == false)      // InA DefaultWordOfFlags
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

			let R0A = signalA.propertyValue("R0_Ohm");
			if (R0A == undefined) 
			{
				log.errCFG3000("R0_Ohm", signalStrIdA);
				return false;
			}

			let RTDConfModeA = signalA.propertyValue("RTDConfMode");
			if (RTDConfModeA == undefined) 
			{
				log.errCFG3000("RTDConfMode", signalStrIdA);
				return false;
			}

			let PGAGainA = signalA.propertyValue("PGAGain");
			if (PGAGainA == undefined) 
			{
				log.errCFG3000("PGAGain", signalStrIdA);
				return false;
			}

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

			let R0B = signalB.propertyValue("R0_Ohm");
			if (R0B == undefined) 
			{
				log.errCFG3000("R0_Ohm", signalStrIdB);
				return false;
			}

			let RTDConfModeB = signalB.propertyValue("RTDConfMode");
			if (RTDConfModeB == undefined) 
			{
				log.errCFG3000("RTDConfMode", signalStrIdB);
				return false;
			}

			let PGAGainB = signalB.propertyValue("PGAGain");
			if (PGAGainB == undefined) 
			{
				log.errCFG3000("PGAGain", signalStrIdB);
				return false;
			}
			

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
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "HighValidRange");
			}

			if (signalA.lowValidRange() != signalB.lowValidRange())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "LowValidRange");
			}

			if (electricHighLimit != electricHighLimitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "ElectricHighLimit");
			}

			if (electricLowLimit != electricLowLimitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "ElectricLowLimit");
			}

			if (electricUnit != electricUnitB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "ElectricUnit");
			}

			if (sensorType != sensorTypeB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "SensorType");
			}

			if (signalA.highEngineeringUnits() != signalB.highEngineeringUnits())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "HighEngineeringUnits");
			}

			if (signalA.lowEngineeringUnits() != signalB.lowEngineeringUnits())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "LowEngineeringUnits");
			}

			if (signalA.filteringTime() != signalB.filteringTime())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "FilteringTime");
			}

			if (signalA.spreadTolerance() != signalB.spreadTolerance())
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "SpreadTolerance");
			}

			if (R0A != R0B)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "R0_Ohm");
			}

			if (PGAGainA != PGAGainB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "PGAGain");
			}

			if (RTDConfModeA != RTDConfModeB)
			{
				log.errCFG3028(signalStrIdA, signalStrIdB, module.equipmentId, "RTDConfMode");
			}

			// Convert electric to physical
			
			let highPhysical = unitsConvertor.electricToPhysical_ThermoResistor(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, R0A);
			let lowPhysical = unitsConvertor.electricToPhysical_ThermoResistor(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, R0A);
			
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
					{
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysical.errorCode + "), signal " + signalStrIdA);
					}
				}
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
					{
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysical.errorCode + "), signal " + signalStrIdA);
					}
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
			
			//
	
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

			let sensorTypeCode = 0;
			
			let decimalPlaces = signalA.propertyValue("DecimalPlaces");
			
			let PGAGainCode = PGAGainA;
			
			let PGAGainCode_AUTO = 255;

			let highValidRangeMax = 0;
			let lowValidRangeMin = 0;
			
			switch (sensorType)
			{
				case SensorType.Ohm_Pt_a_385:		
					sensorTypeCode = 0;	
					highValidRangeMax = 850;
					lowValidRangeMin = -200;
					
					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						if (R0A > 200) PGAGainCode = 0;					//6.25	
						if (R0A > 100 && R0A <= 200) PGAGainCode = 1;	//12.5	
						if (R0A >= 50 && R0A <= 100) PGAGainCode = 2;	//25	
						if (R0A < 50) PGAGainCode = 3;					//50	
					}
				
				break;
				case SensorType.Ohm_Pt_a_391:		
					sensorTypeCode = 1;	
					highValidRangeMax = 850;
					lowValidRangeMin = -200;

					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						if (R0A > 200) PGAGainCode = 0;					//6.25	
						if (R0A > 100 && R0A <= 200) PGAGainCode = 1;	//12.5	
						if (R0A >= 50 && R0A <= 100) PGAGainCode = 2;	//25	
						if (R0A < 50) PGAGainCode = 3;					//50	
					}
						
				break;
				case SensorType.Ohm_Cu_a_428:		
					sensorTypeCode = 2;	
					highValidRangeMax = 200;
					lowValidRangeMin = -180;

					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						if (R0A > 200) PGAGainCode = 1;					//12.5	
						if (R0A > 100 && R0A <= 200) PGAGainCode = 2;	//25	
						if (R0A <= 100) PGAGainCode = 3;				//50	
					}

				break;
				case SensorType.Ohm_Cu_a_426:		
					sensorTypeCode = 3;	
					highValidRangeMax = 200;
					lowValidRangeMin = -50;

					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						if (R0A > 200) PGAGainCode = 1;					//12.5	
						if (R0A > 100 && R0A <= 200) PGAGainCode = 2;	//25	
						if (R0A <= 100) PGAGainCode = 3;				//50	
					}

				break;
				case SensorType.Ohm_Ni_a_617:		
					sensorTypeCode = 4;	
					highValidRangeMax = 180;
					lowValidRangeMin = -60;

					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						if (R0A > 170) PGAGainCode = 1;					//12.5	
						if (R0A > 80 && R0A <= 170) PGAGainCode = 2;	//25	
						if (R0A <= 80) PGAGainCode = 3;					//50	
					}
	
				break;
				case SensorType.Ohm_Raw:		
					sensorTypeCode = 5;	
					highValidRangeMax = 1500;
					lowValidRangeMin = 5;
					if (PGAGainA == PGAGainCode_AUTO) // AUTO
					{
						log.errCFG3041("PGAGain", "Auto", "Gain x6,25..Gain x50", signalStrIdA);
					}
				break;
				default:
				{
					log.errINT1001("Unknown sensor type " + sensorType + " in " + signalStrIdA);
				}
			}
			
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

			let flags = (RTDConfModeA << 8) | (PGAGainCode << 4) | sensorTypeCode;

			confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime + 
			"; [" + frame + ":" + (ptr + 2) + "] SpreadTolerance = " + spreadTolerance +
			"; [" + frame + ":" + (ptr + 4) + "] K1 = " + k1 +
			"; [" + frame + ":" + (ptr + 8) + "] K2 = " + k2 +
			"; HighPhysicalRange = " + highPhysical.toDouble +
			"; LowPhysicalRange = " + lowPhysical.toDouble +
			"; [" + frame + ":" + (ptr + 12) + "] HighValidRange = " + highValidRange +
			"; [" + frame + ":" + (ptr + 16) + "] LowValidRange = " + lowValidRange +
			"; [" + frame + ":" + (ptr + 20) + "] R0 = " + R0A +
			"; [" + frame + ":" + (ptr + 24) + "] WordOfFlags = " + flags + "\r\n");

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
			
            
			if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", k2) == false)         // K2
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
			
            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "R0_Ohm", R0A) == false)          // InA Low Bound
			{
				return false;
			}
			ptr += 4;
			
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", flags) == false)      // InA WordOfFlags
			{
				return false;
			}
            ptr += 2;
        }
    }

    ptr = 248;
   
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
    
    let configFramesQuantity = 2;
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
    
    return true;
}