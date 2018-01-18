//
// Library interfaces, constatns, functions
//
var CommandWidth = 10; // "mov       "
var CommandWidthToComment = 48; // "mov       ......." Comment
var CyclePhase;
(function (CyclePhase) {
    CyclePhase[CyclePhase["IdrPhase"] = 0] = "IdrPhase";
    CyclePhase[CyclePhase["AlpPhase"] = 1] = "AlpPhase";
    CyclePhase[CyclePhase["ODT"] = 2] = "ODT";
    CyclePhase[CyclePhase["ST"] = 3] = "ST";
})(CyclePhase || (CyclePhase = {}));
;
// 
// Service function for checking if param exists, if param does not exist the exception is thrown
//
function check_param_exist(instance, opIndex, paramName) {
    if (instance.paramExists(opIndex) == false) {
        throw new Error("Param " + paramName + " is not found.");
    }
    return true;
}
// 
// Service function for checking param range, if param out of range the exception is thrown
//
function check_param_range(paramValue, minValue, maxValue, paramName) {
    if (paramValue < minValue ||
        paramValue > maxValue) {
        throw new Error("Param " + paramName + " is out of range, value = " + paramValue +
            ", range = [" + minValue + ", " + maxValue + "].");
    }
    return true;
}
function leftJustified(str, width, fill) {
    while (str.length < width) {
        str += fill;
    }
    return str;
}
function rightJustified(str, width, fill) {
    while (str.length < width) {
        str = fill + str;
    }
    return str;
}
function hex(value, width) {
    return rightJustified(value.toString(16), width, "0") + "h";
}
//
// Logic Unit command pasring and simylation functions
//
// Command: nop
// Code: 1
//
function parse_nop(device, command) {
    command.Size = 1; // 1 word
    command.AsString = command.Caption;
    return "";
}
function command_nop(device, command) {
    return "NotImplemented";
}
// Command: startafb
// Code: 2
//
function parse_startafb(device, command) {
    command.Size = 2;
    command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F; // Lowest 6 bit
    command.AfbInstance = device.getWord(command.Offset + 1) >>> 6; // Highest 10 bits
    var afb = device.afbComponent(command.AfbOpCode);
    if (afb == null) {
        return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
    }
    if (afb.SimulationFunc.length == 0) {
        return "Simultaion function is not found";
    }
    if (command.AfbInstance >= afb.MaxInstCount) {
        return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
    }
    // startafb LOGIC.0
    //
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") + afb.Caption + "." + command.AfbInstance;
    return "";
}
function asdf(paramm) {
    return "ASDFTF" + paramm;
}
function command_startafb(device, command) {
    var afb = device.afbComponent(command.AfbOpCode);
    if (afb == null) {
        return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
    }
    var afbInstance = device.afbComponentInstance(command.AfbOpCode, command.AfbInstance);
    if (afbInstance == null) {
        return "Cannot find afbInstance with OpCode " + command.AfbOpCode + ", InstanceNo " + command.AfbInstance;
    }
    var simulationFuncString = "(function(instance){ return " + afb.SimulationFunc + "(instance); })";
    var functionVar = eval(simulationFuncString);
    var result = functionVar(afbInstance);
    return result;
}
// Command: stop
// Code: 3
//
function parse_stop(device, command) {
    command.Size = 1; // 1 word
    command.AsString = command.Caption;
    return "";
}
function command_stop(device, command) {
    if (device.Phase == CyclePhase.IdrPhase) {
        device.Phase = CyclePhase.AlpPhase;
        device.ProgramCounter = device.AppStartAddress;
        return "";
    }
    if (device.Phase == CyclePhase.AlpPhase) {
        device.Phase = CyclePhase.ODT;
        return "";
    }
    return "Command stop is cannot be run in phase " + device.Phase.toString;
}
// Command: movmem
// Code: 5
//
function parse_movmem(device, command) {
    command.Size = 4;
    command.Word0 = device.getWord(command.Offset + 1); // Word0 - adderess2
    command.Word1 = device.getWord(command.Offset + 2); // Word1 - adderess1
    command.Word2 = device.getWord(command.Offset + 3); // Words to move
    // movmem     B402h, DD02h , #2
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        hex(command.Word0, 4) + ", " +
        hex(command.Word1, 4) + ", " +
        hex(command.Word2, 4);
    return "";
}
function command_movmem(device, command) {
    var size = command.Word2;
    var src = command.Word1;
    var dst = command.Word0;
    for (var i = 0; i < size; i++) {
        var data = device.readRamWord(src + i);
        device.writeRamWord(dst + i, data);
    }
    return "";
}
// Command: movc
// Code: 6
//
function parse_movc(device, command) {
    command.Size = 3;
    command.Word0 = device.getWord(command.Offset + 1); // Word0 - address
    command.Word1 = device.getWord(command.Offset + 2); // Word1 - data
    // movc     B402h, #0123h
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        hex(command.Word0, 4) + ", #" +
        hex(command.Word1, 4);
    command.AsString = leftJustified(command.AsString, CommandWidthToComment, " ") +
        "-- " + hex(command.Word0, 4) + " <= " + hex(command.Word1, 4) + " (" + command.Word1 + ")";
    return "";
}
function command_movc(device, command) {
    device.writeRamWord(command.Word0, command.Word1);
    return "";
}
// Command: movbc
// Code: 7
//
function parse_movbc(device, command) {
    command.Size = 4;
    command.Word0 = device.getWord(command.Offset + 1); // Word0 - data address
    command.Word1 = device.getWord(command.Offset + 2); // Word1 - data
    command.BitNo0 = device.getWord(command.Offset + 3); // BitNo
    check_param_range(command.BitNo0, 0, 15, "BitNo");
    // MOVBC     B402h[0], #0
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        hex(command.Word0, 4) + "[" + command.BitNo0 + "]" + ", #" + command.Word1;
    return "";
}
function command_movbc(device, command) {
    var value = device.getWord(command.Word0);
    device.writeRamBit(command.Word0, command.BitNo0, command.Word1);
    return "";
}
// Command: wrfbc
// Code: 10
//
function parse_wrfbc(device, command) {
    command.Size = 3;
    command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F; // Lowest 6 bit
    command.AfbInstance = device.getWord(command.Offset + 1) >>> 6; // Highest 10 bits
    command.AfbPinOpCode = device.getWord(command.Offset + 1) & 63; // Lowest 6 bit
    command.Word0 = device.getWord(command.Offset + 2); // Word0 - data address
    var afb = device.afbComponent(command.AfbOpCode);
    if (afb == null) {
        return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
    }
    if (command.AfbInstance >= afb.MaxInstCount) {
        return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
    }
    var pinCaption = afb.pinCaption(command.AfbPinOpCode);
    // wrfbc LOGIC.0[0], #0003h
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        afb.Caption + "." + command.AfbInstance + "[" + command.AfbPinOpCode + "], #" +
        hex(command.Word0, 4);
    command.AsString = leftJustified(command.AsString, CommandWidthToComment, " ") +
        "-- " + pinCaption + " <= " + hex(command.Word0, 4) + " (" + command.Word0 + ")";
    return "";
}
function command_wrfbc(device, command) {
    var param = device.createComponentParam();
    param.OpIndex = command.AfbPinOpCode;
    param.AsWord = command.Word0;
    var ok = device.setAfbParam(command.AfbOpCode, command.AfbInstance, param);
    if (ok == false) {
        return "setAfbParam error";
    }
    return "";
}
// Command: wrfbb
// Code: 11
//
function parse_wrfbb(device, command) {
    command.Size = 4;
    command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F; // Lowest 6 bit
    command.AfbInstance = device.getWord(command.Offset + 1) >>> 6; // Highest 10 bits
    command.AfbPinOpCode = device.getWord(command.Offset + 1) & 63; // Lowest 6 bit
    command.Word0 = device.getWord(command.Offset + 2); // Word0 - data address
    command.BitNo0 = device.getWord(command.Offset + 3); // BitNo
    // Checks
    //
    var afb = device.afbComponent(command.AfbOpCode);
    if (afb == null) {
        return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
    }
    if (command.AfbInstance >= afb.MaxInstCount) {
        return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
    }
    check_param_range(command.BitNo0, 0, 15, "BitNo");
    var pinCaption = afb.pinCaption(command.AfbPinOpCode);
    // wrfbb LOGIC.0[20], 46083[0]
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        afb.Caption + "." + command.AfbInstance + "[" + command.AfbPinOpCode + "], " +
        hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
    command.AsString = leftJustified(command.AsString, CommandWidthToComment, " ") +
        "-- " +
        afb.Caption + "." + command.AfbInstance + "[" + pinCaption + "] <=" +
        hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
    return "";
}
function command_wrfbb(device, command) {
    var param = device.createComponentParam();
    param.OpIndex = command.AfbPinOpCode;
    param.AsWord = device.readRamBit(command.Word0, command.BitNo0);
    var ok = device.setAfbParam(command.AfbOpCode, command.AfbInstance, param);
    if (ok == false) {
        return "setAfbParam error";
    }
    return "";
}
// Command: rdfbb
// Code: 12
//
function parse_rdfbb(device, command) {
    command.Size = 4;
    command.AfbOpCode = device.getWord(command.Offset + 0) & 0x003F; // Lowest 6 bit
    command.AfbInstance = device.getWord(command.Offset + 1) >>> 6; // Highest 10 bits
    command.AfbPinOpCode = device.getWord(command.Offset + 1) & 63; // Lowest 6 bit
    command.Word0 = device.getWord(command.Offset + 2); // Word0 - data address
    command.BitNo0 = device.getWord(command.Offset + 3); // BitNo
    // Checks
    //
    var afb = device.afbComponent(command.AfbOpCode);
    if (afb == null) {
        return "Cannot find AfbComponent with OpCode " + command.AfbOpCode;
    }
    if (command.AfbInstance >= afb.MaxInstCount) {
        return "AfbComponent.Instance (" + command.AfbInstance + ") is out of limits " + afb.MaxInstCount;
    }
    check_param_range(command.BitNo0, 0, 15, "BitNo");
    var pinCaption = afb.pinCaption(command.AfbPinOpCode);
    // rdfbb 46083[0], LOGIC.0[20]
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        hex(command.Word0, 4) + "[" + command.BitNo0 + "], " +
        afb.Caption + "." + command.AfbInstance + "[" + command.AfbPinOpCode + "]";
    command.AsString = leftJustified(command.AsString, CommandWidthToComment, " ") +
        "-- " +
        hex(command.Word0, 4) + "[" + command.BitNo0 + "] <= " +
        afb.Caption + "." + command.AfbInstance + "[" + pinCaption + "]";
    return "";
}
function command_rdfbb(device, command) {
    var afbInstance = device.afbComponentInstance(command.AfbOpCode, command.AfbInstance);
    if (afbInstance == null) {
        return "Cannot find afbInstance with OpCode " + command.AfbOpCode + ", InstanceNo " + command.AfbInstance;
    }
    if (afbInstance.paramExists(command.AfbPinOpCode) == false) {
        return "Param is not exist, AfbPinOpIndex " + command.AfbPinOpCode;
    }
    var param = afbInstance.param(command.AfbPinOpCode);
    device.writeRamBit(command.Word0, command.BitNo0, param.AsWord & 0x01);
    return "";
}
// Command: appstart
// Code: 17
//
function parse_appstart(device, command) {
    command.Size = 2; // 2 words
    command.Word0 = device.getWord(command.Offset + 1); // Word0 keeps ALP phase start address
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") + hex(command.Word0, 4);
    return "";
}
function command_appstart(device, command) {
    device.AppStartAddress = command.Word0;
    return "";
}
//
// AFB's simultaion code
//
//
//	LOGIC, OpCode 1
//
function afb_logic(instance) {
    // Define input opIndexes
    //
    var i_oprd_quant = 0;
    var i_bus_width = 1;
    var i_conf = 2;
    var i_input_0 = 3;
    var o_result = 20;
    // Get params,  check_param throws exception in case of error
    //
    check_param_exist(instance, i_oprd_quant, "i_oprd_quant");
    check_param_exist(instance, i_bus_width, "i_bus_width");
    check_param_exist(instance, i_conf, "i_conf");
    var oprdQuant = instance.param(i_oprd_quant);
    var busWidth = instance.param(i_bus_width);
    var conf = instance.param(i_conf);
    check_param_range(oprdQuant.AsWord, 1, 16, "i_oprd_quant");
    check_param_range(busWidth.AsWord, 1, 16, "i_bus_width");
    // Logic
    //
    var inputs = new Array(oprdQuant.AsWord);
    for (var i = 0; i < inputs.length; i++) {
        check_param_exist(instance, i_input_0 + i, "i_oprd_" + i);
        inputs[i] = instance.param(i_input_0 + i);
    }
    var result = inputs[0].AsWord;
    switch (conf.AsWord) {
        case 1:
            for (var i = 1; i < inputs.length; i++) {
                result &= inputs[i].AsWord;
            }
            break;
        case 2:
            for (var i = 1; i < inputs.length; i++) {
                result |= inputs[i].AsWord;
            }
            break;
        case 2:
            for (var i = 1; i < inputs.length; i++) {
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
function afb_not(instance) {
    // Define input opIndexes
    //
    var i_oprd = 0;
    var o_result = 2;
    // Get params,  check_param throws exception in case of error
    //
    check_param_exist(instance, i_oprd, "i_oprd");
    // Logic
    //
    var input = instance.param(i_oprd);
    var result = ~input.AsWord;
    // Save result
    //	
    instance.addParamWord(o_result, result);
    return "";
}
//
//	MATH, OpCode 13
//
function afb_math(instance) {
    // Define input opIndexes
    //
    var i_conf = 0;
    var i_1_oprd = 1;
    var i_2_oprd = 3;
    var o_result = 6;
    var o_mat_edi = 8;
    var o_overflow = 9;
    var o_underflow = 10;
    var o_zero = 11;
    var o_nan = 12;
    var o_div_by_zero = 13;
    // Get params,  check_param throws exception in case of error
    //
    check_param_exist(instance, i_conf, "i_conf");
    check_param_exist(instance, i_1_oprd, "i_1_oprd");
    check_param_exist(instance, i_2_oprd, "i_2_oprd");
    var conf = instance.param(i_conf);
    var operand1 = instance.param(i_1_oprd);
    var operand2 = instance.param(i_2_oprd);
    // Logic	conf: 1'-'+' (SI),  '2'-'-' (SI),  '3'-'*' (SI),  '4'-'/' (SI), '5'-'+' (FP),  '6'-'-' (FP),  '7'-'*' (FP),  '8'-'/' (FP)   
    //
    switch (conf.AsWord) {
        case 1:
            operand1.addSignedInteger(operand2);
            break;
        case 2:
            operand1.subSignedInteger(operand2);
            break;
        case 3:
            operand1.mulSignedInteger(operand2);
            break;
        case 4:
            operand1.divSignedInteger(operand2);
            break;
        case 5:
            operand1.addFloatingPoint(operand2);
            break;
        case 6:
            operand1.subFloatingPoint(operand2);
            break;
        case 7:
            operand1.mulFloatingPoint(operand2);
            break;
        case 8:
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
