// Non-strict mode part
//

function runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage)
{
	//var funcStr = "(function (confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage){log.writeMessage(\"Hello\"); return true; })";
	//
	var funcStr = "(" + configScript + ")";
	var funcVar = eval(funcStr);
	if (funcVar(confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false)
	{
		return false;
	}
}

// Strict mode part
//

"use strict";
  
var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var WorkstationType = 6;
var SoftwareType = 7;
var SignalType = 8;

var FamilyLMID = 	0x1100;

//var configScriptVersion = 1;		// first logged version
//var configScriptVersion = 2;		// TuningDataSize in LM port has been changed to 716 (1432 / 2)
//var configScriptVersion = 3;		// AIM and AOM signal are now found not by place but by identifier, findSignalByPlace is not used.
//var configScriptVersion = 4;		// AIM filteringTime calculation algorithm has been changed
//var configScriptVersion = 5;		// LM-1 properties SubsysID and Channel have been renamed to SubsystemID and SubsystemChannel
//var configScriptVersion = 6;		// SubsystemChannel renamed to LMNumber
//var configScriptVersion = 7;		// MAC address calculation changed
//var configScriptVersion = 8;		// IP address of LAN controller is written even service is not specified
//var configScriptVersion = 9;		// IP address and port of LAN controller can't be zero - an error is reported
//var configScriptVersion = 10;		// AIM signals parameters algorithm has been changed
//var configScriptVersion = 11;		// If software for LM ethernet controller is not found, default values are used
//var configScriptVersion = 12;		// DiagDataSize changed to TxDiagDataSize
//var configScriptVersion = 13;		// AppDataSize changed to AppLANDataSize
//var configScriptVersion = 14;		// connectionCaption=>connectionID; strID=>equipmentID
//var configScriptVersion = 15;		// added text description fields for every value
//var configScriptVersion = 16;		// added dynamic custom module family
//var configScriptVersion = 17;		// LMNumber limit is 12
//var configScriptVersion = 18;		// i/o modules scripts moved to ConfiurationScript in presets
//var configScriptVersion = 19;		// i/o modules family checking has been removed
//var configScriptVersion = 21;		// DataUID for LM modules is taken from linked port
//var configScriptVersion = 22;		// added builder parameter to interrupt build
//var configScriptVersion = 23;		// added OverrideApp(Diag)DataWordCount properties processing
//var configScriptVersion = 25;		// buildThread has been replaced by builder, added buildNo
//var configScriptVersion = 26;		// added UniqueID computing
var configScriptVersion = 27;		// First script that supports subsystems filtering


function main(builder, root, logicModules, confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription)
{
	if (logicModules.length != 0)
	{
		var subSysID = logicModules[0].propertyValue("SubsystemID");
		log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
	}

	for (var i = 0; i < logicModules.length; i++)
	{
		var result = module_lm_1(builder, root, logicModules[i], confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
		if (result == false)
		{
			return false;
		}
		
		if (builder.jsIsInterruptRequested() == true)
		{
			return true;
		}

	}
	
	for (var i = 0; i < logicModules.length; i++)
	{
		var result = module_lm_1_statistics(builder, logicModules[i], confCollection, log, subsystemStorage, logicModuleDescription);
		if (result == false)
		{
			return false;
		}

		if (builder.jsIsInterruptRequested() == true)
		{
			return true;
		}
	}

	return true;
}

function setData8(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data)
{
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + data.toString(16));

    if (confFirmware.setData8(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
        return false;
    }
}

function setData16(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data)
{
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + data.toString(16));

    if (confFirmware.setData16(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
        return false;
    }
}

function setData32(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data)
{
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + data.toString(16));
	
    if (confFirmware.setData32(frameIndex, offset, data) == false)
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
        return false;
    }
}

function storeCrc64(confFirmware, log, channel, equpmentID, frameIndex, start, count, offset)
{
	var result = confFirmware.storeCrc64(frameIndex, start, count, offset);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);
	
    if (result == "")
	{
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
    }
	return result;
}

function storeHash64(confFirmware, log, channel, equpmentID, frameIndex, offset, caption, data)
{
	var result = confFirmware.storeHash64(frameIndex, offset, data);
	
	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);
	
    if (result == "")
    {
        log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
    }
    return result;
}

function ipToString(ip)
{
	var ip0 = (ip >> 24) & 0xff;
	var ip1 = (ip >> 16) & 0xff;
	var ip2 = (ip >> 8) & 0xff;
	var ip3 = (ip) & 0xff;
	var result = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
	return result;
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

function module_lm_1(builder, root, module, confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription)
{
    if (module.jsDeviceType() != ModuleType)
	{
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "LM-1");
		return false;
	}
		
	var checkProperties = ["ModuleFamily"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}

    if (module.propertyValue("ModuleFamily") == FamilyLMID)
    {
		// Generate Configuration
		//
		return generate_lm_1_rev3(builder, module, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
    }
        
	return false;
}

function module_lm_1_statistics(builder, module, confCollection, log, subsystemStorage, logicModuleDescription)
{
    if (module.jsDeviceType() != ModuleType)
	{
		return false;
	}

	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "LM-1");
		return false;
	}

	var checkProperties = ["ModuleFamily", "SubsystemID"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
			return false;
		}
	}
		
    if (module.propertyValue("ModuleFamily") == FamilyLMID)
    {
		
	   var checkProperties = ["LMNumber"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (module.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], module.propertyValue("EquipmentID"));
				return false;
			}
		}

		// Generate Configuration
		//
		// Variables
		//
		var subSysID = module.propertyValue("SubsystemID");
		var LMNumber = module.propertyValue("LMNumber");
		
		var frameSize = logicModuleDescription.FlashMemory_ConfigFrameSize;
		var frameCount = logicModuleDescription.FlashMemory_ConfigFrameCount;
		
		var uartId = 0x0102;                   // Check it !!!!
		
		var ssKeyValue = subsystemStorage.ssKey(subSysID);
		if (ssKeyValue == -1)
		{
			log.errCFG3001(subSysID, module.propertyValue("EquipmentID"));
			return false;
		}

		var configStartFrames = 2;
		var configFrameCount = 19;          // number of frames in each configuration

		var confFirmware = confCollection.jsGet("LM-1", subSysID, ssKeyValue, uartId, frameSize, frameCount);            
		
		var frameStorageConfig = 1;
		var ptr = 14;

		var LMNumberCount = confFirmware.data16(frameStorageConfig, ptr);
		confFirmware.writeLog("---\r\n");
		confFirmware.writeLog("LM-1 for subsystem " + subSysID + ", LMNumber " + LMNumber +": Frame " + frameStorageConfig + ", offset " + ptr +": LMNumberCount = " + LMNumberCount + "\r\n");
		return true;
	}

	return false;
}


function Config_SYSTEMID_RACKID_CH00_MD02(log)
{
	log.writeMessage("Hello world!!!");
}

// Generate configuration for module LM-1
//
//
function generate_lm_1_rev3(builder, module, root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "LM-1");
		return false;
	}
    var checkProperties = ["SubsystemID", "LMNumber", "AppLANDataSize", "TuningLANDataUID",	"AppLANDataUID", "DiagLANDataUID"];//, "TxDiagDataSize", ];
	
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
    var LMNumber = module.propertyValue("LMNumber");
    
    // Constants
    //
	
    var frameSize = logicModuleDescription.FlashMemory_ConfigFrameSize;
    var frameCount = logicModuleDescription.FlashMemory_ConfigFrameCount;
    
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
	
	var appWordsCount = module.jsPropertyInt("AppLANDataSize");
    var diagWordsCount = logicModuleDescription.Memory_TxDiagDataSize;
    
    var ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1)
    {
		log.errCFG3001(subSysID, equipmentID);
        return false;
    }

    var maxLMNumber = 12;               // Can be changed!
    var configStartFrames = 2;
    var configFrameCount = 19;          // number of frames in each configuration
    var ioModulesMaxCount = 14;
    
    if (LMNumber < 1 || LMNumber > maxLMNumber)
    {
		log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.propertyValue("EquipmentID"));
        return false;
    }

    var confFirmware = confCollection.jsGet("LM-1", subSysID, ssKeyValue, uartId, frameSize, frameCount);
	
	confFirmware.jsSetDescriptionFields("EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");
	
	var equipmentID = module.propertyValue("EquipmentID");

    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: LM-1\r\n");
	confFirmware.writeLog("EquipmentID = " + equipmentID + "\r\n");
	confFirmware.writeLog("Subsystem ID = " + subSysID+ "\r\n");
	confFirmware.writeLog("Key value = " + ssKeyValue+ "\r\n");
	confFirmware.writeLog("UartID = " + uartId+ "\r\n");
	confFirmware.writeLog("Frame size = " + frameSize+ "\r\n");
	confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");

    // Configuration storage format
    //
    var frameStorageConfig = 1;
    var ptr = 0;
    
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Marker", 0xca70) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
    ptr += 2;
    
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "Version", 0x0001) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
    ptr += 2;
    
    
    var ssKey = ssKeyValue << 6;             //0000SSKEYY000000b
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
    ptr += 2;
    
	var buildNo = builder.jsBuildNo();
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "BuildNo", buildNo) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
    ptr += 2;

    // reserved
    ptr += 6;
    
    // write LMNumberCount, if old value is less than current. If it is the same, output an error.
    //
    var oldLMNumberCount = confFirmware.data16(frameStorageConfig, ptr);
    
    if (oldLMNumberCount == LMNumber)
    {
        log.errCFG3003(LMNumber, module.propertyValue("EquipmentID"));
        return false;
    }
    
    if (oldLMNumberCount < LMNumber)
    {
        if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, ptr, "LMNumberCount", LMNumber) == false)
		{
			return false;
		}
    }
    ptr += 2;
    
    var configIndexOffset = ptr + (LMNumber - 1) * (2/*offset*/ + 4/*reserved*/);
    var configFrame = configStartFrames + configFrameCount * (LMNumber - 1);
    
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false)
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

    // Service information
    //
	confFirmware.writeLog("Writing service information.\r\n");
	
    var frameServiceConfig = configFrame;
    ptr = 0;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false)   //CFG_Ch_Vers
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
    ptr += 2;
    
	if (setData16(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "UartID", uartId) == false)  //CFG_Ch_Dtype == UARTID?
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = "+ uartId + "\r\n");
    ptr += 2;
	
	//var hashString = storeHash64(confFirmware, log, LMNumber, equipmentID, frameServiceConfig, ptr, "SubSystemID Hash", subSysID);
	//if (hashString == "")
	//{
	//		return false;
	//}
	//confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] subSysID HASH-64 = 0x" + hashString + "\r\n");
	//Hash (UniqueID) will be counted later
    ptr += 8;
    
    // I/O Modules configuration
    //
	
	confFirmware.writeLog("Writing I/O modules configuration.\r\n");

    var frameIOConfig = configFrame + 1;

    var parent = module.jsParent();

    for (var i = 0; i < parent.childrenCount(); i++)
    {
		if (builder.jsIsInterruptRequested() == true)
		{
			return true;
		}

        var ioModule = parent.jsChild(i);
		
		if (ioModule.jsDeviceType() != ModuleType)
		{
			continue;
		}
		
		var place = ioModule.propertyValue("Place");
		if (place < 1 || place > ioModulesMaxCount)
        {
			continue;
        }

		var ioEquipmentID = ioModule.propertyValue("EquipmentID");
		if (ioEquipmentID == undefined)
		{
			log.errCFG3000("EquipmentID", "I/O_module");
			return false;
		}

		var checkProperties = ["ModuleFamily", "ModuleVersion", "Place", "PresetName", "ConfigurationScript"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (ioModule.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], ioEquipmentID);
				return false;
			}
		}
		
		var ioModuleFamily = ioModule.propertyValue("ModuleFamily");
			
	        var frame = frameIOConfig + place - 1;
				
		confFirmware.writeLog("Generating configuration for " + ioModule.propertyValue("Caption") + ": " + ioEquipmentID + " Place: " + ioModule.propertyValue("Place") + " Frame: " + frame + "\r\n");

		var configScript = ioModule.propertyValue("ConfigurationScript");
		if (configScript != "")
		{
			if (runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false)
			{
				return false;
			}
		}
			
		var diagWordsIoCount = ioModule.propertyValue("TxDiagDataSize");
		if (diagWordsIoCount == null)
		{
			log.errCFG3000("TxDiagDataSize", ioEquipmentID);
			return false;
		}
			
		diagWordsCount += diagWordsIoCount;
    }
	
	var lanConfigFrame = frameIOConfig + ioModulesMaxCount;

	// Create LANs configuration
    //
	
	confFirmware.writeLog("Writing LAN configuration.\r\n");

    var lanFrame = lanConfigFrame;
	
	// Tuning
	//

	var ethernetcontrollerID = "_ETHERNET01";
	var ethernetController = module.jsFindChildObjectByMask(equipmentID + ethernetcontrollerID);
	if (ethernetController == null)
	{
		log.errCFG3004(equipmentID + ethernetcontrollerID, equipmentID);
		return false;
	}
	var checkTuningProperties = ["TuningServiceID", "TuningEnable", "TuningIP", "TuningPort", "OverrideTuningDataWordCount"];
	for (var cp = 0; cp < checkTuningProperties.length; cp++)
	{
		if (ethernetController.propertyValue(checkTuningProperties[cp]) == undefined)
		{
			log.errCFG3000(checkTuningProperties[cp], ethernetController.propertyValue("EquipmentID"));
			return false;
		}
	}
	confFirmware.writeLog("    Ethernet Controller "  + equipmentID + ethernetcontrollerID + "\r\n");
	
	// Controller
	
	var tuningWordsCount = 716;

	var tuningIP = ethernetController.jsPropertyIP("TuningIP");
	var tuningPort = ethernetController.propertyValue("TuningPort");
	
	if (tuningIP == 0)
	{
		log.errCFG3011("TuningIP", tuningIP, ethernetController.propertyValue("EquipmentID"));
		return false;
	}
	if (tuningPort == 0)
	{
		log.errCFG3012("TuningPort", tuningPort, ethernetController.propertyValue("EquipmentID"));
		return false;
	}
	
	// Service
		
	var tuningServiceIP = 0;
	var tuningServicePort = 0;

	var serviceID = ethernetController.propertyValue("TuningServiceID");

	if (ethernetController.propertyValue("TuningEnable") == true)
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
			
			tuningServiceIP = service.jsPropertyIP("TuningDataIP");
			tuningServicePort = service.propertyValue("TuningDataPort");
		}
	}
	
	var controllerTuningWordsCount = tuningWordsCount;
	
	var tuningDataID = module.propertyValue("TuningLANDataUID");
	
	var overrideTuningWordsCount =	ethernetController.propertyValue("OverrideTuningDataWordCount");
	if (overrideTuningWordsCount != -1)
	{
		controllerTuningWordsCount = overrideTuningWordsCount;
		tuningDataID = 0;
	}

	if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController,
							controllerTuningWordsCount, tuningIP, tuningPort, tuningServiceIP, tuningServicePort, tuningDataID,
							0, 0, 0, 0, 0, 0) == false)	//Subnet2 is not used
							{
								return false;
							}								
	lanFrame++;
								
	// REG / DIAG
	//

	
	
	for (var i = 0; i < 2; i++)
	{
	
		var ip = [0, 0];
		var port = [0, 0];
		
		var serviceIP = [0, 0];		
		var servicePort = [0, 0];


		ethernetcontrollerID = "_ETHERNET0" + (i + 2);
		ethernetController = module.jsFindChildObjectByMask(equipmentID + ethernetcontrollerID);
		if (ethernetController == null)
		{
			log.errCFG3004(equipmentID + ethernetcontrollerID, equipmentID);
			return false;
		}
		var checkProperties = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort", 
			"DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
			"OverrideAppDataWordCount", "OverrideDiagDataWordCount"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (ethernetController.propertyValue(checkProperties[cp]) == undefined)
			{
				log.errCFG3000(checkProperties[cp], ethernetController.propertyValue("EquipmentID"));
				return false;
			}
		}
		confFirmware.writeLog("    Ethernet Controller "  + equipmentID + ethernetcontrollerID + "\r\n");
		
		var servicesName = ["App", "Diag"];
		
		for (var s = 0; s < 2; s++)
		{
			// Controller
		
			ip[s] = ethernetController.jsPropertyIP(servicesName[s] + "DataIP");
			port[s] = ethernetController.propertyValue(servicesName[s] + "DataPort");
			
			if (ip[s] == 0)
			{
				log.errCFG3011(servicesName[s] + "DataIP", ip[s], ethernetController.propertyValue("EquipmentID"));
				return false;
			}
			if (port[s] == 0)
			{
				log.errCFG3012(servicesName[s] + "DataPort", port[s], ethernetController.propertyValue("EquipmentID"));
				return false;
			}
			
			// Service
			var serviceID = ethernetController.propertyValue(servicesName[s] + "DataServiceID");

			if (ethernetController.propertyValue(servicesName[s] + "DataEnable") == true)
			{
				
				var service = root.jsFindChildObjectByMask(serviceID);
				if (service == null)
				{
		
					log.wrnCFG3008(serviceID, module.propertyValue("EquipmentID"));

					if (i == 0)	// in Ethernet port 1, if service was not found, use default IP addresses
					{
						if (s == 0)	// this is App
						{
							serviceIP[s] = 0xc0a80bfe;	//	192.168.11.254
							servicePort[s] = 13322;
						}

						if (s == 1)	// this is Diag
						{
							serviceIP[s] = 0xc0a815fe;	//	192.168.21.254
							servicePort[s] = 13352;
						}

						if (serviceIP[s] != 0 && servicePort[s] != 0)
						{
							log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.propertyValue("EquipmentID"));
						}
					}
					
				}
				else
				{
					var serviceDataChannel = service.jsFindChildObjectByMask(serviceID + "_DATACH0" + (i + 1));
					if (serviceDataChannel == null)
					{
						log.errCFG3004(serviceID + "_DATACH01", equipmentID);
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
					
					serviceIP[s] = serviceDataChannel.jsPropertyIP(servicesName[s] + "DataReceivingIP");
					servicePort[s] = serviceDataChannel.propertyValue(servicesName[s] + "DataReceivingPort");
				}
			}
		}
			
		var regDataID = module.propertyValue("AppLANDataUID");
		var diagDataID = module.propertyValue("DiagLANDataUID");
		
		var controllerAppWordsCount = appWordsCount;
		
		var overrideRegWordsCount =	ethernetController.propertyValue("OverrideAppDataWordCount");
		if (overrideRegWordsCount != -1)
		{
			controllerAppWordsCount = overrideRegWordsCount;
			regDataID = 0;
		}
		
		var controllerDiagWordsCount = diagWordsCount;
		
		var overrideDiagWordsCount = ethernetController.propertyValue("OverrideDiagDataWordCount");
		if (overrideDiagWordsCount != -1)
		{
			controllerDiagWordsCount = overrideDiagWordsCount;
			diagDataID = 0;
		}

		if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController,
									controllerAppWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], regDataID,
									controllerDiagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1], diagDataID) == false)
									{
										return false;
									}								
		lanFrame++;
	}
	
	// Create TX/RX configuration
	//
	
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	var txRxConfigFrame = lanConfigFrame + 3;
	
	if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false)
	{
		return false;
	}
	
	// create UniqueID
	//
	var startFrame = configStartFrames + configFrameCount * (LMNumber - 1);

	var uniqueID = 0;
	
	for (var i = 0; i < configFrameCount; i++)
	{
		var crc = confFirmware.calcCrc32(startFrame + i, 0, frameSize);

		uniqueID ^= crc;
	}
	
	confFirmware.jsSetUniqueID(LMNumber, uniqueID);
  
	return true;
}

function generate_txRxIoConfig(confFirmware, equipmentID, LMNumber, frame, offset, log, flags, configFrames, dataFrames, txId)
{
	// TxRx Block's configuration structure
    //
    var ptr = offset;

	confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] flags = "+ flags + 
	"; [" + frame + ":" + (ptr + 2) +"] configFrames = "+ configFrames +
	"; [" + frame + ":" + (ptr + 4) +"] dataFrames = "+ dataFrames +
	"; [" + frame + ":" + (ptr + 6) +"] txId = "+ txId + "\r\n");
    
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "TxRxFlags", flags) == false)        // Flags word
	{
		return false;
	}
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Configuration words quantity", configFrames) == false) // Configuration words quantity
	{
		return false;
	}
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Data words quantity", dataFrames) == false)   // Data words quantity
	{
		return false;
	}
    ptr += 2;
    if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Tx ID", txId) == false)         // Tx ID
	{
		return false;
	}
    ptr += 2;
    
    return true;
}

function generate_LANConfiguration(confFirmware, log, frame, module, ethernetController,
									regWordsCount, regIP, regPort, regServiceIP, regServicePort, regDataID,
									diagWordsCount, diagIP, diagPort, diagServiceIP, diagServicePort, diagDataID)
{
	var ptr = 0;		
	
	var moduleEquipmentID = module.propertyValue("EquipmentID");
	var LMNumber = module.propertyValue("LMNumber");
	var controllerEquipmentID = ethernetController.propertyValue("EquipmentID");
	
	//mac
	//
	var hashName = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
	var hashList = confFirmware.calcHash64(hashName);
	var size = hashList.jsSize();
	if (size != 2)
	{
		log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
		return false;
	}

	var h0 = hashList.jsAt(0);
	var h1 = hashList.jsAt(1);

	var m1 = 0x4200;
	var m2 = h0 & 0x7fff;
	var m3 = (h0 >> 16) & 0x7fff;
	
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false)
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false)
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false)
	{
		return false;
	}
	ptr += 2;
	
	// regIP

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP 1 = " + ipToString(regIP) + "\r\n");
	
	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 1", regIP) == false)
	{
		return false;
	}
	ptr += 4;

	// regPort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 = " + regPort + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1", regPort) == false)
	{
		return false;
	}
	ptr += 2;
	
	// diagIP

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP 2 = " + ipToString(diagIP) + "\r\n");
	
	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 2", diagIP) == false)
	{
		return false;
	}
	ptr += 4;

	// diagPort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 = " + diagPort + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2", regPort) == false)
	{
		return false;
	}
	ptr += 2;
		
	// regServiceIP

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service IP 1 = " + ipToString(regServiceIP) + "\r\n");
	
	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 1", regServiceIP) == false) 
	{
		return false;
	}
	ptr += 4;

	// regServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service Port 1 = " + regServicePort + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 1", regServicePort) == false)
	{
		return false;
	}
	ptr += 2;

	// regWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 words count = " + regWordsCount + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 words count", regWordsCount) == false)
	{
		return false;
	}
	ptr += 2;

	// diagServiceIP

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service IP 2 = " + ipToString(diagServiceIP) + "\r\n");
	
	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 2", diagServiceIP) == false)
	{
		return false;
	}
	ptr += 4;

	// diagServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Service Port 2 = " + diagServicePort + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 2", diagServicePort) == false)
	{
		return false;
	}
	ptr += 2;

	// diagWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 words count = " + diagWordsCount + "\r\n");
	
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 words count", diagWordsCount) == false)
	{
		return false;
	}
	ptr += 2;
	
	// appDUID

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 1 DUID = " + regDataID + "\r\n");
	
	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 DUID", regDataID) == false)
	{
		return false;
	}
	ptr += 4;
	
	// diagDUID

	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port 2 DUID = " + diagDataID + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 DUID", diagDataID) == false) 
	{
		return false;
	}
	ptr += 4;

	return true;
}

// function returns the amount of transmitting words
//
function generate_lmTxRxOptoConfiguration(confFirmware, log, frame, module, LMNumber, opticModuleStorage, logicModuleDescription)
{
	if (module.propertyValue("EquipmentID") == undefined)
	{
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}
	
	var portCount = logicModuleDescription.OptoInterface_OptoPortCount;
	
	var txWordsCount = 0;
	
	for (var p = 0; p < portCount; p++)
	{
		var controllerID = module.propertyValue("EquipmentID") + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);
	    
		var controller = module.jsFindChildObjectByMask(controllerID);
		if (controller == null)
		{
			log.errCFG3004(controllerID, module.propertyValue("EquipmentID"));
			return false;
		}
		
		if (controller.propertyValue("EquipmentID") == undefined)
		{
			log.errCFG3000("EquipmentID", "Class_Controller");
			return false;
		}
		
		var controllerEquipmentID = controller.propertyValue("EquipmentID");

		var optoPort = opticModuleStorage.jsGetOptoPort(controllerEquipmentID);
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
					
		var ptr = 0 + p * 2;
		var value = optoPort.txStartAddress();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 5 * 2 + p * 2;
		value = optoPort.txDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
		txWordsCount += value;
				
		ptr = 10 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
		ptr = 15 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false)
		{
			return false;
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
		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr +"]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
	} // p
	
	return true;
}
