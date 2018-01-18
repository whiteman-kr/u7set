//
// Library interfaces, constatns, functions
//
const CommandWidth = 10;				// "mov       "
const CommandWidthToComment = 48;		// "mov       ......." Comment

enum CyclePhase
{
	IdrPhase = 0,
	AlpPhase = 1,
	ODT = 2,
	ST = 3
};

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

	addParam(opIndex: number, value: ComponentParam): boolean;
	addParamWord(opIndex: number, value: number): boolean;
	addParamFloat(opIndex: number, value: number): boolean;
	addParamSignedInt(opIndex: number, value: number): boolean;
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

function hex(value: number, width: number) : string
{
	return rightJustified(value.toString(16), width, "0") + "h";
}

//
//  AfbComponent interface for cpp class Afb::AfbComponent
//
interface AfbComponent
{
	OpCode: number;
	Caption: string;
	MaxInstCount: number;
	SimulationFunc: string;

	pinExists(pinOpIndex: number) : boolean;
	pinCaption(pinOpIndex: number) : boolean;
}

//
// Device Emultaor interface for cpp class DeviceEmulator
//
interface DeviceEmulator
{
	// Properties
	//
	AppStartAddress : number;			// ALP phase start address
	Phase : CyclePhase;					// Current ApplicationUnit phase
	ProgramCounter : number;			// Current ProgramCounter

	// Functions
	//
	createComponentParam() : ComponentParam;
	setAfbParam(afbOpCode: number, instanceNo: number, param: ComponentParam) : boolean;
	afbComponent(afbOpCode: number) : AfbComponent;
	afbComponentInstance(afbOpCode: number, instanceNo: number) : ComponentInstance;

	// RAM access
	//
	writeRamBit(offsetW: number, bitNo: number, data: number) : boolean;
	readRamBit(offsetW: number, bitNo: number) : number;	// returns quint16

	writeRamWord(offsetW: number, data: number) : boolean;
	readRamWord(offsetW: number) : number;					// returns quint16

	writeRamDword(offsetW: number, data: number) : boolean;
	readRamDword(offsetW: number) : number;					// returns quint32

	// App code memory access
	//
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
	Word2: number;

	Dword0: number;
	Dword1: number;
}

//
// Logic Unit command pasring and simylation functions
//

// Command: nop
// Code: 1
//
function parse_nop(device: DeviceEmulator, command: Command) : string
{
	command.Size = 1;	// 1 word
	command.AsString = command.Caption;
	return "";
}

function command_nop(device: DeviceEmulator, command: Command) : string
{
	return "NotImplemented";
}


// Command: startafb
// Code: 2
//
function parse_startafb(device: DeviceEmulator, command: Command) : string
{
	command.Size = 2;
	
	command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F;		// Lowest 6 bit
	command.AfbInstance = device.getWord(command.Offset + 1) >>> 6;			// Highest 10 bits

	var afb = device.afbComponent(command.AfbOpCode);
	if (afb == null)
	{
		return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
	}

	if (afb.SimulationFunc.length == 0)
	{
		return "Simultaion function is not found";
	}

	if (command.AfbInstance >= afb.MaxInstCount)
	{
		return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
	}

	// startafb LOGIC.0
	//
	command.AsString = leftJustified(command.Caption, CommandWidth, " ") +  afb.Caption + "."  + command.AfbInstance;

	return "";
}

function asdf(paramm: string)
{
	return "ASDFTF" + paramm;
}

function command_startafb(device: DeviceEmulator, command: Command) : string
{
	var afb = device.afbComponent(command.AfbOpCode);
	if (afb == null)
	{
		return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
	}

	var afbInstance = device.afbComponentInstance(command.AfbOpCode, command.AfbInstance);
	if (afbInstance == null)
	{
		return "Cannot find afbInstance with OpCode " + command.AfbOpCode + ", InstanceNo " + command.AfbInstance;
	}

	var simulationFuncString = "(function(instance){ return " + afb.SimulationFunc + "(instance); })";
	var functionVar = eval(simulationFuncString);

	var result: string = functionVar(afbInstance);

	return result;
}

// Command: stop
// Code: 3
//
function parse_stop(device: DeviceEmulator, command: Command) : string
{
	command.Size = 1;	// 1 word
	command.AsString = command.Caption;
	return "";
}

function command_stop(device: DeviceEmulator, command: Command) : string
{
	if (device.Phase == CyclePhase.IdrPhase)
	{
		device.Phase = CyclePhase.AlpPhase;
		device.ProgramCounter = device.AppStartAddress;
		return "";
	}

	if (device.Phase == CyclePhase.AlpPhase)
	{
		device.Phase = CyclePhase.ODT;
		return "";
	}

	return "Command stop is cannot be run in phase " + device.Phase.toString;
}

// Command: movmem
// Code: 5
//
function parse_movmem(device: DeviceEmulator, command: Command) : string
{
	command.Size = 4;
	
	command.Word0 = device.getWord(command.Offset + 1);		// Word0 - adderess2
	command.Word1 = device.getWord(command.Offset + 2);		// Word1 - adderess1
	command.Word2 = device.getWord(command.Offset + 3);		// Words to move

	// movmem     B402h, DD02h , #2
	command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
						hex(command.Word0, 4) + ", " +
						hex(command.Word1, 4) + ", " +
						hex(command.Word2, 4);

	return "";
}

function command_movmem(device: DeviceEmulator, command: Command) : string
{
	var size: number = command.Word2;
	var src: number = command.Word1;
	var dst: number = command.Word0;

	for (var i = 0; i < size; i++) 
	{ 
		var data = device.readRamWord(src + i);
		device.writeRamWord(dst + i, data);
	}

	return "";
}

// Command: movc
// Code: 6
//
function parse_movc(device: DeviceEmulator, command: Command) : string
{
	command.Size = 3;
	
	command.Word0 = device.getWord(command.Offset + 1);		// Word0 - address
	command.Word1 = device.getWord(command.Offset + 2);		// Word1 - data

	// movc     B402h, #0123h
	command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
						hex(command.Word0, 4) + ", #" +
						hex(command.Word1, 4);

	command.AsString =  leftJustified(command.AsString, CommandWidthToComment, " ") +  
						"-- " + hex(command.Word0, 4) + " <= " + hex(command.Word1, 4) + " (" + command.Word1 + ")";						

	return "";
}

function command_movc(device: DeviceEmulator, command: Command) : string
{
	device.writeRamWord(command.Word0, command.Word1);
	return "";
}

// Command: movbc
// Code: 7
//
function parse_movbc(device: DeviceEmulator, command: Command) : string
{
	command.Size = 4;
	
	command.Word0 = device.getWord(command.Offset + 1);		// Word0 - data address
	command.Word1 = device.getWord(command.Offset + 2);		// Word1 - data
	command.BitNo0 = device.getWord(command.Offset + 3);	// BitNo

	check_param_range(command.BitNo0, 0, 15, "BitNo");

	// MOVBC     B402h[0], #0
	command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
						hex(command.Word0, 4) + "[" + command.BitNo0 + "]" + ", #" + command.Word1;

	return "";
}

function command_movbc(device: DeviceEmulator, command: Command) : string
{
	var value : number = device.getWord(command.Word0);
	device.writeRamBit(command.Word0, command.BitNo0, command.Word1);
	return "";
}

// Command: wrfbc
// Code: 10
//
function parse_wrfbc(device: DeviceEmulator, command: Command) : string
{
	command.Size = 3;
	
	command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F;		// Lowest 6 bit
	command.AfbInstance = device.getWord(command.Offset + 1) >>> 6;			// Highest 10 bits
	command.AfbPinOpCode = device.getWord(command.Offset + 1) & 0b111111;	// Lowest 6 bit

	command.Word0 = device.getWord(command.Offset + 2);						// Word0 - data address

	var afb = device.afbComponent(command.AfbOpCode);
	if (afb == null)
	{
		return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
	}

	if (command.AfbInstance >= afb.MaxInstCount)
	{
		return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
	}

	var pinCaption = afb.pinCaption(command.AfbPinOpCode);
	
	// wrfbc LOGIC.0[0], #0003h
	command.AsString = leftJustified(command.Caption, CommandWidth, " ") +  
							afb.Caption + "."  + command.AfbInstance + "[" + command.AfbPinOpCode + "], #" +
							hex(command.Word0, 4);
							
	command.AsString =  leftJustified(command.AsString, CommandWidthToComment, " ") +  
						"-- " + pinCaption + " <= " + hex(command.Word0, 4) + " (" + command.Word0 + ")";
	return "";
}

function command_wrfbc(device: DeviceEmulator, command: Command) : string
{
	var param : ComponentParam = device.createComponentParam();
	param.OpIndex = command.AfbPinOpCode;
	param.AsWord = command.Word0;

	var ok = device.setAfbParam(command.AfbOpCode, command.AfbInstance, param);
	if (ok == false)
	{
		return "setAfbParam error";
	}

	return "";
}

// Command: wrfbb
// Code: 11
//
function parse_wrfbb(device: DeviceEmulator, command: Command) : string
{
	command.Size = 4;
	
	command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F;		// Lowest 6 bit
	command.AfbInstance = device.getWord(command.Offset + 1) >>> 6;			// Highest 10 bits
	command.AfbPinOpCode = device.getWord(command.Offset + 1) & 0b111111;	// Lowest 6 bit

	command.Word0 = device.getWord(command.Offset + 2);						// Word0 - data address
	command.BitNo0 = device.getWord(command.Offset + 3);					// BitNo

	// Checks
	//
	var afb = device.afbComponent(command.AfbOpCode);
	if (afb == null)
	{
		return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
	}

	if (command.AfbInstance >= afb.MaxInstCount)
	{
		return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
	}

	check_param_range(command.BitNo0, 0, 15, "BitNo");

	var pinCaption = afb.pinCaption(command.AfbPinOpCode);
	
	// wrfbb LOGIC.0[20], 46083[0]
	command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
						afb.Caption + "."  + command.AfbInstance + "[" + command.AfbPinOpCode + "], " +
						hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
							

	command.AsString =  leftJustified(command.AsString, CommandWidthToComment, " ") +  
						"-- " + 
						afb.Caption + "."  + command.AfbInstance + "[" + pinCaption + "] <=" +
						hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
	return "";
}

function command_wrfbb(device: DeviceEmulator, command: Command) : string
{
	var param : ComponentParam = device.createComponentParam();
	param.OpIndex = command.AfbPinOpCode;
	param.AsWord = device.readRamBit(command.Word0, command.BitNo0);

	var ok = device.setAfbParam(command.AfbOpCode, command.AfbInstance, param);
	if (ok == false)
	{
		return "setAfbParam error";
	}

	return "";
}

// Command: rdfbb
// Code: 12
//
function parse_rdfbb(device: DeviceEmulator, command: Command) : string
{
	command.Size = 4;
	
	command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F;		// Lowest 6 bit
	command.AfbInstance = device.getWord(command.Offset + 1) >>> 6;			// Highest 10 bits
	command.AfbPinOpCode = device.getWord(command.Offset + 1) & 0b111111;	// Lowest 6 bit

	command.Word0 = device.getWord(command.Offset + 2);						// Word0 - data address
	command.BitNo0 = device.getWord(command.Offset + 3);					// BitNo

	// Checks
	//
	var afb = device.afbComponent(command.AfbOpCode);
	if (afb == null)
	{
		return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
	}

	if (command.AfbInstance >= afb.MaxInstCount)
	{
		return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
	}

	check_param_range(command.BitNo0, 0, 15, "BitNo");

	var pinCaption = afb.pinCaption(command.AfbPinOpCode);
	
	// rdfbb 46083[0], LOGIC.0[20]
	command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
						hex(command.Word0, 4) + "[" + command.BitNo0 + "], " +
						afb.Caption + "."  + command.AfbInstance + "[" + command.AfbPinOpCode + "]";
							

	command.AsString =  leftJustified(command.AsString, CommandWidthToComment, " ") +  
						"-- " + 
						hex(command.Word0, 4) + "[" + command.BitNo0 + "] <= " +
						afb.Caption + "."  + command.AfbInstance + "[" + pinCaption + "]";
	return "";
}

function command_rdfbb(device: DeviceEmulator, command: Command) : string
{
	var afbInstance: ComponentInstance = device.afbComponentInstance(command.AfbOpCode, command.AfbInstance);
	if (afbInstance == null)
	{
		return "Cannot find afbInstance with OpCode " + command.AfbOpCode + ", InstanceNo " + command.AfbInstance;
	}

	if (afbInstance.paramExists(command.AfbPinOpCode) == false)
	{
		return "Param is not exist, AfbPinOpIndex " + command.AfbPinOpCode;
	}

	var param : ComponentParam = afbInstance.param(command.AfbPinOpCode);
	device.writeRamBit(command.Word0, command.BitNo0, param.AsWord & 0x01);

	return "";
}

// Command: appstart
// Code: 17
//
function parse_appstart(device: DeviceEmulator, command: Command) : string
{
	command.Size = 2;										// 2 words
	command.Word0 = device.getWord(command.Offset + 1);		// Word0 keeps ALP phase start address
	command.AsString = leftJustified(command.Caption, CommandWidth, " ") + hex(command.Word0, 4);
	return "";
}

function command_appstart(device: DeviceEmulator, command: Command) : string
{
	device.AppStartAddress = command.Word0;
	return "";
}

//
// AFB's simultaion code
//

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
	instance.addParamWord(o_result, result);
	
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
	instance.addParamWord(o_result, result);
	
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
	instance.addParam(o_result, operand1);

	instance.addParamWord(o_overflow, operand1.MathOverflow ? 0x0001 : 0x0000);	
	instance.addParamWord(o_underflow, operand1.MathUnderflow ? 0x0001 : 0x0000);
	instance.addParamWord(o_zero, operand1.MathZero ? 0x0001 : 0x0000);
	instance.addParamWord(o_nan, operand1.MathNan ? 0x0001 : 0x0000);
	instance.addParamWord(o_div_by_zero, operand1.MathDivByZero ? 0x0001 : 0x0000);

	return "";
}
