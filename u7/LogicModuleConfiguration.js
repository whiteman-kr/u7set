var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var WorkstationType = 6;
var SoftwareType = 7;
var SignalType = 8;


var DiagDiscrete = 0;
var DiagAnalog = 1;
var InputDiscrete = 2;
var InputAnalog = 3;
var OutputDiscrete = 4;
var OutputAnalog = 5;


function(root, confCollection, log, signalSet)
{
    log.writeMessage("Start LogicModuleConfiguration");

    var result = true;

    result = module_lm_1(root, confCollection, log, signalSet);

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
        log.writeMessage("Error: SetData8, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function setData16(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData16(frameIndex, offset, data) == false)
    {
        log.writeMessage("Error: SetData16, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function setData32(confFirmware, log, frameIndex, offset, data)
{
    if (confFirmware.setData32(frameIndex, offset, data) == false)
    {
        log.writeMessage("Error: SetData32, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}

function storeCrc64(confFirmware, log, frameIndex, start, count, offset)
{
    if (confFirmware.storeCrc64(frameIndex, start, count, offset) == false)
    {
        log.writeMessage("Error: StoreCrc64, Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range!");
        return false;
    }
}


function module_lm_1(device, confCollection, log, signalSet)
{
    if (device.jsDeviceType() == ModuleType)
    {
        if (device.ConfType == "LM-1")
        {
            log.writeMessage("MODULE LM-1: " + device.StrID);

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
    var confName = module.ConfName;
    var confIndex = module.ConfIndex;
    var frameSize = 1016;
    var frameCount = 22;                // Check it !!!!
    var uartId = 456;                   // Check it !!!!

    var confFirmware = confCollection.jsGet("LM-1", confName, uartId, frameSize, frameCount);

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
        if (ioModule.ConfType == "AIM" || ioModule.ConfType == "AIFM" || ioModule.ConfType == "AOM" || ioModule.ConfType == "OCM")
        {
            var frame = ioModulesStartFrame + ioModule.Place - 1;
            if (frame < ioModulesStartFrame || frame >= ioModulesStartFrame + ioModulesMaxCount)
            {
                log.writeMessage("Wrong I/O module place: " + ioModule.StrID + ", place: " + ioModule.Place + ", expected 1..14.");
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
                generate_aom(confFirmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "OCM")
            {
                generate_ocm(confFirmware, ioModule, frame, log);
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
    log.writeMessage("MODULE AIM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);

    var ptr = 0;
    
    var AIMSignalMaxCount = 32;
    
    var defaultTf = valToADC(50, 0, 65535, 0, 0xffff);
    var defaultHighBound = valToADC(5.1, 0, 5.1, 0, 0xffff);
    var defaultLowBound = valToADC(0, 0, 5.1, 0, 0xffff);
    var defaultMaxDiff = valToADC(0.5, 0, 5.1, 0, 0xffff);
    
    //
    // WARNING!!! "Signal" object has no Place property now. So adding only existing signals in order!!!
    //

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
                log.writeMessage("AIM InputSignal: " + s.StrID);
                
                signal = signalSet.getSignalByDeviceStrID(s.StrID);
                if (signal == null)    
                {
                    log.writeMessage("WARNING: Signal " + s.StrID + " was not found in the signal database! Using default.");
                }
                break;
            }
        }
        
        if (signal == null)
        {
            // Generate default values, there is no signal on this place
            //
            log.writeMessage("Default place" + i + ": tf = " + defaultTf + ", hi = " + defaultHighBound + ", lo = " + defaultLowBound + ", diff = " + defaultMaxDiff);
            
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
            log.writeMessage("Place" + i + ": tf = " + defaultTf + ", hi = " + signal.highADC() + ", lo = " + signal.lowADC() + ", diff = " + defaultMaxDiff);

            setData16(confFirmware, log, frame, ptr, defaultTf);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.highADC());         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.lowADC());          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultMaxDiff);      // InA MaxDiff
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultTf);          // InA Filtering time constant
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.highADC());         // InA High bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, signal.lowADC());          // InA Low Bound
            ptr += 2;
            setData16(confFirmware, log, frame, ptr, defaultMaxDiff);      // InA MaxDiff
            ptr += 2;
        }
    }
    
    // crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;
    
    // reserved
    ptr += 120;
    
    // assert if we not on the correct place
    //
    if (ptr != 640)
    {
        log.writeMessage("WARNING!!! PTR != 640!!! " + ptr);
        ptr = 640;
    }
   
    // final crc
    storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;
    
    // ------------------------------------------ TX/RX Config (12 bytes) ---------------------------------
    //

    //  Flags word
    //
    var configEnableFlag = true;
    var dataEnableFlag = true;
    var dataReceiveEnableFlag = true;
    
    var flags = 0;
    if (configEnableFlag == true)
        flags |= 1;
    if (dataEnableFlag == true)
        flags |= 2;
    if (dataReceiveEnableFlag == true)
        flags |= 4;
    
    generate_txRxConfig(confFirmware, frame, ptr, flags, log);
    ptr += 12;
    
    //reserved
    ptr += 356;

    //storeCrc64(confFirmware, log, frame, 0, ptr, ptr);   //CRC-64, calculated by mconf
    //ptr += 8;
  
    return true;
}

// Generate configuration for module AIFM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aifm(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE AIFM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

// Generate configuration for module AOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aom(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE AOM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmware, module, frame, log)
{
    log.writeMessage("MODULE OCM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

function generate_txRxConfig(confFirmware, frame, offset, flags, log)
{
    // TxRx Block's configuration structure
    //
    var ptr = offset;
    
    setData16(confFirmware, log, frame, ptr, flags);     // Flags word
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, 24);        // Configuration words quantity
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, 24);        // Data words quantity
    ptr += 2;
    setData16(confFirmware, log, frame, ptr, 24);        // Tx ID
    ptr += 2;
    
    return true;
}
