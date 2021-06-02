// Generate configuration for module MPS-35
//
//
		
function generate_mps35(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	var Mode_420mA = 1;
	var Mode_020mA = 4;
	var Mode_024mA = 5;
		
    var ptr = 0;
    
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN", module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();
	
    let outControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLOUT");
    if (outControllerObject == null || outControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLOUT", module.equipmentId);
		return false;
    }

    let outController =  outControllerObject.toController();
	
	var moduleSignals = ["_CTRLIN_IN01", "_CTRLIN_IN02", "_CTRLIN_IN03", "_CTRLIN_IN04", 
						"_CTRLOUT_OUT01", "_CTRLOUT_OUT02", "_CTRLOUT_OUT03", 
						"_CTRLOUT_OUT04", "_CTRLOUT_OUT05", "_CTRLOUT_OUT06"];
    var moduleSignalsCount = moduleSignals.length;
	
	var tsConstant = 100 * 0.000001;	// 100 us
    
    var tfMin = 100 * 0.000001;	// 100 us
	
	var tfMax = 10;	// 10 s

    var defaultTf = (5 * 0.001) / tsConstant;	// 5 ms
	
	var defaultFlags = [0x10, 0x10, 0x10, 0x10, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe];
						
	if (defaultFlags.length != moduleSignalsCount)
	{
		log.errINT1001("defaultFlags.length != moduleSignalsCount in " + module.equipmentId);
		return false;
	}
	
    var defaultHighBound = [78.125, 78.125, 78.125, 78.125];
    var defaultLowBound = [-78.125, -78.125, -78.125, -78.125];
	var defaultK1 = 1.0;
	var defaultK2 = 0.0;
	
	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (var i = 0; i < moduleSignalsCount; i++)
    {
	
		var isInputSignal = i <= 3;
	
        // find a signal with Place = i
        //
		var signalStrId = module.equipmentId + moduleSignals[i];
		
		var signal = signalSet.getSignalByEquipmentID(signalStrId);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			if (isInputSignal == true)
			{
				confFirmware.writeLog("    in" + (i + 1) + "[default]: [" + frame + ":" + ptr + "] WordOfFlags = " + defaultFlags[i] + 
				"; [" + frame + ":" + (ptr + 2) + "] Tf = " + defaultTf +
				"; [" + frame + ":" + (ptr + 6) + "] K1 = " + defaultK1 +
				"; [" + frame + ":" + (ptr + 10) + "] K2 = " + defaultK2 +
				"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + defaultHighBound[i] +
				"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + defaultLowBound[i] + "\r\n");
			}
			else
			{
				confFirmware.writeLog("    out" + (i - 3) + "[default]: [" + frame + ":" + ptr + "] WordOfFlags = " + defaultFlags[i] + 
				"; [" + frame + ":" + (ptr + 2) + "] Tf = " + defaultTf +
				"; [" + frame + ":" + (ptr + 6) + "] K1 = " + defaultK1 +
				"; [" + frame + ":" + (ptr + 10) + "] K2 = " + defaultK2 + "\r\n");
			}
            
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", defaultFlags[i]) == false)      // DefaultWordOfFlags
			{
				return false;
			}
            ptr += 2;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "Tf", defaultTf) == false)          // Filtering time constant
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", defaultK1) == false)      // DefaultK1
			{
				return false;
			}
            ptr += 4;

            if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K2", defaultK2) == false)      // DefaultK2
			{
				return false;
			}
            ptr += 4;

			if (isInputSignal == true)
			{
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", defaultHighBound[i]) == false)         // High bound
				{
					return false;
				}
				ptr += 4;
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", defaultLowBound[i]) == false)          // Low Bound
				{
					return false;
				}
				ptr += 4;
			}
			
			ptr += 2;	// Reserved
        }
        else
        {
			// Check properties of signal

			var compareEqual = 0;
			var compareLess = 1;
			var compareMore = 2;
            
			var highADC = signal.propertyValue("HighPhysicalUnits");
			var lowADC = signal.propertyValue("LowPhysicalUnits");

			var highEngineeringUnits = signal.highEngineeringUnits();
			var lowEngineeringUnits = signal.lowEngineeringUnits();

			if (highADC < lowADC)
			{
				// error
				log.errCFG3013("HighPhysicalUnits", highADC, compareLess, "LowPhysicalUnits", lowADC, 0, signalStrId);
				return false;
			}
			if (highADC == lowADC)
			{
				// error
				log.errCFG3013("HighPhysicalUnits", highADC, compareEqual, "LowPhysicalUnits", lowADC, 0, signalStrId);
				return false;
			}
			if (highEngineeringUnits < lowEngineeringUnits)
			{
				// error
				log.errCFG3013("HighEngineeringUnits", highEngineeringUnits, compareLess, "LowEngineeringUnits", lowEngineeringUnits, signal.decimalPlaces(), signalStrId);
				return false;
			}
			if (highEngineeringUnits == lowEngineeringUnits)
			{
				// error
				log.errCFG3013("HighEngineeringUnits", highEngineeringUnits, compareEqual, "LowEngineeringUnits", lowEngineeringUnits, signal.decimalPlaces(), signalStrId);
				return false;
			}
			
			if (isInputSignal == true)
			{
				if (signal.highValidRange() == signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareEqual, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
					return false;
				}
				if (highEngineeringUnits > lowEngineeringUnits && signal.highValidRange() < signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareLess, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
					return false;
				}
				if (highEngineeringUnits < lowEngineeringUnits && signal.highValidRange() > signal.lowValidRange())
				{
					// error
					log.errCFG3013("HighValidRange", signal.highValidRange(), compareMore, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
					return false;
				}
			}
	
			// Get signals properties
			
			var filteringTime = 0;
			
			if (isInputSignal == true)
			{
				var tf = signal.filteringTime();
			
				if (tf < tfMin || tf > tfMax)
				{
					log.errCFG3010("FilteringTime", tf, tfMin, tfMax, 6, signalStrId);
					return false;
				}
            
				filteringTime = tf / tsConstant;
			}
		
			var flags = 0;
			
			var unitEnable = signal.propertyValue("UnitEnable");
			if (unitEnable == undefined) 
			{
				log.errCFG3000("UnitEnable", signalStrId);
				return false;
			}
			
			if (unitEnable == true)
			{
				flags |= 1;
			}

			// Calculate flags and ranges depending on signal type

			if (isInputSignal == true)
			{
				// T
				
				var sensorType = signal.propertyValue("SensorType");
				if (sensorType == undefined) 
				{
					log.errCFG3000("SensorType", signalStrId);
					return false;
				}
				
				var sensorTypeCode = 0;
				
				var mV_Type_B=14;
				var mV_Type_E=15;
				var mV_Type_J=16;
				var mV_Type_K=17;
				var mV_Type_N=18;
				var mV_Type_R=19;
				var mV_Type_S=20;
				var mV_Type_T=21;
				var mV_Raw_Mul_8=22;
				var mV_Raw_Mul_32=23;
			
				var highLimitMax = 0;
				var lowLimitMax = 0;
			
				var decimalPlaces = signal.propertyValue("DecimalPlaces");

				switch (sensorType)
				{
					case mV_Type_B:		
						sensorTypeCode = 0;	
						highValidRangeMax = 1820;
						lowValidRangeMin = 250;
					break;
					case mV_Type_E:		
						sensorTypeCode = 1;	
						highValidRangeMax = 1000;
						lowValidRangeMin = -200;
					break;
					case mV_Type_J:		
						sensorTypeCode = 2;	
						highValidRangeMax = 1200;
						lowValidRangeMin = -210;
					break;
					case mV_Type_K:		
						sensorTypeCode = 3;	
						highValidRangeMax = 1372;
						lowValidRangeMin = -200;
					break;
					case mV_Type_N:		
						sensorTypeCode = 4;	
						highValidRangeMax = 1300;
						lowValidRangeMin = -200;
					break;
					case mV_Type_R:		
						sensorTypeCode = 5;	
						highValidRangeMax = 1768;
						lowValidRangeMin = -50;
					break;
					case mV_Type_S:		
						sensorTypeCode = 6;	
						highValidRangeMax = 1768;
						lowValidRangeMin = -50;
					break;
					case mV_Type_T:		
						sensorTypeCode = 7;	
						highValidRangeMax = 400;
						lowValidRangeMin = -200;
					break;
					case mV_Raw_Mul_8:	
						sensorTypeCode = 8;	
						highValidRangeMax = 78.125;
						lowValidRangeMin = -78.125;
						if (decimalPlaces < 3)
						{
							decimalPlaces = 3;
						}
					break;
					case mV_Raw_Mul_32:	
						sensorTypeCode = 12;	
						highValidRangeMax = 19,531;
						lowValidRangeMin = -19,531;
						if (decimalPlaces < 3)
						{
							decimalPlaces = 3;
						}
					break;
					default:
						log.errINT1001("Unknown T sensor type " + sensorType + " in " + signalStrId);
						return false;
				}				
				
				if (sensorType != mV_Raw_Mul_8 && sensorType != mV_Raw_Mul_32)
				{
					if (lowEngineeringUnits < lowValidRangeMin)
					{
						log.errCFG3010("LowEngineeringUnits", lowEngineeringUnits, lowValidRangeMin, highValidRangeMax, decimalPlaces, signalStrId);
						return false;
					}
					if (highEngineeringUnits > highValidRangeMax)
					{
						log.errCFG3010("HighEngineeringUnits", highEngineeringUnits, lowValidRangeMin, highValidRangeMax, decimalPlaces, signalStrId);
						return false;
					}
				}
				
				flags |= (sensorTypeCode << 1);
			}
			else
			{
				// OUT
				
				var outputMode = signal.propertyValue("OutputMode");
				if (outputMode == undefined) 
				{
					log.errCFG3000("OutputMode", signalStrId);
					return false;
				}
				
				var outputModeCode = 0;
				
				switch (outputMode)
				{
					case Mode_420mA:	outputModeCode = 5;	break;
					case Mode_020mA:	outputModeCode = 6;	break;
					case Mode_024mA:	outputModeCode = 7;	break;
					default:
					{
						log.errINT1001("Unknown OutputMode type " + outputMode + " in " + signalStrId);
						return false;
					}
				}
				
				flags |= (outputModeCode << 1);
			}

			var y1 = 0;
			var y2 = 0;
			
			var x1 = 0;
			var x2 = 0;
			
			if (isInputSignal == true)
			{
				y1 = lowEngineeringUnits;
				y2 = highEngineeringUnits;
				
				x1 = lowADC;
				x2 = highADC;
			}
			else
			{
				x1 = lowEngineeringUnits;
				x2 = highEngineeringUnits;
				
				y1 = lowADC;
				y2 = highADC;
			}

			var k1 = (y2 - y1) / (x2 - x1);	// K
			var k2 = y1 - k1 * x1;			// B

			if (isInputSignal == true)
			{
				confFirmware.writeLog("    in" + (i + 1) + ": [" + frame + ":" + ptr + "] WordOfFlags = " + flags + 
				"; [" + frame + ":" + (ptr + 2) + "] Tf = " + filteringTime +
				"; [" + frame + ":" + (ptr + 6) + "] K1 = " + k1 +
				"; [" + frame + ":" + (ptr + 10) + "] K2 = " + k2 +
				"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + signal.highValidRange() +
				"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + signal.lowValidRange() + "\r\n");
			}
			else
			{
				confFirmware.writeLog("    out" + (i - 3) + ": [" + frame + ":" + ptr + "] WordOfFlags = " + flags + 
				"; [" + frame + ":" + (ptr + 2) + "] Tf = " + filteringTime +
				"; [" + frame + ":" + (ptr + 6) + "] K1 = " + k1 +
				"; [" + frame + ":" + (ptr + 10) + "] K2 = " + k2 + "\r\n");
			}


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
			
            
			if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "K1", k2) == false)         // K2
			{
				return false;
			}
            ptr += 4;
			
            
			if (isInputSignal == true)
			{
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", signal.highValidRange()) == false)         // InA High bound
				{
					return false;
				}
				ptr += 4;
				
				if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", signal.lowValidRange()) == false)          // InA Low Bound
				{
					return false;
				}
				ptr += 4;
			}
			
			ptr += 2;	// Reserved
			
        }
    }

    // final crc
	
	ptr = 248;
	
    var stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
    ptr += 8;
    
	ptr = 1008;

    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    var dataTransmittingEnableFlag = true;
    var dataReceiveEnableFlag = true;
    
    var flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    var configFramesQuantity = 2;
    var dataFramesQuantity = 1;
 
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
