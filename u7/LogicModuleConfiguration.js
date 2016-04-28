var RootType = 0;
var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var WorkstationType = 6;
var SoftwareType = 7;
var SignalType = 8;

var FamilyOTHER = 0x0000;
var FamilyLM = 0x0100;
var FamilyAIM = 0x0200;
var FamilyAOM = 0x0300;
var FamilyDIM = 0x0400;
var FamilyDOM = 0x0500;
var FamilyAIFM = 0x0600;
var FamilyOCM = 0x0700;

var Analog = 0;
var Discrete = 1;

var Input = 0;
var Output = 1;
var Validity = 2;
var Diagnostics = 3;

var aimTxId = 0x1200;
var aomTxId = 0x1300;
var dimTxId = 0x1400;
var domTxId = 0x1500;
var aifmTxId = 0x1600;
var ocmTxId = 0x1700;

var Mode_05V = 0;
var Mode_420mA = 1;
var Mode_10V = 2;
var Mode_05mA = 3;

var Mode_Optical = 0;
var Mode_Serial = 1;

var Mode_RS232 = 0;
var Mode_RS485 = 1;

//var configScriptVersion = 1;		// first logged version
//var configScriptVersion = 2;		// TuningDataSize in LM port has been changed to 716 (1432 / 2)
//var configScriptVersion = 3;		// AIM and AOM signal are now found not by place but by identifier, findSignalByPlace is not used.
//var configScriptVersion = 4;		// AIM filteringTime calculation algorithm has been changed
var configScriptVersion = 5;		// LM-1 properties SubsysID and Channel have been renamed to SubsystemID and SubsystemChannel

function(root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage)
{
    log.writeMessage("Start LogicModuleConfiguration");
    log.writeMessage("Configuration script version is " + configScriptVersion);

    var result = true;

    result = module_lm_1(root, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage);
    if (result == false)
    {
        return false;
    }

    result = module_lm_1_statistics(root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage);
    if (result == false)
    {
        return false;
    }

    log.writeMessage("Finish LogicModuleConfiguration");

    return result;
}

function setData8(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData8(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
        return false;
    }
}

function setData16(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData16(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
        return false;
    }
}

function setData32(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData32(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
        return false;
    }
}

function storeCrc64(confFirmware, log, frameIndex, start, count, offset)
{
	var result = confFirmware.storeCrc64(frameIndex, start, count, offset);
    if (result == "")
	{
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
    }
	return result;
}

function storeHash64(confFirmware, log, frameIndex, offset, data)
{
	var result = confFirmware.storeHash64(frameIndex, offset, data);
    if (result == "")
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
    }
    return result;
}

function module_lm_1(device, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage)
{
    if (device.jsDeviceType() == ModuleType)
    {
		if (device.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "LM-1");
			return false;
		}
		var checkProperties = ["ModuleFamily"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (device.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], device.propertyValue("EquipmentID"));
				return false;
			}
		}

        if (device.propertyValue("ModuleFamily") == FamilyLM)
        {
            log.writeMessage("Generating configuration for LM-1: " + device.propertyValue("EquipmentID"));

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1(child, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage) == false)
        {
            return false;
        }
    }

    return true;
}

function module_lm_1_statistics(device, confCollection, log, signalSet, subsystemStorage, opticModuleStorage)
{
    if (device.jsDeviceType() == ModuleType)
    {
		if (device.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "LM-1");
			return false;
		}
		var checkProperties = ["ModuleFamily"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (device.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], device.propertyValue("EquipmentID"));
				return false;
			}
		}
		
        if (device.propertyValue("ModuleFamily") == FamilyLM)
        {
            log.writeMessage("Generating statistics for LM-1: " + device.propertyValue("EquipmentID"));

            var checkProperties = ["SubsystemID", "SubsystemChannel", "ConfigFrameSize", "ConfigFrameCount"];
			for (var cp = 0; cp < checkProperties.length; cp++)
			{
				if (device.propertyValue(checkProperties[cp]) == undefined)
				{
					log.errCFG3000(checkProperties[cp], device.propertyValue("EquipmentID"));
					return false;
				}
			}

			// Generate Configuration
            //
			// Variables
			//
            var subSysID = device.propertyValue("SubsystemID");
            var channel = device.propertyValue("SubsystemChannel");
			
			var frameSize = device.jsPropertyInt("ConfigFrameSize");
			var frameCount = device.jsPropertyInt("ConfigFrameCount");			
			
			var uartId = 0x0102;                   // Check it !!!!
			
			var ssKeyValue = subsystemStorage.ssKey(subSysID);
			if (ssKeyValue == -1)
			{
				log.errCFG3001(subSysID, device.propertyValue("EquipmentID"));
				return false;
			}

			var configStartFrames = 2;
			var configFrameCount = 19;          // number of frames in each configuration

			var confFirmware = confCollection.jsGet("LM-1", subSysID, ssKeyValue, uartId, frameSize, frameCount);            
			
			var frameStorageConfig = 1;
			var ptr = 14;

			var channelCount = confFirmware.data16(frameStorageConfig, ptr);
			confFirmware.writeLog("---\r\n");
			confFirmware.writeLog("LM-1 for subsystem " + subSysID + ", Channel " + channel +": Frame " + frameStorageConfig + ", offset " + ptr +": ChannelCount = " + channelCount + "\r\n");
			return true;
		}

		return true;
	}

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1_statistics(child, confCollection, log, signalSet, subsystemStorage, opticModuleStorage) == false)
        {
            return false;
        }
    }

    return true;
}

// Generate configuration for module LM-1
// module - Hardware::DeviceModule (LM-1)
// confCollection - Hardware::ModuleConfCollection
//
//
function generate_lm_1_rev3(module, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "LM-1");
		return false;
	}
    var checkProperties = ["SubsystemID", "SubsystemChannel", "ConfigFrameSize", "ConfigFrameCount", "AppDataSize", "DiagDataSize"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

	// Variables
    //
    var subSysID = module.propertyValue("SubsystemID");
    var channel = module.propertyValue("SubsystemChannel");
    
    // Constants
    //
    var frameSize = module.jsPropertyInt("ConfigFrameSize");
    var frameCount = module.jsPropertyInt("ConfigFrameCount");
    
    if (frameSize < 1016)
    {
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.propertyValue("EquipmentID"));
        return false;
    }

	if (frameCount < 78 /*2 + 19  frames * 4 channels*/)
    {
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.propertyValue("EquipmentID"));
        return false;
    }
    
    var uartId = 0x0102; 
	
	var appWordsCount = module.jsPropertyInt("AppDataSize");
    var diagWordsCount = module.jsPropertyInt("DiagDataSize");
    
    var ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1)
    {
		log.errCFG3001(subSysID, module.propertyValue("EquipmentID"));
        return false;
    }

    var maxChannel = 4;                 // Can be changed!
    var configStartFrames = 2;
    var configFrameCount = 19;          // number of frames in each configuration
    var ioModulesMaxCount = 14;
    
    if (channel < 1 || channel > maxChannel)
    {
		log.errCFG3002("System/Channel", channel, 1, maxChannel, module.propertyValue("EquipmentID"));
        return false;
    }

    var confFirmware = confCollection.jsGet("LM-1", subSysID, ssKeyValue, uartId, frameSize, frameCount);
	
    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: LM-1\r\n");
	confFirmware.writeLog("EquipmentID = " + module.propertyValue("EquipmentID") + "\r\n");
	confFirmware.writeLog("Subsystem ID = " + subSysID+ "\r\n");
	confFirmware.writeLog("Key value = " + ssKeyValue+ "\r\n");
	confFirmware.writeLog("UartID = " + uartId+ "\r\n");
	confFirmware.writeLog("Frame size = " + frameSize+ "\r\n");
	confFirmware.writeLog("Channel = " + channel + "\r\n");

    // Configuration storage format
    //
    var frameStorageConfig = 1;
    var ptr = 0;
    
    setData16(confFirmware, log, frameStorageConfig, ptr, 0xca70);     //CFG_Marker
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
    ptr += 2;
    
    setData16(confFirmware, log, frameStorageConfig, ptr, 0x0001);     //CFG_Version
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
    ptr += 2;
    
    
    var ssKey = ssKeyValue << 6;             //0000SSKEYY000000b
    setData16(confFirmware, log, frameStorageConfig, ptr, ssKey);
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
    ptr += 2;
    
    // reserved
    ptr += 8;
    
    // write channelCount, if old value is less than current. If it is the same, output an error.
    //
    var oldChannelCount = confFirmware.data16(frameStorageConfig, ptr);
    
    if (oldChannelCount == channel)
    {
        log.errCFG3003(channel, module.propertyValue("EquipmentID"));
        return false;
    }
    
    if (oldChannelCount < channel)
    {
        setData16(confFirmware, log, frameStorageConfig, ptr, channel);

    }
    ptr += 2;
    
    var configIndexOffset = ptr + (channel - 1) * (2/*offset*/ + 4/*reserved*/);
    var configFrame = configStartFrames + configFrameCount * (channel - 1);
    
    setData16(confFirmware, log, frameStorageConfig, configIndexOffset, configFrame);
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

    // Service information
    //
	confFirmware.writeLog("Writing service information.\r\n");
	
    var frameServiceConfig = configFrame;
    ptr = 0;
    setData16(confFirmware, log, frameServiceConfig, ptr, 0x0001);   //CFG_Ch_Vers
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
    ptr += 2;
    setData16(confFirmware, log, frameServiceConfig, ptr, uartId);   //CFG_Ch_Dtype == UARTID?
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = "+ uartId + "\r\n");
    ptr += 2;
	var hashString = storeHash64(confFirmware, log, frameServiceConfig, ptr, subSysID);
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] subSysID HASH-64 = " + hashString + "\r\n");
    ptr += 8;
    
    // I/O Modules configuration
    //
	
	confFirmware.writeLog("Writing I/O modules configuration.\r\n");

    var frameIOConfig = configFrame + 1;

    var parent = module.jsParent();

    for (var i = 0; i < parent.childrenCount(); i++)
    {
        var ioModule = parent.jsChild(i);
		
		if (ioModule.jsDeviceType() != ModuleType)
		{
			continue;
		}
		
		if (ioModule.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "I/O_module");
			return false;
		}
		var checkProperties = ["ModuleFamily", "Place", "DiagDataSize"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (ioModule.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], ioModule.propertyValue("EquipmentID"));
				return false;
			}
		}
		
        if (ioModule.propertyValue("ModuleFamily") == FamilyAIM || ioModule.propertyValue("ModuleFamily") == FamilyAIFM || 
            ioModule.propertyValue("ModuleFamily") == FamilyAOM || ioModule.propertyValue("ModuleFamily") == FamilyOCM ||
            ioModule.propertyValue("ModuleFamily") == FamilyDIM || ioModule.propertyValue("ModuleFamily") == FamilyDOM)
        {
		
			var place = ioModule.propertyValue("Place");
		
            if (place < 1 || place > ioModulesMaxCount)
            {
				log.errCFG3002("Place", place, 1, ioModulesMaxCount, ioModule.propertyValue("EquipmentID"));
                return false;
            }
			
            var frame = frameIOConfig + place - 1;
			
			var result = true;
            
            if (ioModule.propertyValue("ModuleFamily") == FamilyAIM)
            {
                result = generate_aim(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyAIFM)
            {
                result = generate_aifm(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyAOM)
            {
                result = generate_aom(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyOCM)
            {
                result = generate_ocm(confFirmware, ioModule, frame, log, opticModuleStorage);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDIM)
            {
                result = generate_dim(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDOM)
            {
                result = generate_dom(confFirmware, ioModule, frame, log);
            }
			
			if (result == false)
			{
				return false;
			}
			
			var diagWordsIoCount = ioModule.propertyValue("DiagDataSize");
			diagWordsCount += diagWordsIoCount;
        }
    }
	
	var lanConfigFrame = frameIOConfig + 14;

	// Create LANs configuration
    //
	
	confFirmware.writeLog("Writing LAN configuration.\r\n");

    var lanFrame = lanConfigFrame;
	
	var lmID = module.propertyValue("EquipmentID");
	
	// Tuning
	//

	var ethernetcontrollerID = "_ETHERNET01";
	var ethernetController = module.jsFindChildObjectByMask(lmID + ethernetcontrollerID);
	if (ethernetController == null)
	{
		log.errCFG3004(lmID + ethernetcontrollerID, lmID);
		return false;
	}
	var checkTuningProperties = ["TuningServiceID", "TuningEnable", "TuningIP", "TuningPort"];
	for (var cp = 0; cp < checkTuningProperties.length; cp++)
	{
		if (ethernetController.propertyValue(checkTuningProperties[cp]) == undefined)
		{
			log.errCFG3000(checkTuningProperties[cp], ethernetController.propertyValue("EquipmentID"));
			return false;
		}
	}
	confFirmware.writeLog("    Ethernet Controller "  + lmID + ethernetcontrollerID + "\r\n");
	
	var serviceID = ethernetController.propertyValue("TuningServiceID");
	
	if (ethernetController.propertyValue("TuningEnable") == true && serviceID != "")
	{
		var service = root.jsFindChildObjectByMask(serviceID);
		if (service == null)
		{
			log.wrnCFG3008(serviceID, module.propertyValue("EquipmentID"));
		}
		else
		{
			var checkTuningProperties = ["TuningDataIP", "TuningDataPort"];
			for (var cp = 0; cp < checkTuningProperties.length; cp++)
			{
				if (service.propertyValue(checkTuningProperties[cp]) == undefined)
				{
					log.errCFG3000(checkTuningProperties[cp], service.propertyValue("EquipmentID"));
					return false;
				}
			}	
			
			var tuningWordsCount = 716;
	
			var tuningIP = ethernetController.jsPropertyIP("TuningIP");
			var tuningPort = ethernetController.propertyValue("TuningPort");
			
			var tuningServiceIP = service.jsPropertyIP("TuningDataIP");
			var tuningServicePort = service.propertyValue("TuningDataPort");
			
		
			generate_LANConfiguration(confFirmware, log, lanFrame, module, 
										tuningWordsCount, tuningIP, tuningPort, tuningServiceIP, tuningServicePort, 
										0, 0, 0, 0, 0);	//Subnet2 is not used
		}
	}
	lanFrame++;
								
	// REG / DIAG
	//

	
	
	for (var i = 0; i < 2; i++)
	{
	
		var ip = [0, 0];
		var port = [0, 0];
		
		var serviceIP = [0, 0];		//	Take from software!!!
		var servicePort = [0, 0];

		ethernetcontrollerID = "_ETHERNET0" + (i + 2);
		ethernetController = module.jsFindChildObjectByMask(lmID + ethernetcontrollerID);
		if (ethernetController == null)
		{
			log.errCFG3004(lmID + ethernetcontrollerID, lmID);
			return false;
		}
		var checkProperties = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort", 
			"DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (ethernetController.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], ethernetController.propertyValue("EquipmentID"));
				return false;
			}
		}
		confFirmware.writeLog("    Ethernet Controller "  + lmID + ethernetcontrollerID + "\r\n");
		
		var servicesName = ["App", "Diag"];
		
		for (var s = 0; s < 2; s++)
		{
			var serviceID = ethernetController.propertyValue(servicesName[s] + "DataServiceID");

			if (ethernetController.propertyValue(servicesName[s] + "DataEnable") == false || serviceID == "")
			{
				continue;
			}
		
			var service = root.jsFindChildObjectByMask(serviceID);
			if (service == null)
			{
				log.wrnCFG3008(serviceID, module.propertyValue("EquipmentID"));
				continue;
			}
				
			var serviceDataChannel = service.jsFindChildObjectByMask(serviceID + "_DATACH0" + (i + 1));
			if (serviceDataChannel == null)
			{
				log.errCFG3004(serviceID + "_DATACH01", lmID);
				return false;
			}

			var checkProperties = ["DataReceivingIP", "DataReceivingPort"];
			for (var cp = 0; cp < checkProperties.length; cp++)
			{
				if (serviceDataChannel.propertyValue(servicesName[s] + checkProperties[cp]) == undefined)
				{
					log.errCFG3000(servicesName[s] + checkProperties[cp], serviceDataChannel.propertyValue("EquipmentID"));
					return false;
				}
			}	
			
			ip[s] = ethernetController.jsPropertyIP(servicesName[s] + "DataIP");
			port[s] = ethernetController.propertyValue(servicesName[s] + "DataPort");
				
			serviceIP[s] = serviceDataChannel.jsPropertyIP(servicesName[s] + "DataReceivingIP");
			servicePort[s] = serviceDataChannel.propertyValue(servicesName[s] + "DataReceivingPort");
		}
			
		
		generate_LANConfiguration(confFirmware, log, lanFrame, module, 
									appWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], 
									diagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1]);
		lanFrame++;
	}
	
	// Create TX/RX configuration
	//
	
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	var txRxConfigFrame = lanConfigFrame + 3;
	
	var txWordsCount = generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, opticModuleStorage, false/*modeOCM*/);
	if (txWordsCount == -1)
	{
		return false;
	}
  
	return true;
}

function truncate_to_int(x)
{
    if(x > 0)
    {
         return Math.floor(x);
    }
    else
    {
         return Math.ceil(x);
    }
 }
 
 function valToADC(val, lowLimit, highLimit, lowADC, highADC)
{
	if ((highLimit - lowLimit) == 0)
	{
		return 0;		// to exclude division by zero
	}

	var res = (highADC-lowADC) * (val-lowLimit) / (highLimit - lowLimit) + lowADC;

    return Math.round(res);
}

// Generate configuration for module AIM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aim(confFirmware, module, frame, log, signalSet)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_AIM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for AIM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for AIM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AIMSignalMaxCount = 64;
	
	var tsConstant = 100 * 0.000001;
    
    var defaultTf = valToADC(5000 * 0.000001 / tsConstant, 0, 65535, 0, 0xffff);
    var defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    var defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    var defaultMaxDiff = valToADC(0.5, 0, 5.1, 0, 0xffff);
    
    var inController = module.jsFindChildObjectByMask(module.propertyValue("EquipmentID") + "_CTRLIN");
    if (inController == null)
    {
		log.errCFG3004(module.propertyValue("EquipmentID") + "_CTRLIN", module.propertyValue("EquipmentID"));
		return false;
    }
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //

	var channelAPlace = 0;
	var channelAMaxDifference = 0;

    for (var i = 0; i < AIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
        //var signal = findSignalByPlace(inController, i, Analog, Input, signalSet, log);
		var signalStrId = inController.propertyValue("EquipmentID") + "_IN";
		
		var entry = Math.floor(i / 2) + 1;
		if (entry < 10)
		{
			signalStrId = signalStrId + "0";
		}
		signalStrId = signalStrId + entry;
		if ((i % 2) == 0)
		{
			signalStrId = signalStrId + "A";
		}		
		else
		{
			signalStrId = signalStrId + "B";
		}
		
		var signal = signalSet.getSignalByEquipmentID(signalStrId);
         
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
			log.wrnCFG3007(signalStrId);
			
			confFirmware.writeLog("    in" + i + "[default]: [" + frame + ":" + ptr + "] Tf = " + defaultTf + 
			"; [" + frame + ":" + (ptr + 2) + "] HighADC = " + defaultHighBound +
			"; [" + frame + ":" + (ptr + 4) + "] LowADC = " + defaultLowBound +
			"; [" + frame + ":" + (ptr + 6) + "] MaxDiff = " + defaultMaxDiff + "\r\n");
            
            setData16(confFirmware, log, frame, ptr, defaultTf);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultHighBound);         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultLowBound);          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultMaxDiff);      // InA MaxDiff
            ptr += 2;
        }
        else
        {
		
			var tf = signal.filteringTime();
			
			if (tf < 1 * 100 * 0.000001 || tf > 65535 * 100 * 0.000001)
			{
				log.errCFG3010("FilteringTime", tf, 1 * 100 * 0.000001, 65535 * 100 * 0.000001, 6, signalStrId);
				return false;
			}
			
			tf = tf / tsConstant;
            
            var filternigTime = valToADC(tf, 0, 65535, 0, 0xffff);
            var maxDifference = valToADC(signal.maxDifference(), signal.lowLimit(), signal.highLimit(), signal.lowADC(), signal.highADC());

			if ((i & 1) == 0)
			{
				// this is A input
				channelAPlace = i;
				channelAMaxDifference = maxDifference;
			}
			else
			{
				if (i == channelAPlace + 1)
				{
					// this is B input, next to saved A
					if (maxDifference != channelAMaxDifference)
					{
						log.errCFG3009(channelAPlace, channelAMaxDifference, i, maxDifference, signal.propertyValue("EquipmentID"));
						return false;
					}
				}
			}

			confFirmware.writeLog("    in" + i + ": [" + frame + ":" + ptr + "] Tf = " + filternigTime + 
			"; [" + frame + ":" + (ptr + 2) + "] HighADC = " + signal.highADC() +
			"; [" + frame + ":" + (ptr + 4) + "] LowADC = " + signal.lowADC() +
			"; [" + frame + ":" + (ptr + 6) + "] MaxDiff = " + maxDifference + "\r\n");

            setData16(confFirmware, log, frame, ptr, filternigTime);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.highADC());         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.lowADC());          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, maxDifference);      // InA MaxDiff
            ptr += 2;
        }
    }

    // reserved
    ptr += 120;
   
    // final crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
    ptr += 8;
    
    //reserved
    ptr += 368;

    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    var dataTransmittingEnableFlag = false;
    var dataReceiveEnableFlag = true;
    
    var flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    var configFramesQuantity = 5;
    var dataFramesQuantity = 0;
    var txId = aimTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;
    
    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }
    
    return true;
}

// Generate configuration for module AIFM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aifm(confFirmware, module, frame, log)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_AIFM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for AIFM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for AIFM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
	
    var ptr = 0;
    
 
    var inController = module.jsFindChildObjectByMask(module.propertyValue("EquipmentID") + "_CTRLIN");
    if (inController == null)
    {
		log.errCFG3004(module.propertyValue("EquipmentID") + "_CTRLIN", module.propertyValue("EquipmentID"));
		return false;
    }
	
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
	
	var aifmChannelCount = 3;

	// CUR, FLU, FRQ
	//
	var koeffs = ["CUR", "FLU", "FRQ"];
	
	for (var k = 0; k < 3; k++)
	{
		for (var i = 0; i < aifmChannelCount; i++)
		{
			var signalID = inController.propertyValue("EquipmentID") + "_IN0" + (i + 1) + koeffs[k];
			var signal = inController.jsFindChildObjectByMask(signalID);
			var value = 1;
			if (signal == null)
			{
				log.wrnCFG3005(signalID, inController.propertyValue("EquipmentID"));
			}
			else
			{
				if (signal.propertyExists("PowerCoefficient") == true)
				{
					value = signal.propertyValue("PowerCoefficient");
				}
				else
				{
					log.errCFG3000("PowerCoefficient", signal.propertyValue("EquipmentID"));
					return false;
				}
			}
			
			var v = 0;
			var mantissa = 0;
			var exponent = 0;
			if (signal != null)
			{
				v = signal.valueToMantExp1616(value);
				mantissa =  v >> 16;
				exponent = v & 0xffff;
			}
				
			confFirmware.writeLog("    IN" + i + koeffs[k] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + " [" + mantissa + "^" + exponent + "]\r\n");
	
			setData16(confFirmware, log, frame, ptr, mantissa); 
			ptr += 2;
			setData16(confFirmware, log, frame, ptr, exponent); 
			ptr += 2;
		}
	}
	
	// POWER, REACT, PERIOD
	//
	var modeNames = ["PERIOD", "POWER", "REACT"];
	
	var setPointCount = 5;
	
	for (var k = 0; k < 3; k++)
	{
		for (var i = 0; i < aifmChannelCount; i++)
		{
			for (var m = 0; m < 3; m++)
			{
				for (var p = 0; p < setPointCount; p++)
				{
					var signalID = inController.propertyValue("EquipmentID") + "_IN0" + (i + 1) + koeffs[k] + modeNames[m];
					var signal = inController.jsFindChildObjectByMask(signalID);
					var value = 1;
					if (signal == null)
					{
						log.wrnCFG3005(signalID, inController.propertyValue("EquipmentID"));
					}
					else
					{
						var setPointName = "SetPoint0" + (p + 1);
						
						if (signal.propertyExists(setPointName) == true)
						{
							value = signal.propertyValue(setPointName);
						}
						else
						{
							log.errCFG3000(setPointName, signal.propertyValue("EquipmentID"));
							return false;
						}
					}
					
					var v = 0;
					var mantissa =  0;
					var exponent = 0;
					if (signal != null)
					{
						v = signal.valueToMantExp1616(value);
						mantissa =  v >> 16;
						exponent = v & 0xffff;
					}	
						
					confFirmware.writeLog("    IN" + (i + 1) + koeffs[k] + modeNames[m] + " : [" + frame + ":" + ptr + "] PowerCoefficient = " + value + " [" + mantissa + "^" + exponent + "]\r\n");
			
					setData16(confFirmware, log, frame, ptr, mantissa); 
					ptr += 2;
					setData16(confFirmware, log, frame, ptr, exponent); 
					ptr += 2;
				}
			}
			ptr += 2; //reserved
			ptr += 2; //reserved
		}
	}
	
	ptr = 632;
	
    // final crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
    ptr += 8;
    
    //reserved
    ptr += 368;

    // ------------------------------------------ TX/RX Config (8 bytes) ---------------------------------
    //
    var dataTransmittingEnableFlag = true;
    var dataReceiveEnableFlag = true;
    
    var flags = 0;
    if (dataTransmittingEnableFlag == true)
        flags |= 1;
    if (dataReceiveEnableFlag == true)
        flags |= 2;
    
    var configFramesQuantity = 5;
    var dataFramesQuantity = 1;
    var txId = aifmTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;
    
    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }
    
    return true;

}

// Generate configuration for module AOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aom(confFirmware, module, frame, log, signalSet)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_AOM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for AOM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for AOM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AOMWordCount = 4;                       // total words count
    var AOMSignalsInWordCount = 8;              // signals in a word count
    
    var outController = module.jsFindChildObjectByMask(module.propertyValue("EquipmentID") + "_CTRLOUT");
    if (outController == null)
    {
		log.errCFG3004(module.propertyValue("EquipmentID") + "_CTRLOUT", module.propertyValue("EquipmentID"));
		return false;
    }

    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
    var place = 0;
    
    for (var w = 0; w < AOMWordCount; w++)
    {
        var data = 0;
        
        for (var c = 0; c < AOMSignalsInWordCount; c++)
        {
            var mode = Mode_05V;    //default

            //var signal = findSignalByPlace(outController, place, Analog, Output, signalSet, log);
			var signalStrId = outController.propertyValue("EquipmentID") + "_OUT";
			
			var entry = place + 1;
			if (entry < 10)
			{
				signalStrId = signalStrId + "0";
			}
			signalStrId = signalStrId + entry;
		
			var signal = signalSet.getSignalByEquipmentID(signalStrId);

            if (signal == null || signal.propertyValue("EquipmentID") == undefined)
			{
				log.wrnCFG3007(signalStrId);
			}
			else
            {
				var outputRangeMode = signal.jsOutputRangeMode();
                if (outputRangeMode < 0 || outputRangeMode > Mode_05mA)
                {
					log.errCFG3002("Signal/OutputRangeMode", outputRangeMode, 0, Mode_05mA, signal.propertyValue("EquipmentID"));
					return false;
                }

                mode = outputRangeMode;
            }

            
            place++;
            
            var bit = c * 2;
            data |= (mode << bit);
        }
        
        setData16(confFirmware, log, frame, ptr + w * 2, data);          // InA Filtering time constant
		confFirmware.writeLog("    [" + frame + ":" + (ptr + w * 2) + "] data" + w + " = " + data + "\r\n");
    }
    
    ptr += 120;
    
    // crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
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
    var txId = aomTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }

    return true;

}

/*function findSignalByPlace(parent, place, type, func, signalSet, log)
{
    if (parent == null)
    {
        return null;
    }
	if (parent.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Class_Controller");
		return null;
	}

    for (var j = 0; j < parent.childrenCount(); j++)
    {
        var s = parent.jsChild(j);
                    
        if (s.jsDeviceType() != SignalType)
        {
            continue;
        }
        if (s.jsType() != type)
        {
            continue;
        }
        if (s.jsFunction() != func)
        {
            continue;
        }
		
		if (s.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "Class_Signal");
			return null;
		}

		var eqipmentID = s.propertyValue("EquipmentID");
		
        if (s.jsPlace() == place)
        {
            signal = signalSet.getSignalByEquipmentID(eqipmentID);
            if (signal == null)    
            {
				log.wrnCFG3007(eqipmentID);
            }
            return signal;
        }
    }
    
	log.wrnCFG3006(place, parent.propertyValue("EquipmentID"));
    return null;
}*/

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmware, module, frame, log, opticModuleStorage)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_OCM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for OCM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for OCM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
	
	var txRxConfigFrame = frame;
	
	var txWordsCount = generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, opticModuleStorage, true/*modeOCM*/);
	if (txWordsCount == -1)
	{
		return false;
	}
	
    var ptr = 120;
    
    // crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
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
	
    var txId = ocmTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }

    return true;

}

// Generate configuration for module DIM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_dim(confFirmware, module, frame, log)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_DIM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for DIM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for DIM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 120;
    
    // crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
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
    var txId = dimTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }

    return true;

}

// Generate configuration for module DOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_dom(confFirmware, module, frame, log)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Module_DOM");
		return false;
	}
	var checkProperties = ["Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    log.writeMessage("Generating configuration for DOM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("Generating configuration for DOM: " + module.propertyValue("EquipmentID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 120;
    
    // crc
    var stringCrc64 = storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
	confFirmware.writeLog("    [" + frame + ":" + ptr + "] crc64 = " + stringCrc64 + "\r\n");
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
    var txId = domTxId + module.propertyValue("ModuleVersion");
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        ptr = 1016;
    }

    return true;

}
function generate_txRxIoConfig(confFirmware, frame, offset, log, flags, configFrames, dataFrames, txId)
{
	// TxRx Block's configuration structure
    //
    var ptr = offset;

	confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] flags = "+ flags + 
	"; [" + frame + ":" + (ptr + 2) +"] configFrames = "+ configFrames +
	"; [" + frame + ":" + (ptr + 4) +"] dataFrames = "+ dataFrames +
	"; [" + frame + ":" + (ptr + 6) +"] txId = "+ txId + "\r\n");
    
    setData16(confFirmware, log, frame, ptr, flags);        // Flags word
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, configFrames); // Configuration words quantity
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, dataFrames);   // Data words quantity
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, txId);         // Tx ID
    ptr += 2;
    
    return true;
}

function generate_LANConfiguration(confFirmware, log, frame, module, 
									regWordsCount, regIP, regPort, regServiceIP, regServicePort,
									diagWordsCount, diagIP, diagPort, diagServiceIP, diagServicePort)
{
	var ptr = 0;		
	
	//mac
	//
	var hashName = "S" + regIP + diagIP + module.propertyValue("EquipmentID") + regServiceIP + diagServiceIP;
	var hashList = confFirmware.calcHash64(hashName);
	var size = hashList.jsSize();
	if (size != 2)
	{
		log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
		return;
	}

	var h0 = hashList.jsAt(0);
	var h1 = hashList.jsAt(1);
	
	var m1 = h0 & 0x7fff;
	var m2 = (h0 >> 16) & 0x7fff;
	var m3 = h1 & 0x7fff;
	
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : MAC address of LM = " + m1 + ":" + m2 + ":" + m3 + "\r\n");
	setData16(confFirmware, log, frame, ptr, m1); 
	ptr += 2;
	setData16(confFirmware, log, frame, ptr, m2); 
	ptr += 2;
	setData16(confFirmware, log, frame, ptr, m3); 
	ptr += 2;
	
	// regIP

	var ip0 = (regIP >> 24) & 0xff;
	var ip1 = (regIP >> 16) & 0xff;
	var ip2 = (regIP >> 8) & 0xff;
	var ip3 = (regIP) & 0xff;
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP 1 = " + ip0 + "." + ip1 + "." + ip2 + "." + ip3 + "\r\n");
	
	setData32(confFirmware, log, frame, ptr, regIP); 
	ptr += 4;

	// regPort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 = " + regPort + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, regPort); 
	ptr += 2;
	
	// diagIP

	ip0 = (diagIP >> 24) & 0xff;
	ip1 = (diagIP >> 16) & 0xff;
	ip2 = (diagIP >> 8) & 0xff;
	ip3 = (diagIP) & 0xff;
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP 2 = " + ip0 + "." + ip1 + "." + ip2 + "." + ip3 + "\r\n");
	
	setData32(confFirmware, log, frame, ptr, diagIP); 
	ptr += 4;

	// diagPort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 = " + diagPort + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, regPort); 
	ptr += 2;
		
	// regServiceIP

	ip0 = (regServiceIP >> 24) & 0xff;
	ip1 = (regServiceIP >> 16) & 0xff;
	ip2 = (regServiceIP >> 8) & 0xff;
	ip3 = (regServiceIP) & 0xff;
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service IP 1 = " + ip0 + "." + ip1 + "." + ip2 + "." + ip3 + "\r\n");
	
	setData32(confFirmware, log, frame, ptr, regServiceIP); 
	ptr += 4;

	// regServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service Port 1 = " + regServicePort + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, regServicePort); 
	ptr += 2;

	// regWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 words count = " + regWordsCount + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, regWordsCount); 
	ptr += 2;

	// diagServiceIP

	ip0 = (diagServiceIP >> 24) & 0xff;
	ip1 = (diagServiceIP >> 16) & 0xff;
	ip2 = (diagServiceIP >> 8) & 0xff;
	ip3 = (diagServiceIP) & 0xff;
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service IP 2 = " + ip0 + "." + ip1 + "." + ip2 + "." + ip3 + "\r\n");
	
	setData32(confFirmware, log, frame, ptr, diagServiceIP); 
	ptr += 4;

	// diagServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service Port 2 = " + diagServicePort + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, diagServicePort); 
	ptr += 2;

	// diagWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 words count = " + diagWordsCount + "\r\n");
	
	setData16(confFirmware, log, frame, ptr, diagWordsCount); 
	ptr += 2;
	
	// appDUID
	
	var appDataID = 0;
	
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 DUID = " + appDataID + "\r\n");
	
	setData32(confFirmware, log, frame, ptr, appDataID); 
	ptr += 4;
	
	// diagDUID

	var diagDataID = 0;
	
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 DUID = " + diagDataID + "\r\n");

	setData16(confFirmware, log, frame, ptr, diagDataID); 
	ptr += 4;

	return true;
}

// function returns the amount of transmitting words
//
function generate_txRxOptoConfiguration(confFirmware, log, frame, module, opticModuleStorage, modeOCM)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Class_Module");
		return -1;
	}
	if (module.propertyValue("OptoPortCount") == undefined)
	{
		log.errCFG3000("OptoPortCount", module.propertyValue("EquipmentID"));
		return -1;
	}
	
	var portCount = module.propertyValue("OptoPortCount");
	
	var txWordsCount = 0;
	
	for (var p = 0; p < portCount; p++)
	{
		var controllerID = module.propertyValue("EquipmentID") + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);
	    
		var controller = module.jsFindChildObjectByMask(controllerID);
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

		var optoPort = opticModuleStorage.jsGetOptoPort(controller.propertyValue("EquipmentID"));
		if (optoPort == null)
		{
			continue;
		}
		
		if (optoPort.connectionCaption() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0)
		{
			continue;
		}			
		
		if (optoPort.mode() == Mode_Optical)
		{
			confFirmware.writeLog("    OptoPort " + controller.propertyValue("EquipmentID") + ": Opto connection ID = " + optoPort.strID() + 
				" (" + optoPort.connectionCaption() + ")\r\n");
		}
		else
		{
			confFirmware.writeLog("    OptoPort " + controller.propertyValue("EquipmentID") + ": RS connection ID = " + optoPort.strID() + 
				" (" + optoPort.connectionCaption() + ")\r\n");
		}
					
		var ptr = 0 + p * 2;
		var value = optoPort.txStartAddress();
		setData16(confFirmware, log, frame, ptr, value); 
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 5 * 2 + p * 2;
		value = optoPort.txDataSizeW();
		setData16(confFirmware, log, frame, ptr, value); 
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
		txWordsCount += value;
				
		ptr = 10 * 2 + p * 2;
		value = optoPort.portID();
		setData16(confFirmware, log, frame, ptr, value); 
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 15 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		setData16(confFirmware, log, frame, ptr, value); 
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 20 * 2 + p * 4;
		value = optoPort.txDataID();
		setData32(confFirmware, log, frame, ptr, value); 
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + value + "\r\n");
				
		if (modeOCM == true && optoPort.mode() == Mode_Serial)
		{
			ptr = 30 * 2 + p * 2;
			value = optoPort.portID();	//???
			setData16(confFirmware, log, frame, ptr, value); 
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX ID for RS-232/485 transmitter " + (p + 1) + " = " + value + "\r\n");
					
			ptr = 35 * 2 + p * 4;
			value = optoPort.txDataID();	//???
			setData32(confFirmware, log, frame, ptr, value); 
			confFirmware.writeLog("    [" + frame + ":" + ptr +"]: RS-232/485 Data UID " + (p + 1) + " = " + value + "\r\n");
				
			//
			// RS232/485_CFG
			//
				
			var txEn = 0;	//1 - enabled
			if (optoPort.enable() == true)
			{
				txEn = 1;
			}
			confFirmware.writeLog("    enabled = " + txEn + "\r\n");
				
			var txStandard = Mode_RS232;	//0 - rs232, 1 - rs485
			if (optoPort.serialMode() == Mode_RS485)
			{
				txStandard = Mode_RS485;
			}
			
			confFirmware.writeLog("    serialMode = " + txStandard + "\r\n");

			ptr = 45 * 2;
				
			//bits 9..0
			//
			var txMode = (txEn << 1) | txStandard;
			txMode <<= (p * 2);
				
			if (module.propertyValue("ModuleVersion") == 255)	// this is only for OCM version 255 (ACNF)
			{
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
			}
				
				
			var allModes = confFirmware.data16(frame, ptr);
			allModes |= txMode;
			setData16(confFirmware, log, frame, ptr, allModes);
		}
	} // p
	
	if (modeOCM == true)
	{
		var ptr = 45 * 2;
		var allModes = confFirmware.data16(frame, ptr);
		confFirmware.writeLog("    [" + frame + ":" + ptr +"] : RS mode configuration = " + allModes + "\r\n");
	}
	
	return txWordsCount;
}
