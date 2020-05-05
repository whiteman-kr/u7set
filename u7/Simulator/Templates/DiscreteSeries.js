// Square for Discrete
//
var counter = 0;
var index = -1;

// Feel free to change these params
// Intervals in ms
//
var lowHighIntervals = [1000, 1000, 845, 845, 680, 680, 540, 540, 435, 435, 345, 345,
                        280, 280, 220, 220, 180, 180, 140, 140, 115, 115, 90, 90,
                        75, 75, 60, 60, 45, 45, 35, 35, 30, 30, 25, 25, 20, 20,
                        15, 15, 10, 10, 10, 10, 5, 5, 5, 5];

// Example: 0 - 200ms, 1 - 100ms, 0 - 50ms, 1 - 10ms
// var lowHighIntervals = [200, 100, 50, 10]

(function(lastValue, workcycle)
{
    // lastValue - The last value returned from this function
    // workcycle - Workcycle counter

    // Calc result
    //
    if (lowHighIntervals.length == 0)
    {
        return 0;
    }

    if (index == -1)
    {
        index = 0;
        counter = lowHighIntervals[index] / 5; 	// workcycle is 5ms
        return 0;	// Start from 0
    }

    counter --;

    var result = lastValue;
    if (counter <= 0)
    {
        if (lastValue == 0)
        {
            result = 1;
        }
        else
        {
            result = 0;
        }

        index ++;
        if (index >= lowHighIntervals.length)
        {
            index = 0;
        }

        counter = lowHighIntervals[index] / 5; 	// workcycle is 5ms
    }

    return result;	// Return value for signal overriding
})
