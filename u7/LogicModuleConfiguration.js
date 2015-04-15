var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var WorkstationType = 6;
var SoftwareType = 7;
var SignalType = 8;

var	ModuleTypeLm = 1;
var	ModuleTypeAim = 2;
var	ModuleTypeAom = 3;
var	ModuleTypeDim = 4;
var	ModuleTypeDom = 5;
var	ModuleTypeAifm = 6;
var	ModuleTypeOcm = 7;

var DiagDiscrete = 0;
var DiagAnalog = 1;
var InputDiscrete = 2;
var InputAnalog = 3;
var OutputDiscrete = 4;
var OutputAnalog = 5;

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

function(root, confCollection, log, signalSet)
{
    log.writeMessage("Start LogicModuleConfiguration", false);

    var result = true;

    result = module_lm_1(root, confCollection, log, signalSet);

    if (result == false)
    {
        return false;
    }

    log.writeMessage("Finish LogicModuleConfiguration", false);

    return result;
}

function setData8(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData8(frameIndex, offset, data) == false)
    {
        log.writeError("Error: SetData8, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!", false, true);
        return false;
    }
}

function setData16(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData16(frameIndex, offset, data) == false)
    {
        log.writeError("Error: SetData16, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!", false, true);
        return false;
    }
}

function setData32(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData32(frameIndex, offset, data) == false)
    {
        log.writeError("Error: SetData32, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!", false, true);
        return false;
    }
}

function storeCrc64(confFirmware, log, frameIndex, start, count, offset)
{
    if (confFirmware.storeCrc64(frameIndex, start, count, offset) == false)
    {
        log.writeError("Error: StoreCrc64, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!", false, true);
        return false;
    }
}


function module_lm_1(device, confCollection, log, signalSet)
{
    if (device.jsDeviceType() == ModuleType)
    {
        if (device.ConfType == "LM-1")
        {
            log.writeMessage("MODULE LM-1: " + device.StrID, false);

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, confCollection, log, signalSet);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1(child, confCollection, log, signalSet) == false)
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
function generate_lm_1_rev3(module, confCollection, log, signalSet)
{
    // Variables
    //
    var subSysID = module.SubsysID;
    var confIndex = module.ConfIndex;
    var frameSize = 1016;
    var frameCount = 22;                // Check it !!!!
    var uartId = 456;                   // Check it !!!!

    var confFirmware = confCollection.jsGet("LM-1", subSysID, uartId, frameSize, frameCount);

    // Generation
    //

    // EXAMPLES                  
    // To write byte to specific frame
    //setData8(confFirmware, log, 0, 1015, 0x88);
    //setData16(confFirmware, log, 0, 1014, 0x9129);
    //setData32(confFirmware, log, 0, 1012, 0xA123456A);

    // Create I/O Modules configuration (Frames 2..15)
    //
    var ioModulesStartFrame = 2;
    var ioModulesMaxCount = 14;

    var parent = module.jsParent();

    for (var i = 0; i < parent.childrenCount(); i++)
    {
        var ioModule = parent.jsChild(i);
        if (ioModule.ConfType == "AIM" || ioModule.ConfType == "AIFM" || ioModule.ConfType == "AOM" || ioModule.ConfType == "OCM" || ioModule.ConfType == "DIM"|| ioModule.ConfType == "DOM")
        {
            var frame = ioModulesStartFrame + ioModule.Place - 1;
            if (frame < ioModulesStartFrame || frame >= ioModulesStartFrame + ioModulesMaxCount)
            {
                log.writeError("Wrong I/O module place: " + ioModule.StrID + ", place: " + ioModule.Place + ", expected 1..14.", false, true);
                return false;
            }
            
            if (ioModule.ConfType == "AIM")
            {
                generate_aim(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.ConfType == "AIFM")
            {
                generate_aifm(confFirmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "AOM")
            {
                generate_aom(confFirmware, ioModule, frame, log, signalSet);
            }
            if (ioModule.ConfType == "OCM")
            {
                generate_ocm(confFirmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "DIM")
            {
                generate_dim(confFirmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "DOM")
            {
                generate_dom(confFirmware, ioModule, frame, log);
            }
        }
    }
    /*
    // Create TxRx Blocks (Opto) configuration
    //
    var txRxOptoCount = 3;
    var txRxOptoStartFrame = 16;
    
    for (var i = 0; i < txRxOptoCount; i++)
    {
        var ptr = 0;
        setData16(confFirmware, log, txRxOptoStartFrame + i, ptr, 10);       //Start address
        ptr += 2;
        setData16(confFirmware, log, txRxOptoStartFrame + i, ptr, 20);           //Quantity of words
        ptr += 2;
        setData16(confFirmware, log, txRxOptoStartFrame + i, ptr, 30);           //Tx ID
        ptr += 2;
        setData16(confFirmware, log, txRxOptoStartFrame + i, ptr, 40);           //Quantity of words
        ptr += 2;
        //reserved
        ptr += 1008;
        storeCrc64(confFirmware, log, txRxOptoStartFrame + i, 0, ptr, ptr);  //CRC-64
        ptr += 8;
    }
    
    // Create LANs configuration
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
    log.writeMessage("MODULE AIM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);

    var ptr = 0;
    
    var AIMSignalMaxCount = 32;
    
    var defaultTf = valToADC(50, 0, 65535, 0, 0xffff);
    var defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    var defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    var defaultMaxDiff = valToADC(0.5, 0, 5.1, 0, 0xffff);

    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
    for (var i = 0; i < AIMSignalMaxCount; i++)
    {
        // find a signal with Place = i
        //
        var signal = null;
        
        for (var j = 0; j < module.childrenCount(); j++)
        {
            var s = module.jsChild(j);
            
            if (s.jsDeviceType() != SignalType)
            {
                continue;
            }
            if (s.jsType() != InputAnalog)
            {
                continue;
            }
            if (s.jsPlace() == i)
            {
                log.writeMessage("AIM InputSignal: " + s.StrID, false);
                
                signal = signalSet.getSignalByDeviceStrID(s.StrID);
                if (signal == null)    
                {
                    log.writeWarning("WARNING: Signal " + s.StrID + " was not found in the signal database! Using default.", false, true);
                }
                break;
            }
        }
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
            log.writeMessage("Default place" + i + ": tf = " + defaultTf + ", hi = " + defaultHighBound + ", lo = " + defaultLowBound + ", diff = " + defaultMaxDiff, false);
            
            setData16(confFirmware, log, frame, ptr, defaultTf);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultHighBound);         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultLowBound);          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultMaxDiff);      // InA MaxDiff
            ptr += 2;
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

            log.writeMessage("Place" + i + ": tf = " + filternigTime + ", hi = " + signal.highADC() + ", lo = " + signal.lowADC() + ", diff = " + maxDifference, false);

            setData16(confFirmware, log, frame, ptr, filternigTime);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.highADC());         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.lowADC());          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, maxDifference);      // InA MaxDiff
            ptr += 2;
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
    
    // crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;
    
    // reserved
    ptr += 120;
   
    // final crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;
    
    //reserved
    ptr += 360;

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
    
    generate_txRxConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;
    
    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr, false, true);
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
    log.writeMessage("MODULE AIFM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);
    return true;

}

// Generate configuration for module AOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aom(confFirmware, module, frame, log, signalSet)
{
    log.writeMessage("MODULE AOM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);

    var ptr = 0;
    
    var AOMWordCount = 4;                       // total words count
    var AOMSignalsInWordCount = 8;              // signals in a word count
    
    // ------------------------------------------ I/O Module configuration (640 bytes) ---------------------------------
    //
    var place = 0;
    
    for (var w = 0; w < AOMWordCount; w++)
    {
        var data = 0;
        
        for (var c = 0; c < AOMSignalsInWordCount; c++)
        {
            // find a signal with Place = place
            //
            var signal = null;
            
            for (var j = 0; j < module.childrenCount(); j++)
            {
                var s = module.jsChild(j);
                
                if (s.jsDeviceType() != SignalType)
                {
                    continue;
                }
                if (s.jsType() != OutputAnalog)
                {
                    continue;
                }
                if (s.jsPlace() == place)
                {
                    log.writeMessage("AOM OutputSignal: " + s.StrID, false);
                    
                    signal = signalSet.getSignalByDeviceStrID(s.StrID);
                    if (signal == null)    
                    {
                        log.writeWarning("WARNING: Signal " + s.StrID + " was not found in the signal database! Using default.", false, true);
                    }
                    break;
                }
            }
            
            place++;
        
            var mode = Mode_05V;    //default
           
            if (signal != null)
            {
                var outputRangeMode = signal.jsOutputRangeMode();
                if (outputRangeMode < 0 || outputRangeMode > Mode_05mA)
                {
                    log.writeError("ERROR: Signal " + s.StrID + " - wrong outputRangeMode()! Using default.", false, true);
                }
                else
                {
                    mode = outputRangeMode;
                }
            }
            
            var bit = c * 2;
            data |= (mode << bit);
            
        }
        
        log.writeMessage("Place" + place + ": Word = " + w + " = " + data, false);
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
    
    generate_txRxConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr, false, true);
        ptr = 1016;
    }

    return true;

}

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE OCM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);
    return true;

}

// Generate configuration for module DIM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_dim(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE DIM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);

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
    
    generate_txRxConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr, false, true);
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
    log.writeMessage("MODULE DOM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame, false);

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
    
    generate_txRxConfig(confFirmware, frame, ptr, log, flags, configFramesQuantity, dataFramesQuantity, txId);
    ptr += 8;

    // assert if we not on the correct place
    //
    if (ptr != 1016)
    {
        log.writeWarning("WARNING!!! PTR != 1016!!! " + ptr, false, true);
        ptr = 1016;
    }

    return true;

}
function generate_txRxConfig(confFirmware, frame, offset, log, flags, configFrames, dataFrames, txId)
{
    // TxRx Block's configuration structure
    //
    var ptr = offset;
    
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
