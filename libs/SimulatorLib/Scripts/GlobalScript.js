'use strict'

var RamReadAccess = 1;        // Use these constants as param for ReadRam*/WriteRam* functions
var RamWriteAccess = 2;
var RamReadWriteAccess = 3;

function assert(condition, message)
{
	// rgergerg,
	
    if (!condition)
    {
        message = message || "Assertion failed";
        throw new Error(message);
    }
}


