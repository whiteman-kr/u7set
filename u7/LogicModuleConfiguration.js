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

function(root, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
    log.writeMessage("Start LogicModuleConfiguration");

    var result = true;

    result = module_lm_1(root, confCollection, log, signalSet, subsystemStorage, connectionStorage);

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
        log.writeError("Error: SetData8, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function setData16(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData16(frameIndex, offset, data) == false)
    {
        log.writeError("Error: SetData16, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function setData32(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData32(frameIndex, offset, data) == false)
    {
        log.writeError("Error: SetData32, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function storeCrc64(confFirmware, log, frameIndex, start, count, offset)
{
    if (confFirmware.storeCrc64(frameIndex, start, count, offset) == false)
    {
        log.writeError("Error: StoreCrc64, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function storeHash64(confFirmware, log, frameIndex, offset, data)
{
	var result = confFirmware.storeHash64(frameIndex, offset, data);
    if (result == "")
    {
        log.writeError("Error: storeHash64, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
    }
    return result;
}

function module_lm_1(device, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
    if (device.jsDeviceType() == ModuleType)
    {
        if (device.propertyValue("ModuleFamily") == FamilyLM)
        {
            log.writeMessage("MODULE LM-1: " + device.propertyValue("StrID"));

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, confCollection, log, signalSet, subsystemStorage, connectionStorage);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1(child, confCollection, log, signalSet, subsystemStorage, connectionStorage) == false)
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
function generate_lm_1_rev3(module, confCollection, log, signalSet, subsystemStorage, connectionStorage)
{
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
        log.writeError("Wrong LM-1 frameSize or frameCount: frameSize = " + frameSize + ", frameCount: " + frameCount);
        return false;
    }
    
    var uartId = 0x0102;                   // Check it !!!!
    
    var ssKeyValue = subsystemStorage.ssKey(subSysID);
    if (ssKeyValue == -1)
    {
        log.writeError("Subsystem key for " + subSysID + " was not found!");
        return false;
    }

    var maxChannel = 4;                 // Can be changed!
    var configStartFrames = 2;
    var configFrameCount = 19;          // number of frames in each configuration
    var ioModulesMaxCount = 14;
    
    if (channel < 1 || channel > maxChannel)
    {
        log.writeError("Wrong LM-1 channel (should be 1 - " + maxChannel + "): " + module.propertyValue("StrID") + ", channel: " + channel);
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

    // Configuration storage format
    //
    var frameStorageConfig = 1;
    var ptr = 0;
    
    setData16(confFirmware, log, frameStorageConfig, ptr, 0xca70);     //CFG_Marker
	confFirmware.writeLog("Frame " + frameStorageConfig + ", offset " + ptr +": CFG_Marker = 0xca70" + "\r\n");
    ptr += 2;
    
    setData16(confFirmware, log, frameStorageConfig, ptr, 0x0001);     //CFG_Version
	confFirmware.writeLog("Frame " + frameStorageConfig + ", offset " + ptr +": CFG_Version = 0x0001" + "\r\n");
    ptr += 2;
    
    
    var ssKey = ssKeyValue << 6;             //0000SSKEYY000000b
    setData16(confFirmware, log, frameStorageConfig, ptr, ssKey);
	confFirmware.writeLog("Frame " + frameStorageConfig + ", offset " + ptr +": ssKey = " + ssKey + "\r\n");
    ptr += 2;
    
    // reserved
    ptr += 8;
    
    // write channelCount, if old value is less than current. If it is the same, output an error.
    //
    var oldChannelCount = confFirmware.data16(frameStorageConfig, ptr);
    
    if (oldChannelCount == channel)
    {
        log.writeError("LM-1 channel is not unique: " + module.propertyValue("StrID") + ", channel: " + channel + "\r\n");
        return false;
    }
    
    if (oldChannelCount < channel)
    {
        setData16(confFirmware, log, frameStorageConfig, ptr, channel);
		confFirmware.writeLog("Frame " + frameStorageConfig + ", offset " + ptr +": ChannelCount = " + channel + "\r\n");

    }
    ptr += 2;
    
    var configIndexOffset = ptr + (channel - 1) * (2/*offset*/ + 4/*reserved*/);
    var configFrame = configStartFrames + configFrameCount * (channel - 1);
    
    setData16(confFirmware, log, frameStorageConfig, configIndexOffset, configFrame);
	confFirmware.writeLog("Frame " + frameStorageConfig + ", offset " + configIndexOffset +": configFrame = " + configFrame + "\r\n");

    // Service information
    //
	confFirmware.writeLog("Writing service information.\r\n");
	
    var frameServiceConfig = configFrame;
    ptr = 0;
    setData16(confFirmware, log, frameServiceConfig, ptr, 0x0001);   //CFG_Ch_Vers
	confFirmware.writeLog("Frame " + frameServiceConfig + ", offset " + ptr +": CFG_Ch_Vers = 0x0001\r\n");
    ptr += 2;
    setData16(confFirmware, log, frameServiceConfig, ptr, uartId);   //CFG_Ch_Dtype == UARTID?
	confFirmware.writeLog("Frame " + frameServiceConfig + ", offset " + ptr +": uartId = "+ uartId + "\r\n");
    ptr += 2;
	var hashString = storeHash64(confFirmware, log, frameServiceConfig, ptr, subSysID);
	confFirmware.writeLog("Frame " + frameServiceConfig + ", offset " + ptr +": subSysID HASH-64 = " + hashString + "\r\n");
    ptr += 8;
    
    // I/O Modules configuration
    //
	confFirmware.writeLog("Writing I/O modules configuration.\r\n");

    var frameIOConfig = configFrame + 1;

    var parent = module.jsParent();

    for (var i = 0; i < parent.childrenCount(); i++)
    {
        var ioModule = parent.jsChild(i);
        if (ioModule.propertyValue("ModuleFamily") == FamilyAIM || ioModule.propertyValue("ModuleFamily") == FamilyAIFM || 
            ioModule.propertyValue("ModuleFamily") == FamilyAOM || ioModule.propertyValue("ModuleFamily") == FamilyOCM ||
            ioModule.propertyValue("ModuleFamily") == FamilyDIM || ioModule.propertyValue("ModuleFamily") == FamilyDOM)
        {
            if (ioModule.propertyValue("Place") < 1 || ioModule.propertyValue("Place") > ioModulesMaxCount)
            {
                log.writeError("Wrong I/O module place: " + ioModule.propertyValue("StrID") + ", place: " + ioModule.propertyValue("Place") + ", expected 1..14.");
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
                generate_ocm(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDIM)
            {
                generate_dim(confFirmware, ioModule, frame, log);
            }
            if (ioModule.propertyValue("ModuleFamily") == FamilyDOM)
            {
                generate_dom(confFirmware, ioModule, frame, log);
            }
        }
    }
	
	var lanConfigFrame = frameIOConfig + 14;

/*    // Create LANs configuration
    //
    var lanCount = 3;
    var lanStartFrame = 19;
    
    for (var i = 0; i < lanCount; i++)
    {
        var ptr = 0;
        //setData16(confFirmware, log, lanStartFrame + i, ptr, 10);      //LAN configuration
        ptr += 62;
        //reserved
        ptr += 954;
        storeCrc64(confFirmware, log, lanStartFrame + i, 0, ptr, ptr);   //CRC-64
        ptr += 8;
    }
*/

	var txRxConfigFrame = lanConfigFrame + 3;
	var optoCount = 3;
	
	generate_txRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, connectionStorage, optoCount, false/*modeWordGeneration*/);
  
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
    log.writeMessage("MODULE AIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AIMSignalMaxCount = 64;
    
    var defaultTf = valToADC(50, 0, 65535, 0, 0xffff);
    var defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    var defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    var defaultMaxDiff = valToADC(0.5, 0, 5.1, 0, 0xffff);
    
    var inController = module.jsFindChildObjectByMask("*_*_*_*_CTRLIN");
    if (inController == null)
    {
        log.writeWarning("WARNING: no input controller found in " + module.propertyValue("StrID") + "! Using default values.");
    }

    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
    for (var i = 0; i < AIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
        var signal = findSignalByPlace(inController, i, Analog, Input, signalSet, log);
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
            //log.writeMessage("Default place" + i + ": tf = " + defaultTf + ", hi = " + defaultHighBound + ", lo = " + defaultLowBound + ", diff = " + defaultMaxDiff);
            
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
            var maxDifference = valToADC(signal.filteringTime(), signal.lowLimit(), signal.highLimit(), signal.lowADC(), signal.highADC());

            //log.writeMessage("Place" + i + ": tf = " + filternigTime + ", hi = " + signal.highADC() + ", lo = " + signal.lowADC() + ", diff = " + maxDifference);

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
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
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
    var txId = aimTxId;
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;
    
    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr);
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
    log.writeMessage("MODULE AIFM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AIFM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
    return true;

}

// Generate configuration for module AOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aom(confFirmware, module, frame, log, signalSet)
{
    log.writeMessage("MODULE AOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE AOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 0;
    
    var AOMWordCount = 4;                       // total words count
    var AOMSignalsInWordCount = 8;              // signals in a word count
    
    var outController = module.jsFindChildObjectByMask("*_*_*_*_CTRLOUT");
    if (outController == null)
    {
        log.writeWarning("WARNING: no output controller found in " + module.propertyValue("StrID") + "! Using default values.");
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

            if (outController != null)
            {
                var signal = findSignalByPlace(outController, place, Analog, Output, signalSet, log);
                if (signal != null)
                {
                    var outputRangeMode = signal.jsOutputRangeMode();
                    if (outputRangeMode < 0 || outputRangeMode > Mode_05mA)
                    {
                        log.writeError("ERROR: Signal " + signal.propertyValue("StrID") + " - wrong outputRangeMode()! Using default.");
                    }
                    else
                    {
                        mode = outputRangeMode;
                    }
                }
            }
            
            place++;
            
            var bit = c * 2;
            data |= (mode << bit);
        }
        
        //log.writeMessage("Place" + place + ": Word = " + w + " = " + data);
        setData16(confFirmware, log, frame, ptr + w * 2, data);          // InA Filtering time constant
    }
    
    ptr += 120;
    
    // crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
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
    var txId = aomTxId;
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr);
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
        if (s.jsPlace() == place)
        {
            signal = signalSet.getSignalByDeviceStrID(s.propertyValue("StrID"));
            if (signal == null)    
            {
                log.writeWarning("WARNING: Signal " + s.propertyValue("StrID") + " was not found in the signal database!");
            }
            return signal;
        }
    }
    
    log.writeWarning("WARNING: Parent " + parent.propertyValue("StrID")	+ ", no signal with place " + place + " was not found!");
    return null;
}

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE OCM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE OCM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");
    return true;

}

// Generate configuration for module DIM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_dim(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE DIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE DIM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 120;
    
    // crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
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
    var txId = dimTxId;
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr);
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
    log.writeMessage("MODULE DOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame);
	confFirmware.writeLog("MODULE DOM: " + module.propertyValue("StrID") + " Place: " + module.propertyValue("Place") + " Frame: " + frame + "\r\n");

    var ptr = 120;
    
    // crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
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
    var txId = domTxId;
    
    generate_txRxIoConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr);
        ptr = 1016;
    }

    return true;

}
function generate_txRxIoConfig(confFirmware, frame, offset, log, flags, configFrames, dataFrames, txId)
{
    confFirmware.writeLog("Writing TxRx Blocks configuration.\r\n");
	// TxRx Block's configuration structure
    //
    var ptr = offset;
    
    setData16(confFirmware, log, frame, ptr, flags);        // Flags word
	confFirmware.writeLog("Frame " + frame + ", offset " + ptr +": flags = "+ flags + "\r\n");
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, configFrames); // Configuration words quantity
	confFirmware.writeLog("Frame " + frame + ", offset " + ptr +": configFrames = "+ configFrames + "\r\n");
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, dataFrames);   // Data words quantity
	confFirmware.writeLog("Frame " + frame + ", offset " + ptr +": dataFrames = "+ dataFrames + "\r\n");
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, txId);         // Tx ID
	confFirmware.writeLog("Frame " + frame + ", offset " + ptr +": txId = "+ txId + "\r\n");
    ptr += 2;
    
    return true;
}

function generate_txRxOptoConfiguration(confFirmware, log, frame, module, connections, txRxOptoCount, modeWordGeneration)
{
    // Create TxRx Blocks (Opto) configuration
	//
	confFirmware.writeLog("Writing TxRx Blocks (Opto) configuration.\r\n");
		
	confFirmware.writeLog("There are " + connections.count() + " connections in the project.\r\n");
	
	for (var c = 0; c < connections.count(); c++)
	{
		var connection = connections.jsGet(c);
		if (connection == null)
		{
			continue;
		}
	
		if (connection.propertyValue("Device1StrID") == module.propertyValue("StrID") || 
				connection.propertyValue("Device2StrID") == module.propertyValue("StrID"))
		{
			confFirmware.writeLog("Connection " + connection.propertyValue("Caption") + ":" + 
				connection.propertyValue("Device1StrID") + ":" + connection.propertyValue("Device1Port") + " <=> " +
				connection.propertyValue("Device2StrID") + ":" + connection.propertyValue("Device2Port") + "\r\n");
			
		}
	}
	
    /*for (var i = 0; i < txRxOptoCount; i++)
    {
        var ptr = (0 + i) * 2;
		var startAddress = 0;
        setData16(confFirmware, log, frame, ptr, startAddress); 
		confFirmware.writeLog("generate_txRxOptoConfiguration: Frame " + frame + ", offset " + ptr + ": TX startAddress for TxRx Block (Opto) " + (i + 1) + " = " + startAddress + "\r\n");
		
        ptr = (5 + i) * 2;
		var txWordsQuantity = 0;
        setData16(confFirmware, log, frame, ptr, txWordsQuantity);
		confFirmware.writeLog("generate_txRxOptoConfiguration: Frame " + frame + ", offset " + ptr +": TX data words quantity for TxRx Block (Opto) " + (i + 1) + " = " + txWordsQuantity + "\r\n");

        ptr = (10 + i) * 2;
		var id = 0;
        setData16(confFirmware, log, frame, ptr, id);       //Start address
		confFirmware.writeLog("generate_txRxOptoConfiguration: Frame " + frame + ", offset " + ptr +": TX id for TxRx Block (Opto) " + (i + 1) + " = " + id + "\r\n");

        ptr = (15 + i) * 2;
		var rxWordsQuantity = 0;
        setData16(confFirmware, log, frame, ptr, rxWordsQuantity);
		confFirmware.writeLog("generate_txRxOptoConfiguration: Frame " + frame + ", offset " + ptr +": RX data words quantity for TxRx Block (Opto) " + (i + 1) + " = " + rxWordsQuantity + "\r\n");
		
		ptr = 20 * 2;
		
		var txEn = 1;	//1 - enabled, 0 - disabled
		var txStandard = 0;	//0 - rs232, 1 - rs485
		
		var txMode = (txEn << 1) | txStandard;
		txMode <<= (i * 2);
		
		if (modeWordGeneration == true)
		{
			var allModes = confFirmware.data16(frame, ptr);
			allModes |= txMode;
			setData16(confFirmware, log, frame, ptr, allModes);
		}
    }
	
	if (modeWordGeneration == true)
	{
		var allModes = confFirmware.data16(frame, ptr);
		allModes |= txMode;
		confFirmware.writeLog("generate_txRxOptoConfiguration: Frame " + frame + ", offset " + ptr +": RS mode configuration = " + allModes + "\r\n");
	}*/
 
}