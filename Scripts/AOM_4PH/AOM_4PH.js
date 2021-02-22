// Generate configuration for module AOM-4PH
//
//
		
function generate_aom4ph(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_AOM4PH");
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
    
    let outController = module.jsFindChildObjectByMask(equipmentID + "_CTRLOUT");
    if (outController == null)
    {
		log.errCFG3004(equipmentID + "_CTRLOUT", equipmentID);
		return false;
    }
	
    let moduleSignalsCount = 32;
	let defaultFlags = 0x0;
	let defaultK1 = 1.0;
	let defaultK2 = 0.0;
	
	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (let i = 1; i <= moduleSignalsCount; i++)
    {
		let signalStrId = equipmentID + "_CTRLOUT";
		if (i < 10)
		{
			signalStrId += "_OUT0" + i;
		}
		else
		{
			signalStrId += "_OUT" + i;
		}
			
		
		let signal = signalSet.getSignalByEquipmentID(signalStrId);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    Out" + i + " [default]: [" + frame + ":" + ptr + "] K1 DAC = " + defaultK1 + 
			"; [" + frame + ":" + (ptr + 4) + "] K2 DAC = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 8) + "] K1 ADC = " + defaultK1 +
			"; [" + frame + ":" + (ptr + 12) + "] K2 ADC = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 16) + "] Flags = " + defaultFlags + "\r\n");
            
            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1 DAC", defaultK1) == false)      // DefaultK1 DAC
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K2 DAC", defaultK2) == false)      // DefaultK2 DAC
			{
				return false;
			}
            ptr += 4;
	
            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1 ADC", defaultK1) == false)      // DefaultK1 ADC
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K2 ADC", defaultK2) == false)      // DefaultK2 ADC
			{
				return false;
			}
            ptr += 4;
			
			if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "WordOfFlags", defaultFlags) == false)      // DefaultWordOfFlags
			{
				return false;
			}
            ptr += 2;

			ptr += 2;	// Reserved
        }
        else
        {
			// Check properties of signal

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
			
			let highEngineeringUnits = signal.highEngineeringUnits();
			let lowEngineeringUnits = signal.lowEngineeringUnits();

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
			if (highEngineeringUnits < lowEngineeringUnits)
			{
				// error
				log.errCFG3013("HighEngineeringUnits", highEngineeringUnits, compareLess, "LowEngineeringUnits", lowEngineeringUnits, signal.decimalPlaces(), signalStrId);
			}
			if (highEngineeringUnits == lowEngineeringUnits)
			{
				// error
				log.errCFG3013("HighEngineeringUnits", highEngineeringUnits, compareEqual, "LowEngineeringUnits", lowEngineeringUnits, signal.decimalPlaces(), signalStrId);
			}
			
			// Get signals properties
			
			// OUT
				
			let outputMode = signal.propertyValue("OutputMode");
			if (outputMode == undefined) 
			{
				log.errCFG3000("OutputMode", signalStrId);
			}
			
			// Convert electric to physical
			
			let highPhysical = unitsConvertor.electricToPhysical_Output(electricHighLimit, electricLowLimit, electricHighLimit, electricUnit, outputMode);
			let lowPhysical = unitsConvertor.electricToPhysical_Output(electricLowLimit, electricLowLimit, electricHighLimit, electricUnit, outputMode);
			
			if (highPhysical.ok == false)
			{
				switch (highPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(highPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrId);
					}
						break;
					case UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, signal.decimalPlaces(), signalStrId);
					}
						break;
					case UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, highPhysical.expectedLowValidRange, highPhysical.expectedHighValidRange, signal.decimalPlaces(), signalStrId);
					}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + highPhysical.errorCode + "), signal " + signalStrId);
				}
			}
			if (lowPhysical.ok == false)
			{
				switch (lowPhysical.errorCode)
				{
					case UnitsConvertorErrorCode.ErrorGeneric:
					{
						log.errINT1001(lowPhysical.errorMessage + ", module " + module.propertyValue("EquipmentID") + ", signal " + signalStrId);
					}
						break;
					case UnitsConvertorErrorCode.LowLimitOutOfRange:
					{
						log.errCFG3010("ElectricLowLimit", electricLowLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, signal.decimalPlaces(), signalStrId);
					}
						break;
					case UnitsConvertorErrorCode.HighLimitOutOfRange:
					{
						log.errCFG3010("ElectricHighLimit", electricHighLimit, lowPhysical.expectedLowValidRange, lowPhysical.expectedHighValidRange, signal.decimalPlaces(), signalStrId);
					}
						break;
					default:
						log.errINT1001("unitsConvertor.electricToPhysical_Input() - unknown error code (" + lowPhysical.errorCode + "), signal " + signalStrId);
				}
			}
			
			if (highPhysical.toDouble == lowPhysical.toDouble)
			{
				// error
				log.errCFG3013("calculated HighPhysical", highPhysical.toDouble, compareEqual, "calculated LowPhysical", lowPhysical.toDouble, 0, signalStrId);
			}

			// end of convert electric to physical
				
			let outputModeCode = 0;
			
			switch (outputMode)
			{
				case OutputMode.Plus0_Plus5_V:		outputModeCode = 0;	break;
				case OutputMode.Plus4_Plus20_mA:	outputModeCode = 1;	break;
				case OutputMode.Minus10_Plus10_V:	outputModeCode = 2;	break;
				case OutputMode.Plus0_Plus5_mA:		outputModeCode = 3;	break;
				default:
				{
					log.errINT1001("Unknown OutputMode type " + outputMode + " in " + signalStrId);
				}
			}
				
			let flags = outputMode;
			
			// IN AOM_SR outputs x and y are reversed (x is Engineering, y is ADC)

			let x1 = lowEngineeringUnits;
			let x2 = highEngineeringUnits;
			
			let y1 = lowPhysical.toDouble;
			let y2 = highPhysical.toDouble;

			if (x1 == x2) // Prevent division by zero
			{
				x1 = 0;	
				x2 = 1;
			}

			if (y1 == y2) // Prevent division by zero
			{
				y1 = 0;	
				y2 = 1;
			}

			let k1DAC = (y2 - y1) / (x2 - x1);		// K
			let k2DAC = y1 - k1DAC * x1;			// B

			let k1ADC = (x2 - x1) / (y2 - y1);		// K
			let k2ADC = x1 - k1ADC * y1;			// B

			confFirmware.writeLog("    Out" + i + ": [" + frame + ":" + ptr + "] K1 DAC = " + k1DAC + 
			"; [" + frame + ":" + (ptr + 4) + "] K2 DAC = " + k2DAC +
			"; [" + frame + ":" + (ptr + 8) + "] K1 ADC = " + k1ADC +
			"; [" + frame + ":" + (ptr + 12) + "] K2 ADC = " + k2ADC +
			"; HighPhysicalRange = " + highPhysical.toDouble +
			"; LowPhysicalRange = " + lowPhysical.toDouble +
			"; [" + frame + ":" + (ptr + 16) + "] Flags = " + flags + "\r\n");

			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1 DAC", k1DAC) == false)         // K1
			{
				return false;
			}
            ptr += 4;
            
			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K2 DAC", k2DAC) == false)         // K2
			{
				return false;
			}
            ptr += 4;
			
			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K1 ADC", k1ADC) == false)         // K1
			{
				return false;
			}
            ptr += 4;
            
			if (setDataFloat(confFirmware, log, LMNumber, equipmentID, frame, ptr, "K2 ADC", k2ADC) == false)         // K2
			{
				return false;
			}
            ptr += 4;

            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "WordOfFlags", flags) == false)      // InA WordOfFlags
			{
				return false;
			}
            ptr += 2;

			
			ptr += 2;	// Reserved
			
        }
    }

    // final crc
	
	ptr = 760;
	
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
    let dataTransmittingEnableFlag = true;
    let dataReceiveEnableFlag = true;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 6;
    let dataFramesQuantity = 1;
 
    let txId = module.jsModuleFamily() + module.propertyValue("ModuleVersion");
    
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
