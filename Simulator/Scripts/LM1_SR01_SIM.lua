--
-- Library interfaces, constatns, functions
--
CyclePhase = 
{
	IdrPhase = 0,
	AlpPhase = 1,
	ODT = 2,
	ST = 3
}

simFuncTable = 
{
	["afb_logic"]	= function(x)		afb_logic(x)		end,
	["afb_not"]		= function(x)		afb_not(x)			end,
	["afb_math"]	= function(x)		afb_math(x)			end,
}

-- Service functions
--

-- Convert word value to string in hex format
--
function word2hex(value)
    local str = string.format("%04xh", value);
	if (str:sub(1, 1) ~= "0")
	then
	    str = "0" .. str;
	end
	return str;
end

-- Convert dword value to string in hex format
--
function dword2hex(value)
    local str = string.format("%08xh", value);
	if (str:sub(1, 1) ~= "0")
	then
	    str = "0" .. str;
	end
	return str;
end

-- Get AFB, if such item is not exist then terminate
--
function check_afb(device, afbOpCode, afbInstance)
	local afb = device:afbComponent(afbOpCode);
	if (afb:isNull() == true)
	then
		error("Cannot find AfbComponent with OpCode " .. afbOpCode);
	end

	if (afbInstance < 0 or 
		afbInstance >= afb.maxInstCount)
	then
		error("AfbComponent.Instance (" .. afbInstance .. ") is out of limits " .. afb.maxInstCount);
	end
	
	return afb;
end

-- Check value range, if value out of range terminate script
--
function check_param_range(paramValue, minValue, maxValue, paramName)
	if (paramValue < minValue or 
		paramValue > maxValue)
	then
		error("Param " .. paramName .. " is out of range, value = " .. paramValue .. ", range = [" .. minValue .. ", " .. maxValue .. "].");
	end 
end 

-- Check if param exists, if param does not exist the exception is thrown
--
function check_param_exist(afbInstance, opIndex, paramName)
	if (afbInstance:paramExists(opIndex) == false)
	then
		error("Param " .. paramName .. " is not found.");
	end
end

function check_param_exist(afbInstance, opIndex)
	if (afbInstance:paramExists(opIndex) == false)
	then
		error("Param " .. opIndex .. " is not found.");
	end
end


--
-- Logic Unit command pasring and simylation functions
--

-- Command: startafb
-- Code: 2
--
function parse_startafb(device, command)
	command.size = 2;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits

	local afb = check_afb(device, command.afbOpCode, command.afbInstance);
	
	if (afb.simulationFunc:len() == 0)
	then
		error("Simultaion function is not found");
	end

	-- startafb   LOGIC.0
	--
	command.asString = string.format("%-010s %s.%d", command.caption, afb.caption, command.afbInstance);
end

function command_startafb(device, command)

	local afb = device:afbComponent(command.afbOpCode);
	if (afb:isNull() ==  true)
	then
		error("Cannot find AfbComponent with OpCode " .. command.afbOpCode);
	end
	
	local afbInstance = device:afbComponentInstance(command.afbOpCode, command.afbInstance);
	if (afbInstance == null)
	then
		error("Cannot find afbInstance with OpCode " .. command.afbOpCode .. ", InstanceNo " .. command.afbInstance);
	end

	simFunc = simFuncTable[afb.simulationFunc];
	if (simFunc == nil)
	then
		error("Caonnot find simualtion func " .. afb.simulationFunc);
	end

	simFunc(afbInstance);
end

-- Command: stop
-- Code: 3
--
function parse_stop(device, command)
    command.size = 1;
	command.asString = command.caption;
end

function command_stop(device, command)
	if (device.phase == CyclePhase.IdrPhase)
	then
		device.phase = CyclePhase.AlpPhase;
		device.programCounter = device.appStartAddress;
		return;
	end
	
	if (device.phase == CyclePhase.AlpPhase)
	then
		device.phase = CyclePhase.ODT;
		return;
	end
	
	error("Command stop is cannot be run in current phase: " .. device.phase);
end

-- Command: movmem
-- Code: 5
--
function parse_movmem(device, command)
	command.size = 4;
	
	command.word0 = device:getWord(command.offset + 1);		-- word0 - adderess2
	command.word1 = device:getWord(command.offset + 2);		-- word1 - adderess1
	command.word2 = device:getWord(command.offset + 3);		-- word2 - words to move

	-- movmem     0b402h, 0dd02h, #0002h
	--
	command.asString = string.format("%-010s%s, %s, %s", command.caption, word2hex(command.word0), word2hex(command.word1), word2hex(command.word2));
end

function command_movmem(device, command)
	local size = command.word2;
	local src = command.word1;
	local dst = command.word0;

	device:movRamMem(src, dst, size);
end

-- Command: movc
-- Code: 6
--
function parse_movc(device, command)
	command.size = 3;
	
	command.word0 = device:getWord(command.offset + 1);		-- word0 - address
	command.word1 = device:getWord(command.offset + 2);		-- word1 - data

	-- movc     0b402h, #0
	--
	command.asString = string.format("%-010s%s, #%s", command.caption, word2hex(command.word0), word2hex(command.word1));
end

function command_movc(device, command)
	device:writeRamWord(command.word0, command.word1);
end

-- Command: movbc
-- Code: 7
--
function parse_movbc(device, command)
	command.size = 4;
	
	command.word0 = device:getWord(command.offset + 1);		-- word0 - data address
	command.word1 = device:getWord(command.offset + 2);		-- word1 - data
	command.bitNo0 = device:getWord(command.offset + 3);	-- bitNo0 - bitno

	check_param_range(command.bitNo0, 0, 15, "BitNo");

	-- MOVBC     0B402h[0], #0
	--
	command.asString = string.format("%-010s%s[%d], #%d", command.caption, word2hex(command.word0), command.bitNo0, command.word1);
end

function command_movbc(device, command)
	device:writeRamBit(command.word0, command.bitNo0, command.word1);
end

-- Command: wrfbc
-- Code: 10
--
function parse_wrfbc(device, command)
	command.size = 3;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits
	command.afbPinOpCode = device:getWord(command.offset + 1) & 0x003F;		-- Lowest 6 bit

	command.word0 = device:getWord(command.offset + 2);						-- word0 - data address

	local afb = check_afb(device, command.afbOpCode, command.afbInstance);

	-- String representation
	-- wrfbc LOGIC.0[i_oprd_15], #0003h
	--
	local pinCaption = afb:pinCaption(command.afbPinOpCode);
	command.asString = string.format("%-010s%s.%d[%s], #%s", command.caption, afb.caption, command.afbInstance, pinCaption, word2hex(command.word0));
end

function command_wrfbc(device, command)
	local param = AfbComponentParam(command.afbPinOpCode);
	param.asWord = command.word0;

	device:setAfbParam(command.afbOpCode, command.afbInstance, param);
end

-- Command: wrfbb
-- Code: 11
--
function parse_wrfbb(device, command)
	command.size = 4;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits
	command.afbPinOpCode = device:getWord(command.offset + 1) & 0x003F;		-- Lowest 6 bit

	command.word0 = device:getWord(command.offset + 2);						-- Word0 - data address
	command.bitNo0 = device:getWord(command.offset + 3);					-- BitNo

	-- Checks
	--
	local afb = check_afb(device, command.afbOpCode, command.afbInstance);
	check_param_range(command.bitNo0, 0, 15, "BitNo");

	-- String representation
	-- wrfbb LOGIC.0[20], 46083h[0]
	--
	local pinCaption = afb:pinCaption(command.afbPinOpCode);
	command.asString = string.format("%-010s%s.%d[%s], %s[%d]", command.caption, afb.caption, command.afbInstance, pinCaption, word2hex(command.word0), command.bitNo0);
end

function command_wrfbb(device, command)
	local param = AfbComponentParam(command.afbPinOpCode);
	param.asWord = device:readRamBit(command.word0, command.bitNo0);
	
	device:setAfbParam(command.afbOpCode, command.afbInstance, param);
end

-- Command: rdfbb
-- Code: 12
--
function parse_rdfbb(device, command)
	command.size = 4;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits
	command.afbPinOpCode = device:getWord(command.offset + 1) & 0x003F;		-- Lowest 6 bit

	command.word0 = device:getWord(command.offset + 2);						-- Word0 - data address
	command.bitNo0 = device:getWord(command.offset + 3);					-- BitNo

	-- Checks
	--
	local afb = check_afb(device, command.afbOpCode, command.afbInstance);
	check_param_range(command.bitNo0, 0, 15, "BitNo");

	-- String representation
	-- rdfbb 46083h[0], LOGIC.0[20]
	--
	local pinCaption = afb:pinCaption(command.afbPinOpCode);
	command.asString = string.format("%-010s%s[%d], %s.%d[%s]", command.caption, word2hex(command.word0), command.bitNo0, afb.caption, command.afbInstance, pinCaption);
end

function command_rdfbb(device, command)
	local afbInstance = device:afbComponentInstance(command.afbOpCode, command.afbInstance);

	check_param_exist(afbInstance, command.afbPinOpCode);
	
	local param = afbInstance:param(command.afbPinOpCode);
	device:writeRamBit(command.word0, command.bitNo0, param.asWord & 1);
end

-- Command: appstart
-- Code: 17
--
function parse_appstart(device, command)
	command.size = 2;
	command.word0 = device:getWord(command.offset + 1);		-- Word0 keeps ALP phase start address
	command.asString = string.format("%-010s#%s", command.caption, word2hex(command.word0));
end

function command_appstart(device, command)
	device.appStartAddress = command.word0;
end

-- Command: rdfb32
-- Code: 21
-- Description: Read 32bit data from AFB output and write it to RAM
--
function parse_rdfb32(device, command)
	command.size = 3;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits
	command.afbPinOpCode = device:getWord(command.offset + 1) & 0x003F;		-- Lowest 6 bit

	command.word0 = device:getWord(command.offset + 2);						-- Word0 - data address

	-- Checks
	--
	local afb = check_afb(device, command.afbOpCode, command.afbInstance);

	-- String representation
	-- rdfb32 0478h, LOGIC.0[i_2_oprd]
	--
	local pinCaption = afb:pinCaption(command.afbPinOpCode);
	command.asString = string.format("%-010s#%s, %s.%d[%s]", command.caption, word2hex(command.word0), afb.caption, command.afbInstance, pinCaption);	
end

function command_rdfb32(device, command)
	local afbInstance = device:afbComponentInstance(command.afbOpCode, command.afbInstance);

	check_param_exist(afbInstance, command.afbPinOpCode);
	
	local param = afbInstance:param(command.afbPinOpCode);
	device:writeRamDword(command.word0, param.asDword);
end

-- Command: wrfbc32
-- Code: 22
-- Description: Write 32bit constant to FunctionalBlock input
--
function parse_wrfbc32(device, command)
	command.size = 4;
	
	command.afbOpCode = device:getWord(command.offset + 0) & 0x003F;		-- Lowest 6 bit
	command.afbInstance = device:getWord(command.offset + 1) >> 6;			-- Highest 10 bits
	command.afbPinOpCode = device:getWord(command.offset + 1) & 0x003F;		-- Lowest 6 bit

	command.dword0 = device:getDword(command.offset + 2);					-- Dword0 - data

	-- Checks
	--
	local afb = check_afb(device, command.afbOpCode, command.afbInstance);

	-- String representation
	-- wrfbc32 MATH.0[i_oprd_1], #0423445h
	--
	local pinCaption = afb:pinCaption(command.afbPinOpCode);
	command.asString = string.format("%-010s%s.%d[%s], #%s", command.caption, afb.caption, command.afbInstance, pinCaption, dword2hex(command.dword0));	
end

function command_wrfbc32(device, command)
	local param = AfbComponentParam(command.afbPinOpCode);
	param.asDword = command.dword0;

	device:setAfbParam(command.afbOpCode, command.afbInstance, param);
end


--
-- AFB's simultaion code
--

--
--	LOGIC, OpCode 1
--
function afb_logic(instance)
	-- Define input opIndexes
	--
	local i_oprd_quant = 0;
	local i_bus_width = 1;
	local i_conf = 2;
	local i_input_0 = 3;
	local o_result = 20;
	
	-- Get params,  check_param throws exception in case of error
	--
	check_param_exist(instance, i_oprd_quant, "i_oprd_quant");
	check_param_exist(instance, i_bus_width, "i_bus_width");
	check_param_exist(instance, i_conf, "i_conf");
	
	local oprdQuant = instance:param(i_oprd_quant);				-- AfbComponentParam
	local busWidth = instance:param(i_bus_width);				-- AfbComponentParam
	local conf = instance:param(i_conf);						-- AfbComponentParam

	check_param_range(oprdQuant.asWord, 1, 16, "i_oprd_quant");
	check_param_range(busWidth.asWord, 1, 16, "i_bus_width");	
	
	-- Logic
	--
	local inputs = {};		-- array of params
	local inputCount = oprdQuant.asWord;

	for i = 1, inputCount do 				-- in Lua arrays are 1-based, yeah, crazy
		check_param_exist(instance, i_input_0 + i - 1, "i_oprd_" .. (i - 1));
		inputs[i] = instance:param(i_input_0 + i - 1);		
	end

	local result = inputs[1].asWord;		-- in Lua arrays are 1-based, yeah, crazy
	
	-- AND
	--
	if (conf.asWord == 1)
	then
		for i = 2, inputCount do 
			result = result & inputs[i].asWord;
		end

		instance:addParamWord(o_result, result);
		return;		
	end

	-- OR
	--
	if (conf.asWord == 2)
	then
		for i = 2, inputCount do 
			result = result | inputs[i].asWord;
		end

		instance:addParamWord(o_result, result);
		return;		
	end	

	-- XOR
	--
	if (conf.asWord == 3)
	then
		for i = 2, inputCount do 
			result = result ~ inputs[i].asWord;
		end
		
		instance:addParamWord(o_result, result);
		return;
	end		

	error("Unknown AFB LOGIC configuration: " .. conf.asWord);
end

--
--	NOT, OpCode 2
--
function afb_not(afbInstance)
	-- Define input opIndexes
	--
	local i_oprd = 0;
	local o_result = 2;
	
	-- Get params,  check_param throws exception in case of error
	--
	check_param_exist(afbInstance, i_oprd, "i_oprd");
	
	-- Logic
	--
	local input = afbInstance:param(i_oprd);
	local result = ~input.asWord;

	-- Save result
	--	
	afbInstance:addParamWord(o_result, result);
end

--
--	MATH, OpCode 13
--
function afb_math(afbInstance)
	-- Define input opIndexes
	--
	local i_conf = 0;
	local i_1_oprd = 1;
	local i_2_oprd = 3;
	local o_result = 6;
	local o_mat_edi = 8;
	local o_overflow = 9;
	local o_underflow = 10;
	local o_zero = 11;
	local o_nan = 12;
	local o_div_by_zero = 13;
	
	-- Get params,  check_param throws exception in case of error
	--
	check_param_exist(afbInstance, i_conf, "i_conf");
	check_param_exist(afbInstance, i_1_oprd, "i_1_oprd");
	check_param_exist(afbInstance, i_2_oprd, "i_2_oprd");
	
	local conf = afbInstance:param(i_conf);
	local operand1 = afbInstance:param(i_1_oprd);
	local operand2 = afbInstance:param(i_2_oprd);

	-- Logic	conf: 1'-'+' (SI),  '2'-'-' (SI),  '3'-'*' (SI),  '4'-'/' (SI), '5'-'+' (FP),  '6'-'-' (FP),  '7'-'*' (FP),  '8'-'/' (FP)   
	--

	mathFuncTable = 
	{
		[1] = -- SI +
			function(x)
				operand1:addSignedInteger(x);
			end,
	};

	mathFuncTable[conf.asWord](operand2);


--[[	
	
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
	]]	

	-- Save result
	--	
	operand1.opIndex = o_result;
	afbInstance:addParam(operand1);

	afbInstance:addParamWord(o_overflow, operand1.mathOverflow);
	afbInstance:addParamWord(o_underflow, operand1.mathUnderflow);
	afbInstance:addParamWord(o_zero, operand1.mathZero);
	afbInstance:addParamWord(o_nan, operand1.mathNan);
	afbInstance:addParamWord(o_div_by_zero, operand1.mathDivByZero);
end
