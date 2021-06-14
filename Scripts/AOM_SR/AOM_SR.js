// Generate configuration for module AOM-SR
//
//
		
function generate_aomsr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	var Mode_420mA = 1;
	var Mode_020mA = 4;
	var Mode_024mA = 5;
	
    var ptr = 0;
    
    let outControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLOUT");
    if (outControllerObject == null || outControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLOUT",module.equipmentId);
		return false;
    }

    let outController =  outControllerObject.toController();
	
    var moduleSignalsCount = 32;
    var defaultTf = 0;
	var defaultFlags = 0x7;
	var defaultK1 = 1.0;
	var defaultK2 = 0.0;
	
	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (var i = 1; i < moduleSignalsCount - 1; i++)
    {
		var signalStrId = module.equipmentId + "_CTRLOUT";
		if (i < 10)
		{
			signalStrId += "_OUT0" + i;
		}
		else
		{
			signalStrId += "_OUT" + i;
		}
			
		
		var signal = signalSet.getSignalByEquipmentID(signalStrId);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    Out" + i + " [default]: [" + frame + ":" + ptr + "] WordOfFlags = " + defaultFlags + 
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + defaultTf +
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + defaultK1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + defaultK2 + "\r\n");
            
            if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", defaultFlags) == false)      // DefaultWordOfFlags
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
			
			// Get signals properties
			
			var filteringTime = 0;
			
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
				
			var flags = outputMode;
			
			// IN AOM_SR outputs x and y are reversed (x is engineering, y is ADC)

			var x1 = lowEngineeringUnits;
			var x2 = highEngineeringUnits;
			
			var y1 = lowADC;
			var y2 = highADC;

			var k1 = (y2 - y1) / (x2 - x1);	// K
			var k2 = y1 - k1 * x1;			// B

			confFirmware.writeLog("    out" + i + ": [" + frame + ":" + ptr + "] WordOfFlags = " + flags + 
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + filteringTime +
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + k1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + k2 + "\r\n");


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
			
			ptr += 2;	// Reserved
			
        }
    }

    // final crc
	
	ptr = 632;
	
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
    
    var configFramesQuantity = 5;
    var dataFramesQuantity = 1;
 
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
