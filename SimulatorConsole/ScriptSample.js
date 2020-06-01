(function(simulator)
{
    // Example script
    //

    // Start simultion for N milliseconds
    //
    var ok = simulator.startForMs(100);

    // Fatal assertion
    //

    // Boolean condiontion assertion
    //
    test.assert(simulator.signalValue("#APPSID1") === 1);
    test.assert(simulator.signalValue("#APPSID1") === 1, "#APPSID1 == 1 expected after xxxxxx");

    // val1 == val2 assertion
    //
    test.assertEq("#APPSID1", 1);
    test.assertEq("#APPSID1", 1, "Message");
    test.assertEq("#APPSID1", "#APPSID2");
    test.assertEq("#APPSID1", "#APPSID2", "Message");

    // val1 != val2 assertion
    //
    test.assertNeq("#APPSID1", 1);
    test.assertNeq("#APPSID1", 1, "Message");
    test.assertNeq("#APPSID1", "#APPSID2");
    test.assertNeq("#APPSID1", "#APPSID2", "Message");

    // val1 < val2 assertion
    //
    test.assert_lq("#APPSID1", 1);
    test.assert_lt("#APPSID1", 1, "Message");
    test.assert_lt("#APPSID1", "#APPSID2");
    test.assert_lt("#APPSID1", "#APPSID2", "Message");

    // val1 <= val2 assertion
    //
    test.assert_le("#APPSID1", 1);
    test.assert_le("#APPSID1", 1, "Message");
    test.assert_le("#APPSID1", "#APPSID2");
    test.assert_le("#APPSID1", "#APPSID2", "Message");

    // val1 > val2 assertion
    //
    test.assert_gt("#APPSID1", 1);      // Signal   #APPSID1 >  1
    test.assert_gt("#APPSID1", 1);

    // val1 >= val2 assertion
    //
    test.assert_ge("#APPSID1", 1);      // Signal   #APPSID1 >= 1
    test.assert_ge("#APPSID1", 1);

    // value > minLimid && value < minLimid
    //
    test.assert_rex("#APPSID1", 1, 2);
    test.assert_rex("#APPSID1", 1, 2);

    // value >= minLimid && value =< minLimid
    //
    test.assert_rin("#APPSID1", 1, 2);
    test.assert_rin("#APPSID1", 1, 2);

//    // Nonfatal assertion
//    //
//    test.expect_eq("#APPSID1", 1);      // Signal   #APPSID1 == 1
//    test.expect_ne("#APPSID1", 1);      // Signal   #APPSID1 != 1
//    test.expect_lt("#APPSID1", 1);      // Signal   #APPSID1 <  1
//    test.expect_le("#APPSID1", 1);      // Signal   #APPSID1 <= 1
//    test.expect_gt("#APPSID1", 1);      // Signal   #APPSID1 >  1
//    test.expect_ge("#APPSID1", 1);      // Signal   #APPSID1 >= 1

/*
    simulator.checkSignalEq("#APPSIGNALID", 1);		// Check signal value, exception occurs if it is not equal to specified value
    simulator.checkSignalNotEq("#APPSIGNALID", 1);	// Check signal value, !=

    simulator.checkSignalLs("#APPSIGNALID", 100);	// Check signal value, <
    simulator.checkSignalLsEq("#APPSIGNALID", 100);	// Check signal value, <=

    simulator.checkSignalGt("#APPSIGNALID", 100);	// Check signal value, >
    simulator.checkSignalGtEq("#APPSIGNALID", 100);	// Check signal value, >=

    let value = simulator.signalValue("#APPSIGNALID");	// Get signal value
    if (value !== 1)
    {
        return false;	// Report error
    }

    console.log("Value of signal #APPSIGNALID is " + value);	// Console output, for more info on console API visit https://developer.mozilla.org/en-US/docs/Web/API/Console

    console.assert(simulator.signalValue("#APPSIGNALID") === 10, "Signal #APPSIGNALID expected value 10");
*/

    // return Boolean type
    //
    return ok;
})
