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
	
	--if (afb == null)
	--{
	--	throw new Error("Cannot find AfbComponent with OpCode " + afbOpCode);
	--}
	--	
	--if (afbInstance >= afb.MaxInstCount)
	--{
	--	throw new Error("AfbComponent.Instance (" + afbInstance + ") is out of limits " + afb.MaxInstCount);
	--}
	
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

-- Command: stop
-- Code: 2
--
function parse_stop(device, command)
    command.size = 1;
	command.asString = command.caption;
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
	--check_param_range(command.bitNo0, 0, 15, "BitNo");

	-- String representation
	-- wrfbb LOGIC.0[20], 46083h[0]
	--
	--local pinCaption = afb:pinCaption(command.afbPinOpCode);

	--command.AsString =	leftJustified(command.Caption, CommandWidth, " ") +  
	--					afb.Caption + "."  + command.AfbInstance + "[" + command.AfbPinOpCode + "], " +
	--					hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
							

	--command.AsString =  leftJustified(command.AsString, CommandWidthToComment, " ") +  
	--					"-- " + 
	--					afb.Caption + "."  + command.AfbInstance + "[" + pinCaption + "] <=" +
	--					hex(command.Word0, 4) + "[" + command.BitNo0 + "]";
end

-- Command: appstart
-- Code: 17
--
function parse_appstart(device, command)
	command.size = 2;
	command.word0 = device:getWord(command.offset + 1);		-- Word0 keeps ALP phase start address
	command.asString = string.format("%-010s#%s", command.caption, word2hex(command.word0));
end

