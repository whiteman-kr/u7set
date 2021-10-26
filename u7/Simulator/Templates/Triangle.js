// Triangle
//
let delta = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    const time1 = 500;          // ms, going up
    const time2 = 500;          // ms, going down

    const amplitude = 100.0;	// Amplitude
    const base = 0.0;			// Shift base

    // Calc result
    //
    if (delta === 0)
    {
        delta = amplitude / (time1 / 5);
    }

    let result = lastValue + delta;

    if (result >= base + amplitude)
    {
        delta = amplitude / -(time2 / 5);
    }

    if (result <= base)
    {
        delta = amplitude / (time1 / 5);
    }

    return result;	// Return value for signal overriding
})
