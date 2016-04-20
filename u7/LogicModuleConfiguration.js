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

function(root, confCollection, log, signalSet, subsystemStorage, opticModuleStorage)
{
    log.writeMessage("Start LogicModuleConfiguration");

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

			var checkProperties = ["SubsysID", "Channel", "ConfigFrameSize", "ConfigFrameCount"];
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
			var subSysID = device.propertyValue("SubsysID");
			var channel = device.propertyValue("Channel");
			
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
	var checkProperties = ["SubsysID", "Channel", "ConfigFrameSize", "ConfigFrameCount", "TuningDataSize", 
	/*"AppIP1", "AppIP2", "DiagIP1", "DiagIP2", 
	"DiagDataServiceID1", "DiagDataServiceID2", 
	"AppDataServiceID1", "AppDataServiceID2", 
	"SourcePort", 
	"TuningPort", "TuningIP", "TuningServiceIP",*/
	"RegDataSize", "DiagDataSize"];
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
    var subSysID = module.propertyValue("SubsysID");
    var channel = module.propertyValue("Channel");
    
    // Constants
    //
    var frameSize = module.jsPropertyInt("ConfigFrameSize");
    var frameCount = module.jsPropertyInt("ConfigFrameCount");
    
    if (frameSize < 1016)
    {
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.propertyValue("EquipmentID"));
        return false;
    }

	if (frameCount < 76 /*19  frames * 4 channels*/)
    {
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 76, 65535, module.propertyValue("EquipmentID"));
        return false;
    }
    
    var uartId = 0x0102; 
    
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

	var regWordsCount = module.propertyValue("RegDataSize");
	var diagWordsCount = module.propertyValue("DiagDataSize");

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
	/*
	
	confFirmware.writeLog("Writing LAN configuration.\r\n");

    var lanStartFrame = 19;
	
	var tuningWordsCount = module.propertyValue("TuningDataSize");

	
	var sourcePort = module.propertyValue("SourcePort");

	// Tuning
	//

	var tuningServiceIP = module.jsPropertyIP("TuningServiceIP");
	var tuningServiceAddress = tuningServiceIP & 0xff;
	var tuningServiceSubnetwork = (tuningServiceIP >> 8) & 0xff;
	
	var tuningIP = module.jsPropertyIP("TuningIP");
	var tuningAddress = tuningIP & 0xff;
	var tuningPort = module.propertyValue("TuningPort");

	generate_LANConfiguration(confFirmware, log, lanConfigFrame, module, tuningWordsCount, 0, 
								tuningIP, sourcePort, tuningPort,
								tuningServiceSubnetwork, tuningServiceAddress, 13332,
								0, 0, 0);	//Subnet2 is not used
	lanConfigFrame++;
								
	// REG / DIAG
	//

	var appServiceSubnetwork = [0, 0];			//	Take from software!!!
	var appServiceIP = [0, 0];
	var appServicePort = [0, 0];
	
	var diagServiceSubnetwork = [0, 0];		//	Take from software!!!
	var diagServiceIP = [0, 0];
	var diagServicePort = [0, 0];
	
	for (var i = 0; i < 2; i++)
	{
		var appAcqProp = "AppDataServiceID" + (i + 1);
		var appAcqID = module.propertyValue(appAcqProp);
		
		if (appAcqID != "")
		{
			var appAcq = root.jsFindChildObjectByMask(appAcqID);
		
			if (appAcq == null)
			{
				log.wrnCFG3008(appAcqID, module.propertyValue("EquipmentID"));
			}
			else
			{
				var appServiceIPValue  = appAcq.jsPropertyIP("RegDataReceivingIP" + (i + 1));
				if (appServiceIPValue == null)
				{
					log.errCFG3000("RegDataReceivingIP" + (i + 1), appAcq.propertyValue("EquipmentID"));
					return false;
				}
				appServiceSubnetwork[i] = (appServiceIPValue >> 8) & 0xff;
				appServiceIP[i] = appServiceIPValue & 0xff;
				
				appServicePort[i] = appAcq.propertyValue("RegDataReceivingPort" + (i + 1))
				if (appServicePort[i] == null)
				{
					log.errCFG3000("RegDataReceivingPort" + (i + 1), appAcq.propertyValue("EquipmentID"));
					return false;
				}
			}
		}

		var diagAcqProp = "DiagDataServiceID" + (i + 1);
		var diagAcqID = module.propertyValue(diagAcqProp);
		
		if (diagAcqID != "")
		{
			var diagAcq = root.jsFindChildObjectByMask(diagAcqID);

			if (diagAcq == null)
			{
				log.wrnCFG3008(diagAcqID, module.propertyValue("EquipmentID"));
			}
			else
			{
				var diagServiceIPValue  = diagAcq.jsPropertyIP("DiagDataReceivingIP" + (i + 1));
				if (diagServiceIPValue == null)
				{
					log.errCFG3000("DiagDataReceivingIP" + (i + 1), diagAcq.propertyValue("EquipmentID"));
					return false;
				}
				diagServiceSubnetwork[i] = (diagServiceIPValue >> 8) & 0xff;
				diagServiceIP[i] = diagServiceIPValue & 0xff;
				diagServicePort[i] = diagAcq.propertyValue("DiagDataReceivingPort" + (i + 1))
				if (diagServicePort[i] == null)
				{
					log.errCFG3000("DiagDataReceivingPort" + (i + 1), diagAcq.propertyValue("EquipmentID"));
					return false;
				}
			}
		}
	
		var AppIP = module.jsPropertyIP("AppIP" + (i + 1));
		var regAddress = AppIP & 0xff;
		
		var diagIP = module.jsPropertyIP("DiagIP" + (i + 1));
		var diagAddress = diagIP & 0xff;
		
		if (regAddress != diagAddress || tuningAddress != regAddress || tuningAddress != diagAddress)
		{
			//log.writeWarning("Different module " + module.propertyValue("EquipmentID") + " IP addresses! regAddress = " + regAddress + ", diagAddress = " + diagAddress + ", tuningAddress = " + tuningAddress + "! regAddress is used.");
		}

		generate_LANConfiguration(confFirmware, log, lanConfigFrame, module, regWordsCount, diagWordsCount, 
				AppIP, sourcePort, 0,
				appServiceSubnetwork[i], appServiceIP[i], appServicePort[i],
				diagServiceSubnetwork[i], diagServiceIP[i], diagServicePort[i]); 
				
		lanConfigFrame++;
	}
	*/
	
	// Create TX/RX configuration
	//
	
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	var txRxConfigFrame = lanConfigFrame;
	
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
    
    var defaultTf = valToADC(50, 0, 65535, 0, 0xffff);
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
        var signal = findSignalByPlace(inController, i, Analog, Input, signalSet, log);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
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
            
            var filternigTime = valToADC(signal.filteringTime(), signal.lowLimit(), signal.highLimit(), signal.lowADC(), signal.highADC());
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

            var signal = findSignalByPlace(outController, place, Analog, Output, signalSet, log);
            if (signal != null && signal.propertyValue("EquipmentID") != undefined)
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

function findSignalByPlace(parent, place, type, func, signalSet, log)
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
}

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
function generate_LANConfiguration(confFirmware, log, frame, module, regWordsCount, diagWordsCount, 
									sourceIP, sourcePort, destPort,
									appServiceSubnetwork, appServiceAddress, appServicePort, 
									diagServiceSubnetwork, diagServiceAddress, diagServicePort)
{

	var ptr = 0;		
	
	confFirmware.writeLog("    ----------\r\n");
	
	//mac
	//
	var hashName = "S" + appServiceSubnetwork + module.propertyValue("EquipmentID") + sourceIP;
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

	var netNum = (sourceIP >> 16) & 0xffff;
	setData16(confFirmware, log, frame, ptr, netNum); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Network number = " + netNum + "\r\n");
	ptr += 2;
	
	ptr += 1;	//reserved

	var hostNum = sourceIP & 0xff;
	setData8(confFirmware, log, frame, ptr, hostNum); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Host number = " + hostNum + "\r\n");
	ptr += 1;
	
	setData16(confFirmware, log, frame, ptr, sourcePort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Source port = " + sourcePort + "\r\n");
	ptr += 2;
	
	setData16(confFirmware, log, frame, ptr, destPort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Destination port [Tuning] = " + destPort + "\r\n");
	ptr += 2;
	
	// subnet 1
	//
	
	setData8(confFirmware, log, frame, ptr, appServiceSubnetwork); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Subnetwork1 [REG/TUN] = " + appServiceSubnetwork + "\r\n");
	ptr += 1;
	
	setData8(confFirmware, log, frame, ptr, appServiceAddress); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP1 [REG/TUN] = " + appServiceAddress + "\r\n");
	ptr += 1;
	
	setData16(confFirmware, log, frame, ptr, appServicePort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port1 [REG/TUN] = " + appServicePort + "\r\n");
	ptr += 2;
	
	setData16(confFirmware, log, frame, ptr, regWordsCount); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Quantity1 [REG/TUN] = " + regWordsCount + "\r\n");
	ptr += 2;

	// subnet 2
	//
	
	setData8(confFirmware, log, frame, ptr, diagServiceSubnetwork); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Subnetwork2 [DIAG] = " + diagServiceSubnetwork + "\r\n");
	ptr += 1;
	
	setData8(confFirmware, log, frame, ptr, diagServiceAddress); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP2 [DIAG] = " + diagServiceAddress + "\r\n");
	ptr += 1;
	
	setData16(confFirmware, log, frame, ptr, diagServicePort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port2 [DIAG] = " + diagServicePort + "\r\n");
	ptr += 2;
	
	setData16(confFirmware, log, frame, ptr, diagWordsCount); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Quantity2 [DIAG] = " + diagWordsCount + "\r\n");
	ptr += 2;

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
