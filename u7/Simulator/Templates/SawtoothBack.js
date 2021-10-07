// Sawtooth Back
//
let delta = 0;
let counter = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    const time = 500;           // ms, going up
    const amplitude = 100.0;	// Amplitude
    const base = 0.0;			// Shift base

    // Calc result
    //
    if (delta === 0)
    {
        delta = amplitude / ((time - 5) / 5);
    }

    counter --;
    if (counter < 0)
    {
        counter = (time - 5) / 5;
    }

    return base + delta * counter;	// Return value for signal overriding
})
