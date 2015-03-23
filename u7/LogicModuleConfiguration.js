
var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var DiagSignalType = 6;

function(root, confCollection, log)
{
    log.writeMessage("Start LogicModuleConfiguration");

    var result = true;

    result = module_lm_1(root, confCollection, log);

    if (result == false)
    {
        return false;
    }

    log.writeMessage("Finish LogicModuleConfiguration");

    return result;
}


function module_lm_1(device, confCollection, log)
{
    if (device.jsDeviceType() == ModuleType)
    {
        if (device.ConfType == "LM-1")
        {
            log.writeMessage("MODULE LM-1: " + device.StrID);

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, confCollection, log);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        if (module_lm_1(child, confCollection, log) == false)
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
function generate_lm_1_rev3(module, confCollection, log)
{
    // Variables
    //
    var confName = module.ConfName;
    var confIndex = module.ConfIndex;
    var frameSize = 1024;
    var frameCount = 22;                // Check it !!!!
    var uartId = 456;                   // Check it !!!!

    var confFirmmware = confCollection.jsGet("LM-1", confName, uartId, frameSize, frameCount);

    // Generation
    //

    // EXAMPLES                  
    // To write byte to specific frame
    //confFirmmware.setData8(0, 8, 0x88);
    //confFirmmware.setData16(0, 9, 0x9129);
    //confFirmmware.setData32(0, 11, 0xA123456A);

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
                generate_aim(confFirmmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "AIFM")
            {
                generate_aifm(confFirmmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "AOM")
            {
                generate_aom(confFirmmware, ioModule, frame, log);
            }
            if (ioModule.ConfType == "OCM")
            {
                generate_ocm(confFirmmware, ioModule, frame, log);
            }
        }
    }
    
    // Create TxRx Blocks (Opto) configuration
    //
    var txRxOptoCount = 3;
    var txRxOptoStartFrame = 16;
    
    for (var i = 0; i < txRxOptoCount; i++)
    {
        var ptr = 0;
        confFirmmware.setData16(txRxOptoStartFrame + i, ptr, 10);       //Start address
        ptr += 2;
        confFirmmware.setData16(txRxOptoStartFrame + i, ptr, 20);           //Quantity of words
        ptr += 2;
        confFirmmware.setData16(txRxOptoStartFrame + i, ptr, 30);           //Tx ID
        ptr += 2;
        confFirmmware.setData16(txRxOptoStartFrame + i, ptr, 40);           //Quantity of words
        ptr += 2;
        //reserved
        ptr += 1008;
        confFirmmware.storeCrc64(txRxOptoStartFrame + i, 0, ptr, ptr);  //CRC-64
        ptr += 8;
    }
    
    // Create LANs configuration
    //
    var lanCount = 3;
    var lanStartFrame = 19;
    
    for (var i = 0; i < lanCount; i++)
    {
        var ptr = 0;
        //confFirmmware.setData16(lanStartFrame + i, ptr, 10);      //LAN configuration
        ptr += 62;
        //reserved
        ptr += 954;
        confFirmmware.storeCrc64(lanStartFrame + i, 0, ptr, ptr);   //CRC-64
        ptr += 8;
    }

    return true;
}

// Generate configuration for module AIM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aim(confFirmmware, module, frame, log)
{
    log.writeMessage("AIM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);

    var ptr = 0;

    // I/O Module configuration
    //
    // i/o module data... 640 bytes
    ptr += 640;
    
    confFirmmware.storeCrc64(frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;

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
    
    generate_txRxConfig(confFirmmware, frame, ptr, flags, log);
    ptr += 12;
    
    //reserved
    ptr += 356;

    confFirmmware.storeCrc64(frame, 0, ptr, ptr);   //CRC-64
    ptr += 8;
    
    return true;
}

// Generate configuration for module AIFM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aifm(confFirmmware, module, frame, log)
{
    log.writeMessage("AIFM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

// Generate configuration for module AOM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_aom(confFirmmware, module, frame, log)
{
    log.writeMessage("AOM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

// Generate configuration for module OCM
// module - Hardware::DeviceModule (LM-1)
// frame - Number of frame to generate
//
//
function generate_ocm(confFirmmware, module, frame, log)
{
    log.writeMessage("AIFM: " + module.StrID + " Place: " + module.Place + " Frame: " + frame);
    return true;

}

function generate_txRxConfig(confFirmmware, frame, offset, flags, log)
{
    // TxRx Block's configuration structure
    //
    var ptr = offset;
    
    confFirmmware.setData16(frame, ptr, flags);     // Flags word
    ptr += 2;
    confFirmmware.setData16(frame, ptr, 24);        // Configuration words quantity
    ptr += 2;
    confFirmmware.setData16(frame, ptr, 24);        // Data words quantity
    ptr += 2;
    confFirmmware.setData16(frame, ptr, 24);        // Tx ID
    ptr += 2;
    
    return true;
}
