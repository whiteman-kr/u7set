// Generate configuration for module MSO5
//
//

function generate_mso5(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	let checkProperties = ["OptoPortCount", "TxBusTypeId", "RxBusTypeId"];
	for (let cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}
	
	let optoPortCount = module.propertyValue("OptoPortCount");
	
	let txWordsCount = 0;
	
	for (let p = 0; p < optoPortCount; p++)
	{
		let controllerID = module.equipmentId + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);
	    
		let controllerObject = module.childByEquipmentId(controllerID);
		if (controllerObject == null || controllerObject.isController() == false)
		{
			log.errCFG3004(controllerID, module.equipmentId);
			return false;
		}
			
		let controller = controllerObject.toController();

		let optoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
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
					
		let ptr = 0 + p * 2;
		
		let startAddress = optoPort.txStartAddress();
		
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), startAddress) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + startAddress + "\r\n");
				
		ptr = 3 * 2 + p * 2;
		let value = optoPort.txDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		let txPortWordsCount = startAddress + value;
		if (txWordsCount < txPortWordsCount)
		{
			txWordsCount = txPortWordsCount;
		}
				
		ptr = 6 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 12 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
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

		ptr = 15 * 2 + p * 4;
		if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");

	} // p
	
	let ptr = 21 * 2;
	
	let lanPortCount = module.propertyValue("LanPortCount");
	
	for (let p = 0; p < lanPortCount; p++)
	{
		let controllerID = module.equipmentId + "_ETHERNET0";
		controllerID = controllerID + (p + 1);
	    
		let controllerObject = module.childByEquipmentId(controllerID);
		if (controllerObject == null || controllerObject.isController() == false)
		{
			log.errCFG3004(controllerID, module.equipmentId);
			return false;
		}
			
		let ethernetController = controllerObject.toController();
		
		let dataIP = ethernetController.propertyIP("DataIP");
		let serverIP = ethernetController.propertyIP("ServerIP");
		let serverPort = ethernetController.propertyValue("ServerPort");
		let modbusDeviceIndex = ethernetController.propertyValue("ModbusDeviceIndex");
		let modbusDeviceInput = ethernetController.propertyValue("ModbusDeviceInput");

		//mac
		//
		let hashName = "S" + dataIP + module.equipmentId + serverIP + serverPort;
		let hashList = confFirmware.calcHash64(hashName);
		let size = hashList.jsSize();
		if (size != 2) {
			log.writeError("Hash is not 2 32-bitwords in function generate_mso5!");
			return false;
		}

		let h0 = hashList.jsAt(0);
		let h1 = hashList.jsAt(1);

		let m1 = 0x4200;
		let m2 = h0 & 0x7fff;
		let m3 = (h0 >> 16) & 0x7fff;
	
		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of MSO5 = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
		if (setData16(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "MAC1", m1) == false) {
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "MAC2", m2) == false) {
			return false;
		}
		ptr += 2;
		if (setData16(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "MAC3", m3) == false) {
			return false;
		}	
		ptr += 2;

		// dataIP

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : DataIP = " + ipToString(dataIP) + "\r\n");
	
		if (setData32(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "DataIP", dataIP) == false) {
			return false;
		}
		ptr += 4;

		// serverIP

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : ServerIP = " + ipToString(serverIP) + "\r\n");
	
		if (setData32(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "ServerIP", serverIP) == false) {
			return false;
		}
		ptr += 4;

		// serverPort
	
		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : ServerPort = " + serverPort + "\r\n");
	
		if (setData16(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "ServerPort", serverPort) == false) {
			return false;
		}
		ptr += 2;

		// dataWordsCount (reserved)
		
		let dataWordsCount = 0;

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Data words count (reserved) = " + 0 + "\r\n");
		
		if (setData16(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "Data words count (reserved)", 0) == false) {
			return false;
		}
		ptr += 2;

		// modbusDeviceIndex

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : ModbusDeviceIndex = " + modbusDeviceIndex + "\r\n");
	
		if (setData32(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "ModbusDeviceIndex", modbusDeviceIndex) == false) {
			return false;
		}
		ptr += 2;

		// modbusDeviceInput

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : ModbusDeviceInput = " + modbusDeviceInput + "\r\n");
	
		if (setData32(confFirmware, log, LMNumber, ethernetController.equipmentId, frame, ptr, "ModbusDeviceInput", modbusDeviceInput) == false) {
			return false;
		}
		ptr += 2;

	} // p

	// VOTE_CFG
	{
		const MSO5Discrete = 1;
		const MSO5Float = 2;
		const MSO5SignedInt = 3;

		const SignalTypeAnalog = 0;
		const SignalTypeDiscrete = 1;
		const SignalTypeBus = 2;

		const AnalogAppSignalFormatSignedInt = 1;
		const AnalogAppSignalFormatFloat32 = 2;

		// Version of protocol

		ptr = 80;

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : VOTE_CFG_ProtocolVersion = " + 1 + "\r\n");

		if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "ProtocolVersion", 1) == false) {
			return false;
		}
		ptr += 2;

		// Quantity of records will be filled later

		const ptrQuantityOfRecords = ptr;

		ptr += 2;

		// Parse bus signals

		let txBusTypeId = module.propertyValue("TxBusTypeId");

		let busSignalList = signalSet.getFlatBusSignalsList(txBusTypeId)
		if (busSignalList == null) {
			log.writeError(module.equipmentId +  ", Bus " + txBusTypeId + " was not found!");
			return false;
		}

		let partCount = 0;

		let i = 0;

		while (i < busSignalList.length)
		{

			let bs = busSignalList[i];

			/*
			if (bs.SignalType == SignalTypeAnalog) {
				confFirmware.writeLog(bs.SignalID + " IA OffsetW: " + bs.OffsetW +  "SizeW: " + bs.SizeW + "\r\n");
			}
			else {
				confFirmware.writeLog(bs.SignalID + " ID OffsetW: " + bs.OffsetW + " SizeW: " + bs.SizeW + "\r\n");
			}
			*/

			let partOffset = bs.OffsetW;
			let partDataSize = bs.SizeW;

			// Look for the last signal with the same parameters

			if (i < busSignalList.length - 1) {

				for (let j = i + 1; j < busSignalList.length; j++) {
					let bs2 = busSignalList[j];

					if (bs.SignalType == bs2.SignalType && bs.AnalogFormat == bs2.AnalogFormat && bs.SizeW == bs2.SizeW && bs2.OffsetW <= partOffset + partDataSize) {

						/*
						if (bs2.SignalType == SignalTypeAnalog) {
							confFirmware.writeLog(bs2.SignalID + " JA OffsetW: " + bs2.OffsetW + "SizeW: " + bs2.SizeW + "\r\n");
						}
						else {
							confFirmware.writeLog(bs2.SignalID + " JD OffsetW: " + bs2.OffsetW + " SizeW: " + bs2.SizeW + "\r\n");
						}
						*/

						if (bs2.OffsetW != partOffset)
						{
							partDataSize += bs2.SizeW;
						}
						i = j + 1;
					}
					else {
						//confFirmware.writeLog("OTHER\r\n");
						i = j;
						break;
					}
				}
			}
			else
			{
				//confFirmware.writeLog("END\r\n");
				i++;	// This will end the loop
			}

			partCount++;

			let partDataType = -1;

			if (bs.SignalType == SignalTypeDiscrete) {
				partDataType = MSO5Discrete;
			}
			else {
				if (bs.SignalType == SignalTypeAnalog) {
					switch (bs.AnalogFormat) {
						case AnalogAppSignalFormatFloat32:
							{
								partDataType = MSO5Float;
							}
							break;
						case AnalogAppSignalFormatSignedInt:
							{
								partDataType = MSO5SignedInt;
							}
							break;
						default:
							log.errINT1001(module.equipmentId + ", Bus " + txBusTypeId + ", Signal " + bs.SignalID + " - unknown AnalogFormat");
							return false;
					}
				}
				else {
					log.errINT1001(module.equipmentId + ", Bus " + txBusTypeId + ", Signal " + bs.SignalID + " - unknown SignalType");
					return false;
				}
			}
			
			// Offset

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : VOTE_CFG Part " + partCount + " Offset = " + partOffset + "\r\n");

			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "VOTE_CFG " + partCount + " Offset", partOffset) == false) {
				return false;
			}
			ptr += 2;

			// Data Size and Type

			let dataSizeType = (partDataSize << 8) + partDataType;

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : VOTE_CFG Part " + partCount + " DataSize = " + partDataSize + ", DataType = " + partDataType + ", DatsSizeType = " + dataSizeType + "\r\n");

			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "VOTE_CFG " + partCount + " DatsSizeType", dataSizeType) == false) {
				return false;
				ptr += 2;
			}
		}

		// PartCount

		confFirmware.writeLog("    [" + frame + ":" + ptrQuantityOfRecords + "] : VOTE_CFG_QuantityOfRecords = " + partCount + "\r\n");

		if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptrQuantityOfRecords, "VOTE_CFG_QuantityOfRecords", partCount) == false) {
			return false;
		}
		ptr += 2;
	}

	//----------------------------------------

    ptr = 120;
    
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

