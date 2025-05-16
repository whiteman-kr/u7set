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
    const noise = false;		// Allow adding noise to result

    // Calc result
    //
    let c = (workcycle * Math.PI * 0.01) / period;
    let result = base + Math.sin(c) * amplitude;

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
