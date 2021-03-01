// Generate configuration for module AIFM
//
//
function generate_aifm(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    let ptr = 0;
 
    let inControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLIN");
    if (inControllerObject == null || inControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLIN",module.equipmentId);
		return false;
    }

    let inController =  inControllerObject.toController();
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
	
	let aifmChannelCount = 3;

	// CUR, FLU, FRQ
	//
	let koeffs = ["CUR", "FLU", "FRQ"];
	
	for (let k = 0; k < 3; k++)
	{
		for (let i = 0; i < aifmChannelCount; i++)
		{
			let signalID = inController.equipmentId + "_IN0" + (i + 1) + koeffs[k];
			let signal = inController.childByEquipmentId(signalID).toAppSignal();
			let value = 1;
			if (signal == null)
			{
				log.wrnCFG3005(signalID, inController.equipmentId);
			}
			else
			{
				value = signal.propertyValue("PowerCoefficient");
				if (value == null)
				{
					log.errCFG3000("PowerCoefficient", signal.equipmentId);
					return false;
				}
			}

			confFirmware.writeLog("    IN" + i + koeffs[k] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + "\r\n");
			
			if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID, value) == false)         // K2
			{
				return false;
			}
            ptr += 4;
			
			/*
			let v = 0;
			let mantissa = 0;
			let exponent = 0;
			if (signal != null)
			{
				v = signal.valueToMantExp1616(value);
				mantissa =  v >> 16;
				exponent = v & 0xffff;
			}
				
			confFirmware.writeLog("    IN" + i + koeffs[k] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + " [" + mantissa + "^" + exponent + "]\r\n");
	
			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID + "_mantissa", mantissa) == false)
			{
				return false;
			}
			
			ptr += 2;
			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID + "_exponent", exponent) == false)
			{
				return false;
			}
			ptr += 2;
			*/
		}
	}
	
	// POWER, REACT, PERIOD
	//
	let modeNames = ["PERIOD", "POWER", "REACT"];
	
	let setPointCount = 5;
	
	for (let k = 0; k < 3; k++)
	{
		for (let i = 0; i < aifmChannelCount; i++)
		{
			for (let m = 0; m < 3; m++)
			{
				for (let p = 0; p < setPointCount; p++)
				{
					let signalID = inController.equipmentId + "_IN0" + (i + 1) + koeffs[k] + modeNames[m];
					let signal = inController.childByEquipmentId(signalID).toAppSignal();
					let value = 1;
					if (signal == null)
					{
						log.wrnCFG3005(signalID, inController.equipmentId);
					}
					else
					{
						let setPointName = "SetPoint0" + (p + 1);
						
						value = signal.propertyValue(setPointName);
						if (value == null)
						{
							log.errCFG3000(setPointName, signal.equipmentId);
							return false;
						}
					}
					
					confFirmware.writeLog("    IN" + (i + 1) + koeffs[k] + modeNames[m] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + "\r\n");
			
					if (setDataFloat(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID, value) == false)         // K2
					{
						return false;
					}
					ptr += 4;
					
					/*
					let v = 0;
					let mantissa =  0;
					let exponent = 0;
					if (signal != null)
					{
						v = signal.valueToMantExp1616(value);
						mantissa =  v >> 16;
						exponent = v & 0xffff;
					}	
						
					confFirmware.writeLog("    IN" + (i + 1) + koeffs[k] + modeNames[m] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + " [" + mantissa + "^" + exponent + "]\r\n");
			
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID + "_mantissa", mantissa) == false)
					{
						return false;
					}

					ptr += 2;
					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, signalID + "_exponent", exponent) == false)
					{
						return false;
					}
					ptr += 2;*/
				}
			}
			ptr += 2; //reserved
			ptr += 2; //reserved
		}
	}
	
	ptr = 632;
	
    // final crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
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
    let dataTransmittingEnableFlag = true;
    let dataReceiveEnableFlag = true;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 5;
    let dataFramesQuantity = 1;

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
