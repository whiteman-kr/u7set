// Generate configuration for module MVK-6
//
//
function generate_mvk6(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
    var ptr = 120;
	
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
    
    var flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    var configFramesQuantity = 1;
    var dataFramesQuantity = 1;

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
