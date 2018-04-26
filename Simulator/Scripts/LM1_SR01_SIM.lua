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

--
-- Logic Unit command pasring and simylation functions
--

-- Command: appstart
-- Code: 17
--
function parse_appstart(device, command)
    command.size = 2;
	command.word0 = device:getWord(command.offset + 1);		-- Word0 keeps ALP phase start address
	command.asString = string.format("%-010s#%s", command.caption, word2hex(command.word0));
end

