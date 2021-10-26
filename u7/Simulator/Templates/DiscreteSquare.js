// Square for Discrete
//
let counter = 0;

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Feel free to change these params
    //
    const lowTime = 300;		// ms
    const highTime = 200;		// ms

    // Calc result
    //
    counter --;

    let result = lastValue;

    if (counter <= 0)
    {
        if (lastValue === 0)
        {
            counter = highTime / 5; 	// workcycle is 5ms
            result = 1;
        }
        else
        {
            counter = lowTime / 5;      // workcycle is 5ms
            result = 0;
        }
    }

    return result;	// Return value for signal overriding
})
