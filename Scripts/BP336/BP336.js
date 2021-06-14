// Generate configuration for module BP3-36
//

function generatebp336(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{   
	var ptr = 120;
	    
	// crc
	var stringCrc64 = storeCrc64(confFirmware, log, LMNumber, module.equipmentId, frame, 0, ptr, ptr);   //CRC-64
	if (stringCrc64 == "")
	{
		return false;
	}
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
	ptr += 8;    

	// reserved
	ptr += 880;
    
	// ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
	//
	var dataTransmittingEnableFlag = false;
	var dataReceiveEnableFlag = true;
    
	var flags = 0;
	if (dataTransmittingEnableFlag == true)
		flags |= 1;
	if (dataReceiveEnableFlag == true)
		flags |= 2;

   
	var configFramesQuantity = 1;
	var dataFramesQuantity = 0;

	let txId = module.moduleFamily + module.moduleVersion;
    
	if (generate_txRxIoConfig(confFirmware, module.equipmentId, LMNumber, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId) == false)
	{
		return false;
	}
	ptr += 8;

	return true; 
}
