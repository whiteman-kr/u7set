
interface ComponentParam
{
    OpIndex: number;
	AsWord: number;
    AsFloat: number;
    AsSignedInt: number;
}

interface ComponentInstance 
{
	paramExists(opIndex: number): boolean;
    param(opIndex: number): ComponentParam;
}

function check_param(instance: ComponentInstance, opIndex: number, paramName: string) : boolean
{
    if (instance.paramExists(opIndex) == false)
    {
        throw new Error("Param " + paramName + " is not found.");
    }    

    return true;
}


function afb_logic(instance: ComponentInstance) : string
{
    // Define input opIndexes
    //
    const i_oprd_quant = 0;
    const i_bus_width = 1;
    const i_conf = 2;

    // Get params,  check_param throws exception in case of error
    //
    check_param(instance, i_oprd_quant, "i_oprd_quant");
    check_param(instance, i_bus_width, "i_bus_width");
    check_param(instance, i_conf, "i_conf");

    let oprdQuant: ComponentParam = instance.param(i_oprd_quant);
    let busWidth: ComponentParam = instance.param(i_bus_width);
    let conf: ComponentParam = instance.param(i_conf);

    // Logic
    //
    switch (conf.AsWord)
    {
        case 2: // OR
        {
            return "OR ELEMENT";
        }
        //break;

        default:
        {
            throw new Error("Unknown AFB configuration: " + conf);
        }
    }
}
