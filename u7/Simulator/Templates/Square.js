// Square
//
let counter = 0;
let level = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    const lowTime = 250;		// ms
    const highTime = 250;		// ms

    const amplitude = 100.0;	// Amplitude
    const base = 0.0;           // Shift base

    // Calc result
    //
    counter --;

    let result = lastValue;
    if (counter <= 0)
    {
        if (level <= 0)
        {
            counter = highTime / 5; 	// workcycle is 5ms
            result = base + amplitude;
            level = 1;
        }
        else
        {
            counter = lowTime / 5; 	// workcycle is 5ms
            result = base;
            level = 0;
        }
    }

    return result;	// Return value for signal overriding
})
