function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
       component.addOperation("CreateShortcut", "@TargetDir@/Simulator.exe", "@StartMenuDir@/Simulator.lnk",
            "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut", "@TargetDir@/Simulator.exe", "@DesktopDir@/RPCT (@Version@)/Simulator.lnk",
            "workingDirectory=@TargetDir@");
    }
}