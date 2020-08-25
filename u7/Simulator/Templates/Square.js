// Square
//
var counter = 0;
var level = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    var lowTime = 250;		// ms
    var highTime = 250;		// ms

    var amplitude = 100.0;	// Amplitude
    var base = 0.0;         // Shift base

    // Calc result
    //
    counter --;

    var result = lastValue;
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
