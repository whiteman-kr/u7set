var CommandWidth = 10; // leftJustified("appstart", CommandWidth, " ") + rightJustified(command.Word0.toString(16), 4, "0") + "h";
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
// Command: nop
// Code: 1
//
function parse_nop(device, command) {
    command.Size = 1; // 1 word
    command.AsString = command.Caption;
    return true;
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
    // WRFBC OR.0[0], #3
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") +
        command.AfbOpCode + "." + command.AfbInstance + "[" + command.AfbPinOpCode + "], #" +
        rightJustified(command.Word0.toString(16), 4, "0") + "h";
    return true;
}
// Command: appstart
// Code: 17
//
function parse_appstart(device, command) {
    command.Size = 2; // 2 words
    command.Word0 = device.getWord(command.Offset + 1); // Word0 keeps ALP phase start address
    command.AsString = leftJustified(command.Caption, CommandWidth, " ") + rightJustified(command.Word0.toString(16), 4, "0") + "h";
    return true;
}
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
    instance.addOutputParamWord(o_result, result);
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
    instance.addOutputParamWord(o_result, result);
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
    instance.addOutputParam(o_result, operand1);
    instance.addOutputParamWord(o_overflow, operand1.MathOverflow ? 0x0001 : 0x0000);
    instance.addOutputParamWord(o_underflow, operand1.MathUnderflow ? 0x0001 : 0x0000);
    instance.addOutputParamWord(o_zero, operand1.MathZero ? 0x0001 : 0x0000);
    instance.addOutputParamWord(o_nan, operand1.MathNan ? 0x0001 : 0x0000);
    instance.addOutputParamWord(o_div_by_zero, operand1.MathDivByZero ? 0x0001 : 0x0000);
    return "";
}
