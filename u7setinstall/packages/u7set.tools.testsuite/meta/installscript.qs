function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
       component.addOperation("CreateShortcut", "@TargetDir@/TestSuite.exe", "@StartMenuDir@/TestSuite.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/TestSuite.exe", "@DesktopDir@/RPCT (@Version@)/TestSuite.lnk",
            "workingDirectory=@TargetDir@");
    }
}