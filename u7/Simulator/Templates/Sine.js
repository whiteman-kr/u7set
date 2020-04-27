// Sine
//
(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    var period = 5.0;		// Period in seconds
    var amplitude = 100.0;	// Amplitude
    var base = 0.0;			// Shift base

    // Calc result
    //
    var c = (workcycle * Math.PI * 0.01) / period;
    var result = base + Math.sin(c) * amplitude;

    return result;	// Return value for signal overriding
})
