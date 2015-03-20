
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
        module_lm_1(child, confCollection, log);
    }

    return true;
}

// Generate configuration formodule LM-1
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
    var frameCount = 64;                // Check it !!!!
    var uartId = 456;                   // Check it !!!!

    var confFirmmware = confCollection.jsGet(confName, uartId, frameSize, frameCount);

    // Generation
    //

    // EXAMPLES
    // To write byte tospecific frame
    confFirmmware.setData8(0, 8, 0x88);
    confFirmmware.setData16(0, 9, 0x9129);
    confFirmmware.setData32(0, 11, 0xA123456A);

//    // ---------------------------------------------------
//    // Frame 0
//    //
//    var frameIndex = 0;

//    // ---------------------------------------------------
//    // Frame 1
//    //
//    frameIndex = 1;

//    // CFG Registers LM
//    // Type: CFG Registers
//    // Name: Учетная запись конфигурации
//    //

//    // ---------------------------------------------------
//    // Frame 2
//    //
//    frameIndex = 2;

//    // CFG Registers LM
//    // Type: LM common CFG regs
//    // Name: Общие конфигурационные регисты LM
//    //

//    // ---------------------------------------------------
//    // Frame 3
//    //
//    frameIndex = 3;

//    // Opto1
//    // Type: TxRxController CFG Registers
//    // Name: Optical Channel 1
//    //

//    // Opto2
//    // Type: TxRxController CFG Registers
//    // Name: Optical Channel 2
//    //

//    // Opto3
//    // Type: TxRxController CFG Registers
//    // Name: Optical Channel 3
//    //


    return true;
}
