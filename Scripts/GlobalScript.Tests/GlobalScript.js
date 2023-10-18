'use strict'

var RamReadAccess = 1;        // Use these constants as param for ReadRam*/WriteRam* functions
var RamWriteAccess = 2;
var RamReadWriteAccess = 3;

function assert(condition, message)
{
    if (!condition)
    {
        message = message || "Assertion failed";
        throw new Error(message);
    }
}

// Optional allowGlobal function is used to give permission for running all tests.
// To use global running permission, uncomment this function and write code that
// returns true if running is allowed, otherwise returns false
/*
function allowGlobal(ctrl) 
{
	// Test scripts execution is allowed only when #GLOBAL_PERMISSION_CH1, #GLOBAL_PERMISSION_CH2 and #GLOBAL_PERMISSION_CH3 values are valid and set to 1.
	//
    const enabledSignals = ["#GLOBAL_PERMISSION_CH1", "#GLOBAL_PERMISSION_CH2", "#GLOBAL_PERMISSION_CH3"];

    for (let i = 0; i < enabledSignals.length; i++) 
    {
        let state = ctrl.signalState(enabledSignals[i]);
        if (state.stateAvailable === false || state.valid === false || state.value === 0) 
        {
            return false;
        }
    }
    return true;
}
*/