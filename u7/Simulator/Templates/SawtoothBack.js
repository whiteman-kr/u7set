// Sawtooth Back
//
var delta = 0;
var counter = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    var time = 500;         // ms, going up

    var amplitude = 100.0;	// Amplitude
    var base = 0.0;			// Shift base

    // Calc result
    //
    if (delta == 0)
    {
        delta = amplitude / ((time - 5) / 5);
    }
	
	counter --;
	if (counter < 0)
	{
		counter = (time - 5) / 5;
	}

    var result = base + delta * counter;
    return result;	// Return value for signal overriding
})
