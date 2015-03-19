
var RootType = 0;
var SystemType = 1;
var RackType = 2;
var ChassisType = 3;
var ModuleType = 4;
var ControllerType = 5;
var DiagSignalType = 6;

function(root, log)
{
    log.writeMessage("Start LogicModuleConfiguration");

    var result = true;

    result = module_lm_1(root, log);

    if (result == false)
    {
        return false;
    }

    log.writeMessage("Finish LogicModuleConfiguration");

    return result;
}


function module_lm_1(device, log)
{
    if (device.jsDeviceType() == ModuleType)
    {
        if (device.ConfType == "LM-1")
        {
            log.writeMessage("MODULE LM-1: " + device.StrID);

            // Generate Configuration
            //
            return generate_lm_1_rev3(device, log);
        }
        return true;
    }

    for (var i = 0; i < device.childrenCount(); i++)
    {
        var child = device.jsChild(i);
        module_lm_1(child, log);
    }

    return true;
}

// Generate configuration formodule LM-1, revision 3
//
function generate_lm_1_rev3(module, log)
{
    // Variables
    //
    var confName = module.ConfName;
    var confIndex = module.ConfIndex;
    var frameSize = 1024;
    var frameCount = 64;                // Check it !!!!
    var uartId = 456;                   // Check it !!!!

    // ---------------------------------------------------
    // Frame 0
    //
    var frameIndex = 0;

    // ---------------------------------------------------
    // Frame 1
    //
    frameIndex = 1;

    // CFG Registers LM
    // Type: CFG Registers
    // Name: Учетная запись конфигурации
    //

    // ---------------------------------------------------
    // Frame 2
    //
    frameIndex = 2;

    // CFG Registers LM
    // Type: LM common CFG regs
    // Name: Общие конфигурационные регисты LM
    //

    // ---------------------------------------------------
    // Frame 3
    //
    frameIndex = 3;

    // Opto1
    // Type: TxRxController CFG Registers
    // Name: Optical Channel 1
    //

    // Opto2
    // Type: TxRxController CFG Registers
    // Name: Optical Channel 2
    //

    // Opto3
    // Type: TxRxController CFG Registers
    // Name: Optical Channel 3
    //


    return true;
}
