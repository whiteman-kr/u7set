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

    const noise = false;		// Allow adding noise to result

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
