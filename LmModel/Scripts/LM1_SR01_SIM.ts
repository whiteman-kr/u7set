
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

	addOutputParamWord(opIndex: number, value: number): boolean;
	addOutputParamFloat(opIndex: number, value: number): boolean;
	addOutputParamSignedInt(opIndex: number, value: number): boolean;
}

function check_param_exist(instance: ComponentInstance, opIndex: number, paramName: string) : boolean
{
	if (instance.paramExists(opIndex) == false)
	{
		throw new Error("Param " + paramName + " is not found.");
	}    
	
	return true;
}

function check_param_range(paramValue: number, minValue: number, maxValue: number, paramName: string) : boolean
{
	if (paramValue < minValue ||
		paramValue > maxValue)
	{
		throw new Error("Param " + paramName + " is out of range, value = " + paramValue + 
						", range = [" + minValue + ", " + maxValue + "].");
	}    
	
	return true;
}

//
//	LOGIC, OpCode 1
//
function afb_logic(instance: ComponentInstance) : boolean
{
	// Define input opIndexes
	//
	const i_oprd_quant = 0;
	const i_bus_width = 1;
	const i_conf = 2;
	const i_input_0 = 3;
	const o_result = 20;
	
	// Get params,  check_param throws exception in case of error
	//
	check_param_exist(instance, i_oprd_quant, "i_oprd_quant");
	check_param_exist(instance, i_bus_width, "i_bus_width");
	check_param_exist(instance, i_conf, "i_conf");
	
	let oprdQuant: ComponentParam = instance.param(i_oprd_quant);
	let busWidth: ComponentParam = instance.param(i_bus_width);
	let conf: ComponentParam = instance.param(i_conf);

	check_param_range(oprdQuant.AsWord, 1, 16, "i_oprd_quant");
	check_param_range(busWidth.AsWord, 1, 16, "i_bus_width");	
	
	// Logic
	//
	let inputs: ComponentParam[] = new Array(oprdQuant.AsWord);
	for (var i = 0; i < inputs.length; i++)
	{
		check_param_exist(instance, i_input_0 + i, "i_oprd_" + i);
		inputs[i] = instance.param(i_input_0 + i);
	}

	let result: number = inputs[0].AsWord;
	
	switch (conf.AsWord)
	{
		case 1: // AND
			{
				for (var i = 1; i < inputs.length; i++)
				{
					result &= inputs[i].AsWord;
				}
			}
			break;        
		case 2: // OR
			{
				for (var i = 1; i < inputs.length; i++)
				{
					result |= inputs[i].AsWord;
				}
			}
			break;
		case 2: // XOR
			{
				for (var i = 1; i < inputs.length; i++)
				{
					result ^= inputs[i].AsWord;
				}			
			}
			break;        
		default:
			throw new Error("Unknown AFB configuration: " + conf);
	}

	// Save result
	//	
	instance.addOutputParamWord(o_result, result);
	
	return true;
}

//
//	NOT, OpCode 2
//
function afb_not(instance: ComponentInstance) : boolean
{
	// Define input opIndexes
	//
	const i_oprd = 0;
	const o_result = 2;
	
	// Get params,  check_param throws exception in case of error
	//
	check_param_exist(instance, i_oprd, "i_oprd");
	
	// Logic
	//
	let input: ComponentParam = instance.param(i_oprd);
	let result: number = ~input.AsWord;

	// Save result
	//	
	instance.addOutputParamWord(o_result, result);
	
	return true;
}
