(function(simulator)
{
    // Example script
    //
    var ok = simulator.startForMs(100);						// Start simultion for N milliseconds, -1 means infinite loop
/*
    simulator.checkSignalEq("#APPSIGNALID", 1);		// Check signal value, exception occurs if it is not equal to specified value
    simulator.checkSignalNotEq("#APPSIGNALID", 1);	// Check signal value, !=

    simulator.checkSignalLs("#APPSIGNALID", 100);	// Check signal value, <
    simulator.checkSignalLsEq("#APPSIGNALID", 100);	// Check signal value, <=

    simulator.checkSignalGt("#APPSIGNALID", 100);	// Check signal value, >
    simulator.checkSignalGtEq("#APPSIGNALID", 100);	// Check signal value, >=

    let value = simulator.signalValue("#APPSIGNALID");	// Get signal value
    if (value !== 1)
    {
        return false;	// Report error
    }

    console.log("Value of signal #APPSIGNALID is " + value);	// Console output, for more info on console API visit https://developer.mozilla.org/en-US/docs/Web/API/Console

    console.assert(simulator.signalValue("#APPSIGNALID") === 10, "Signal #APPSIGNALID expected value 10");
*/

    // return Boolean type
    //
    return ok;
})
