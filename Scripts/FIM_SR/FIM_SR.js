// Generate configuration for module FIM_SR
//
//
function generate_fimsr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    let ptr = 0;
    
    let FIMSignalMaxCount = 8;
	
	let tsConstant = 200 * 0.000001;	// 200 us
    
    let defaultTf = valToADC(0 / tsConstant, 0, 65535, 0, 0xffff);
    let defaultHighBound = 4000;
    let defaultLowBound = 0.1;
	let defaultK1 = 0.0;
	let defaultK2 = 0.0;
	let defaultWordOfFlags = 0;
    
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (let i = 0; i < FIMSignalMaxCount; i++)
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
		
		let signal = signalSet.getSignalByEquipmentID(signalStrId);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + (ptr + 0) + "] WordOfFlags = " + defaultWordOfFlags +
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + defaultTf + 
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + defaultK1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + defaultLowBound + "\r\n");
            
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", defaultWordOfFlags) == false)      // InA DefaultWordOfFlags
			{
				return false;
			}
            ptr += 2;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "Tf", defaultTf) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 4;

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

            ptr += 2;	//Reserved
	
        }
        else
        {
			let compareEqual = 0;
			let compareLess = 1;
			let compareMore = 2;

			let unitsConvertor = confFirmware.jsGetUnitsConvertor();
			if (unitsConvertor == null)
			{
				log.errINT1001("confFirmware.jsGetUnitsConvertor returned null");
				return false;
			}
			
			let electricHighLimit = signal.propertyValue("ElectricHighLimit");
			if (electricHighLimit == undefined) 
			{
				log.errCFG3000("ElectricHighLimit", signalStrId);
				return false;
			}

			let electricLowLimit = signal.propertyValue("ElectricLowLimit");
			if (electricLowLimit == undefined) 
			{
				log.errCFG3000("ElectricLowLimit", signalStrId);
				return false;
			}

			let electricUnit = signal.propertyValue("ElectricUnit");
			if (electricUnit == undefined) 
			{
				log.errCFG3000("ElectricUnit", signalStrId);
				return false;
			}
			
			let sensorType = signal.propertyValue("SensorType");
			if (sensorType == undefined) 
			{
				log.errCFG3000("SensorType", signalStrId);
				return false;
			}

			let highLimit = signal.highEngineeringUnits();
			let lowLimit = signal.lowEngineeringUnits();
			
			let highValidRange = signal.highValidRange();
			let lowValidRange = signal.lowValidRange();

			if (electricHighLimit < electricLowLimit)
			{
				// error
				log.errCFG3013("ElectricHighLimit", electricHighLimit, compareLess, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
			}
			if (electricHighLimit == electricLowLimit)
			{
				// error
				log.errCFG3013("ElectricHighLimit", electricHighLimit, compareEqual, "ElectricLowLimit", electricLowLimit, 0, signalStrId);
			}
			if (signal.highEngineeringUnits() == signal.lowEngineeringUnits())
			{
				// error
				log.errCFG3013("HighEngineeringUnits", signal.highEngineeringUnits(), compareEqual, "LowEngineeringUnits", signal.lowEngineeringUnits(), signal.decimalPlaces(), signalStrId);
			}
			if (signal.highValidRange() == signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareEqual, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
			}
			if (signal.highEngineeringUnits() > signal.lowEngineeringUnits() && signal.highValidRange() < signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareLess, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
			}
			if (signal.highEngineeringUnits() < signal.lowEngineeringUnits() && signal.highValidRange() > signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareMore, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
			}
		
			// Convert electric to physical
			
			let highPhysical = unitsConvertor.electricToPhysical_Input(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0);
			let lowPhysical = unitsConvertor.electricToPhysical_Input(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, sensorType, 0);
			
			if (highPhysical.ok == false)
			{
				switch (highPhysical.errorCode)
				{
					case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(highPhysical.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrId);
					}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrId);
					}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, 4, signalStrId);
					}
						break;
					default:
					{
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysical.errorCode + "), signal " + signalStrId);
					}
				}
			}
			if (lowPhysical.ok == false)
			{
				switch (lowPhysical.errorCode)
				{
					case ConfigStruct.UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(lowPhysical.errorMessage + ", module " + module.equipmentId + ", signal " + signalStrId);
					}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrId);
					}
						break;
					case ConfigStruct.UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, 4, signalStrId);
					}
						break;
					default:
					{
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysical.errorCode + "), signal " + signalStrId);
					}
				}
			}
			
			if (highPhysical.toDouble == lowPhysical.toDouble)
			{
				// error
				log.errCFG3013("calculated HighPhysical", highPhysical.toDouble, compareEqual, "calculated LowPhysical", lowPhysical.toDouble, 0, signalStrId);
			}

			// end of convert electric to physical
			
			let tf = signal.filteringTime();
			
			if (tf < 0 * tsConstant || tf > 65535 * tsConstant)
			{
				log.errCFG3010("FilteringTime", tf, 0 * tsConstant, 65535 * tsConstant, 6, signalStrId);
			}
			
			tf = tf / tsConstant;
		
            let filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
			
			//
	
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

			let lowValidRangeMin = 0.0;
			let highValidRangeMax = 50000;

			let lowValidRangeMinEngineering = lowValidRangeMin * k1 + k2;
			let highValidRangeMaxEngineering = highValidRangeMax * k1 + k2;
			
			// Round this value to supplied decimal places
			
			let decimalPlaces = signal.propertyValue("DecimalPlaces");

			lowValidRangeMinEngineering = parseFloat(lowValidRangeMinEngineering.toFixed(decimalPlaces));
			highValidRangeMaxEngineering = parseFloat(highValidRangeMaxEngineering.toFixed(decimalPlaces));
			
			//

			if (lowValidRange < lowValidRangeMinEngineering)
			{
				log.errCFG3010("LowValidRange", lowValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrId);
			}
			if (highValidRange > highValidRangeMaxEngineering)
			{
				log.errCFG3010("HighValidRange", highValidRange, lowValidRangeMinEngineering, highValidRangeMaxEngineering, decimalPlaces, signalStrId);
			}

			// UnitEnable

			var unitEnable = signal.propertyValue("UnitEnable");
			if (unitEnable == undefined) 
			{
				log.errCFG3000("UnitEnable", signalStrId);
				return false;
			}
			
			if (unitEnable == false)
			{
				k1 = 0.0;
				k2 = 0.0;
			}
			
			//

			let flags = 0;
			
			confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] WordOfFlags = " + flags +
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + filteringTime + 
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + k1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + k2 +
			"; HighPhysicalRange = " + highPhysical.toDouble +
			"; LowPhysicalRange = " + lowPhysical.toDouble +
			"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + highValidRange +
			"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + lowValidRange + "\r\n");

            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", flags) == false)      // InA WordOfFlags
			{
				return false;
			}
            ptr += 2;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "FilteringTime", filteringTime) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 4;

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

            ptr += 2;	// Reserved
	
        }
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
 
	let txId = module.customModuleFamily + module.moduleVersion;
     
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