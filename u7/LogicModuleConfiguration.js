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

function(root, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
    log.writeMessage("Start LogicModuleConfiguration");

    var result = true;

    result = module_lm_1(root, root, confCollection, log, signalSet, subsystemStorage, connectionStorage);
    if (result == false)
    {
        return false;
    }

    result = module_lm_1_statistics(root, confCollection, log, signalSet, subsystemStorage, connectionStorage);
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

function module_lm_1(device, root, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
    if (device.jsDeviceType() == ModuleType)
    {
	
		var checkProperties = ["ModuleFamily", "StrID"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (device.propertyValue(checkProperties[cp]) == undefined)
			{
				log.writeError("Property " + checkProperties[cp] + " was not found in module " + device.propertyValue("StrID") + " in function module_lm_1");
				return false;
			}
		}

        if (device.propertyValue("ModuleFamily") == FamilyLM)
        {
            log.writeMessage("MODULE LM-1: " + device.propertyValue("StrID"));

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, root, confCollection, log, signalSet, subsystemStorage, connectionStorage);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1(child, root, confCollection, log, signalSet, subsystemStorage, connectionStorage) == false)
        {
            return false;
        }
    }

    return true;
}

function module_lm_1_statistics(device, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
    if (device.jsDeviceType() == ModuleType)
    {
		var checkProperties = ["ModuleFamily", "StrID"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (device.propertyValue(checkProperties[cp]) == undefined)
			{
				log.writeError("Property " + checkProperties[cp] + " was not found in module " + device.propertyValue("StrID") + " in function module_lm_1_statistics");
				return false;
			}
		}
		
        if (device.propertyValue("ModuleFamily") == FamilyLM)
        {
            log.writeMessage("MODULE LM-1 (statistics): " + device.propertyValue("StrID"));

			var checkProperties = ["SubsysID", "Channel", "ConfigFrameSize", "ConfigFrameCount"];
			for (var cp = 0; cp < checkProperties.length; cp++)
			{
				if (device.propertyValue(checkProperties[cp]) == undefined)
				{
					log.writeError("Property " + checkProperties[cp] + " was not found in module " + device.propertyValue("StrID") + " in function module_lm_1_statistics");
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
				log.writeError("Subsystem key for " + subSysID + " was not found in function module_lm_1_statistics");
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
        if (module_lm_1_statistics(child, confCollection, log, signalSet, subsystemStorage, connectionStorage) == false)
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
function generate_lm_1_rev3(module, root, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
	var checkProperties = ["StrID", "SubsysID", "Channel", "ConfigFrameSize", "ConfigFrameCount", "TuningDataSize", 
	"RegIP1", "RegIP2", "DiagIP1", "DiagIP2", 
	"DiagDataAcquisitionServiceStrID1", "DiagDataAcquisitionServiceStrID2", 
	"RegDataAcquisitionServiceStrID1", "RegDataAcquisitionServiceStrID2", 
	"SourcePort", "RegDataSize", "DiagDataSize", "TuningPort", "TuningServerPort", "TuningIP", "TuningServerIP"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_lm_1_rev3");
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
    
    if (frameSize == 0 || frameCount == 0)
    {
        log.writeError("Module " + module.propertyValue("StrID") + ": wrong frameSize or frameCount: frameSize = " + frameSize + ", frameCount: " + frameCount + " in function generate_lm_1_rev3");
        return false;
    }
    
    var uartId = 0x0102; 
    
    var ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1)
    {
        log.writeError("Subsystem key for " + subSysID + " was not found in function generate_lm_1_rev3");
        return false;
    }

    var maxChannel = 4;                 // Can be changed!
    var configStartFrames = 2;
    var configFrameCount = 19;          // number of frames in each configuration
    var ioModulesMaxCount = 14;
    
    if (channel < 1 || channel > maxChannel)
    {
        log.writeError("Module " + module.propertyValue("StrID") + ", channel: " + channel + ": wrong channel (should be 1 - " + maxChannel + " in function generate_lm_1_rev3");
        return false;
    }

    var confFirmware = confCollection.jsGet("LM-1", subSysID, ssKeyValue, uartId, frameSize, frameCount);
	
    confFirmware.writeLog("---\r\n");
    confFirmware.writeLog("Module: LM-1\r\n");
	confFirmware.writeLog("StrID = " + module.propertyValue("StrID") + "\r\n");
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
        log.writeError("Module " + module.propertyValue("StrID") + ": channel is not unique in function generate_lm_1_rev3");
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
		
		var checkProperties = ["ModuleFamily", "StrID", "Place", "DiagDataSize"];
		for (var cp = 0; cp < checkProperties.length; cp++)
		{
			if (ioModule.propertyValue(checkProperties[cp]) == undefined)
			{
				log.writeError("Property " + checkProperties[cp] + " was not found in module " + ioModule.propertyValue("StrID") + " in function generate_lm_1_rev3");
				return false;
			}
		}
		
        if (ioModule.propertyValue("ModuleFamily") == FamilyAIM || ioModule.propertyValue("ModuleFamily") == FamilyAIFM || 
            ioModule.propertyValue("ModuleFamily") == FamilyAOM || ioModule.propertyValue("ModuleFamily") == FamilyOCM ||
            ioModule.propertyValue("ModuleFamily") == FamilyDIM || ioModule.propertyValue("ModuleFamily") == FamilyDOM)
        {
            if (ioModule.propertyValue("Place") < 1 || ioModule.propertyValue("Place") > ioModulesMaxCount)
            {
                log.writeError("Wrong I/O module place: " + ioModule.propertyValue("StrID") + ", place: " + ioModule.propertyValue("Place") + ", expected 1..14 in function generate_lm_1_rev3");
                return false;
            }
            var frame = frameIOConfig + (ioModule.propertyValue("Place") - 1);
            
            if (ioModule.propertyValue("ModuleFamily") == FamilyAIM)
            {
                generate_aim(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyAIFM)
            {
                generate_aifm(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyAOM)
            {
                generate_aom(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyOCM)
            {
                generate_ocm(confFirmware, ioModule, frame, log, connectionStorage);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDIM)
            {
                generate_dim(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDOM)
            {
                generate_dom(confFirmware, ioModule, frame, log);
            }
			
			var diagWordsIoCount = ioModule.propertyValue("DiagDataSize");
			diagWordsCount += diagWordsIoCount;
        }
    }
	
	var lanConfigFrame = frameIOConfig + 14;

	// Create LANs configuration
    //
	confFirmware.writeLog("Writing LAN configuration.\r\n");

    var lanStartFrame = 19;
	
	var tuningWordsCount = module.propertyValue("TuningDataSize");

	
	var sourcePort = module.propertyValue("SourcePort");

	// Tuning
	//

	var tuningServerIP = module.jsPropertyIP("TuningServerIP");
	var tuningServerAddress = tuningServerIP & 0xff;
	var tuningServerSubnetwork = (tuningServerIP >> 8) & 0xff;
	var tuningServerPort = module.propertyValue("TuningServerPort");
	
	var tuningIP = module.jsPropertyIP("TuningIP");
	var tuningAddress = tuningIP & 0xff;
	var tuningPort = module.propertyValue("TuningPort");

	generate_LANConfiguration(confFirmware, log, lanConfigFrame, module, tuningWordsCount, 0, 
								tuningIP, sourcePort, tuningPort,
								tuningServerSubnetwork, tuningServerAddress, tuningServerPort,
								0, 0, 0);	//Subnet2 is not used
	lanConfigFrame++;
								
	// REG / DIAG
	//

	var regServerSubnetwork = [0, 0];			//	Take from software!!!
	var regServerIP = [0, 0];
	var regServerPort = [0, 0];
	
	var diagServerSubnetwork = [0, 0];		//	Take from software!!!
	var diagServerIP = [0, 0];
	var diagServerPort = [0, 0];
	
	for (var i = 0; i < 2; i++)
	{
		var regAcqID = module.propertyValue("RegDataAcquisitionServiceStrID" + (i + 1));
		var diagAcqID = module.propertyValue("DiagDataAcquisitionServiceStrID" + (i + 1));
		
		var regAcq = root.jsFindChildObjectByMask(regAcqID);
		var diagAcq = root.jsFindChildObjectByMask(diagAcqID);
		
		if (diagAcq == null || regAcq == null)
		{
			log.writeWarning(module.propertyValue("StrID") + ": one of data acquisition services " + diagAcqID + ", " + regAcqID + " was not found, using defaults.");
		}
		else
		{
			var regServerIPValue  = regAcq.jsPropertyIP("RegDataReceivingIP" + (i + 1));
			if (regServerIPValue == null)
			{
				log.writeError("Software " + regAcq.propertyValue("StrID") + " has no property " + "RegDataReceivingIP" + (i + 1) + " in function generate_lm_1_rev3" );
				return false;
			}
			regServerSubnetwork[i] = (regServerIPValue >> 8) & 0xff;
			regServerIP[i] = regServerIPValue & 0xff;
			regServerPort[i] = regAcq.propertyValue("RegDataReceivingPort" + (i + 1))
			if (regServerPort[i] == null)
			{
				log.writeError("Software " + regAcq.propertyValue("StrID") + " has no property " + "RegDataReceivingPort" + (i + 1) + " in function generate_lm_1_rev3");
				return false;
			}
			
			var diagServerIPValue  = diagAcq.jsPropertyIP("DiagDataReceivingIP" + (i + 1));
			if (diagServerIPValue == null)
			{
				log.writeError("Software " + diagAcq.propertyValue("StrID") + " has no property " + "DiagDataReceivingIP" + (i + 1) + " in function generate_lm_1_rev3");
				return false;
			}
			diagServerSubnetwork[i] = (diagServerIPValue >> 8) & 0xff;
			diagServerIP[i] = diagServerIPValue & 0xff;
			diagServerPort[i] = diagAcq.propertyValue("DiagDataReceivingPort" + (i + 1))
			if (diagServerPort[i] == null)
			{
				log.writeError("Software " + diagAcq.propertyValue("StrID") + " has no property " + "DiagDataReceivingPort" + (i + 1) + " in function generate_lm_1_rev3");
				return false;
			}
		}
	
		var regIP = module.jsPropertyIP("RegIP" + (i + 1));
		var regAddress = regIP & 0xff;
		
		var diagIP = module.jsPropertyIP("DiagIP" + (i + 1));
		var diagAddress = diagIP & 0xff;
		
		if (regAddress != diagAddress || tuningAddress != regAddress || tuningAddress != diagAddress)
		{
			log.writeWarning("Different module " + module.propertyValue("StrID") + " IP addresses! regAddress = " + regAddress + ", diagAddress = " + diagAddress + ", tuningAddress = " + tuningAddress + "! regAddress is used.");
		}

		generate_LANConfiguration(confFirmware, log, lanConfigFrame, module, regWordsCount, diagWordsCount, 
				regIP, sourcePort, 0,
				regServerSubnetwork[i], regServerIP[i], regServerPort[i],
				diagServerSubnetwork[i], diagServerIP[i], diagServerPort[i]); 
				
		lanConfigFrame++;
	}
	
	// Create TX/RX configuration
	//
	
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	var txRxConfigFrame = lanConfigFrame;
	var optoCount = 3;
	
	/*var txWordsCount = */generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, connectionStorage, optoCount, false/*modeOCM*/);
  
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
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_aim");
			return false;
		}
	}

    log.writeMessage("MODULE AIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AIMSignalMaxCount = 64;
    
    var defaultTf = valToADC(50, 0, 65535, 0, 0xffff);
    var defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    var defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    var defaultMaxDiff = valToADC(0.5, 0, 5.1, 0, 0xffff);
    
    var inController = module.jsFindChildObjectByMask(module.propertyValue("StrID") + "_CTRLIN");
    if (inController == null)
    {
        log.writeError("No input controller found in " + module.propertyValue("StrID") + " in function generate_aim");
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
						log.writeError("Error - AIM input " + channelAPlace + " maxDifference ADC "+ channelAMaxDifference + " is not equal to input " + i + " maxDifference ADC " + maxDifference + "in function generate_aim");
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
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_aifm");
			return false;
		}
	}

    log.writeMessage("MODULE AIFM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AIFM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
	
    var ptr = 0;
    
 
    var inController = module.jsFindChildObjectByMask(module.propertyValue("StrID") + "_CTRLIN");
    if (inController == null)
    {
        log.writeError("No input controller found in " + module.propertyValue("StrID") + " in function generate_aifm");
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
			var signal = inController.jsFindChildObjectByMask(inController.propertyValue("StrID") + "_IN0" + (i + 1) + koeffs[k]);
			var value = 1;
			if (signal == null)
			{
				log.writeWarning("No signal " + inController.propertyValue("StrID") + "_IN0" + (i + 1) + koeffs[k] + " found in " + inController.propertyValue("StrID") + "! Using default values.");
			}
			else
			{
				if (signal.propertyExists("PowerCoefficient") == true)
				{
					value = signal.propertyValue("PowerCoefficient");
				}
				else
				{
					log.writeWarning("No property PowerCoefficient exists in " + signal.propertyValue("StrID") + "! Using default values.");
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
					var signal = inController.jsFindChildObjectByMask(inController.propertyValue("StrID") + "_IN0" + (i + 1) + koeffs[k] + modeNames[m]);
					var value = 1;
					if (signal == null)
					{
						log.writeWarning("No signal " + inController.propertyValue("StrID") + "_IN0" + (i + 1) + koeffs[k]  + modeNames[m] + " found in " + inController.propertyValue("StrID") + "! Using default values.");
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
							log.writeWarning("No property " + setPointName + "	exists in " + signal.propertyValue("StrID") + "! Using default values.");
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
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_aom");
			return false;
		}
	}

    log.writeMessage("MODULE AOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AOMWordCount = 4;                       // total words count
    var AOMSignalsInWordCount = 8;              // signals in a word count
    
    var outController = module.jsFindChildObjectByMask(module.propertyValue("StrID") + "_CTRLOUT");
    if (outController == null)
    {
        log.writeError("No output controller found in " + module.propertyValue("StrID") + " in function generate_aom");
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
            if (signal != null && signal.propertyValue("StrID") != undefined)
            {
				var outputRangeMode = signal.jsOutputRangeMode();
                if (outputRangeMode < 0 || outputRangeMode > Mode_05mA)
                {
                    log.writeError("Signal " + signal.propertyValue("StrID") + " has wrong outputRangeMode() in function generate_aom");
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
	if (parent.propertyValue("StrID") == undefined)
	{
		log.writeError("Property StrID was not found in an object in function findSignalByPlace");
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
		
		var strID = s.propertyValue("StrID");
		if (strID == undefined)
		{
			log.writeError("Property StrID was not found in an object in function findSignalByPlace");
			return null;
		}
		
        if (s.jsPlace() == place)
        {
            signal = signalSet.getSignalByDeviceStrID(strID);
            if (signal == null)    
            {
                log.writeWarning("Signal " + strID + " was not found in the signal database!");
            }
            return signal;
        }
    }
    
    log.writeWarning("No signal with place " + place + " was found in " + parent.propertyValue("StrID"));
    return null;
}

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmware, module, frame, log, connectionStorage)
{
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_ocm");
			return false;
		}
	}

    log.writeMessage("MODULE OCM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE OCM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
	
	var txRxConfigFrame = frame;
	var optoCount = 5;
	
	var txWordsCount = generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, connectionStorage, optoCount, true/*modeOCM*/);
	
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
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_dim");
			return false;
		}
	}

    log.writeMessage("MODULE DIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE DIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

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
	var checkProperties = ["StrID", "Place", "ModuleVersion"];
	for (var cp = 0; cp < checkProperties.length; cp++)
	{
		if (module.propertyValue(checkProperties[cp]) == undefined)
		{
			log.writeError("Property " + checkProperties[cp] + " was not found in module " + module.propertyValue("StrID") + " in function generate_dom");
			return false;
		}
	}

    log.writeMessage("MODULE DOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE DOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

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
									regServerSubnetwork, regServerAddress, regServerPort, 
									diagServerSubnetwork, diagServerAddress, diagServerPort)
{

	var ptr = 0;		
	
	confFirmware.writeLog("    ----------\r\n");
	
	//mac
	//
	var hashName = "S" + regServerSubnetwork + module.propertyValue("StrID") + sourceIP;
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
	
	setData8(confFirmware, log, frame, ptr, regServerSubnetwork); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Subnetwork1 [REG/TUN] = " + regServerSubnetwork + "\r\n");
	ptr += 1;
	
	setData8(confFirmware, log, frame, ptr, regServerAddress); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP1 [REG/TUN] = " + regServerAddress + "\r\n");
	ptr += 1;
	
	setData16(confFirmware, log, frame, ptr, regServerPort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port1 [REG/TUN] = " + regServerPort + "\r\n");
	ptr += 2;
	
	setData16(confFirmware, log, frame, ptr, regWordsCount); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Quantity1 [REG/TUN] = " + regWordsCount + "\r\n");
	ptr += 2;

	// subnet 2
	//
	
	setData8(confFirmware, log, frame, ptr, diagServerSubnetwork); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Subnetwork2 [DIAG] = " + diagServerSubnetwork + "\r\n");
	ptr += 1;
	
	setData8(confFirmware, log, frame, ptr, diagServerAddress); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : IP2 [DIAG] = " + diagServerAddress + "\r\n");
	ptr += 1;
	
	setData16(confFirmware, log, frame, ptr, diagServerPort); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Port2 [DIAG] = " + diagServerPort + "\r\n");
	ptr += 2;
	
	setData16(confFirmware, log, frame, ptr, diagWordsCount); 
	confFirmware.writeLog("    [" + frame + ":" + ptr +"] : Quantity2 [DIAG] = " + diagWordsCount + "\r\n");
	ptr += 2;

	return true;
}

// function returns the amount of transmitting words
//
function generate_txRxOptoConfiguration(confFirmware, log, frame, module, connections, portCount, modeOCM)
{
	if (module.propertyValue("StrID") == undefined)
	{
		log.writeError("Property StrID was not found in a module in function generate_txRxOptoConfiguration");
		return 0;
	}
	
    // Create TxRx Blocks (Opto) configuration
	//
	confFirmware.writeLog("    There are " + connections.count() + " connections in the project.\r\n");
	
	var txStartAddress = 0;
	
	for (var p = 0; p < portCount; p++)
	{
		var controllerID = module.propertyValue("StrID") + "_PORT0";
		controllerID = controllerID + (p + 1);
	    
		var controller = module.jsFindChildObjectByMask(controllerID);
		if (controller == null)
		{
			log.writeError("No controller " + controllerID + " was found in " + module.propertyValue("StrID") + " in function generate_txRxOptoConfiguration");
			return 0;
		}
		
		if (controller.propertyValue("StrID") == undefined)
		{
			log.writeError("Property StrID was not found in a controller in function generate_txRxOptoConfiguration");
			return 0;
		}

		for (var c = 0; c < connections.count(); c++)
		{
			var connection = connections.jsGet(c);
			if (connection == null)
			{
				continue;
			}
	
			var checkProperties = ["Caption", "Port1StrID", "Port2StrID", 
			"Enable", "EnableDuplex", "SerialMode", "Mode",
			"Port1TxWordsQuantity", "Port1TxRxOptoID", "Port1RxWordsQuantity", "Port1TxRxOptoDataUID", "Port1TxRsID", "Port1TxRsDataUID", 
			"Port2TxWordsQuantity", "Port2TxRxOptoID", "Port2RxWordsQuantity", "Port2TxRxOptoDataUID", "Port2TxRsID", "Port2TxRsDataUID" ];

			for (var cp = 0; cp < checkProperties.length; cp++)
			{
				if (connection.propertyValue(checkProperties[cp]) == undefined)
				{
					log.writeError("Property " + checkProperties[cp] + " was not found in class Connection in function generate_txRxOptoConfiguration");
					return 0;
				}
			}
				
			var deviceNo = -1;
			var rsConnection = false;
		
            if (controller.propertyValue("Mode") == Mode_Serial)
			{
				// this is rs connection
				//
				rsConnection = true;
				deviceNo = 1;
			}
			else
			{
				// this is optical connection
				//
				if (controller.propertyValue("StrID") == connection.propertyValue("Port1StrID"))
				{
					deviceNo = 1;
				}
				else
				{
					if (controller.propertyValue("StrID") == connection.propertyValue("Port2StrID"))
					{
						deviceNo = 2;
					}
				}
			}
			
			if (deviceNo == -1)
			{
				continue;
			}
			
			//
			// A connection was found for this controller
			//
			if (rsConnection == true)
			{
				confFirmware.writeLog("    Controller " + controller.propertyValue("StrID") + ": Rs connection  ID = " + connection.propertyValue("Caption") + ":" + 
                    connection.propertyValue("Port1StrID") + "\r\n");
			}
			else
			{
				confFirmware.writeLog("    Controller " + controller.propertyValue("StrID") + ": Opto connection device No " + deviceNo + " ID = " + connection.propertyValue("Caption") + ":" + 
					connection.propertyValue("Port1StrID") + " <=> " + connection.propertyValue("Port2StrID") + "\r\n");	
			}
					
			var deviceName = "Port" + deviceNo;

			var ptr = 0 + p * 2;
			var value = txStartAddress;
			setData16(confFirmware, log, frame, ptr, value); 
			confFirmware.writeLog("    [" + frame + ":" + ptr +"] : TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
				
			ptr = 5 * 2 + p * 2;
			value = connection.propertyValue(deviceName + "TxWordsQuantity");
			if (value == undefined)
			{
				return 0;
			}
			else
			{
				txStartAddress += value;
				setData16(confFirmware, log, frame, ptr, value); 
				confFirmware.writeLog("    [" + frame + ":" + ptr +"] : TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
			}
				
			ptr = 10 * 2 + p * 2;
			value = connection.propertyValue(deviceName + "TxRxOptoID");
			if (value == undefined)
			{
				return 0;
			}
			else
			{
				setData16(confFirmware, log, frame, ptr, value); 
				confFirmware.writeLog("    [" + frame + ":" + ptr +"] : TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
			}
				
			ptr = 15 * 2 + p * 2;
			value = connection.propertyValue(deviceName + "RxWordsQuantity");
			if (value == undefined)
			{
				return 0;
			}
			else
			{
				setData16(confFirmware, log, frame, ptr, value); 
				confFirmware.writeLog("    [" + frame + ":" + ptr +"] : RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
			}
				
			ptr = 20 * 2 + p * 4;
			value = connection.propertyValue(deviceName + "TxRxOptoDataUID");
			if (value == undefined)
			{
				return 0;
			}
			else
			{
				setData32(confFirmware, log, frame, ptr, value); 
				confFirmware.writeLog("    [" + frame + ":" + ptr +"] : TxRx Block (Opto) Data UID " + (p + 1) + " = " + value + "\r\n");
			}
				
			if (modeOCM == true && rsConnection == true)
			{
				ptr = 30 * 2 + p * 2;
				value = connection.propertyValue(deviceName + "TxRsID");
				if (value == undefined)
				{
					return 0;
				}
				else
				{
					setData16(confFirmware, log, frame, ptr, value); 
					confFirmware.writeLog("    [" + frame + ":" + ptr +"] : TX ID for RS-232/485 transmitter " + (p + 1) + " = " + value + "\r\n");
				}
					
				ptr = 35 * 2 + p * 4;
				value = connection.propertyValue(deviceName + "TxRsDataUID");
				if (value == undefined)
				{
					return 0;
				}
				else
				{
					setData32(confFirmware, log, frame, ptr, value); 
					confFirmware.writeLog("    [" + frame + ":" + ptr +"] : RS-232/485 Data UID " + (p + 1) + " = " + value + "\r\n");
				}
				
				//
				// RS232/485_CFG
				//
				
				var txEn = 0;	//1 - enabled
				if (connection.propertyValue("Enable") == true)
				{
					txEn = 1;
				}
				
				var txStandard = Mode_RS232;	//0 - rs232, 1 - rs485
				if (connection.propertyValue("SerialMode") == Mode_RS485)
				{
					txStandard = Mode_RS485;
				}

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
					
					if (connection.propertyValue("EnableDuplex") == true)
					{
						txDuplex = 1;
					}
					txDuplex <<= 10;
					txDuplex <<= p;
					txMode |= txDuplex;
				}
				
				
				var allModes = confFirmware.data16(frame, ptr);
				allModes |= txMode;
				setData16(confFirmware, log, frame, ptr, allModes);
			}
			
			break;
		} // c
	} // p
	
	if (modeOCM == true)
	{
		var ptr = 45 * 2;
		var allModes = confFirmware.data16(frame, ptr);
		confFirmware.writeLog("    [" + frame + ":" + ptr +"] : RS mode configuration = " + allModes + "\r\n");
	}
	
	return txStartAddress;
}
