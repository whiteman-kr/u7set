// Triangle
//
var delta = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    var time1 = 500;		// ms, going up
    var time2 = 500;		// ms, going down

    var amplitude = 100.0;	// Amplitude
    var base = 0.0;			// Shift base

    // Calc result
    //
    if (delta == 0)
    {
        delta = amplitude / (time1 / 5);
    }

    var result = lastValue + delta;

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
