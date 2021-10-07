// Sine
//
(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    const period = 5.0;         // Period in seconds
    const amplitude = 100.0;	// Amplitude
    const base = 0.0;			// Shift base

    // Calc result
    //
    let c = (workcycle * Math.PI * 0.01) / period;
    let result = base + Math.sin(c) * amplitude;

    return result;	// Return value for signal overriding
})
