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

    const noise = false;		// Allow adding noise to result

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

    // Add noise if noise is allowed
    //
    if (noise === true)
    {
        const prob = 0.03;				// High peaks probability
        const k = amplitude / 50.0;

        let r = (Math.random() * 2 - 1) * k

        result = Math.random() < prob ?
                    result += r * 7 :
                    result += r;
    }

    return result;	// Return value for signal overriding
})
