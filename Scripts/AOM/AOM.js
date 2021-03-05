
// Generate configuration for module AOM
//
//

function generate_aom(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	let Mode_05V = 0;
	let Mode_420mA = 1;
	let Mode_10V = 2;
	let Mode_05mA = 3;

    let ptr = 0;
    
    let AOMWordCount = 4;                       // total words count
    let AOMSignalsInWordCount = 8;              // signals in a word count
    
    let outControllerObject = module.childByEquipmentId(module.equipmentId + "_CTRLOUT");
    if (outControllerObject == null || outControllerObject.isController() == false)
    {
		log.errCFG3004(module.equipmentId + "_CTRLOUT",module.equipmentId);
		return false;
    }

    let outController =  outControllerObject.toController();

    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
    let place = 0;
    
    for (let w = 0; w < AOMWordCount; w++)
    {
        let data = 0;
        
        for (let c = 0; c < AOMSignalsInWordCount; c++)
        {
            let mode = Mode_05V;    //default

            //let signal = findSignalByPlace(outController, place, Analog, Output, signalSet, log);
			let signalStrId = outController.equipmentId + "_OUT";
			
			let entry = place + 1;
			if (entry < 10)
			{
				signalStrId = signalStrId + "0";
			}
			signalStrId = signalStrId + entry;
		
			let signal = signalSet.getSignalByEquipmentID(signalStrId);

            if (signal == null)
			{
				log.wrnCFG3007(signalStrId);
			}
			else
            {
				let outputMode = signal.jsOutputMode();
                if (outputMode < 0 || outputMode > Mode_05mA)
                {
					log.errCFG3002("Signal/OutputMode", outputMode, 0, Mode_05mA, signalStrId);
					return false;
                }

                mode = outputMode;
            }

            
            place++;
            
            let bit = c * 2;
            data |= (mode << bit);
        }
        
        if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr + w * 2, "OutputModeFlags" + w, data) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + (ptr + w * 2) + "] data" + w + " = " + data + "\r\n");
    }
    
    ptr += 120;
    
    // crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = 0x" + stringCrc64 + "\r\n");
    ptr += 8;    

    // reserved
    ptr += 880;
    
    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    let dataTransmittingEnableFlag = true;
    let dataReceiveEnableFlag = true;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 1;
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
