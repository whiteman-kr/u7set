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

	if (afbInstance >= afb.maxInstCount)
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

-- Command: stop
-- Code: 3
--
function parse_stop(device, command)
    command.size = 1;
	command.asString = command.caption;
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

-- Command: appstart
-- Code: 17
--
function parse_appstart(device, command)
	command.size = 2;
	command.word0 = device:getWord(command.offset + 1);		-- Word0 keeps ALP phase start address
	command.asString = string.format("%-010s#%s", command.caption, word2hex(command.word0));
end

