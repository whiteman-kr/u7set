// Generate configuration for module AIM
//
//
function generate_aim(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_AIM");
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
    
    let AIMSignalMaxCount = 64;
	
	let tsConstant = 100 * 0.000001;
    
    let defaultTf = valToADC(5000 * 0.000001 / tsConstant, 0, 65535, 0, 0xffff);
    let defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    let defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    let defaultSpreadTolerance = Math.round((0xffff - 0) * 0.005);		// 2%
    
    let inController = module.jsFindChildObjectByMask(equipmentID + "_CTRLIN");
    if (inController == null)
    {
		log.errCFG3004(equipmentID + "_CTRLIN", equipmentID);
		return false;
    }
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

	let channelAPlace = 0;
	let channelASpreadTolerance = 0;
	let channelAStrID = "";

    for (let i = 0; i < AIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
        //let signal = findSignalByPlace(inController, i, Analog, Input, signalSet, log);
		let signalStrId = inController.propertyValue("EquipmentID") + "_IN";
		
		let entry = Math.floor(i / 2) + 1;
		if (entry < 10)
		{
			signalStrId = signalStrId + "0";
		}
		signalStrId = signalStrId + entry;
		
		let signalStrIdA = signalStrId + "A";
		
		if ((i % 2) == 0)
		{
			signalStrId = signalStrIdA;
		}		
		else
		{
			signalStrId = signalStrId + "B";
		}
		
		let signal = signalSet.getSignalByEquipmentID(signalStrId);
		
		// if no B channel is found, take A channel
		//
		if ((i % 2) != 0 && signal == null)
		{
			signal = signalSet.getSignalByEquipmentID(signalStrIdA);
		}
         
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + ptr + "] Tf = " + defaultTf + 
			"; [" + frame + ":" + (ptr + 2) + "] HighADC = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 4) + "] LowADC = " + defaultLowBound +
			"; [" + frame + ":" + (ptr + 6) + "] SpreadTolerance = " + defaultSpreadTolerance + "\r\n");
            
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "DefaultTf", defaultTf) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "DefaultHighBound", defaultHighBound) == false)         // InA High bound
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "DefaultLowBound", defaultLowBound) == false)          // InA Low Bound
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "DefaultSpreadTolerance", defaultSpreadTolerance) == false)      // InA SpreadTolerance
			{
				return false;
			}
            ptr += 2;
        }
        else
        {
		
			let tf = signal.filteringTime();
			
			if (tf < 1 * 100 * 0.000001 || tf > 65535 * 100 * 0.000001)
			{
				log.errCFG3010("FilteringTime", tf, 1 * 100 * 0.000001, 65535 * 100 * 0.000001, 6, signalStrId);
				return false;
			}
			
			tf = tf / tsConstant;
			
			let compareEqual = 0;
			let compareLess = 1;
			let compareMore = 2;
            
            let filteringTime = valToADC(tf, 0, 65535, 0, 0xffff);
            
			if (signal.highADC() < signal.lowADC())
			{
				// error
				log.errCFG3013("HighADC", signal.highADC(), compareLess, "LowADC", signal.lowADC(), 0, signalStrId);
				return false;
			}
			if (signal.highADC() == signal.lowADC())
			{
				// error
				log.errCFG3013("HighADC", signal.highADC(), compareEqual, "LowADC", signal.lowADC(), 0, signalStrId);
				return false;
			}
			if (signal.highEngineeringUnits() == signal.lowEngineeringUnits())
			{
				// error
				log.errCFG3013("HighEngineeringUnits", signal.highEngineeringUnits(), compareEqual, "LowEngineeringUnits", signal.lowEngineeringUnits(), signal.decimalPlaces(), signalStrId);
				return false;
			}
			if (signal.highValidRange() == signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareEqual, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				return false;
			}
			if (signal.highEngineeringUnits() > signal.lowEngineeringUnits() && signal.highValidRange() < signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareLess, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				return false;
			}
			if (signal.highEngineeringUnits() < signal.lowEngineeringUnits() && signal.highValidRange() > signal.lowValidRange())
			{
				// error
				log.errCFG3013("HighValidRange", signal.highValidRange(), compareMore, "LowValidRange", signal.lowValidRange(), signal.decimalPlaces(), signalStrId);
				return false;
			}
			
			let highValidRangeADC = valToADC(signal.highValidRange(), signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.lowADC(), signal.highADC());
			let lowValidRangeADC = valToADC(signal.lowValidRange(), signal.lowEngineeringUnits(), signal.highEngineeringUnits(), signal.lowADC(), signal.highADC());
			
			if (lowValidRangeADC < 0)
			{
				log.errCFG3010("calculated_lowValidRangeADC", lowValidRangeADC, 0, 65535, 0, signalStrId);
				return false;
			}
			if (highValidRangeADC > 65535)
			{
				log.errCFG3010("calculated_highValidRangeADC", highValidRangeADC, 0, 65535, 0, signalStrId);
				return false;
			}

			let spreadTolerance = Math.round((signal.spreadTolerance() * 0.01) * (highValidRangeADC - lowValidRangeADC));

			if ((i & 1) == 0)
			{
				// this is A input
				channelAPlace = i;
				channelASpreadTolerance = spreadTolerance;
				channelAStrID = signal.appSignalID();
			}
			else
			{
				if (i == channelAPlace + 1)
				{
					// this is B input, next to saved A
					if (spreadTolerance != channelASpreadTolerance)
					{
						log.errCFG3009(channelAStrID, signal.appSignalID(), module.propertyValue("EquipmentID"));
						return false;
					}
				}
			}

			confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filteringTime + 
			"; [" + frame + ":" + (ptr + 2) + "] HighValidRangeADC = " + highValidRangeADC +
			"; [" + frame + ":" + (ptr + 4) + "] LowValidRangeADC = " + lowValidRangeADC +
			"; [" + frame + ":" + (ptr + 6) + "] SpreadTolerance = " + spreadTolerance + "\r\n");

            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "FilteringTime", filteringTime) == false)          // InA Filtering time constant
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "HighValidRangeADC", highValidRangeADC) == false)         // InA High bound
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "LowValidRangeADC", lowValidRangeADC) == false)          // InA Low Bound
			{
				return false;
			}
            ptr += 2;
            if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "SpreadTolerance", spreadTolerance) == false)      // InA SpreadTolerance
			{
				return false;
			}
            ptr += 2;
        }
    }

    // reserved
    ptr += 120;
   
    // final crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, equipmentID, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
    ptr += 8;
    
    //reserved
    ptr += 368;

    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    let dataTransmittingEnableFlag = false;
    let dataReceiveEnableFlag = true;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 5;
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