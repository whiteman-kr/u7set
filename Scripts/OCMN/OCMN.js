// Generate configuration for module OCMN (ModuleVersion = 255)
//
//

function generate_ocmn(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	let Mode_RS232 = 0;
	let Mode_RS485 = 1;

	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_OCM");
		return false;
	}
	
	let equipmentID = module.propertyValue("EquipmentID");
	
	let checkProperties = ["Place", "ModuleVersion", "OptoPortCount"];
	for (let cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], equipmentID);
			return false;
		}
	}

	//let txRxConfigFrame = frame;
	// generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage);
	//
	
	let portCount = module.propertyValue("OptoPortCount");
	
	let txWordsCount = 0;
	
	for (let p = 0; p < portCount; p++)
	{
		let controllerID = module.propertyValue("EquipmentID") + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);
	    
		let controller = module.jsFindChildObjectByMask(controllerID);
		if (controller == null)
		{
			log.errCFG3004(controllerID, module.propertyValue("EquipmentID"));
			return -1;
		}
		
		if (controller.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "Class_Controller");
			return -1;
		}
		
		let controllerEquipmentID = controller.propertyValue("EquipmentID");

		let optoPort = opticModuleStorage.jsGetOptoPort(controllerEquipmentID);
		if (optoPort == null)
		{
			continue;
		}
		
		if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0)
		{
			continue;
		}			
		
		confFirmware.writeLog("    OptoPort " + controllerEquipmentID + ": connection ID = " + optoPort.equipmentID() + 
			" (" + optoPort.connectionID() + ")\r\n");
					
		let ptr = 0 + p * 2;
		
		let startAddress = optoPort.txStartAddress();
		
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), startAddress) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + startAddress + "\r\n");
				
		ptr = 5 * 2 + p * 2;
		let value = optoPort.txDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		let txPortWordsCount = startAddress + value;
		if (txWordsCount < txPortWordsCount)
		{
			txWordsCount = txPortWordsCount;
		}
				
		ptr = 10 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 15 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		let dataUID = 0;
		if (optoPort.isLinked() == true)
		{
			let linkedPort = optoPort.linkedPortID();
			let linkedOptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
			if (linkedOptoPort != null)
			{
				dataUID = linkedOptoPort.txDataID();
			}
		}

		ptr = 20 * 2 + p * 4;
		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
		{
			return -1;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
				
		if (optoPort.enableSerial() == true)
		{
			ptr = 30 * 2 + p * 2;
			value = optoPort.portID();	//???
			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX ID for RS-232/485 transmitter " + (p + 1), value) == false)
			{
				return -1;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX ID for RS-232/485 transmitter " + (p + 1) + " = " + value + "\r\n");
					
			ptr = 35 * 2 + p * 4;
			if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "RS-232/485 Data UID " + (p + 1), dataUID) == false)
			{
				return -1;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RS-232/485 Data UID " + (p + 1) + " = " + dataUID + "\r\n");
				
			//
			// RS232/485_CFG
			//
				
				
			let txStandard = Mode_RS232;	//0 - rs232, 1 - rs485
			if (optoPort.jsSerialMode() == Mode_RS485)
			{
				txStandard = Mode_RS485;
			}
			
			confFirmware.writeLog("    serialMode = " + txStandard + "\r\n");

			ptr = 45 * 2;
				
			//bits 9..0
			//
			let txEn = 1;	//1 - enabled
			let txMode = (txEn << 1) | txStandard;
			txMode <<= (p * 2);
				
			// this is only for OCM version 255 (ACNF)
			//
			// bits 14..10
			//
			let txDuplex = 0;	//1 - enabled
				
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
				
			let allModes = confFirmware.data16(frame, ptr);
			allModes |= txMode;
			if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "ModeFlags", allModes) == false)
			{
				return -1;
			}
		}
	} // p
	
	let ptr = 45 * 2;
	
	let allModes = confFirmware.data16(frame, ptr);
	confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RS mode configuration = " + allModes + "\r\n");

	// end of generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage);
	//
	
    ptr = 120;
    
    // crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, equipmentID, frame, 0, ptr, ptr);   //CRC-64
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
	
	let ocmFrameSize = 64;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 1;
    let dataFramesQuantity = 0;
	if (txWordsCount > 0)
	{
		dataFramesQuantity = Math.ceil(txWordsCount / ocmFrameSize);
	}
	confFirmware.writeLog("    txWordsCount = " + txWordsCount + "\r\n");
	
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