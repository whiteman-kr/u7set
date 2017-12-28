
var CommandWidth = 10;			// leftJustified("appstart", CommandWidth, " ") + rightJustified(command.Word0.toString(16), 4, "0") + "h";

interface ComponentParam
{
	OpIndex: number;
	AsWord: number;
	AsFloat: number;
	AsSignedInt: number;

	// Math operations result flags
	//
	MathOverflow: boolean;
	MathUnderflow: boolean;
	MathZero: boolean;
	MathNan: boolean;
	MathDivByZero: boolean;

	// Math operations
	//
	addSignedInteger(operand: ComponentParam) : void;	// +=
	subSignedInteger(operand: ComponentParam) : void;	// -=
	mulSignedInteger(operand: ComponentParam) : void;	// *=
	divSignedInteger(operand: ComponentParam) : void;	// /=

	addFloatingPoint(operand: ComponentParam) : void;	// +=
	subFloatingPoint(operand: ComponentParam) : void;	// -=
	mulFloatingPoint(operand: ComponentParam) : void;	// *=
	divFloatingPoint(operand: ComponentParam) : void;	// /=	
}

interface ComponentInstance 
{
	paramExists(opIndex: number): boolean;
	param(opIndex: number): ComponentParam;

	addOutputParam(opIndex: number, value: ComponentParam): boolean;
	addOutputParamWord(opIndex: number, value: number): boolean;
	addOutputParamFloat(opIndex: number, value: number): boolean;
	addOutputParamSignedInt(opIndex: number, value: number): boolean;
}

// 
// Service function for checking if param exists, if param does not exist the exception is thrown
//
function check_param_exist(instance: ComponentInstance, opIndex: number, paramName: string) : boolean
{
	if (instance.paramExists(opIndex) == false)
	{
		throw new Error("Param " + paramName + " is not found.");
	}    
	
	return true;
}

// 
// Service function for checking param range, if param out of range the exception is thrown
//
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

function leftJustified(str: string, width: number, fill: string) : string
{
	while (str.length < width)
	{
		str +=  fill;
	}	
	return str;
}

function rightJustified(str: string, width: number, fill: string) : string
{
	while (str.length < width)
	{
		str = fill + str
	}	
	return str;
}

//
//  AfbComponent interface for cpp class Afb::AfbComponent
//
interface AfbComponent
{
	OpCode: number;
	Caption: string;
	MaxInstCount: number;
}

//
// Device Emultaor interface for cpp class DeviceEmulator
//
interface DeviceEmulator
{
	afbComponent(afbOpCode: number) : AfbComponent;

	getWord(offset: number) : number;		// Get word (16 bit) by offset from code memory, offset is word aligned
	getDword(offset: number) : number;		// Get double word (32 bit) by offset from code memory, offset is word aligned
}

//
// Logic Module parsed command
//
interface Command
{
	Caption: string;

	Offset: number;
	Size: number;
	AsString: string;
	
	AfbOpCode: number;
	AfbInstance: number;
	AfbPinOpCode: number;

	BitNo0: number;
	BitNo1: number;

	Word0: number;
	Word1: number;

	Dword0: number;
	Dword1: number;
}

// Command: nop
// Code: 1
//
function parse_nop(device: DeviceEmulator, command: Command) : boolean
{
	command.Size = 1;	// 1 word
	command.AsString = command.Caption;
	return true;
}

// Command: wrfbc
// Code: 10
//
function parse_wrfbc(device: DeviceEmulator, command: Command) : boolean
{
	command.Size = 3;
	
	command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F;		// Lowest 6 bit
	command.AfbInstance = device.getWord(command.Offset + 1) >>> 6;			// Highest 10 bits
	command.AfbPinOpCode = device.getWord(command.Offset + 1) & 0b111111;	// Lowest 6 bit

	command.Word0 = device.getWord(command.Offset + 2);						// Word0 - data address

	// WRFBC OR.0[0], #3
	command.AsString = leftJustified(command.Caption, CommandWidth, " ") +  
							command.AfbOpCode + "."  + command.AfbInstance + "[" + command.AfbPinOpCode + "], #" +
							rightJustified(command.Word0.toString(16), 4, "0") + "h";
	return true;
}

// Command: appstart
// Code: 17
//
function parse_appstart(device: DeviceEmulator, command: Command) : boolean
{
	command.Size = 2;										// 2 words
	command.Word0 = device.getWord(command.Offset + 1);		// Word0 keeps ALP phase start address
	command.AsString = leftJustified(command.Caption, CommandWidth, " ") + rightJustified(command.Word0.toString(16), 4, "0") + "h";
	return true;
}


//
//	LOGIC, OpCode 1
//
function afb_logic(instance: ComponentInstance) : string
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
			for (var i = 1; i < inputs.length; i++)
			{
				result &= inputs[i].AsWord;
			}
			break;        
		case 2: // OR
			for (var i = 1; i < inputs.length; i++)
			{
				result |= inputs[i].AsWord;
			}
			break;
		case 2: // XOR
			for (var i = 1; i < inputs.length; i++)
			{
				result ^= inputs[i].AsWord;
			}			
			break;        
		default:
			throw new Error("Unknown AFB configuration: " + conf);
	}

	// Save result
	//	
	instance.addOutputParamWord(o_result, result);
	
	return "";
}

//
//	NOT, OpCode 2
//
function afb_not(instance: ComponentInstance) : string
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
	
	return "";
}

//
//	MATH, OpCode 13
//
function afb_math(instance: ComponentInstance) : string
{
	// Define input opIndexes
	//
	const i_conf = 0;
	const i_1_oprd = 1;
	const i_2_oprd = 3;
	const o_result = 6;
	const o_mat_edi = 8;
	const o_overflow = 9;
	const o_underflow = 10;
	const o_zero = 11;
	const o_nan = 12;
	const o_div_by_zero = 13;
	
	// Get params,  check_param throws exception in case of error
	//
	check_param_exist(instance, i_conf, "i_conf");
	check_param_exist(instance, i_1_oprd, "i_1_oprd");
	check_param_exist(instance, i_2_oprd, "i_2_oprd");
	
	let conf: ComponentParam = instance.param(i_conf);
	let operand1: ComponentParam = instance.param(i_1_oprd);
	let operand2: ComponentParam = instance.param(i_2_oprd);

	// Logic	conf: 1'-'+' (SI),  '2'-'-' (SI),  '3'-'*' (SI),  '4'-'/' (SI), '5'-'+' (FP),  '6'-'-' (FP),  '7'-'*' (FP),  '8'-'/' (FP)   
	//
	switch (conf.AsWord)
	{
		case 1: // SI +
			operand1.addSignedInteger(operand2);
			break;
		 case 2: // SI -
			operand1.subSignedInteger(operand2);
			break;
		case 3: // SI *
			operand1.mulSignedInteger(operand2);
			break;
		case 4: // SI /
			operand1.divSignedInteger(operand2);
			break;
		case 5: // FP +
			operand1.addFloatingPoint(operand2);
			break;
		case 6: // FP -
			operand1.subFloatingPoint(operand2);
			break;
		case 7: // FP *
			operand1.mulFloatingPoint(operand2);
		 	break;
		case 8: // FP /
			operand1.divFloatingPoint(operand2);
		 	break;		
		default:
			throw new Error("Unknown AFB configuration: " + conf.AsSignedInt + ", or this configuration is not implemented yet.");
	}

	// Save result
	//	
	instance.addOutputParam(o_result, operand1);

	instance.addOutputParamWord(o_overflow, operand1.MathOverflow ? 0x0001 : 0x0000);	
	instance.addOutputParamWord(o_underflow, operand1.MathUnderflow ? 0x0001 : 0x0000);
	instance.addOutputParamWord(o_zero, operand1.MathZero ? 0x0001 : 0x0000);
	instance.addOutputParamWord(o_nan, operand1.MathNan ? 0x0001 : 0x0000);
	instance.addOutputParamWord(o_div_by_zero, operand1.MathDivByZero ? 0x0001 : 0x0000);

	return "";
}
