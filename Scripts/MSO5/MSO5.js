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

	// Build number

	let ptr = 0;

	let buildNo = confFirmware.buildNumber();
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "BuildNo", buildNo) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr +"]: BuildNo = " + buildNo + "\r\n");
	ptr += 2;

	ptr += 4 * 2; // Reserved

	// Opto

	const optoDataPtr = 5 * 2;

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
					
		ptr = optoDataPtr + p * 2;
		
		let startAddress = optoPort.txStartAddress();
		
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), startAddress) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + startAddress + "\r\n");
				
		ptr = optoDataPtr + 3 * 2 + p * 2;
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
				
		ptr = optoDataPtr + 6 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = optoDataPtr + 9 * 2 + p * 2;
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

		ptr = optoDataPtr + 12 * 2 + p * 4;
		if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");

	} // p
	
	const lanDataPtr = 23 * 2;

	ptr = lanDataPtr;
	
	let lanPortCount = module.propertyValue("LanPortCount");
	
	for (let p = 0; p < lanPortCount; p++)
	{
		let controllerID = module.equipmentId + "_ETHERNET0";
		controllerID = controllerID + (p + 1);

		confFirmware.writeLog("    LAN " + (p + 1) + ": Controller ID = " + controllerID + "\r\n");
	    
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

	// ------------------------------------------------------------ VOTE_CFG ---------------------------------------------------------

	let voteDataPtr = 45 * 2;

	confFirmware.writeLog("Writing VOTE_CFG Configuration\r\n");

	var txBusDataStartAddressArray = [];
	var rxBusDataStartAddressArray = [];
	
	var txModbusDataLengthArray = [];
	var rxModbusDataLengthArray = [];

	const LAN_TYPE_TX = 0;
	const LAN_TYPE_RX = 1;

	const maxVotePartCount = 48;

	for (let lanType = LAN_TYPE_TX; lanType <= LAN_TYPE_RX; lanType++)
	{
		let lanTypeName = lanType == LAN_TYPE_TX ? "Tx" : "Rx";

		let votePartCount = 0;
		const maxVotePartSizeW = 128;

		const MSO5Discrete = 1;
		const MSO5Float32 = 2;
		const MSO5SignedInt32 = 3;
		const MSO5UnsignedInt16 = 4;
		const MSO5SignedInt16 = 5;

		const SignalTypeAnalog = 0;
		const SignalTypeDiscrete = 1;
		const SignalTypeBus = 2;

		const DataFormatUnsignedInt = 0;
		const DataFormatSignedInt = 1;
		const DataFormatFloat = 2;

		const AnalogAppSignalFormatSignedInt = 1;
		const AnalogAppSignalFormatFloat32 = 2;

		// Version of protocol

		ptr = voteDataPtr;

		if (lanType == LAN_TYPE_TX) {
			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : VOTE_CFG_ProtocolVersion = " + 1 + "\r\n");

			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "ProtocolVersion", 1) == false) {
				return false;
			}
			ptr += 2;
		}

		// Quantity of records will be filled later

		const ptrQuantityOfRecords = ptr;

		ptr += 2;

		// Parse bus signals

		let busTypeId = module.propertyValue(lanTypeName + "BusTypeId");

		if (signalSet.busExists(busTypeId) == false)
		{
			log.writeError(module.equipmentId +  ", Bus " + busTypeId + " was not found!");
			return false;
		}

		confFirmware.writeLog("Parsing Bus " + busTypeId + "\r\n");

		let busSignalList = signalSet.getFlatBusSignalsList(busTypeId)

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
			}*/
			

			let partOffset = bs.OffsetW;
			let partDataSize = bs.SizeW;

			// Look for the last signal with the same parameters

			if (i < busSignalList.length - 1) {

				for (let j = i + 1; j < busSignalList.length; j++) {
					let bs2 = busSignalList[j];

					if (bs.SignalType == bs2.SignalType && 
						bs.AnalogFormat == bs2.AnalogFormat && 
						bs.DataFormat == bs2.DataFormat && 
						bs.SizeW == bs2.SizeW && 
						bs.BusTypeID == bs2.BusTypeID && 
						bs2.OffsetW <= partOffset + partDataSize &&
						partDataSize + bs2.SizeW <= maxVotePartSizeW) {

						/*
						if (bs2.SignalType == SignalTypeAnalog) {
							confFirmware.writeLog(bs2.SignalID + " JA OffsetW: " + bs2.OffsetW + "SizeW: " + bs2.SizeW + "\r\n");
						}
						else {
							confFirmware.writeLog(bs2.SignalID + " JD OffsetW: " + bs2.OffsetW + " SizeW: " + bs2.SizeW + "\r\n");
						}*/
						

						if (bs2.OffsetW != partOffset)	// Same offset can be for discrete signals, skip them
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

			votePartCount++;

			if (votePartCount > maxVotePartCount)
			{
				log.writeError(module.equipmentId +  ", TX Bus part count is more than 48 parts.");
				return false;
			}

			let partDataType = -1;
			let partDataTypeStr = "";

			if (bs.SignalType == SignalTypeDiscrete) {
				partDataType = MSO5Discrete;
				partDataTypeStr = "(discrete)";
			}
			else {
				if (bs.SignalType == SignalTypeAnalog) {
					switch (bs.AnalogFormat) {
						case AnalogAppSignalFormatFloat32:
							{
								partDataType = MSO5Float32;
								partDataTypeStr = "(float32)";
								break;
							}
						case AnalogAppSignalFormatSignedInt:
							{
								switch (bs.DataFormat) {
									case DataFormatSignedInt:
										{
											switch (bs.SizeBits) {
												case 16:
													{
														partDataType = MSO5SignedInt16;
														partDataTypeStr = "(int16)";
														break;
													}
													case 32:
													{
														partDataType = MSO5SignedInt32;
														partDataTypeStr = "(int32)";
														break;
													}
													default:
													log.errINT1001(module.equipmentId + ", Bus " + txBusTypeId + ", Signal " + bs.SignalID + " - wrong SizeBits (" + bs.SizeBits + ")");
													break;
											}
											break;
										}
									case DataFormatUnsignedInt:
										{
											partDataType = MSO5UnsignedInt16;
											partDataTypeStr = "(uint16)";
											break;
										}
									default:
										log.errINT1001(module.equipmentId + ", Bus " + txBusTypeId + ", Signal " + bs.SignalID + " - wrong DataFormat");
										break;
								}
								break;
							}
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

			if (lanType == LAN_TYPE_TX) {

				// Add part offset to array

				txBusDataStartAddressArray.push(partOffset);

				// Add part data size to array

				txModbusDataLengthArray.push(partDataSize);

				// Offset

				confFirmware.writeLog("    [" + frame + ":" + ptr + "] : TxVOTE_CFG Part " + votePartCount + " Offset = " + partOffset + "\r\n");

				if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "TxVOTE_CFG_" + votePartCount + "_Offset", partOffset) == false) {
					return false;
				}
				ptr += 2;

				// Data Size and Type

				let dataSizeType = (partDataSize << 8) + partDataType;

				confFirmware.writeLog("    [" + frame + ":" + ptr + "] : TxVOTE_CFG Part " + votePartCount + " DataSize = " + partDataSize + ", DataType = " + partDataType + " " + partDataTypeStr + ", BusTypeID = " + bs.BusTypeID + ", DatsSizeType = " + dataSizeType + "\r\n");

				if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "TxVOTE_CFG_" + votePartCount + "_DatsSizeType", dataSizeType) == false) {
					return false;
				}
				ptr += 2;
			}
			else
			{
				// Add part offset to array

				confFirmware.writeLog("    [--:--] : RxVOTE_CFG Part " + votePartCount + " Offset = " + partOffset + "\r\n");

				rxBusDataStartAddressArray.push(partOffset);

				// Add part data size to array

				rxModbusDataLengthArray.push(partDataSize);

				confFirmware.writeLog("    [--:--] : RxVOTE_CFG Part " + votePartCount + " DataSize = " + partDataSize + ", DataType = " + partDataType + " " + partDataTypeStr + ", BusTypeID = " + bs.BusTypeID + "\r\n");
			}
		}

		// PartCount
		if (lanType == LAN_TYPE_TX) {

			confFirmware.writeLog("    [" + frame + ":" + ptrQuantityOfRecords + "] : TxVOTE_CFG_QuantityOfRecords = " + votePartCount + "\r\n");

			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptrQuantityOfRecords, "VOTE_CFG_QuantityOfRecords", votePartCount) == false) {
				return false;
			}
			ptr += 2;
		}
		else{
			confFirmware.writeLog("    [--:--] : RxVOTE_CFG_QuantityOfRecords = " + votePartCount + "\r\n");
		}
	}//lanType

	// ------------------------------------------------------------ LAN_CFG ---------------------------------------------------------

	{
		const LAN_COUNT = 2;

		const niosDataPtr = 143 * 2;

		ptr = niosDataPtr;

		for (let lan = 1; lan <= LAN_COUNT; lan++)
		{

			confFirmware.writeLog("Writing LAN" + lan + " configuration\r\n");

			// ProtocolVersion

			confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN" + lan + "_CFG ProtocolVersion = 1 \r\n");

			if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LAN" + lan + "_CFG ProtocolVersion ", 1) == false) {
				return false;
			}
			ptr += 2;

			let txQuantityOfRecordsPtr = ptr;
			ptr += 2;

			let rxQuantityOfRecordsPtr = ptr;
			ptr += 2;

			ptr += 2; // Reserve 1

			ptr += 2; // Reserve 2
		
			for (let lanType = LAN_TYPE_TX; lanType <= LAN_TYPE_RX; lanType++)
			{
				let lanPartCount = 0;

				let lanTypeName = lanType == LAN_TYPE_TX ? "Tx" : "Rx";

				confFirmware.writeLog("Direction: " + lanTypeName + "\r\n");

				let addressData = module.propertyValue("LAN" + lan +  lanTypeName + "AddressData");

				var addressDataStrings = addressData.split("\n");

				// Count number of records

				for (let s = 0; s < addressDataStrings.length; s++)
				{
					var addressDataString = addressDataStrings[s].trim();
					if (addressDataString.length == 0)
					{
						continue;
					}
					
					lanPartCount++;
				}

				//QuantityOfRecords

				let quantityOfRecordsPtr = lanType == LAN_TYPE_TX ? txQuantityOfRecordsPtr : rxQuantityOfRecordsPtr;
				
				confFirmware.writeLog("    [" + frame + ":" + quantityOfRecordsPtr + "] : LAN" + lan + lanTypeName +"_CFG Quantity of records = " + lanPartCount + "\r\n");

				if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, quantityOfRecordsPtr, "LAN" + lan + lanTypeName + "_CFG_Quantity", lanPartCount) == false) {
					return false;
				}

				for (let s = 0; s < addressDataStrings.length; s++)
				{
					let addressDataString = addressDataStrings[s].trim();
					if (addressDataString.length == 0)
					{
						continue;
					}

					let addressValues = addressDataString.split(" ");

					if (addressValues.length != 2)
					{
						log.writeError(module.equipmentId +  ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "' has incorrect format!");
						continue;
					}

					// PART

					let partStr = addressValues[0].trim();
					if (partStr.search("^PART\\d+$") == -1)
					{
						log.writeError(module.equipmentId +  ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', field '"+ partStr + "' has incorrect format!");
						continue;
					}

					// ADDR

					let addrStr = addressValues[1].trim();
					if (addrStr.search("^ADDR\\d+$") == -1)
					{
						log.writeError(module.equipmentId +  ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', field '"+ addrStr + "' has incorrect format!");
						continue;
					}

					// Get part

					let part = Number.parseInt(partStr.substring(4));

					if (part < 1 || part > maxVotePartCount)
					{
						log.writeError(module.equipmentId +  ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', field PART '"+ part + "' has incorrect value (expected 1..48)!");	
						continue;
					}

					// Get Addresses for wAddr and rAddr and swap them for RX

					let rAddrPtr = ptr;
					ptr += 2;

					let wAddrPtr = ptr;
					ptr += 2;

					if (lanType == LAN_TYPE_RX)
					{
						let temp = wAddrPtr;
						wAddrPtr = rAddrPtr;
						rAddrPtr = temp;
					}
					
					// Write rAddr

					let rAddr = 0; 

					if (lanType == LAN_TYPE_TX)
					{
						// Take rAddr from txBusDataStartAddressArray

						if ((part - 1) > (txBusDataStartAddressArray.length - 1)) {
							log.writeError(module.equipmentId + ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', PART '" + part + "' part offset was not found!");
							continue;
						}

						rAddr = txBusDataStartAddressArray[part - 1];
					}
					else
					{
						// Take rAddr from rxBusDataStartAddressArray

						if ((part - 1) > (rxBusDataStartAddressArray.length - 1)) {
							log.writeError(module.equipmentId + ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', PART '" + part + "' part offset was not found!");
							continue;
						}

						rAddr = rxBusDataStartAddressArray[part - 1];
					}
					
					confFirmware.writeLog("    [" + frame + ":" + rAddrPtr + "] : LAN" + lan + lanTypeName + "_CFG Part " + part + " ReadSizeAddress = " + rAddr + "\r\n");

					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, rAddrPtr, "LAN" + lan + lanTypeName + "_CFG_Part_" + part + "_ReadSizeAddress", rAddr) == false) {
						return false;
					}
					
					// Write wAddr

					var wAddr = Number.parseInt(addrStr.substring(4));

					confFirmware.writeLog("    [" + frame + ":" + wAddrPtr + "] : LAN" + lan + "_CFG Part " + part + " WriteSizeAddress = " + wAddr + "\r\n");

					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, wAddrPtr, "LAN" + lan + lanTypeName + "_CFG_Part_" + part + "_WriteSizeAddress", wAddr) == false) {
						return false;
					}

					// Write DataSize

					let dataSize = 0;

					if (lanType == LAN_TYPE_TX) {
						if ((part - 1) > (txModbusDataLengthArray.length - 1)) {
							log.writeError(module.equipmentId + ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', PART '" + part + "' data size was not found!");
							continue;
						}

						dataSize = txModbusDataLengthArray[part - 1];
					}
					else
					{
						if ((part - 1) > (rxModbusDataLengthArray.length - 1)) {
							log.writeError(module.equipmentId + ", LAN" + lan + lanTypeName + "AddressData string '" + addressDataString + "', PART '" + part + "' data size was not found!");
							continue;
						}

						dataSize = rxModbusDataLengthArray[part - 1];
					}

					confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN" + lan + lanTypeName + "_CFG Part " + part + " DataSize = " + dataSize + "\r\n");

					if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "LAN" + lan + lanTypeName + "_CFG_Part_" + part + "_DataSize", dataSize) == false) {
						return false;
					}
					ptr += 2;

					if (ptr >= 868)
					{
						log.writeError(module.equipmentId +  ", LAN configuration is too large.");
						// Configuration is too large
						///////////
					}
				} // addressDataStrings
			} // lanType
		}// lan
	}

	//---------------------------------------- CRC ------------------------------------------------------

    ptr = 868;
    
    // crc
    let stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
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
	
	let ocmFrameSize = 64;
    
    let flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    let configFramesQuantity = 7;
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

