// Generate configuration for module OCMSR
//
//

function generate_ocmsr(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	//var Mode_RS232 = 0;
	//var Mode_RS485 = 1;

	let checkProperties = ["OptoPortCount"];
	for (let cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}


	//var txRxConfigFrame = frame;
	// generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage);
	//
	
	var portCount = module.propertyValue("OptoPortCount");
	
	var txWordsCount = 0;
	
	for (var p = 0; p < portCount; p++)
	{
		let controllerID = module.equipmentId + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);
	    
		let controllerObject = module.childByEquipmentId(controllerID);
		if (controllerObject == null || controllerObject.isController() == false)
		{
			log.errCFG3004(controllerID, module.equipmentId);
			return -1;
		}
			
		let controller = controllerObject.toController();		

		var optoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
		if (optoPort == null)
		{
			continue;
		}
		
		if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0)
		{
			continue;
		}			
		
		confFirmware.writeLog("    OptoPort " + controller.equipmentId + ": connection ID = " + optoPort.equipmentID() + 
			" (" + optoPort.connectionID() + ")\r\n");
					
		var ptr = 0 + p * 2;
		
		var startAddress = optoPort.txStartAddress();
		
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), startAddress) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + startAddress + "\r\n");
				
		ptr = 5 * 2 + p * 2;
		var value = optoPort.txDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		var txPortWordsCount = startAddress + value;
		if (txWordsCount < txPortWordsCount)
		{
			txWordsCount = txPortWordsCount;
		}
				
		ptr = 10 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 15 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		var dataUID = 0;
		if (optoPort.isLinked() == true)
		{
			var linkedPort = optoPort.linkedPortID();
			var linkedOptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
			if (linkedOptoPort != null)
			{
				dataUID = linkedOptoPort.txDataID();
			}
		}

		ptr = 20 * 2 + p * 4;
		if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
				
		/*if (optoPort.enableSerial() == true)
		{
			ptr = 30 * 2 + p * 2;
			value = optoPort.portID();	//???
			if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX ID for RS-232/485 transmitter " + (p + 1), value) == false)
			{
				return -1;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX ID for RS-232/485 transmitter " + (p + 1) + " = " + value + "\r\n");
					
			ptr = 35 * 2 + p * 4;
			if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RS-232/485 Data UID " + (p + 1), dataUID) == false)
			{
				return -1;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RS-232/485 Data UID " + (p + 1) + " = " + dataUID + "\r\n");
				
			//
			// RS232/485_CFG
			//
				
				
			var txStandard = Mode_RS232;	//0 - rs232, 1 - rs485
			if (optoPort.jsSerialMode() == Mode_RS485)
			{
				txStandard = Mode_RS485;
			}
			
			confFirmware.writeLog("    serialMode = " + txStandard + "\r\n");

			ptr = 45 * 2;
				
			//bits 9..0
			//
			var txEn = 1;	//1 - enabled
			var txMode = (txEn << 1) | txStandard;
			txMode <<= (p * 2);
				
			// this is only for OCM version 255 (ACNF)
			//
			// bits 14..10
			//
			var txDuplex = 0;	//1 - enabled
				
			if (optoPort.enableDuplex() == true)
			{
				txDuplex = 1;
			}
			confFirmware.writeLog("    enableDuplex = " + txDuplex + "\r\n");
			
			txDuplex <<= 10;
			txDuplex <<= p;
			txMode |= txDuplex;
			//
			// this is only for OCM version 255 (ACNF)
				
			var allModes = confFirmware.data16(frame, ptr);
			allModes |= txMode;
			if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "ModeFlags", allModes) == false)
			{
				return -1;
			}
		}*/
	} // p
	
	/*var ptr = 45 * 2;
	
	var allModes = confFirmware.data16(frame, ptr);
	confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RS mode configuration = " + allModes + "\r\n");
	*/

	// end of generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage);
	//
	
    ptr = 120;
    
    // crc
    var stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
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
    var dataTransmittingEnableFlag = true;
    var dataReceiveEnableFlag = true;
	
	var ocmFrameSize = 64;
    
    var flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    var configFramesQuantity = 1;
    var dataFramesQuantity = 0;
	if (txWordsCount > 0)
	{
		dataFramesQuantity = Math.ceil(txWordsCount / ocmFrameSize);
	}
	confFirmware.writeLog("    txWordsCount = " + txWordsCount + "\r\n");
	
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