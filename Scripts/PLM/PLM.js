// Generate configuration for module PLM
//
//
function generate_plm(confFirmware, module, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	var actuatorType = module.propertyValue("ActuatorType");
	if (actuatorType == undefined) 
	{
		log.errCFG3000("ActuatorType", module.equipmentId);
		return false;
	}

	var linearSwitch = module.propertyValue("LinearSwitch");
	if (linearSwitch == undefined) 
	{
		log.errCFG3000("LinearSwitch", module.equipmentId);
		return false;
	}

	var voteMethod = module.propertyValue("VoteMethod");
	if (voteMethod == undefined) 
	{
		log.errCFG3000("VoteMethod", module.equipmentId);
		return false;
	}
	
  	let wordOfFlags = voteMethod | (linearSwitch << 4) | (actuatorType << 8);
  
    // Calculate CRC-4
    //
    let crc4 = 0x0;        // init

	{
    	const POLY = 0x3;     // x^4 + x + 1
	
	    // Process 16 data bits, MSB first
	    //
	    for (let i = 15; i >= 0; i--) 
	    {
	        const bit = (wordOfFlags >> i) & 1;
	        const c15 = (crc4 >> 3) & 1; // MSB of 4-bit CRC
	
	        crc4 = ((crc4 << 1) | bit) & 0xF;
	
	        if (c15) 
	        {
	            crc4 ^= POLY;
	        }
	    }
	
	    // Final CRC is 4 bits
	    //
	    crc4 &= 0xF;
    }
	
	// Add CRC4 to word of flags
	wordOfFlags |= (crc4 << 12);
	
    let ptr = 24;

	confFirmware.writeLog("[" + frame + ":" + ptr + "] WordOfFlags = 0x" + wordOfFlags.toString(16) + 
			" (VoteMethod = " + voteMethod +
			", LinearSwitch = " + linearSwitch +
			", ActuatorType = " + actuatorType +
			", crc4 = 0x" + crc4.toString(16) + ")\r\n");

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frame, ptr, "WordOfFlags", wordOfFlags) == false)      // WordOfFlags
	{
		return false;
	}
    ptr += 2;
        
        
     // crc
    ptr = 120;

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
    let dataFramesQuantity = 0;

    let txId = module.customModuleFamily + module.moduleVersion;
//    let txId = module.moduleFamily + module.moduleVersion;
    
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
