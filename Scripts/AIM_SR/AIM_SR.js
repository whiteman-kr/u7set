// Generate configuration for module AIM_SR
//
//
		
function generate_aimsr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();

	var ptr = 0;
    
    var moduleSignalsCount = 32;
	
	var tsConstant = 100 * 0.000001;	// 100 us
    
    var defaultTf = (5 * 0.001) / tsConstant;	// 5 ms
	
	var defaultFlags = 0x0a;
						

    var defaultHighBound = 24.576;
    var defaultLowBound = 0;
	var defaultK1 = 1.0;
	var defaultK2 = 0.0;
	
	// ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

    for (var i = 1; i <= moduleSignalsCount; i++)
    {
	
        // find a signal with Place = i
        //
		var signalStrId = module.equipmentId;
		
		if (i < 10)
		{
			signalStrId += "_CTRLIN_IN0" + i;
		}
		else
		{
			signalStrId += "_CTRLIN_IN" + i;
		}
			
		var signal = signalSet.getSignalByEquipmentID(signalStrId);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    in" + (i + 1) + "[default]: [" + frame + ":" + ptr + "] WordOfFlags = " + defaultFlags + 
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + defaultTf +
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + defaultK1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + defaultK2 +
			"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + defaultLowBound + "\r\n");
            
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

			if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "HighValidRange", defaultHighBound) == false)         // High bound
			{
				return false;
			}
			ptr += 4;
				
			if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LowValidRange", defaultLowBound) == false)          // Low Bound
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
	
			// Get signals properties
			
			var filteringTime = 0;
			
			var tf = signal.filteringTime();
			
			if (tf < 100 * 0.000001 || tf > 10)
			{
				log.errCFG3010("FilteringTime", tf, 100 * 0.000001, 10, 6, signalStrId);
				return false;
			}
            
			filteringTime = tf / tsConstant;
		
			// Calculate flags and ranges depending on signal type
			
			var inputRange = signal.propertyValue("InputRange");
			if (inputRange == undefined) 
			{
				log.errCFG3000("InputRange", signalStrId);
				return false;
			}
				
			var flags = inputRange;

			var y1 = lowEngineeringUnits;
			var y2 = highEngineeringUnits;
			
			var x1 = lowADC;
			var x2 = highADC;

			var k1 = (y2 - y1) / (x2 - x1);	// K
			var k2 = y1 - k1 * x1;			// B

			confFirmware.writeLog("    in" + (i + 1) + ": [" + frame + ":" + ptr + "] WordOfFlags = " + flags + 
			"; [" + frame + ":" + (ptr + 2) + "] Tf = " + filteringTime +
			"; [" + frame + ":" + (ptr + 6) + "] K1 = " + k1 +
			"; [" + frame + ":" + (ptr + 10) + "] K2 = " + k2 +
			"; [" + frame + ":" + (ptr + 14) + "] HighValidRange = " + signal.highValidRange() +
			"; [" + frame + ":" + (ptr + 18) + "] LowValidRange = " + signal.lowValidRange() + "\r\n");

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
    var dataFramesQuantity = 0;
 
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
